#include <eosio/crypto.hpp>
#include <eosio/datastream.hpp>
#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/permission.hpp>
#include <eosio/privileged.hpp>
#include <eosio/serialize.hpp>
#include <eosio/singleton.hpp>

#include <libre.system/libre.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <type_traits>
#include <limits>
#include <set>
#include <algorithm>
#include <cmath>

namespace libresystem {

   using eosio::const_mem_fun;
   using eosio::current_time_point;
   using eosio::indexed_by;
   using eosio::microseconds;
   using eosio::singleton;

   void system_contract::register_producer( const name& producer, const eosio::block_signing_authority& producer_authority, const std::string& url, uint16_t location ) {
      auto prod = _producers.find( producer.value );
      const auto ct = current_time_point();

      eosio::public_key producer_key{};

      std::visit( [&](auto&& auth ) {
         if( auth.keys.size() == 1 ) {
            // if the producer_authority consists of a single key, use that key in the legacy producer_key field
            producer_key = auth.keys[0].key;
         }
      }, producer_authority );

      if ( prod != _producers.end() ) {
         _producers.modify( prod, producer, [&]( producer_info& info ){
            info.producer_key       = producer_key;
            info.is_active          = true;
            info.url                = url;
            info.location           = location;
            info.producer_authority.emplace( producer_authority );
         });
      } else {
         _producers.emplace( producer, [&]( producer_info& info ){
            info.owner              = producer;
            info.producer_key       = producer_key;
            info.is_active          = true;
            info.url                = url;
            info.location           = location;
            info.producer_authority.emplace( producer_authority );
         });
      }
   }

   void system_contract::regproducer( const name& producer, const eosio::public_key& producer_key, const std::string& url, uint16_t location ) {
      require_auth( producer );

      check( url.size() < 512, "url too long" );
      check (checkPermission(producer, "regprod") == 1, "You are not authorised to register as producer");  // LIBRE Check Permissions

      register_producer( producer, convert_to_block_signing_authority( producer_key ), url, location );
   }

   void system_contract::regproducer2( const name& producer, const eosio::block_signing_authority& producer_authority, const std::string& url, uint16_t location ) {
      require_auth( producer );

      check( url.size() < 512, "url too long" );
      check (checkPermission(producer, "regprod") == 1, "You are not authorised to register as producer");  // LIBRE Check Permissions

      std::visit( [&](auto&& auth ) {
         check( auth.is_valid(), "invalid producer authority" );
      }, producer_authority );

      register_producer( producer, producer_authority, url, location );
   }

   void system_contract::unregprod( const name& producer ) {

      require_auth( producer );

      const auto& prod = _producers.get( producer.value, "producer not found" );
      _producers.modify( prod, same_payer, [&]( producer_info& info ){
         info.deactivate();
      });
   }

   void system_contract::update_elected_producers( const block_timestamp& block_time ) {
      _gstate.last_producer_schedule_update = block_time;

      auto idx = _producers.get_index<"prototalvote"_n>();

      using value_type = std::pair<eosio::producer_authority, uint16_t>;
      std::vector< value_type > top_producers;
      top_producers.reserve(21);

      for( auto it = idx.cbegin(); it != idx.cend() && top_producers.size() < 21 && 0 < it->total_votes && it->active(); ++it ) {
         top_producers.emplace_back(
            eosio::producer_authority{
               .producer_name = it->owner,
               .authority     = it->get_producer_authority()
            },
            it->location
         );
      }

      if( top_producers.size() == 0 || top_producers.size() < _gstate.last_producer_schedule_size ) {
         return;
      }

      std::sort( top_producers.begin(), top_producers.end(), []( const value_type& lhs, const value_type& rhs ) {
         return lhs.first.producer_name < rhs.first.producer_name; // sort by producer name
         // return lhs.second < rhs.second; // sort by location
      } );

      std::vector<eosio::producer_authority> producers;

      producers.reserve(top_producers.size());
      for( auto& item : top_producers )
         producers.push_back( std::move(item.first) );

      if( set_proposed_producers( producers ) >= 0 ) {
         _gstate.last_producer_schedule_size = static_cast<decltype(_gstate.last_producer_schedule_size)>( top_producers.size() );
      }
   }

   double stake2vote( int64_t staked ) {
      /// TODO subtract 2080 brings the large numbers closer to this decade
      double weight = int64_t( (current_time_point().sec_since_epoch() - (block_timestamp::block_timestamp_epoch / 1000)) / (seconds_per_day * 7) )  / double( 52 );
      return double(staked) * std::pow( 2, weight );
   }

   void system_contract::voteproducer( const name& voter_name, const std::vector<name>& producers ) {
      require_auth( voter_name );
      // TODO: VALIDATE THE FOLLOWING LINE
      // vote_stake_updater( voter_name );
      update_votes( voter_name, producers, true );
      // auto rex_itr = _rexbalance.find( voter_name.value );
      // if( rex_itr != _rexbalance.end() && rex_itr->rex_balance.amount > 0 ) {
      //    check_voting_requirement( voter_name, "voter holding REX tokens must vote for at least 21 producers" );
      // }
   }

   /**
    *
    *  Get the total staked for an account
    *
    * @param acc - Account to get the staked tokens
    *
    * @return Total staked tokens, -1 if account does not exist
    */
    int64_t get_staked(name acc){
        stake_table _stake(stake_libre_account, stake_libre_account.value);
        auto it = _stake.find(acc.value);

        if( it == _stake.end() ) { return -1; }

        return it->libre_staked.amount;
    }

   void system_contract::update_votes( const name& voter_name, const std::vector<name>& producers, bool voting ) {
      //validate input
      
      check( producers.size() <= 30, "attempt to vote for too many producers" );
      for( size_t i = 1; i < producers.size(); ++i ) {
         check( producers[i-1] < producers[i], "producer votes must be unique and sorted" );
      }

      auto voter = _voters.find( voter_name.value );
      // TODO: UPDATE FOLLOWING VALIDATION
      int64_t total_voter_staked = get_staked(voter_name);
      check( total_voter_staked > -1, "user must stake before they can vote" );
      // check( voter != _voters.end(), "user must stake before they can vote" ); /// staking creates voter object

      /**
       * The first time someone votes we calculate and set last_vote_weight. Since they cannot unstake until
       * after the chain has been activated, we can use last_vote_weight to determine that this is
       * their first vote and should consider their stake activated.
       */
      if( _gstate.thresh_activated_stake_time == time_point() && voter->last_vote_weight <= 0.0 ) {
         _gstate.total_activated_stake += total_voter_staked;
         if( _gstate.total_activated_stake >= min_activated_stake ) {
            _gstate.thresh_activated_stake_time = current_time_point();
         }
      }

      auto new_vote_weight = stake2vote( total_voter_staked );

      std::map<name, std::pair<double, bool /*new*/> > producer_deltas;
      if ( voter->last_vote_weight > 0 ) {
         for( const auto& p : voter->producers ) {
            auto& d = producer_deltas[p];
            d.first -= voter->last_vote_weight;
            d.second = false;
         }
      }

      if( new_vote_weight >= 0 ) {
         for( const auto& p : producers ) {
            auto& d = producer_deltas[p];
            d.first += new_vote_weight;
            d.second = true;
         }
      }

      const auto ct = current_time_point();
      double delta_change_rate         = 0.0;
      double total_inactive_vpay_share = 0.0;
      for( const auto& pd : producer_deltas ) {
         auto pitr = _producers.find( pd.first.value );
         if( pitr != _producers.end() ) {
            if( voting && !pitr->active() && pd.second.second /* from new set */ ) {
               check( false, ( "producer " + pitr->owner.to_string() + " is not currently registered" ).data() );
            }
            double init_total_votes = pitr->total_votes;
            _producers.modify( pitr, same_payer, [&]( auto& p ) {
               p.total_votes += pd.second.first;
               if ( p.total_votes < 0 ) { // floating point arithmetics can give small negative numbers
                  p.total_votes = 0;
               }
               _gstate.total_producer_vote_weight += pd.second.first;
               //check( p.total_votes >= 0, "something bad happened" );
            });
         } else {
            if( pd.second.second ) {
               check( false, ( "producer " + pd.first.to_string() + " is not registered" ).data() );
            }
         }
      }

      // update_total_votepay_share( ct, -total_inactive_vpay_share, delta_change_rate );

      if(voter == _voters.end()) {
         _voters.emplace( same_payer, [&]( auto& v ) {
            v.owner  = voter_name;
            v.staked = total_voter_staked;
         });
      }
      else {
         _voters.modify( voter, same_payer, [&]( auto& av ) {
            av.last_vote_weight = new_vote_weight;
            av.producers = producers;
         });
      }
   }


   // LIBRE
   void system_contract::kickbp( const name& producer ) {
         require_auth(permission_level("eosio"_n, "active"_n));

         const auto& prod = _producers.get( producer.value, "producer not found" );
         _producers.modify( prod, get_self(), [&]( producer_info& info ){
            info.deactivate();
         });

         auto act = action(
            permission_level{ get_self(), "active"_n },
            "eosio.libre"_n,
            "kickbp"_n,
            producer

         );
         act.send();
   }

} /// namespace libresystem

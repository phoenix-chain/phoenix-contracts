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

   void system_contract::voteproducer( const name& voter_name, const name& producer ) {
      require_auth( voter_name );

      update_votes( voter_name, producer, true );
   }
   
   void system_contract::vonstake( const name& staker ) {
      require_auth( "stake.libre"_n );

      auto voter = _voters.find( staker.value );

      if( voter != _voters.end()) {
         update_votes( staker, voter->producer, false );
      }
   }

   int64_t get_staked(name acc) {
      int64_t totalVotes = 0;
      stake_table _stake(stake_libre_account, stake_libre_account.value);
      auto stake_index =_stake.get_index< name( "account" ) >();
      bool exist = false;

      for ( auto itr = stake_index.lower_bound( acc.value ); itr != stake_index.upper_bound( acc.value ); itr++ ) {
         exist |= itr->account.value == acc.value;

         if ( itr->account.value == acc.value && itr->status == stake_status::STAKE_IN_PROGRESS) {
            totalVotes += itr->libre_staked.amount;
         }
      }

      return exist ? totalVotes : -1;
   }

   void system_contract::update_votes( const name& voter_name, const name& producer, bool voting ) {
      auto voter = _voters.find( voter_name.value );
      
      int64_t total_voter_staked = get_staked(voter_name);

      if ( !voting && total_voter_staked == -1 ) return;

      check( total_voter_staked > -1, "user must stake before they can vote" );
      int64_t old_total_voter_staked = voter != _voters.end() ? voter->staked : 0;

      if( _gstate.thresh_activated_stake_time == time_point() ) {
         _gstate.total_activated_stake += total_voter_staked;
         if( _gstate.total_activated_stake >= min_activated_stake ) {
            _gstate.thresh_activated_stake_time = current_time_point();
         }
      }

      std::map<name, double> producer_delta;

      const name old_producer = voter->producer;
      const name new_producer = producer;

      if ( old_producer.value == new_producer.value ) {
         producer_delta[old_producer] = total_voter_staked - old_total_voter_staked;
      } else {
         producer_delta[old_producer] = 0 - old_total_voter_staked;
         producer_delta[new_producer] = total_voter_staked;
      }

      for( const auto&  pd : producer_delta ) {
         auto p_itr = _producers.find( pd.first.value );
         if( p_itr != _producers.end() ) {
            if( voting && !p_itr->active() ) {
               check( false, ( "producer " + p_itr->owner.to_string() + " is not currently registered" ).data() );
            }
            double init_total_votes = p_itr->total_votes;
            _producers.modify( p_itr, same_payer, [&]( auto& p ) {
               p.total_votes += double(pd.second);
               if ( p.total_votes < 0 ) { // floating point arithmetics can give small negative numbers
                  p.total_votes = 0;
               }
               _gstate.total_producer_vote_weight += double(pd.second);
            });
         } else {
            if( pd.second ) {
               check( false, ( "producer " + pd.first.to_string() + " is not registered" ).data() );
            }
         }
      }

      if(voter == _voters.end()) {
         _voters.emplace( voter_name, [&]( auto& v ) {
            v.owner  = voter_name;
            v.staked = total_voter_staked;
            v.producer = producer;
         });
      }
      else {
         _voters.modify( voter, same_payer, [&]( auto& v ) {
            v.producer = producer;
            v.staked = total_voter_staked;
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

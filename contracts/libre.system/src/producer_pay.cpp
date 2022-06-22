#include <libre.system/libre.system.hpp>
#include <eosio.token/eosio.token.hpp>

namespace libresystem {

   using eosio::current_time_point;
   using eosio::microseconds;
   using eosio::token;

   uint8_t system_contract::calculate_pay_per_block() {
      const int64_t minutes = 6 * 60'000'000;
      const int64_t six_months = microseconds(minutes).count() / 2;

      // const int64_t six_months = microseconds(useconds_per_year).count() / 2;
      const int64_t elapse = (_gstate2.last_block_num.to_time_point() - _gstate.activated_time).count();

      if ( elapse < six_months ) return 4;
      else if ( elapse >= six_months && elapse < six_months * 2 ) return 2;
      else return 1;
   }

   void system_contract::onblock( ignore<block_header> ) {
      using namespace eosio;

      require_auth( get_self() );

      block_timestamp timestamp;
      name producer;
      _ds >> timestamp >> producer;

      _gstate2.last_block_num = timestamp;

      /** until activation, no new rewards are paid */
      // if( _gstate.thresh_activated_stake_time == time_point() )
      //    return;

      if( _gstate.activated_time == time_point() )
         _gstate.activated_time = current_time_point();

      /**
       * At startup the initial producer may not be one that is registered / elected
       * and therefore there may be no producer object for them.
       */
      auto prod = _producers.find( producer.value );
      if ( prod != _producers.end() ) {
         _gstate.total_unpaid_blocks++;
         _producers.modify( prod, same_payer, [&](auto& p ) {
            p.unpaid_blocks++;
         });
      }

      if ( current_time_point() - _gstate.last_updated_reward > microseconds(useconds_per_day) ) {
         updrewards();
      }
      

      /// only update block producers once every minute, block_timestamp is in half seconds
      if( timestamp.slot - _gstate.last_producer_schedule_update.slot > 90 ) {
         update_elected_producers( timestamp );
      }
   }

   void system_contract::updrewards() {
      const auto ct = current_time_point();

      // TODO: add token_supply validation to dont exceed the max_supply
      // const asset token_supply   = token::get_supply(token_account, core_symbol().code() );

      for (auto p_itr = _producers.begin(); p_itr != _producers.end(); p_itr++) {
         if (!p_itr->unpaid_blocks) continue;
         
         int64_t new_tokens = calculate_pay_per_block() * p_itr->unpaid_blocks;

         {
            token::issue_action issue_act{ token_account, { {get_self(), active_permission} } };
            issue_act.send( get_self(), asset(new_tokens, core_symbol()), "issue tokens for producer pay");
         }
         {
            token::transfer_action transfer_act{ token_account, { {get_self(), active_permission} } };
            transfer_act.send( get_self(), bpay_account, asset(new_tokens, core_symbol()), "fund per-block bucket a total of "
               + std::to_string(new_tokens) + " tokens for " + std::to_string(p_itr->unpaid_blocks) + " blocks" );
         }

         _gstate.total_unpaid_blocks -= p_itr->unpaid_blocks;

         auto payment_itr = _payments.find( p_itr->owner.value );

         if (payment_itr != _payments.end() ) {
            _payments.modify( payment_itr, same_payer, [&]( auto &row ) {
               row.amount += new_tokens;
            });
         } else {
            _payments.emplace( get_self(), [&]( auto &row ) {
               row.producer = p_itr->owner;
               row.amount = new_tokens;
            });
         }

         _producers.modify( p_itr, same_payer, [&](auto& row) {
            row.last_claim_time = ct;
            row.unpaid_blocks   = 0;
         });
      }

      _gstate.last_updated_reward      = ct;
   }

   void system_contract::claimrewards( const name& owner ) {
      require_auth( owner );

      auto pay_itr = _payments.find( owner.value );

      check( pay_itr != _payments.end() && pay_itr->amount, "payment is not ready yet" );

      token::transfer_action transfer_act{ token_account, { {bpay_account, active_permission}, {owner, active_permission} } };
      transfer_act.send( bpay_account, owner, asset(pay_itr->amount, core_symbol()), "producer block pay" );

      _payments.modify( pay_itr, same_payer, [&](auto& row) {
         row.amount   = 0;
      });
   }

} //namespace libresystem

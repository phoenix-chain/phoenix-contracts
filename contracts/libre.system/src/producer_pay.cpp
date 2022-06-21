#include <libre.system/libre.system.hpp>
#include <eosio.token/eosio.token.hpp>

namespace libresystem {

   using eosio::current_time_point;
   using eosio::microseconds;
   using eosio::token;

   uint8_t system_contract::calculate_pay_amount() {
      const int64_t six_months = microseconds(useconds_per_year).count() / 2;
      const int64_t elapse = _gstate2.last_block_num.slot / six_months;

      if ( elapse < 6 ) return 4;
      else if ( elapse >= 6 && elapse < 12 ) return 2;
      else return 1;
   }

   void system_contract::onblock( ignore<block_header> ) {
      using namespace eosio;

      require_auth( get_self() );

      block_timestamp timestamp;
      name producer;
      _ds >> timestamp >> producer;

      // _gstate2.last_block_num is not used anywhere in the system contract code anymore.
      // Although this field is deprecated, we will continue updating it for now until the last_block_num field
      // is eventually completely removed, at which point this line can be removed.
      _gstate2.last_block_num = timestamp;

      /** until activation, no new rewards are paid */
      // if( _gstate.thresh_activated_stake_time == time_point() )
      //    return;

      if( _gstate.last_pervote_bucket_fill == time_point() )  // start the presses
         _gstate.last_pervote_bucket_fill = current_time_point();

      if( _gstate.last_updated_reward == time_point() )
         _gstate.last_updated_reward = current_time_point();

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

      if ( current_time_point() - _gstate.last_updated_reward > microseconds(useconds_per_day) && _gstate.last_updated_reward > time_point() ) {
         updatereward();
      }
      

      /// only update block producers once every minute, block_timestamp is in half seconds
      if( timestamp.slot - _gstate.last_producer_schedule_update.slot > 90 ) {
         update_elected_producers( timestamp );
      }
   }

   void system_contract::updatereward() {
      const auto ct = current_time_point();

      // TODO: add token_supply validation to dont exceed the max_supply
      // const asset token_supply   = token::get_supply(token_account, core_symbol().code() );
      const auto usecs_since_last_updated_reward = (ct - _gstate.last_updated_reward).count();

      for (auto p_itr = _producers.begin(); p_itr != _producers.end(); p_itr++) {
         if (!p_itr->unpaid_blocks) continue;
         
         int64_t new_tokens = calculate_pay_amount() * p_itr->unpaid_blocks;

         {
            token::issue_action issue_act{ token_account, { {get_self(), active_permission} } };
            issue_act.send( get_self(), asset(new_tokens, core_symbol()), "issue tokens for producer pay");
         }
         {
            token::transfer_action transfer_act{ token_account, { {get_self(), active_permission} } };
            transfer_act.send( get_self(), bpay_account, asset(new_tokens, core_symbol()), "fund per-block bucket"
               + " " + std::to_string(new_tokens) + " / " + std::to_string(p_itr->unpaid_blocks) );
         }

         _gstate.perblock_bucket         += new_tokens;

         int64_t producer_per_block_pay = 0;
         if ( _gstate.total_unpaid_blocks > 0 ) {
            producer_per_block_pay = (_gstate.perblock_bucket * p_itr->unpaid_blocks) / _gstate.total_unpaid_blocks;
         }

         _gstate.perblock_bucket     -= producer_per_block_pay;
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

      _gstate.last_pervote_bucket_fill = ct;
      _gstate.last_updated_reward      = ct;
   }

   void system_contract::claimrewards( const name& owner ) {
      require_auth( owner );

      auto pay_itr = _payments.find( owner.value );

      check( pay_itr->amount, "no funds to claim" );

      token::transfer_action transfer_act{ token_account, { {bpay_account, active_permission}, {owner, active_permission} } };
      transfer_act.send( bpay_account, owner, asset(pay_itr->amount, core_symbol()), "producer block pay" );

      _payments.modify( pay_itr, same_payer, [&](auto& row) {
         row.amount   = 0;
      });
   }

} //namespace libresystem

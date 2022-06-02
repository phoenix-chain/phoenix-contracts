#include <libre.system/libre.system.hpp>
#include <eosio.token/eosio.token.hpp>

namespace libresystem {

   using eosio::current_time_point;
   using eosio::microseconds;
   using eosio::token;

   void system_contract::onblock( ignore<block_header> ) {
      using namespace eosio;

      require_auth(get_self());

      block_timestamp timestamp;
      name producer;
      _ds >> timestamp >> producer;

      // _gstate2.last_block_num is not used anywhere in the system contract code anymore.
      // Although this field is deprecated, we will continue updating it for now until the last_block_num field
      // is eventually completely removed, at which point this line can be removed.
      _gstate2.last_block_num = timestamp;

      /** until activation, no new rewards are paid */
      if( _gstate.thresh_activated_stake_time == time_point() )
         return;

      if( _gstate.last_pervote_bucket_fill == time_point() )  /// start the presses
         _gstate.last_pervote_bucket_fill = current_time_point();


      /**
       * At startup the initial producer may not be one that is registered / elected
       * and therefore there may be no producer object for them.
       */
      auto prod = _producers.find( producer.value );
      if ( prod != _producers.end() ) {
         _gstate.total_unpaid_blocks++;
         _producers.modify( prod, same_payer, [&](auto& p ) {
            //    p.unpaid_blocks++;
         });
      }

      /// only update block producers once every minute, block_timestamp is in half seconds
      if( timestamp.slot - _gstate.last_producer_schedule_update.slot > 120 ) {
         update_elected_producers( timestamp );

        //  if( (timestamp.slot - _gstate.last_name_close.slot) > blocks_per_day ) {
        //     name_bid_table bids(get_self(), get_self().value);
        //     auto idx = bids.get_index<"highbid"_n>();
        //     auto highest = idx.lower_bound( std::numeric_limits<uint64_t>::max()/2 );
        //     if( highest != idx.end() &&
        //         highest->high_bid > 0 &&
        //         (current_time_point() - highest->last_bid_time) > microseconds(useconds_per_day) &&
        //         _gstate.thresh_activated_stake_time > time_point() &&
        //         (current_time_point() - _gstate.thresh_activated_stake_time) > microseconds(14 * useconds_per_day)
        //     ) {
        //        _gstate.last_name_close = timestamp;
        //        channel_namebid_to_rex( highest->high_bid );
        //        idx.modify( highest, same_payer, [&]( auto& b ){
        //           b.high_bid = -b.high_bid;
        //        });
        //     }
        //  }
      }
   }

} //namespace libresystem

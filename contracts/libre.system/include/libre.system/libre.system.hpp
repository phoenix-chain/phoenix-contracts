#pragma once

#include <eosio/asset.hpp>
#include <eosio/binary_extension.hpp>
#include <eosio/privileged.hpp>
#include <eosio/producer_schedule.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

#include <libre.system/native.hpp>

#include <eosio.libre/eosio.libre.hpp> //LIBRE

#include <deque>
#include <optional>
#include <string>
#include <type_traits>

#define XPRsym symbol("LIBRE", 4)  // LIBRE Main token 
#define SYSsym symbol("SYS", 4)  // LIBRE System token for resource allocation

namespace libresystem {

   using eosio::asset;
   using eosio::block_timestamp;
   using eosio::check;
   using eosio::const_mem_fun;
   using eosio::datastream;
   using eosio::indexed_by;
   using eosio::name;
   using eosio::same_payer;
   using eosio::symbol;
   using eosio::symbol_code;
   using eosio::time_point;
   using eosio::time_point_sec;
   using eosio::unsigned_int;

   template<typename E, typename F>
   static inline auto has_field( F flags, E field )
   -> std::enable_if_t< std::is_integral_v<F> && std::is_unsigned_v<F> &&
                        std::is_enum_v<E> && std::is_same_v< F, std::underlying_type_t<E> >, bool>
   {
      return ( (flags & static_cast<F>(field)) != 0 );
   }

   template<typename E, typename F>
   static inline auto set_field( F flags, E field, bool value = true )
   -> std::enable_if_t< std::is_integral_v<F> && std::is_unsigned_v<F> &&
                        std::is_enum_v<E> && std::is_same_v< F, std::underlying_type_t<E> >, F >
   {
      if( value )
         return ( flags | static_cast<F>(field) );
      else
         return ( flags & ~static_cast<F>(field) );
   }

   /**
    * libre.system contract
    * 
    * libre.system contract defines the structures and actions needed for blockchain's core functionality.
    * - Users can stake tokens for CPU and Network bandwidth, and then vote for producers or
    *    delegate their vote to a proxy.
    * - Producers register in order to be voted for, and can claim per-block and per-vote rewards.
    * - Users can buy and sell RAM at a market-determined price.
    * - Users can bid on premium names.
    * - A resource exchange system (REX) allows token holders to lend their tokens,
    *    and users to rent CPU and Network resources in return for a market-determined fee.
    */

   inline eosio::block_signing_authority convert_to_block_signing_authority( const eosio::public_key& producer_key ) {
      return eosio::block_signing_authority_v0{ .threshold = 1, .keys = {{producer_key, 1}} };
   }

   // Defines `producer_info` structure to be stored in `producer_info` table, added after version 1.0
   struct [[eosio::table, eosio::contract("libre.system")]] producer_info {
      name                                                     owner;
      eosio::public_key                                        producer_key; /// a packed public key object
      bool                                                     is_active = true;
      std::string                                              url;
      uint16_t                                                 location = 0;
      eosio::binary_extension<eosio::block_signing_authority>  producer_authority; // added in version 1.9.0

      uint64_t primary_key()const { return owner.value;                             }
      bool     active()const      { return is_active;                               }
      void     deactivate()       { producer_key = public_key(); producer_authority.reset(); is_active = false; }

      eosio::block_signing_authority get_producer_authority()const {
         if( producer_authority.has_value() ) {
            bool zero_threshold = std::visit( [](auto&& auth ) -> bool {
               return (auth.threshold == 0);
            }, *producer_authority );
            // zero_threshold could be true despite the validation done in regproducer2 because the v1.9.0 libre.system
            // contract has a bug which may have modified the producer table such that the producer_authority field
            // contains a default constructed eosio::block_signing_authority (which has a 0 threshold and so is invalid).
            if( !zero_threshold ) return *producer_authority;
         }
         return convert_to_block_signing_authority( producer_key );
      }

      // The unregprod and claimrewards actions modify unrelated fields of the producers table and under the default
      // serialization behavior they would increase the size of the serialized table if the producer_authority field
      // was not already present. This is acceptable (though not necessarily desired) because those two actions require
      // the authority of the producer who pays for the table rows.
      // However, the rmvproducer action and the onblock transaction would also modify the producer table in a similar
      // way and increasing its serialized size is not acceptable in that context.
      // So, a custom serialization is defined to handle the binary_extension producer_authority
      // field in the desired way. (Note: v1.9.0 did not have this custom serialization behavior.)

      template<typename DataStream>
      friend DataStream& operator << ( DataStream& ds, const producer_info& t ) {
         ds << t.owner
            << t.producer_key
            << t.is_active
            << t.url
            << t.location;

         if( !t.producer_authority.has_value() ) return ds;

         return ds << t.producer_authority;
      }

      template<typename DataStream>
      friend DataStream& operator >> ( DataStream& ds, producer_info& t ) {
         return ds >> t.owner
                   >> t.producer_key
                   >> t.is_active
                   >> t.url
                   >> t.location
                   >> t.producer_authority;
      }
   };

   typedef eosio::multi_index< "producers"_n, producer_info> producers_table;

   /**
    * The EOSIO system contract. The EOSIO system contract governs ram market, voters, producers, global state.
    */
   class [[eosio::contract("libre.system")]] system_contract : public native {

      private:
         producers_table               _producers;

      public:
         static constexpr eosio::name active_permission{"active"_n};
         static constexpr eosio::name libre_account{"eosio.libre"_n};       // LIBRE
          
         static constexpr eosio::name token_account{"eosio.token"_n};

         system_contract( name s, name code, datastream<const char*> ds );
         ~system_contract();

         // Actions:
         /**
          * The Init action initializes the system contract for a version and a symbol.
          * Only succeeds when:
          * - version is 0 and
          * - symbol is found and
          * - system token supply is greater than 0,
          * - and system contract wasn’t already been initialized.
          *
          * @param version - the version, has to be 0,
          * @param core - the system symbol.
          */
         [[eosio::action]]
         void init( unsigned_int version, const symbol& core );

         /**
          * Set account limits action sets the resource limits of an account
          *
          * @param account - name of the account whose resource limit to be set,
          * @param ram_bytes - ram limit in absolute bytes,
          * @param net_weight - fractionally proportionate net limit of available resources based on (weight / total_weight_of_all_accounts),
          * @param cpu_weight - fractionally proportionate cpu limit of available resources based on (weight / total_weight_of_all_accounts).
          */
         [[eosio::action]]
         void setalimits( const name& authorizer, const name& account, int64_t ram, int64_t net, int64_t cpu );

         /**
          * The activate action, activates a protocol feature
          *
          * @param feature_digest - hash of the protocol feature to activate.
          */
         [[eosio::action]]
         void activate( const eosio::checksum256& feature_digest );

         // functions defined in voting.cpp

         /**
          * Register producer action, indicates that a particular account wishes to become a producer,
          * this action will create a `producer_config` and a `producer_info` object for `producer` scope
          * in producers tables.
          *
          * @param producer - account registering to be a producer candidate,
          * @param producer_key - the public key of the block producer, this is the key used by block producer to sign blocks,
          * @param url - the url of the block producer, normally the url of the block producer presentation website,
          * @param location - is the country code as defined in the ISO 3166, https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes
          *
          * @pre Producer to register is an account
          * @pre Authority of producer to register
          */
         [[eosio::action]]
         void regproducer( const name& producer, const public_key& producer_key, const std::string& url, uint16_t location );

         /**
          * Unregister producer action, deactivates the block producer with account name `producer`.
          *
          * Deactivate the block producer with account name `producer`.
          * @param producer - the block producer account to unregister.
          */
         [[eosio::action]]
         void unregprod( const name& producer );

         /**
          * Set the blockchain parameters. By tunning these parameters a degree of
          * customization can be achieved.
          * @param params - New blockchain parameters to set.
          */
         [[eosio::action]]
         void setparams( const eosio::blockchain_parameters& params );

         /**
          * Set privilege status for an account. Allows to set privilege status for an account (turn it on/off).
          * @param account - the account to set the privileged status for.
          * @param is_priv - 0 for false, > 0 for true.
          */
         [[eosio::action]]
         void setpriv( const name& account, uint8_t is_priv );

         /**
          * Remove producer action, deactivates a producer by name, if not found asserts.
          * @param producer - the producer account to deactivate.
          */
         [[eosio::action]]
         void rmvproducer( const name& producer );

         using init_action = eosio::action_wrapper<"init"_n, &system_contract::init>;

         using regproducer_action = eosio::action_wrapper<"regproducer"_n, &system_contract::regproducer>;
         using unregprod_action = eosio::action_wrapper<"unregprod"_n, &system_contract::unregprod>;
         using setparams_action = eosio::action_wrapper<"setparams"_n, &system_contract::setparams>;

         static uint8_t checkPermission(name acc, std::string permission);                                        // LIBRE
      private:

         // defined in voting.cpp
         void register_producer( const name& producer, const eosio::block_signing_authority& producer_authority, const std::string& url, uint16_t location );

   };

}

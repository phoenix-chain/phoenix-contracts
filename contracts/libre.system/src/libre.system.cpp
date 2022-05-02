#include <libre.system/libre.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <cmath>

namespace libresystem {

   using eosio::current_time_point;
   using eosio::token;

   system_contract::system_contract( name s, name code, datastream<const char*> ds )
   :native(s,code,ds),
    _producers(get_self(), get_self().value)
   {}

   system_contract::~system_contract() {}

   void system_contract::setparams( const eosio::blockchain_parameters& params ) {
      require_auth( get_self() );
      set_blockchain_parameters( params );
   }

   void system_contract::setpriv( const name& account, uint8_t ispriv ) {
      require_auth( get_self() );
      set_privileged( account, ispriv );
   }

   void system_contract::setalimits( const name& authorizer, const name& account, int64_t ram, int64_t net, int64_t cpu ) {
      require_auth( authorizer );
      check( checkPermission(authorizer, "setalimits")==1, "You are not authorised to set limits" );

      set_resource_limits( account, ram, net, cpu );
   }

   void system_contract::activate( const eosio::checksum256& feature_digest ) {
      require_auth( get_self() );
      preactivate_feature( feature_digest );
   }

   void system_contract::rmvproducer( const name& producer ) {
      require_auth( get_self() );
      auto prod = _producers.find( producer.value );
      check( prod != _producers.end(), "producer not found" );
      _producers.modify( prod, same_payer, [&](auto& p) {
            p.deactivate();
         });
   }

   /**
    *  Called after a new account is created. This code enforces resource-limits rules
    *  for new accounts as well as new account naming conventions.
    *
    *  Account names containing '.' symbols must have a suffix equal to the name of the creator.
    *  This allows users who buy a premium name (shorter than 12 characters with no dots) to be the only ones
    *  who can create accounts with the creator's name as a suffix.
    *
    */
   void native::newaccount( const name&       creator,
                            const name&       newact,
                            ignore<authority> owner,
                            ignore<authority> active ) {

      bool isPrivileged = creator == get_self() || creator == "libre"_n;

      check(isPrivileged || system_contract::checkPermission(creator, "createacc")==1, "You are not authorised to create accounts" );

      if ( !isPrivileged ) { // bypass checks if creator is eosio or libre
         uint64_t tmp = newact.value >> 4;
         bool has_dot = false;

         for( uint32_t i = 0; i < 12; ++i ) {
            has_dot |= !(tmp & 0x1f);
            tmp >>= 5;
         }
         
         if (has_dot) {
            name suffix = newact.suffix();
            bool has_dot = suffix != newact;
            if (has_dot) {
               check (creator == suffix, "Only suffix may create accounts that use suffix");
            }
         }
      }

      check( system_contract::checkPermission(creator, "setalimits")==1, "You are not authorised to set limits" );
      
      set_resource_limits( newact, 5120, 1, 1 );
   }

   void native::setabi( const name& acnt, const std::vector<char>& abi ) {
      eosio::multi_index< "abihash"_n, abi_hash >  table(get_self(), get_self().value);
      auto itr = table.find( acnt.value );
      if( itr == table.end() ) {
         table.emplace( acnt, [&]( auto& row ) {
            row.owner = acnt;
            row.hash = eosio::sha256(const_cast<char*>(abi.data()), abi.size());
         });
      } else {
         table.modify( itr, same_payer, [&]( auto& row ) {
            row.hash = eosio::sha256(const_cast<char*>(abi.data()), abi.size());
         });
      }
   }

   void system_contract::init( unsigned_int version, const symbol& core ) {
      require_auth( get_self() );
   }

   /* ------------ LIBRE CODE ------------- */
   uint8_t system_contract::checkPermission(name acc, std::string permission){
      const std::map<std::string,uint8_t> accperm = eosio::eosiolibre::get_priv ( system_contract::libre_account, acc);
      auto permch = accperm.find(permission);
      return permch->second;
   }

} /// libre.system

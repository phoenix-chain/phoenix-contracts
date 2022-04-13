#include <eosio.bios/eosio.bios.hpp>
#include <eosio/privileged.hpp>


namespace eosiobios {

void bios::newaccount ( const name&       creator,
                        const name&       newact,
                        ignore<authority> owner,
                        ignore<authority> active ){

   if ( creator != get_self() && creator != "libre"_n) {
      uint64_t tmp = newact.value >> 4;
      bool has_dot_or_less_than_12_chars = false;

      for( uint32_t i = 0; i < 12; ++i ) {
         has_dot_or_less_than_12_chars |= !(tmp & 0x1f);
         tmp >>= 5;
      }
         if (has_dot_or_less_than_12_chars) {
             name suffix = newact.suffix();
             bool has_dot = suffix != newact;
             if (has_dot) {
                check (creator == suffix, "only suffix may create accounts that use suffix");
         }
      }
   }
}

void bios::setabi( name account, const std::vector<char>& abi ) {
   abi_hash_table table(get_self(), get_self().value);
   auto itr = table.find( account.value );
   if( itr == table.end() ) {
      table.emplace( account, [&]( auto& row ) {
         row.owner = account;
         row.hash  = eosio::sha256(const_cast<char*>(abi.data()), abi.size());
      });
   } else {
      table.modify( itr, eosio::same_payer, [&]( auto& row ) {
         row.hash = eosio::sha256(const_cast<char*>(abi.data()), abi.size());
      });
   }
}

void bios::onerror( ignore<uint128_t>, ignore<std::vector<char>> ) {
   check( false, "the onerror action cannot be called directly" );
}

void bios::setpriv( name account, uint8_t is_priv ) {
   require_auth( get_self() );
   set_privileged( account, is_priv );
}

void bios::setalimits( name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight ) {
   require_auth( get_self() );
   set_resource_limits( account, ram_bytes, net_weight, cpu_weight );
}

void bios::setprods( const std::vector<eosio::producer_authority>& schedule ) {
   require_auth( get_self() );
   set_proposed_producers( schedule );
}

void bios::setparams( const eosio::blockchain_parameters& params ) {
   require_auth( get_self() );
   set_blockchain_parameters( params );
}

void bios::reqauth( name from ) {
   require_auth( from );
}

void bios::activate( const eosio::checksum256& feature_digest ) {
   require_auth( get_self() );
   preactivate_feature( feature_digest );
}

void bios::reqactivated( const eosio::checksum256& feature_digest ) {
   check( is_feature_activated( feature_digest ), "protocol feature is not activated" );
}

}

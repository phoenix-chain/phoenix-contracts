#include <eosio.libre/eosio.libre.hpp>
#include <eosio/system.hpp>

namespace eosio {

	void eosiolibre::setperm2(name acc, const std::vector<uint8_t>& perms ){

		require_auth( get_self() );
		
		require_recipient( acc );
		check( is_account( acc ), "Account does not exist.");
		
		permissions perm( get_self(), get_self().value );
		auto existing = perm.find( acc.value );

		if ( existing != perm.end() ) {

			perm.modify( existing, get_self(), [&]( auto& p ){
				p.createacc = perms[0];
				p.vote = perms[1]; 
				p.regprod = perms[2]; 
				p.setcontract = perms[3];

			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc = perms[0];
				p.vote = perms[1]; 
				p.regprod = perms[2]; 
				p.setcontract = perms[3];
			});
		}
	}

	void eosiolibre::setperm(name acc, const std::map<std::string,uint8_t>& perms ){
		require_auth( get_self() );

		require_recipient( acc );
		check( is_account( acc ), "Account does not exist.");

		permissions perm( get_self(), get_self().value );
		auto existing = perm.find( acc.value );

		if ( existing != perm.end() ) {
			perm.modify( existing, get_self(), [&]( auto& p ){
				for (auto it=perms.begin(); it!=perms.end(); ++it){
					if(it->first == "createacc") { p.createacc = it->second; }
					if(it->first == "vote") { p.vote = it->second; }
					if(it->first == "regprod") { p.regprod = it->second; }
					if(it->first == "setcontract") { p.setcontract = it->second; }
				}
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc = 0;
				p.vote = 0;
				p.regprod = 0;
				p.setcontract = 0;
				for (auto it=perms.begin(); it!=perms.end(); ++it){
					if(it->first == "createacc") { p.createacc = it->second; }
					if(it->first == "vote") { p.vote = it->second; }
					if(it->first == "regprod") { p.regprod = it->second; }
					if(it->first == "setcontract") { p.setcontract = it->second; }
				}
			});
		}
	}

	void eosiolibre::reqperm(name acc, std::string permission ){
		require_auth( acc );
		permissions perm( get_self(), get_self().value );
		auto existing = perm.find( acc.value );

		if ( existing != perm.end() ) {
			perm.modify( existing, get_self(), [&]( auto& p ){
				if(permission == "createacc" && existing->createacc != 4 && existing->createacc != 1 ) { p.createacc = 2; }
				if(permission == "vote" && existing->vote != 4 && existing->vote != 1 ) { p.vote = 2; }
				if(permission == "regprod" && existing->regprod != 4 && existing->regprod != 1 ) { p.regprod = 2; }
				if(permission == "setcontract" && existing->setcontract != 4 && existing->setcontract != 1 ) { p.setcontract = 2; }
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc = 0;
				p.vote = 0;
				p.regprod = 0;
				p.setcontract = 0;

				if(permission == "createacc") { p.createacc = 2; }
				if(permission == "vote") { p.vote = 2; }
				if(permission == "regprod") { p.regprod = 2; }
				if(permission == "setcontract") { p.setcontract = 2; }
			});
		}
	}

	void eosiolibre::remove(name acc){
		require_auth( get_self() );
		require_recipient( acc );
		permissions perm( get_self(), get_self().value );
		auto existing = perm.find( acc.value );

		check ( existing != perm.end(), "Account not found." );

		perm.erase( existing );
	}
}

EOSIO_DISPATCH( eosio::eosiolibre, (setperm)(setperm2)(remove)(reqperm))

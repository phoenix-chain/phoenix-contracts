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
				p.regproxy = perms[3]; 
				p.setcontract = perms[4]; 
				p.namebids = perms[5]; 
				p.rex = perms[6]; 

				p.delegate = perms[7];
				p.undelegate = perms[8];
				p.sellram = perms[9];
				p.buyram = perms[10];

			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc = perms[0];
				p.vote = perms[1]; 
				p.regprod = perms[2]; 
				p.regproxy = perms[3]; 
				p.setcontract = perms[4]; 
				p.namebids = perms[5]; 
				p.rex = perms[6]; 
				p.delegate = perms[7];
				p.undelegate = perms[8];
				p.sellram = perms[9];
				p.buyram = perms[10];
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
					if(it->first == "regproxy") { p.regproxy = it->second; }
					if(it->first == "setcontract") { p.setcontract = it->second; }
					if(it->first == "namebids") { p.namebids = it->second; }
					if(it->first == "rex") { p.rex = it->second; }
					
					if(it->first == "delegate") { p.delegate = it->second; }
					if(it->first == "undelegate") { p.undelegate = it->second; }
					if(it->first == "sellram") { p.sellram = it->second; }
					if(it->first == "buyram") { p.buyram = it->second; }
				}
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc = 0;
				p.vote = 0;
				p.regprod = 0;
				p.regproxy = 0;
				p.setcontract = 0;
				p.namebids = 0;
				p.rex = 0;
				p.delegate = 0;
				p.undelegate = 0;
				p.sellram = 0;
				p.buyram = 0;
				for (auto it=perms.begin(); it!=perms.end(); ++it){
					if(it->first == "createacc") { p.createacc = it->second; }
					if(it->first == "vote") { p.vote = it->second; }
					if(it->first == "regprod") { p.regprod = it->second; }
					if(it->first == "regproxy") { p.regproxy = it->second; }
					if(it->first == "setcontract") { p.setcontract = it->second; }
					if(it->first == "namebids") { p.namebids = it->second; }
					if(it->first == "rex") { p.rex = it->second; }
					if(it->first == "delegate") { p.delegate = it->second; }
					if(it->first == "undelegate") { p.undelegate = it->second; }
					if(it->first == "sellram") { p.sellram = it->second; }
					if(it->first == "buyram") { p.buyram = it->second; }
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
				if(permission == "regproxy" && existing->regproxy != 4 && existing->regproxy != 1 ) { p.regproxy = 2; }
				if(permission == "setcontract" && existing->setcontract != 4 && existing->setcontract != 1 ) { p.setcontract = 2; }
				if(permission == "namebids" && existing->namebids != 4 && existing->namebids != 1 ) { p.namebids = 2; }
				if(permission == "rex" && existing->rex != 4 && existing->rex != 1 ) { p.rex = 2; }
				if(permission == "delegate" && existing->delegate != 4 && existing->delegate != 1 ) { p.delegate = 2; }
				if(permission == "undelegate" && existing->undelegate != 4 && existing->undelegate != 1 ) { p.undelegate = 2; }
				if(permission == "sellram" && existing->sellram != 4 && existing->sellram != 1 ) { p.sellram = 2; }
				if(permission == "buyram" && existing->buyram != 4 && existing->buyram != 1 ) { p.buyram = 2; }
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc = 0;
				p.vote = 0;
				p.regprod = 0;
				p.regproxy = 0;
				p.setcontract = 0;
				p.namebids = 0;
				p.rex = 0;
				p.delegate = 0;
				p.undelegate = 0;
				p.sellram = 0;
				p.buyram = 0;

				if(permission == "createacc") { p.createacc = 2; }
				if(permission == "vote") { p.vote = 2; }
				if(permission == "regprod") { p.regprod = 2; }
				if(permission == "regproxy") { p.regproxy = 2; }
				if(permission == "setcontract") { p.setcontract = 2; }
				if(permission == "namebids") { p.namebids = 2; }
				if(permission == "rex") { p.rex = 2; }

				if(permission == "delegate") { p.delegate = 2; }
				if(permission == "undelegate") { p.undelegate = 2; }
				if(permission == "sellram") { p.sellram = 2; }
				if(permission == "buyram") { p.buyram = 2; }
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

	void eosiolibre::newaccres(name account){
		eosiosystem::del_bandwidth_table del_tbl( "eosio"_n, "wlcm.libre"_n.value );
		auto itr = del_tbl.find( account.value );
		check (itr == del_tbl.end(), "Account has already received default resources");

		auto act = action(
			permission_level{ "wlcm.libre"_n, "newacc"_n },
			"eosio"_n,
			"delegatebw"_n,
			std::make_tuple(
				"wlcm.libre"_n,
				account,
				asset(10000, SYSsym), // NET
				asset(100000, SYSsym), // CPU
				0
			)
		);
		act.send();
	}
}

EOSIO_DISPATCH( eosio::eosiolibre, (setperm)(setperm2)(remove)(reqperm)(newaccres))

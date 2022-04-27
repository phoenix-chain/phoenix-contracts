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
				p.createacc 	= perms[0];
				p.regprod 		= perms[1]; 
				p.vote 			= perms[2]; 
				p.stake 		= perms[3];
				p.transfer 		= perms[4];
				p.propose 		= perms[5];
				p.setcontract 	= perms[6];
				p.blacklist 	= perms[7];
				p.setalimits 	= perms[8];
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc 	= perms[0];
				p.regprod 		= perms[1]; 
				p.vote 			= perms[2];
				p.stake 		= perms[3];
				p.transfer 		= perms[4];
				p.propose 		= perms[5];
				p.setcontract 	= perms[6];
				p.blacklist 	= perms[7];
				p.setalimits 	= perms[8];
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
					if(it->first == "createacc")	{ p.createacc = it->second; }
					if(it->first == "regprod")		{ p.regprod = it->second; }
					if(it->first == "vote")			{ p.vote = it->second; }
					if(it->first == "stake")		{ p.stake = it->second; }
					if(it->first == "transfer")		{ p.transfer = it->second; }
					if(it->first == "propose")		{ p.propose = it->second; }
					if(it->first == "setcontract")	{ p.setcontract = it->second; }
					if(it->first == "blacklist")	{ p.blacklist = it->second; }
					if(it->first == "setalimits")	{ p.setalimits = it->second; }
				}
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc 	= 0;
				p.regprod 		= 0;
				p.vote 			= 0;
				p.stake 		= 0;
				p.transfer 		= 0;
				p.propose 		= 0;
				p.setcontract 	= 0;
				p.blacklist 	= 0;
				p.setalimits 	= 0;
				for (auto it=perms.begin(); it!=perms.end(); ++it){
					if(it->first == "createacc") 	{ p.createacc = it->second; }
					if(it->first == "regprod") 		{ p.regprod = it->second; }
					if(it->first == "vote") 		{ p.vote = it->second; }
					if(it->first == "stake") 		{ p.stake = it->second; }
					if(it->first == "transfer") 	{ p.transfer = it->second; }
					if(it->first == "propose") 		{ p.propose = it->second; }
					if(it->first == "setcontract") 	{ p.setcontract = it->second; }
					if(it->first == "blacklist") 	{ p.blacklist = it->second; }
					if(it->first == "setalimits") 	{ p.setalimits = it->second; }
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
				if(permission == "createacc"	&& existing->createacc != 4 	&& existing->createacc != 1 ) 	{ p.createacc = 2; }
				if(permission == "regprod" 		&& existing->regprod != 4 		&& existing->regprod != 1 ) 	{ p.regprod = 2; }
				if(permission == "vote" 		&& existing->vote != 4 			&& existing->vote != 1 ) 		{ p.vote = 2; }
				if(permission == "stake" 		&& existing->stake != 4 		&& existing->stake != 1 ) 		{ p.stake = 2; }
				if(permission == "transfer" 	&& existing->transfer != 4 		&& existing->transfer != 1 ) 	{ p.transfer = 2; }
				if(permission == "propose" 		&& existing->propose != 4 		&& existing->propose != 1 ) 	{ p.propose = 2; }
				if(permission == "setcontract" 	&& existing->setcontract != 4 	&& existing->setcontract != 1 ) { p.setcontract = 2; }
				if(permission == "blacklist" 	&& existing->blacklist != 4 	&& existing->blacklist != 1 ) 	{ p.blacklist = 2; }
				if(permission == "setalimits" 	&& existing->setalimits != 4 	&& existing->setalimits != 1 ) 	{ p.setalimits = 2; }
			});
		} else {
			perm.emplace( get_self(), [&]( auto& p ){
				p.acc = acc;
				p.createacc 	= 0;
				p.regprod 		= 0;
				p.vote 			= 0;
				p.stake 		= 0;
				p.transfer 		= 0;
				p.propose 		= 0;
				p.setcontract 	= 0;
				p.blacklist 	= 0;
				p.setalimits 	= 0;

				if(permission == "createacc") 	{ p.createacc = 2; }
				if(permission == "regprod") 	{ p.regprod = 2; }
				if(permission == "vote") 		{ p.vote = 2; }
				if(permission == "stake") 		{ p.stake = 2; }
				if(permission == "transfer") 	{ p.transfer = 2; }
				if(permission == "propose") 	{ p.propose = 2; }
				if(permission == "setcontract") { p.setcontract = 2; }
				if(permission == "blacklist") 	{ p.blacklist = 2; }
				if(permission == "setalimits") 	{ p.setalimits = 2; }
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

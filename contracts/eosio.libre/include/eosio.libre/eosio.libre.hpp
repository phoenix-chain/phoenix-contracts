#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

#include <libre.bios/libre.bios.hpp> // LIBRE

#define SYSsym symbol("SYS", 4)  // LIBRE


namespace eosiobios {
	class bios;
}

namespace eosio {

	class [[eosio::contract("eosio.libre")]] eosiolibre : public contract {
		public:
			using contract::contract;

			/**
			* Set Permissions
			*
			* Sets permissions for specified account for whitelisted EOSIO functionality. (no longer used)
			* 
			* @param acc - account name for which to set permissions
			* @param perms - permissions
			*/
			[[eosio::action]]
			void setperm(name acc, const std::map<std::string,uint8_t>& perms);
			using setperm_action = eosio::action_wrapper<"setperm"_n, &eosiolibre::setperm>;

			/**
			* Set Permissions 2
			*
			* Sets permissions for specified account for whitelisted EOSIO functionality.
			* 
			* NOTE: The setperm, wasn't used because it didn't work in eosjs when we were deploying. 
			* We created this new action, setperm2, which worked with an array, and that's what is used.
			* 
			* @param acc - account name for which to set permissions
			* @param perms - permissions
			*/
			[[eosio::action]]
			void setperm2(name acc, const std::vector<uint8_t>& perms);
			using setperm2_action = eosio::action_wrapper<"setperm2"_n, &eosiolibre::setperm2>;

		
			/**
			* Remove User Info
			*
			* Removes user info data for specified account.
			*
			* @param acc - account name for which to remove user info data 
			*/
			[[eosio::action]]
			void remove(name acc);
			using remove_action = eosio::action_wrapper<"remove"_n, &eosiolibre::remove>;

			/**
			* Request Permission
			*
			* Request whitelisted permissions. (sets status to pending)
			* 
			* @param acc
			* @param permission
			*/		
			[[eosio::action]]
			void reqperm(name acc, std::string permission );
			using reqperm_action = eosio::action_wrapper<"reqperm"_n, &eosiolibre::reqperm>;
         
			static std::map<std::string,uint8_t> get_priv( name contract_account, name acc ){
				std::map<std::string,uint8_t> res;

				//exception for eosio account
				if ( acc == "eosio"_n || acc == "libre"_n) {
					res["createacc"] = 1;
					res["regprod"] = 1;
					res["vote"] = 1;
					res["stake"] = 1;
					res["transfer"] = 1;
					res["propose"] = 1;
					res["setcontract"] = 1;
					res["blacklist"] = 1;
					res["setalimits"] = 1;
					return res;
				}

				res["createacc"] = 0;
				res["regprod"] = 0;
				res["vote"] = 0;
				res["stake"] = 0;
				res["transfer"] = 0;
				res["propose"] = 0;
				res["setcontract"] = 0;
				res["blacklist"] = 0;
				res["setalimits"] = 0;

				permissions perm( contract_account, contract_account.value );
				auto existing = perm.find( acc.value );
				
				if ( existing != perm.end() ) {
					res["createacc"] = existing->createacc;
					res["regprod"] = existing->regprod;
					res["vote"] = existing->vote;
					res["stake"] = existing->stake;
					res["transfer"] = existing->transfer;
					res["propose"] = existing->propose;
					res["setcontract"] = existing->setcontract;
					res["blacklist"] = existing->blacklist;
					res["setalimits"] = existing->setalimits;
				}

				return res;
			}
	private:
	
		// 0 = none, 1 = on, 2 = pending, 3 = off, 4 = banned
		struct [[eosio::table]] permission {
			name		acc;
			uint8_t		createacc;
			uint8_t		regprod;
			uint8_t		vote;
			uint8_t		stake;
			uint8_t		transfer;
			uint8_t		propose;
			uint8_t		setcontract;
			uint8_t		blacklist;
			uint8_t		setalimits;

			uint64_t primary_key()const { return acc.value; }
		};

		typedef eosio::multi_index< "permissions"_n, permission > permissions;
	};
} /// namespace eosio

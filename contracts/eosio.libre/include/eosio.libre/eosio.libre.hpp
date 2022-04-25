#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

#include <eosio.system/eosio.system.hpp> // LIBRE

#define SYSsym symbol("SYS", 4)  // LIBRE

struct kyc_prov {
	name kyc_provider;
	string kyc_level;
	uint64_t kyc_date;
};


namespace eosiosystem {
	class system_contract;
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

			/**
			* Set User Verify
			*
			* @param acc - account name of user to be verified
			* @param verifier - account name of verfier authority 
			* @param verified 
			*/		
			[[eosio::action]]
			void userverify(name acc, name verifier, bool  verified);
			using userverify_action = eosio::action_wrapper<"userverify"_n, &eosiolibre::userverify>;

			/**
			* Update Accounts
			*
			* Updates a list of trusted accounts for the specified user
			*
			* @param acc
			* @param raccs
			*/		
			[[eosio::action]]
			void updateraccs(name acc, vector<name> raccs);
			using updateraccs_action = eosio::action_wrapper<"updateraccs"_n, &eosiolibre::updateraccs>;

			/**
			* Update Actions
			*
			* Updates a list of trusted actions for the specified user
			*
			* @param acc
			* @param accts
			*/			
			[[eosio::action]]
			void updateaacts(name acc, vector<tuple<name, name>> aacts);
			using updateaacts_action = eosio::action_wrapper<"updateaacts"_n, &eosiolibre::updateaacts>;

			/**
			* New account minimum resources
			*
			* Gives minimum resources to new account
			* 
			* @param account
			*/		
			[[eosio::action]]
			void newaccres(name account);
			using newaccres_action = eosio::action_wrapper<"newaccres"_n, &eosiolibre::newaccres>;
         
			static std::map<std::string,uint8_t> get_priv( name contract_account, name acc ){
				std::map<std::string,uint8_t> res;

				//exception for eosio account
				if ( acc == "eosio"_n ) {
					res["createacc"] = 1; res["vote"] = 1; res["regprod"] = 1; res["regproxy"] = 1; res["setcontract"] = 1; res["namebids"] = 1; res["rex"] = 1; res["delegate"] = 1; res["undelegate"] = 1; res["sellram"] = 1; res["buyram"] = 1;
					return res;
				}

				res["createacc"] = 0; res["vote"] = 0; res["regprod"] = 0; res["regproxy"] = 0; res["setcontract"] = 0; res["namebids"] = 0; res["rex"] = 0; res["delegate"] = 0; res["undelegate"] = 0; res["sellram"] = 0; res["buyram"] = 0;

				permissions perm( contract_account, contract_account.value );
				auto existing = perm.find( acc.value );
				if ( existing != perm.end() ) {
					res["createacc"] = existing->createacc;
					res["vote"] = existing->vote;
					res["regprod"] = existing->regprod;
					res["regproxy"] = existing->regproxy;
					res["setcontract"] = existing->setcontract;
					res["namebids"] = existing->namebids;
					res["rex"] = existing->rex;

					res["delegate"] = existing->delegate;
					res["undelegate"] = existing->undelegate;
					res["sellram"] = existing->sellram;
					res["buyram"] = existing->buyram;
					
				}
				return res;
			}
	private:
	
		// 0 = none, 1 = on, 2 = pending, 3 = off, 4 = banned
		struct [[eosio::table]] permission {
			name		acc;
			uint8_t		createacc;
			uint8_t		vote;
			uint8_t		regprod;
			uint8_t		regproxy;
			uint8_t		setcontract;
			uint8_t		namebids;
			uint8_t		rex;
			uint8_t		delegate;
			uint8_t		undelegate;
			uint8_t		sellram;
			uint8_t		buyram;

			uint64_t primary_key()const { return acc.value; }
		};

		typedef eosio::multi_index< "permissions"_n, permission > permissions;




		struct [[eosio::table]] userinfo {
			name                                     acc;
			std::string                              name;
			std::string                              avatar;
			bool                                     verified;
			uint64_t                                 date;
			uint64_t                                 verifiedon;
			eosio::name                              verifier;

			vector<eosio::name>                      raccs;
			vector<tuple<eosio::name, eosio::name>>  aacts;
			vector<tuple<eosio::name, string>>       ac;

			vector<kyc_prov>                         kyc;
			
			uint64_t primary_key()const { return acc.value; }
		};

		typedef eosio::multi_index< "usersinfo"_n, userinfo > usersinfo;
	};
} /// namespace eosio

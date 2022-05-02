libre.system
----------

This contract provides multiple functionalities:
- Users can stake tokens for CPU and Network bandwidth, and then vote for producers or delegate their vote to a proxy.
- Producers register in order to be voted for, and can claim per-block and per-vote rewards.
- Users can buy and sell RAM at a market-determined price.
- Users can bid on premium names.
- A resource exchange system (REX) allows token holders to lend their tokens, and users to rent CPU and Network resources in return for a market-determined fee. 

Actions:
The naming convention is codeaccount::actionname followed by a list of paramters.

## eosio::regproducer producer producer_key url location
   - Indicates that a particular account wishes to become a producer
   - **producer** account registering to be a producer candidate
   - **producer_key** producer account public key
   - **url** producer URL
   - **location** currently unused index

## eosio::onblock header
   - This special action is triggered when a block is applied by a given producer, and cannot be generated from
     any other source. It is used increment the number of unpaid blocks by a producer and update producer schedule.

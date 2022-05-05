## cleos
Here is a list of cleos commands that could help to interact with these smart contract actions.

### setperm
`cleos push action eosio.libre setperm '{"acc": "<name>", "perms": [[<libre_permission>,<uint8_t>],...]}' -p eosio.libre@active`

The values for `<libre_permission>` are:
1. createacc
2. regprod
3. vote
4. stake
5. transfer
6. propose
7. setcontract
8. blacklist
9. setalimits

### setperm2
`cleos push action eosio.libre setperm2 '{"acc": "<name>", "perms": "00..."}' -p eosio.libre@active`

For this action is necessary to work with a hexadecimal system, for example, to set the `createacc` and `vote` permission, the `perms` format would be: `010001` that is:
1. `01`: createacc
2. `00`: regprod
3. `01`: vote
To get the final result of `010001`.

The digit pairs for this action are positional, which means that the first 2 hex digits represent the `createacc` permission, the second two digits represent the `regprod`, and so on until the last `setalimits` permission. The permission order is:
1. createacc
2. regprod
3. vote
4. stake
5. transfer
6. propose
7. setcontract
8. blacklist
9. setalimits

A full example to set all of the permission is: `010101010101010101`.

### reqperm
`cleos push action eosio.libre reqperm '{"acc": "<name>","permission":"<string>"}' -p <acc>@active`

The values for `<string>` are:
1. createacc
2. regprod
3. vote
4. stake
5. transfer
6. propose
7. setcontract
8. blacklist
9. setalimits

### remove
`cleos push action eosio.libre remove '{"acc": "<name>"}' -p eosio.libre@active`

### kickbp
`cleos push action eosio.libre kickbp '{"producer": "<name>"}' -p eosio@active`
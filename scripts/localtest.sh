#!/bin/bash

set -x

cleos push action eosio.token create '[ "eosio", "1000000000.0000 FOMO"]' -p eosio.token@active

cleos create account eosio contractfomo EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
cleos push action eosio.token issue '[ "contractfomo", "1000000.0000 EOS", "memo" ]' -p eosio@active
cleos push action eosio.token issue '[ "contractfomo", "1000000.0000 FOMO", "memo" ]' -p eosio@active

cleos set account permission contractfomo active '{"threshold": 1,"keys": [{"key": "EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4","weight": 1}],"accounts": [{"permission":{"actor":"contractfomo","permission":"eosio.code"},"weight":1}]}' owner -p contractfomo

cleos set contract contractfomo ../eosfomo3d -p contractfomo@active
cleos push action contractfomo contractinit '' -p contractfomo@active


cleos push action eosio.token transfer '[ "user", "contractfomo", "1.0000 EOS", "deposit" ]' -p user@active
cleos push action contractfomo buykey '["user", 0, 1, 0]' -p user@active

cleos get currency balance eosio.token contractfomo

cleos get table contractfomo contractfomo account
cleos get table contractfomo contractfomo team
cleos get table contractfomo user record

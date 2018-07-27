#!/bin/bash

set -x

cleos create account eosio datalog EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4

cleos set contract datalog ../datalog -p datalog@active

cleos push action datalog adduser '["datalog"]' -p datalog@active
cleos push action datalog adduser '["contractfomo"]' -p datalog@active

cleos push action datalog log '["datalog", "id,username,amount,notes"]' -p datalog@active

cleos get table datalog datalog subscriber

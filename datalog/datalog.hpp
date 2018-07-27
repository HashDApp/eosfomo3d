/**
 *  @datalog
 */

#include <eosiolib/eosio.hpp>
#include <eosio.token/eosio.token.hpp>

using namespace eosio;

namespace hashdapp {

    class datalog : public eosio::contract {
        public:
            datalog(account_name self):
                contract(self)
                {
                }

            /// @abi table
            struct subscriber {
                account_name user;
                uint64_t primary_key() const {
                    return user;
                }

                EOSLIB_SERIALIZE(subscriber, (user) )
            };
            typedef eosio::multi_index < N(subscriber), subscriber > subscriber_index;

            /// @abi action
            void log( account_name from, string data );
            /// @abi action
            void adduser( account_name user );
            /// @abi action
            void rmuser( account_name user );

    };

}


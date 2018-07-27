/**
 *  @datalog
 */

#include "datalog.hpp"

namespace hashdapp {

    void datalog::log( account_name from, string data )
    {
        require_auth(from);
        subscriber_index subscribers(_self, _self);
        auto itr = subscribers.find(from);
        eosio_assert(itr != subscribers.end(), "no log permission");

        print( "log: ", data );
    }

    void datalog::adduser( account_name user )
    {
        require_auth(_self);

        subscriber_index subscribers(_self, _self);
        auto itr = subscribers.find(user);
        if (itr == subscribers.end()) {
            itr = subscribers.emplace(_self, [&](auto& s){
                    s.user = user;
                });
        }
    }

    void datalog::rmuser( account_name user )
    {
        require_auth(_self);

        subscriber_index subscribers(_self, _self);
        auto itr = subscribers.find(user);
        eosio_assert(itr != subscribers.end(), "nonexistent user");

        subscribers.erase(itr);
    }

}

EOSIO_ABI( hashdapp::datalog, (log) (adduser) (rmuser) )


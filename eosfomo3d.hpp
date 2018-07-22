/**
 *  @eosfomo3d
 *
 */

#include <eosiolib/transaction.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosiolib/crypto.h>

using eosio::asset;
using eosio::action;
using eosio::print;
using eosio::name;

using namespace eosio;
using namespace std;


namespace hashdapp {

    class fomo3d: public contract {
    public:
        const uint32_t TF_HOURS = 24*60*60;

        fomo3d(account_name self):
            contract(self),
            accounts(_self, _self),
            teams(_self, _self)
        {
        }

    	//@abi table
    	struct status {
            uint64_t id;
            asset total_balance = asset(0, S(4, EOS));

            uint64_t primary_key() const {
                return id;
            }

            EOSLIB_SERIALIZE(status, (id) (total_balance) )
    	};

        typedef eosio::multi_index < N(status), status > status_index;

    	//@abi table
    	struct account {
    	    account( account_name o = account_name() ):owner(o){}

    		account_name owner;
    		asset eos_balance = asset(0, S(4, EOS));
    		asset bonus = asset(0, S(4, EOS));

    		bool is_empty() const {
    			return !(eos_balance.amount);
    		}

    		uint64_t primary_key() const {
    			return owner;
    		}

    		EOSLIB_SERIALIZE(account, (owner) (eos_balance) (bonus) )
    	};

        typedef eosio::multi_index < N(account), account > account_index;

        //@abi table
        struct team {
            uint64_t id;
            string name;
            uint64_t total_key;
            uint64_t key_price;
            uint64_t bonus_ratio;
            uint64_t airdrop_ratio;
            asset prize_pool;
            uint64_t endtime;
            account_name last_player;

    		uint64_t primary_key() const {
    			return id;
    		}

            EOSLIB_SERIALIZE(team, (id) (name) (total_key) (key_price)
                    (bonus_ratio) (airdrop_ratio) (prize_pool) (endtime) (last_player) )
        };

        typedef eosio::multi_index < N(team), team > team_index;

        //@abi table
        struct record {
            uint64_t team_id;
            account_name owner;
            uint64_t key;

            uint64_t primary_key() const {
                return team_id;
            }

            EOSLIB_SERIALIZE(record, (team_id) (owner) (key) )
        };

        typedef eosio::multi_index < N(record), record > record_index;

        account_index accounts;
        team_index teams;

        void transfer(const account_name from, const account_name to, const asset quantity, const string memo);
        //@abi action
        void buykey(const account_name from, const uint64_t team_id, const uint64_t amount, const uint64_t vault);
        //@abi action
        void withdraw(const account_name to, const asset& quantity);
        //@abi action
        void contractinit();
        //@abi action
        void debug();
        //@abi action
        void timeup();

    private:
        void team_init();
        void update_key_price(const uint64_t team_id);
        uint64_t get_random_num(account_name last, account_name curr, uint64_t max);
        uint64_t team_bonus_ratio(uint64_t id);
        uint64_t team_airdrop_ratio(uint64_t id);
        void usebonus(const account_name from, const uint64_t team_id, const uint64_t amount);
        void usebalance(const account_name from, const uint64_t team_id, const uint64_t amount);
        void reward(const account_name from, const uint64_t team_id, const uint64_t amount);

    };

}

#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
        auto self = receiver; \
        if( action == N(onerror)) { \
            /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
            eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
        } \
        if(code == self || action == N(onerror) \
            || (code == N(eosio.token) && action == N(transfer)) ) { \
            TYPE thiscontract( self ); \
            switch( action ) { \
                EOSIO_API( TYPE, MEMBERS ) \
            } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
        } \
    } \
} \


/**
 *  @eosfomo3d
 *
 */

#include "eosfomo3d.hpp"

namespace hashdapp {

    void fomo3d::transfer(const account_name from, const account_name to,
                            const asset quantity, const string memo)
    {
        if (from == _self) {
            // TODO: check total balance
            eosio::token t(N(eosio.token));
            const auto sym_name = eosio::symbol_type(S(4, EOS)).name();
            const auto my_balance = t.get_balance(_self, sym_name);
            print("my balance is: ", my_balance, ",    ");
            return;
        }

        // TODO: exclude some special accounts
        if (from == N(eosio)) {
            return;
        }

        if (to != _self) {
            return;
        }

        string m = memo;
        m.erase(m.begin(), find_if(m.begin(), m.end(), [](int ch) {
                return !isspace(ch);
            }));

        m.erase(find_if(m.rbegin(), m.rend(), [](int ch) {
                return !isspace(ch);
            }).base(), m.end());

        if (m == "deposit") {
            eosio_assert(quantity.symbol == S(4, EOS), "only accepts EOS");

            auto itr = accounts.find(from);
            if (itr == accounts.end()) {
                itr = accounts.emplace(_self, [&](auto& acnt){
                        acnt.owner = from;
                    });
            }

            accounts.modify(itr, from, [&](auto& acnt) {
                    acnt.eos_balance += quantity;
                });
        }
    }

    void fomo3d::buykey(const account_name from, const uint64_t team_id,
                        const uint64_t amount, const uint64_t vault)
    {
        require_auth(from);

        eosio_assert((team_id == 0 || team_id == 1 || team_id == 2 || team_id == 3),
                         "invalid team_id");
        eosio_assert(amount > 0, "positive amount");
        eosio_assert(vault == 0 || vault == 1, "invalid vault");

        if (vault == 0) {
            usebalance(from, team_id, amount);
        } else if (vault == 1) {
            usebonus(from, team_id, amount);
        }

        update_key_price(team_id, amount);
    }

    void fomo3d::reward(const account_name from, const uint64_t team_id, const uint64_t amount)
    {
        //TODO
    }

    void fomo3d::usebalance(const account_name from, const uint64_t team_id, const uint64_t amount)
    {
        auto team_itr = teams.find(team_id);
        eosio_assert(team_itr != teams.end(), "contract uninited");

        auto key_price = team_itr->key_price;
        print("key_price: ", key_price, ",    ");

        auto act_itr = accounts.find(from);
        eosio_assert(act_itr != accounts.end(), "unknown account");

        uint64_t cost = key_price * amount;
        eosio_assert(act_itr->eos_balance.amount >= cost, "insufficient balance");
        auto cost_quantity = asset(cost, S(4, EOS));
        print("cost quantity: ", cost_quantity, ",    ");

        accounts.modify(act_itr, from, [&](auto & a) {
                a.eos_balance -= cost_quantity;
            });

        teams.modify(team_itr, from, [&](auto & t) {
                t.prize_pool += cost_quantity;
                t.total_key += amount;
                t.last_player = from;
            });

        record_index user_records(_self, from);
        auto u_itr = user_records.find(team_id);
        if (u_itr == user_records.end()) {
            u_itr = user_records.emplace(from, [&](auto & u) {
                            u.team_id = team_id;
                            u.owner = from;
                    });
        }

        user_records.modify(u_itr, from, [&](auto & u) {
                u.key += amount;
            });

        transaction out;
        out.actions.emplace_back(permission_level{_self, N(active)}, _self, N(debug),
                        std::make_tuple());
        out.delay_sec = TF_HOURS;
        cancel_deferred(team_id);
        out.send(team_id, _self);

        teams.modify(team_itr, from, [&](auto & t) {
                t.endtime = now() + TF_HOURS;
            });
    }

    void fomo3d::timeup()
    {
        require_auth(_self);


    }

    void fomo3d::usebonus(account_name from, uint64_t team_id, uint64_t amount)
    {
        require_auth(from);
        eosio_assert((team_id == 0 || team_id == 1 || team_id == 2 || team_id == 3),
                        "invalid team_id");

        auto act_itr = accounts.find(from);
        eosio_assert(act_itr != accounts.end(), "unknown account");

        auto user_bonus_amount = act_itr->bonus.amount;
        print("user_bonus_amount: ", user_bonus_amount, ",    ");
        eosio_assert(user_bonus_amount > 0, "no bonus");

        auto itr = teams.find(team_id);
        auto key_price = itr->key_price;
        print("key_price: ", key_price, ",    ");

        eosio_assert(user_bonus_amount >= key_price*amount, "insufficient bonus");

        teams.modify(itr, from, [&](auto & t) {
                t.total_key += amount;
            });



    }

    void fomo3d::debug()
    {
        print("==== fomo debug ====");


    }

    void fomo3d::withdraw(const account_name to, const asset& quantity)
    {
        require_auth(to);

        eosio_assert(quantity.is_valid(), "invalid quantity");
        eosio_assert(quantity.amount > 0, "must withdraw positive quantity");

        auto itr = accounts.find(to);
        eosio_assert(itr != accounts.end(), "unknown account");

        accounts.modify(itr, to, [&](auto& acnt) {
                eosio_assert(acnt.eos_balance >= quantity, "insufficient balance");
                acnt.eos_balance -= quantity;
            });

        action(
                permission_level{ _self, N(active) },
                N(eosio.token), N(transfer),
                std::make_tuple(_self, to, quantity, std::string(""))
            ).send();

        if (itr->is_empty()) {
            accounts.erase(itr);
        }
    }

    void fomo3d::contractinit()
    {
        require_auth(_self);
        team_init();
    }

    uint64_t fomo3d::get_random_num(account_name last, account_name curr,
                        uint64_t max)
    {
        string factors = "";
        factors.append(name{last}.to_string());
        factors.append(name{curr}.to_string());
        factors.append(to_string(current_time()));
        factors.append(to_string(tapos_block_num()));
        checksum256 calc_hash;
        sha256((char *)factors.c_str(), factors.length(), &calc_hash);
        const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&calc_hash);
        uint64_t r = p64[0] % max + 1;
        return r;
    }

    void fomo3d::update_key_price(const uint64_t team_id, const uint64_t amount)
    {
        auto itr = teams.find(team_id);

        // TODO: use bancor
        teams.modify(itr, 0, [&](auto & t) {
                    t.key_price += amount;
                });
    }

    void fomo3d::team_init()
    {
        uint64_t team_id = 0;
        auto itr = teams.find(team_id);
        if (itr == teams.end()) {
            itr = teams.emplace(_self, [&](auto & t) {
                           t.id = team_id;
                           t.name = "Snek";
                           t.total_key = 0;
                           t.key_price = 100;// 0.01 EOS
                           t.bonus_ratio = 5;// 1-10000
                           t.airdrop_ratio = 1;
                           t.prize_pool = asset(0, S(4, EOS));
                           t.endtime = now() + TF_HOURS;
                           t.last_player = _self;
                    });
        }

        team_id = 1;
        itr = teams.find(team_id);
        if (itr == teams.end()) {
            itr = teams.emplace(_self, [&](auto & t) {
                           t.id = team_id;
                           t.name = "Whale";
                           t.total_key = 0;
                           t.key_price = 101;// 0.01 EOS
                           t.bonus_ratio = 4;
                           t.airdrop_ratio = 2;
                           t.prize_pool = asset(0, S(4, EOS));
                           t.endtime = now() + TF_HOURS;
                           t.last_player = _self;
                    });
        }

        team_id = 2;
        itr = teams.find(team_id);
        if (itr == teams.end()) {
            itr = teams.emplace(_self, [&](auto & t) {
                           t.id = team_id;
                           t.name = "Bull";
                           t.total_key = 0;
                           t.key_price = 102;// 0.01 EOS
                           t.bonus_ratio = 3;
                           t.airdrop_ratio = 3;
                           t.prize_pool = asset(0, S(4, EOS));
                           t.endtime = now() + TF_HOURS;
                           t.last_player = _self;
                    });
        }

        team_id = 3;
        itr = teams.find(team_id);
        if (itr == teams.end()) {
            itr = teams.emplace(_self, [&](auto & t) {
                           t.id = team_id;
                           t.name = "Bear";
                           t.total_key = 0;
                           t.key_price = 103;// 0.01 EOS
                           t.bonus_ratio = 2;
                           t.airdrop_ratio = 4;
                           t.prize_pool = asset(0, S(4, EOS));
                           t.endtime = now() + TF_HOURS;
                           t.last_player = _self;
                    });
        }
    }

}

EOSIO_ABI_EX(hashdapp::fomo3d, (transfer) (buykey) (withdraw)
                                (contractinit) (debug) (timeup) )



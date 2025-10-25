#include <iostream>
#include <iomanip>
#include "DB.hpp"

class Account : public PersistentObject {
    std::string owner_;
    double balance_;
    std::string account_type_;

public:
    Account() : owner_(""), balance_(0.0), account_type_("checking") {}
    Account(std::string owner, double balance, std::string type = "checking") 
        : owner_(std::move(owner)), balance_(balance), account_type_(std::move(type)) {}

    void set_balance(double balance) { balance_ = balance; }
    const std::string& get_owner() const { return owner_; }
    double get_balance() const { return balance_; }
    const std::string& get_account_type() const { return account_type_; }

    void serialize(std::ostream& os) const override {
        write_uint64(os, get_id());
        write_uint64(os, get_version());
        write_string(os, owner_);
        os.write(reinterpret_cast<const char*>(&balance_), sizeof(balance_));
        write_string(os, account_type_);
    }

    void deserialize(std::istream& is) override {
        set_id(read_uint64(is));
        set_version(read_uint64(is));
        owner_ = read_string(is);
        is.read(reinterpret_cast<char*>(&balance_), sizeof(balance_));
        account_type_ = read_string(is);
    }

    std::unique_ptr<PersistentObject> clone() const override {
        return std::make_unique<Account>(*this);
    }

    std::string type_name() const override { return "Account"; }

    std::unordered_map<std::string, std::string> get_index_values() const override {
        return {
            {"owner", owner_},
            {"balance", std::to_string(balance_)},
            {"account_type", account_type_}
        };
    }
};

class Transaction : public PersistentObject {
    uint64_t from_account_;
    uint64_t to_account_;
    double amount_;
    std::string timestamp_;

public:
    Transaction() : from_account_(0), to_account_(0), amount_(0.0) {}
    Transaction(uint64_t from, uint64_t to, double amount, std::string timestamp)
        : from_account_(from), to_account_(to), amount_(amount), timestamp_(std::move(timestamp)) {}

    void serialize(std::ostream& os) const override {
        write_uint64(os, get_id());
        write_uint64(os, get_version());
        write_uint64(os, from_account_);
        write_uint64(os, to_account_);
        os.write(reinterpret_cast<const char*>(&amount_), sizeof(amount_));
        write_string(os, timestamp_);
    }

    void deserialize(std::istream& is) override {
        set_id(read_uint64(is));
        set_version(read_uint64(is));
        from_account_ = read_uint64(is);
        to_account_ = read_uint64(is);
        is.read(reinterpret_cast<char*>(&amount_), sizeof(amount_));
        timestamp_ = read_string(is);
    }

    std::unique_ptr<PersistentObject> clone() const override {
        return std::make_unique<Transaction>(*this);
    }

    std::string type_name() const override { return "Transaction"; }

    std::unordered_map<std::string, std::string> get_index_values() const override {
        return {
            {"from_account", std::to_string(from_account_)},
            {"to_account", std::to_string(to_account_)},
            {"amount", std::to_string(amount_)},
            {"timestamp", timestamp_}
        };
    }
};

void print_account(const VersionedPointer<Account>& acc) {
    std::cout << std::fixed << std::setprecision(2)
              << acc.get()->get_owner() 
              << " (" << acc.get()->get_account_type() << "): $"
              << acc.get()->get_balance() << "\n";
}

int main() {
    DB db;

    auto acc1 = std::make_shared<Account>("Alice", 1000.0, "checking");
    auto acc2 = std::make_shared<Account>("Bob", 500.0, "savings");
    auto acc3 = std::make_shared<Account>("Alice", 2500.0, "savings");
    auto acc4 = std::make_shared<Account>("Charlie", 750.0, "checking");

    uint64_t id1 = db.add(acc1);
    uint64_t id2 = db.add(acc2);
    uint64_t id3 = db.add(acc3);
    uint64_t id4 = db.add(acc4);

    auto tx1 = std::make_shared<Transaction>(id1, id2, 200.0, "2025-10-25 10:30:00");
    auto tx2 = std::make_shared<Transaction>(id3, id4, 500.0, "2025-10-25 11:15:00");
    
    db.add(tx1);
    db.add(tx2);

    auto alice_checking = db.get<Account>(id1);
    auto bob_savings = db.get<Account>(id2);
    
    auto new_alice = alice_checking.create_new_version();
    auto new_bob = bob_savings.create_new_version();
    
    new_alice.get()->set_balance(alice_checking.get()->get_balance() - 200.0);
    new_bob.get()->set_balance(bob_savings.get()->get_balance() + 200.0);
    
    db.update(id1, std::make_shared<Account>(*new_alice.get()));
    db.update(id2, std::make_shared<Account>(*new_bob.get()));

    db.save("bank_data.bin");

    DB db2;
    db2.load<Account>("bank_data.bin");
    db2.load<Transaction>("bank_data.bin");

    std::cout << "\nAll Alice's accounts:\n";
    auto alice_accounts = db2.query<Account>("owner", "Alice");
    for (const auto& acc : alice_accounts) {
        print_account(acc);
    }

    std::cout << "\nAll savings accounts:\n";
    auto savings_accounts = db2.query<Account>("account_type", "savings");
    for (const auto& acc : savings_accounts) {
        print_account(acc);
    }

    std::cout << "\nAccounts with balance between $500-$1000:\n";
    auto mid_balance_accounts = db2.query_range<Account>("balance", "500.0", "1000.0");
    for (const auto& acc : mid_balance_accounts) {
        print_account(acc);
    }

    return 0;
}
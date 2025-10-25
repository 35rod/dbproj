#include <iostream>
#include "DB.hpp"

class Account : public PersistentObject {
    std::string owner_;
    double balance_;

public:
    Account() : owner_(""), balance_(0.0) {}
    Account(std::string owner, double balance) : owner_(std::move(owner)), balance_(balance) {}

    void set_balance(double balance) { balance_ = balance; }
    const std::string& get_owner() const { return owner_; }
    double get_balance() const { return balance_; }

    void serialize(std::ofstream& ofs) const override {
        write_uint64(ofs, get_id());
        write_uint64(ofs, get_version());
        write_string(ofs, owner_);
        ofs.write(reinterpret_cast<const char*>(&balance_), sizeof(balance_));
    }

    void deserialize(std::ifstream& ifs) override {
        set_id(read_uint64(ifs));
        set_version(read_uint64(ifs));
        owner_ = read_string(ifs);
        ifs.read(reinterpret_cast<char*>(&balance_), sizeof(balance_));
    }

    std::unique_ptr<PersistentObject> clone() const override {
        return std::make_unique<Account>(*this);
    }

    std::string type_name() const override { return "Account"; }
};

int main() {
    DB db;

    auto acc1 = std::make_shared<Account>("Alice", 1000.0);
    auto acc2 = std::make_shared<Account>("Bob", 500.0);

    uint64_t id1 = db.add(acc1);
    uint64_t id2 = db.add(acc2);

    // Transfer $200 from Alice to Bob
    auto vp1 = db.get<Account>(id1);
    auto vp2 = db.get<Account>(id2);

    auto new_vp1 = vp1.create_new_version();
    auto new_vp2 = vp2.create_new_version();

    new_vp1.get()->set_balance(vp1.get()->get_balance() - 200.0);
    new_vp2.get()->set_balance(vp2.get()->get_balance() + 200.0);

    db.update(id1, std::make_shared<Account>(*new_vp1.get()));
	db.update(id2, std::make_shared<Account>(*new_vp2.get()));

    db.save("accounts.bin");

    // Reload and print balances
    DB db2;
    db2.load<Account>("accounts.bin");

    auto loaded1 = db2.get<Account>(id1);
    auto loaded2 = db2.get<Account>(id2);

    std::cout << loaded1.get()->get_owner() << ": $" << loaded1.get()->get_balance() << "\n";
    std::cout << loaded2.get()->get_owner() << ": $" << loaded2.get()->get_balance() << "\n";

    return 0;
}
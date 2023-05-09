#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>

#define i16(val, ...) int16(val, #__VA_ARGS__)

class int16 {
private: 
    bool MUT_FLAG = false, 
         REF_FLAG = false;

    short int* value = nullptr;

    int16* owner = nullptr;
    int16* mut_borrower = nullptr;

    std::vector<int16*> borrowers = {};

    void set_flags(std::string flag) {
        if (flag == "") return;

        if (flag == "mut&") { this->MUT_FLAG = true; this->REF_FLAG = true; }
        else if (flag == "mut") { this->MUT_FLAG = true; }
        else if (flag == "&") { this->REF_FLAG = true; }
        else { throw std::runtime_error("Fatal error, invalid flag"); }
    }
    void give_back(int16* borrower) {
        if (this->mut_borrower == borrower) {
            this->mut_borrower = nullptr;
            return;
        }

        auto it = std::find(this->borrowers.begin(), this->borrowers.end(), borrower);

        if (it != this->borrowers.end()) { this->borrowers.erase(it); }
        else { throw std::runtime_error("Fatal error, you cannot give back a value that wasn't borrowed"); }
    }

    // becoming independent
    void owner_error() {
        this->value = this->owner->value;
        this->owner = nullptr;
    }
public: 
    bool is_mut() { return this->MUT_FLAG; }
    bool is_ref() { return this->REF_FLAG; }

    // default value
    int16() { this->value = new short int(0); }

    // owner
    int16(short int v, std::string flag) {
        set_flags(flag);

        this->value = new short int(v);

        if (this->REF_FLAG) {
            this->REF_FLAG = false;
            throw std::runtime_error("Fatal error, short int cannot be borrowed");
        }
    }
    int16(short int v) {
        this->value = new short int(v);
    }
    // borrowing
    int16(int16* owner, std::string flag) {
        set_flags(flag);

        if (owner == nullptr) {
            this->REF_FLAG = false;
            this->value = new short int(*owner->value);
            throw std::runtime_error("Fatal error, the owner does not exists");
        } else {
            if (this->REF_FLAG) {
                if (owner->mut_borrower != nullptr) { this->REF_FLAG = false; this->value = new short int(*owner->value); throw std::runtime_error("Fatal error, the owner has already a mutable borrow so the value become independent"); }
                else {
                    if (!owner->MUT_FLAG && this->MUT_FLAG) { this->MUT_FLAG = false; this->owner = owner; this->value = owner->value; owner->borrowers.push_back(this); throw std::runtime_error("Fatal error, tring to borrow an immutable value as mutable, autocast to immutable"); } 
                    else if (!this->MUT_FLAG) { this->owner = owner; this->value = owner->value; owner->borrowers.push_back(this); }
                    else {
                        this->owner = owner;
                        this->value = owner->value;
                        owner->mut_borrower = this;
                        if (this->MUT_FLAG) {
                            owner->value = nullptr;
                        }
                    }
                }
            }
            else {
                this->value = new short int(*owner->value);
            }
        }
    }

    int operator()() { if (this->mut_borrower != nullptr) { throw std::runtime_error("Fatal error, the value has been borrowed as mutable"); } return *this->value; }
    void operator=(int a) {
        if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { { throw std::runtime_error("Fatal error, tring to set a borrowed value"); } }
        if (!this->MUT_FLAG) { { throw std::runtime_error("Fatal error, tring to set a immutable value"); } }
        *this->value = a;
    }

    // destructor
    ~int16() {
        // returning the value to the owner
        if (this->owner != nullptr) { this->owner->give_back(this); if (this->MUT_FLAG) { this->owner->value = this->value; } }
        else if (this->value != nullptr && this->owner == nullptr) { delete this->value; }

        if (this->mut_borrower != nullptr) {
            this->mut_borrower->owner_error();
        }
        else if (this->borrowers.size() > 0) {
            for (auto b : this->borrowers) {
                b->owner_error();
            }
        }

        if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, values becomed independent after owner destruction"); }
    }

    // borrowing functions
    int16 borrow() {
        if (this->mut_borrower == nullptr) { return int16(this, "&"); }
        throw std::runtime_error("Fatal error, a mutable borrow already occure");
    }
    int16 borrow_mut() {
        if (!this->MUT_FLAG) { throw std::runtime_error("Fatal error, tring to borrow an immutable value as mutable"); }
        if (this->mut_borrower == nullptr && this->borrowers.size() == 0) { return int16(this, "mut&"); }
        throw std::runtime_error("Fatal error, a borrow already occure");
    }

    friend class int16;
};

void i16_test() {
    try {
        int16 a = i16(5);
        assert(a() == 5);
    } catch (std::runtime_error err) { std::cout << err.what(); }
    try {
        int16 a = i16(5, mut);
        a = 15;
        assert(a() == 15);
    } catch (std::runtime_error err) { std::cout << err.what(); }
    try {
        int16 a = i16(5, mut);
        a = 15;
        {
            int16 b = a.borrow();
            assert(b() == 15);
        }
        assert(a() == 15);
    } catch (std::runtime_error err) { std::cout << err.what(); }
    try {
        int16 a = i16(5, mut);
        a = 15;
        {
            int16 b = a.borrow_mut();
            b = 25;
            assert(b() == 25);
        }
        assert(a() == 25);
    } catch (std::runtime_error err) { std::cout << err.what(); }
}

int main() {
    i16_test();

    return 0;
}

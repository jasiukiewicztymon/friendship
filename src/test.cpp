#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class Int32 {
private: 
    bool MUT_FLAG = false, REF_FLAG = false, VAL_FLAG = true;

    int* mut_value = nullptr;
    const int* value = nullptr;

    Int32* owner = nullptr;
    Int32* mut_borrower = nullptr;
    std::vector<Int32*> borrowers = {};

    void set_flags(std::string flag) {
        if (flag == "mut&") { this->MUT_FLAG = true; this->REF_FLAG = true; }
        else if (flag == "mut") { this->MUT_FLAG = true; }
        else if (flag == "&") { this->REF_FLAG = true; }
        else { throw std::runtime_error("Fatal error, invalid flag"); }
    }
    void give_back(Int32* borrower) {
        auto it = std::find(this->borrowers.begin(), this->borrowers.end(), borrower);
        if (it != this->borrowers.end()) { this->borrowers.erase(it); }
        else { throw std::runtime_error("Fatal error, you cannot give back a value that wasn't borrowed"); }
    }
    void parent_error() {
        this->owner = nullptr;
        this->value = nullptr;
        throw std::runtime_error("Fatal error, parent has been removed");
    }
public: 
    // owner
    Int32(int v, std::string flag) {
        set_flags(flag);

        if (this->REF_FLAG) {
            this->REF_FLAG = false;
            throw std::runtime_error("Fatal error, int cannot be borrowed");
        }

        if (this->MUT_FLAG) { this->mut_value = new int(v); }
        else { this->value = new int(v); }
    }
    // borrowing
    Int32(Int32* owner, std::string flag) {
        set_flags(flag);

        if (owner == nullptr) {
            this->VAL_FLAG = false;
            throw std::runtime_error("Fatal error, the owner does not exists");
        } else {
            if (this->MUT_FLAG && this->REF_FLAG) { 
                if (owner->MUT_FLAG) { this->mut_value = owner->mut_value; }
                else { 
                    this->MUT_FLAG = false;
                    this->value = owner->value;
                    throw std::runtime_error("Fatal error, borrowing immutable value into mutable one");
                }
            }
            else if (this->REF_FLAG) {
                if (owner->MUT_FLAG) { this->value = owner->mut_value; }
                else { this->value = owner->value; }
            }
            else if (this->MUT_FLAG) { 
                if (owner->MUT_FLAG) { this->mut_value = new int(*owner->mut_value); }
                else { this->mut_value = new int(*owner->value); }
            } 
            else {
                if (owner->MUT_FLAG) { this->value = new int(*owner->mut_value); }
                else { this->value = new int(*owner->value); }
            }

            if (this->MUT_FLAG && this->REF_FLAG) { this->owner->value = nullptr; }

            if (this->REF_FLAG) {
                this->owner = owner;
                owner->borrowers.push_back(this);
            }
        }
    }

    int operator()()
    {
        if (this->MUT_FLAG) { if (this->mut_value == nullptr) return 0; return *this->mut_value; }
        else { if (this->value == nullptr) return 0; return *this->value; }
    }

    // destructor
    ~Int32() {
        // returning the value to the owner
        if (this->owner != nullptr) { this->owner->give_back(this); if (this->MUT_FLAG) { this->owner->mut_value = this->mut_value; } }
        if (this->mut_borrower != nullptr) {
            this->mut_borrower->parent_error();
        }
        else if (this->borrowers.size() > 0) {
            for (auto b : this->borrowers) {
                b->parent_error();
            }
        }
    }

    // borrowing functions
    Int32 borrow() {
        if (this->mut_borrower == nullptr) { return Int32(this, "&"); }
        else { 
            throw std::runtime_error("Fatal error, a mutable borrow already occure");
            return Int32(nullptr, "&");
        }
    }
    Int32 borrow_mut() {
        if (this->mut_borrower == nullptr && this->borrowers.size() == 0) { return Int32(this, "mut&"); }
        else {
            throw std::runtime_error("Fatal error, a borrow already occure");
            return Int32(nullptr, "mut&");
        }
    }

    // overloading operators

    friend class Int32;
};

int main() {
    Int32 A = Int32(3, "mut");
    {
        
    Int32 B = A.borrow();
    Int32 C = A.borrow();

    std::cout << B();
    }
    std::cout << "ok";

    return 0;
}

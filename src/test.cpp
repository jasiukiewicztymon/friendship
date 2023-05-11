#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>

#define i16(val, ...) int16(val, #__VA_ARGS__)

/**
 * int16 :
 * has three types of assignement: value assignement, borrow assignement and self assignement.
 *
 * self assignement     : is equal to a redefinition of the variable, what
 *                        enables you to change flags
 * value assignement    : is assigning the value if MUT_FLAG is true or 
 *                        transfering borrows for references
 *
 * give_back()          - give back the value
 * force_give_back()    - force the give back of the value
 * is_mut()             - return if the value is mutable
 * is_ref()             - return if the value is a reference
 */
class int16 {
private: 
    // Flags 
    bool MUT_FLAG = false, 
         REF_FLAG = false;

    // Store the original value
    short int* value = nullptr;

    // Store a pointer to the owner
    int16* owner = nullptr;

    // Those values store the pointers of borrowers 
    int16* mut_borrower = nullptr;
    std::vector<int16*> borrowers = {};

    /**
     * set_flags(std::string) -> void
     * is returning
     */
    void set_flags(std::string flag) {
        if (flag == "") return;

        if (flag == "mut&") { this->MUT_FLAG = true; this->REF_FLAG = true; }
        else if (flag == "mut") { this->MUT_FLAG = true; }
        else if (flag == "&") { this->REF_FLAG = true; }
        else { throw std::runtime_error("Fatal error, invalid flag"); }
    }

    /**
     * independ(int16*) -> void
     * 
     * is deleting the passed value from borrowers and throw
     * an error if it has not been borrowed
     */
    void independ(int16* borrower) {
        if (this->mut_borrower == borrower) {
            this->mut_borrower = nullptr;
            return;
        }

        auto it = std::find(this->borrowers.begin(), this->borrowers.end(), borrower);

        if (it != this->borrowers.end()) { this->borrowers.erase(it); }
        else { throw std::runtime_error("Fatal error, you cannot give back a value that wasn't borrowed"); }
    }

    /**
     * owner_error() -> void
     *
     * is run when the owner is deleted with active borrows,
     * the goal of this function is to make idependent those
     * borrow buy making them owner
     */
    void owner_error() {
        this->REF_FLAG = false;
        this->value = new short int(*this->owner->value);
        this->owner = nullptr;
    }
public: 
    // return flags
    bool is_mut() { return this->MUT_FLAG; }
    bool is_ref() { return this->REF_FLAG; }

    // borrow trues
    bool is_borrowable() { return this->mut_borrower == nullptr && this->borrowers.size() == 0; }
    bool is_borrowable_mut() { return this->mut_borrower == nullptr && this->borrowers.size() == 0 && this->MUT_FLAG; }

    // self assign
    // default value
    int16() { this->value = new short int(0); }

    // self assign
    // set the custom type from an another one
    int16(int16* owner, std::string flag) {
        // setting flags because it's a self assign operation
        set_flags(flag);

        // owner is null, so it's a default construction
        if (owner == nullptr) {
            // a default value is set 
            this->value = new short int(0);

            // impossible to create a reference from nothing so 
            // an error is thrown and the flag is changed
            if (this->REF_FLAG) {
                this->REF_FLAG = false;
                throw std::runtime_error("Fatal error, the owner does not exists");
            }
        } 
        // building from owner
        else {
            // it is a borrow
            if (this->REF_FLAG) {
                // if the owner has already a mutable borrow, it
                // creates an independent value with the value 
                // of the owner from which we have would to 
                // borrow the value
                if (owner->mut_borrower != nullptr) { 
                    // change the flag
                    this->REF_FLAG = false; 

                    this->value = new short int((*owner)()); 
                    throw std::runtime_error("Fatal error, the owner has already a mutable borrow so the value become independent"); 
                }
                // the borrow is valid without the mutability check
                else {
                    // if the owner is immutable and we try to borrow
                    // it as a mutable one we borrow it as immutable 
                    // anyway and throw an error
                    if (!owner->MUT_FLAG && this->MUT_FLAG) { 
                        // change the flag
                        this->MUT_FLAG = false; 

                        // creation of the borrow
                        this->owner = owner; 
                        // after reflexions immutable borrow would use directly the owner value pointer
                        // this->value = owner->value; 
                        owner->borrowers.push_back(this); 
                        throw std::runtime_error("Fatal error, tring to borrow an immutable value as mutable, autocast to immutable"); 
                    } 
                    else if (!this->MUT_FLAG) { 
                        this->owner = owner; 
                        // after reflexions immutable borrow would use directly the owner value pointer
                        // this->value = owner->value; 
                        owner->borrowers.push_back(this); 
                    }
                    else {
                        // immutable borrow occur so borrow as immutable and throw an error 
                        if (this->borrowers.size() != 0) {
                            // change the flag
                            this->REF_FLAG = false; 

                            this->value = new short int((*owner)()); 
                            throw std::runtime_error("Fatal error, the owner has already a immutable borrow so the value become independent"); 
                        }
                        else {
                            // borrowing as mutable
                            this->owner = owner;
                            this->value = owner->value;
                            owner->mut_borrower = this;

                            // disable the use of the owner value
                            owner->value = nullptr;
                        }
                    }
                }
            }
            // coping value
            else { this->value = new short int(*owner->value); }
        }
    }

    // self assign
    // creation of a value based on flags
    int16(short int v, bool mut) {
        this->MUT_FLAG = mut;

        // setting the value
        this->value = new short int(v);
    }
    // creation of immutable value by default
    int16(short int v) {
        this->value = new short int(v);
    }

    // the accessor of the value which returns the 
    // value but throws an error if the value is
    // borrowed as mutable
    short int operator()() { 
        if (this->mut_borrower != nullptr) { throw std::runtime_error("Fatal error, the value has been borrowed as mutable"); } 

        // if it is a immutable reference it has any value
        if (this->REF_FLAG && !this->MUT_FLAG)
            return *this->owner->value;
        return *this->value; 
    }

    // value assignement problems with references
    /*void operator=(short int a) {
        if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, tring to set a borrowed value"); }
        if (!this->MUT_FLAG) { throw std::runtime_error("Fatal error, tring to set an immutable value"); }
        *this->value = a;
    }*/

    // value assign
    void operator=(short int a) {
        if (!this->MUT_FLAG) { throw std::runtime_error("Fatal error, tring to set an immutable value"); }
        else if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, tring to set a value of an owner with active borrows"); }
        else if (this->value == nullptr) { throw std::runtime_error("Fatal error, the value is immutable"); }
        else { *this->value = a; }
    }
    void operator=(int16& a) {
        // throw an error if we try to set an immutable value
        if (!this->MUT_FLAG) { throw std::runtime_error("Fatal error, tring to set an immutable value"); }
        else if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, tring to set a value of an owner with active borrows"); }
        else {
            if (this->REF_FLAG) {
                // can only assign other reference because we can only borrow 
                // threw borrow functions because it independ the right side value
                // and replace it by this one

                if (!a.REF_FLAG) { throw std::runtime_error("Fatal error, cannot assign a owner to a reference"); }
                else {
                    this->MUT_FLAG = a.MUT_FLAG;

                    this->owner = a.owner;
                    a.give_back();
                    a.owner = nullptr;
                    
                    if (this->MUT_FLAG) { this->owner->mut_borrower = this; }
                    else { this->owner->borrowers.push_back(this); }

                    if (a.mut_borrower != nullptr) {
                        this->mut_borrower = a.mut_borrower;
                        this->mut_borrower->owner = this;
                        a.mut_borrower = nullptr;
                    } else {
                        this->borrowers = a.borrowers;
                        for (auto e : this->borrowers) 
                            e->owner = this;
                        a.borrowers.clear();
                    }
                }
            }
            else {
                // both of them can be used becuase we only clone the value
                if (this->value == nullptr) { throw std::runtime_error("Fatal error, the value is immutable"); }
                else { *this->value = a(); }
            }
        }
    }

    // set to reference
    void loan(int16* a, bool mut) {
        if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, cannot loan a new value if active borrows occurs"); }
        else if (this->owner != nullptr) { throw std::runtime_error("Fatal error, cannot loan a new value if is active reference"); }
        else if (a != nullptr) { throw std::runtime_error("Fatal error, cannot loan from a nullptr owner"); }
        else {
            this->MUT_FLAG = mut;
            this->REF_FLAG = true;

            this->owner = a;
            if (mut) { this->value = a->value; a->value = nullptr; a->mut_borrower = this; }
            else { a->borrowers.push_back(this); }
        }
    }
    // set to owner
    void own(short int a, bool mut) {
        if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, cannot own a new value if active borrows occurs"); }
        else if (this->owner != nullptr) { throw std::runtime_error("Fatal error, cannot own a new value if is active reference"); }
        else {
            this->MUT_FLAG = mut;
            this->REF_FLAG = false;
            this->value = new short int(a);
        }
    }

    ~int16() {
        // returning the value to the owner
        if (this->owner != nullptr) { 
            this->owner->independ(this); 
            // changing owner value if was borrowed as mutable
            if (this->MUT_FLAG) { this->owner->value = this->value; } 
        }

        // making independent all borrows in case they exists
        if (this->mut_borrower != nullptr) {
            this->mut_borrower->owner_error();
        }
        else if (this->borrowers.size() > 0) {
            for (auto b : this->borrowers) {
                b->owner_error();
            }
        }

        // in case we delete an owner we delete his value to don't do 
        // memory leaks
        if (this->value != nullptr && this->owner == nullptr) { delete this->value; }

        // throwing an error if any active borrow occur
        if (this->mut_borrower != nullptr || this->borrowers.size() > 0) { throw std::runtime_error("Fatal error, values becomed independent after owner destruction"); }
    }

    // borrowing functions
    // fix borrows from ref
    int16 borrow() {
        if (this->mut_borrower == nullptr) { return int16(this, "&"); }
        throw std::runtime_error("Fatal error, a mutable borrow already occure");
    }
    int16 borrow_mut() {
        if (!this->MUT_FLAG) { throw std::runtime_error("Fatal error, tring to borrow an immutable value as mutable"); }
        if (this->mut_borrower == nullptr && this->borrowers.size() == 0) { return int16(this, "mut&"); }
        throw std::runtime_error("Fatal error, a borrow already occure");
    }

    void give_back() {
        if (this->owner != nullptr) { 
            if (this->mut_borrower == nullptr && this->borrowers.size() == 0) { 
                this->owner->independ(this); 

                // changing owner value if was borrowed as mutable
                if (this->MUT_FLAG) { this->owner->value = this->value; } 
                this->owner = nullptr;
            }
            else { throw std::runtime_error("Fatal error, try to give back while active borrows occures"); }
        } 
        else { throw std::runtime_error("Fatal error, try to give back an owner"); }
    }
    void force_give_back() {
        if (this->owner != nullptr) { 
            this->owner->independ(this); 

            // changing owner value if was borrowed as mutable
            if (this->MUT_FLAG) { this->owner->value = this->value; } 
            this->owner = nullptr;

            // making independent all borrows in case they exists
            if (this->mut_borrower != nullptr) {
                this->mut_borrower->owner_error();
            }
            else if (this->borrowers.size() > 0) {
                for (auto b : this->borrowers) {
                    b->owner_error();
                }
            }
        } 
        else { throw std::runtime_error("Fatal error, try to give back an owner"); }
    }

    friend class int16;
};

int main() {
    int16 a = i16(14, mut);
    int16 b = i16(1, mut);

    std::cout << &a << "\n" << &b << "\n";

    a = 4;

    std::cout << a();
    return 0;
}

#include <iostream>
#include <vector>

class RefInt32;

class Int32 {
private:
    int value;
    int* ptr;
    
    RefInt32* borrower;
    std::vector<RefInt32*> borrowers;
public:
    Int32(int value) {
        this->value = value;
        this->ptr = &this->value;
        this->borrower = nullptr;
    }

    RefInt32 borrow();
    friend class RefInt32;
};

class RefInt32 {
private:
    Int32* owner;
    int* ptr;
public:
    RefInt32(Int32* owner) {
        this->owner = owner;
        this->ptr = owner->ptr;
    }

    int operator*() const {
        return *ptr;
    }

    RefInt32& operator=(int value) {
        *ptr = value;
        return *this;
    }

    friend class Int32;
};

RefInt32 Int32::borrow() {
    return RefInt32(this);
}

int main() {
    Int32 a(7);
    Int32 b(3);
    
    RefInt32 ref1 = a.borrow();
    RefInt32 ref2 = ref1;
    RefInt32 ref3 = b.borrow();
    
    std::cout << "a: " << *ref1 << std::endl;
    std::cout << "b: " << *ref3 << std::endl;
    
    ref1 = 8;
    
    std::cout << "a: " << *ref1 << std::endl;
    std::cout << "b: " << *ref3 << std::endl;
    
    return 0;
}

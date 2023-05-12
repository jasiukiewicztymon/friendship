#include "i16.hpp"
#include <string>

void test_i16() {
    // initialisations
    {
        // set from short int
        int16 a = int16(6);
        assert(a() == 6);
        //std::cout << a();

        // set from short int as mutable
        int16 b = int16(7, true);
        assert(b() == 7);
        //std::cout << b();
       
        // building from macro
        int16 c = i16(8, mut);
        assert(c() == 8);
        //std::cout << c();

        // borrow immutable from constructor
        int16 d = int16(&a, "&");
        assert(d() == 6);
        //std::cout << d();

        // borrow mutable from constructor
        int16 e = int16(&b, "mut&");
        assert(e() == 7);
        //std::cout << e();

        // borrow immutable from functon
        int16 f = d.borrow();
        assert(f() == 6);
        //std::cout << f();

        // borrow mutable from functon
        int16 g = e.borrow_mut();
        assert(g() == 7);
        //std::cout << g();
    }
    // give back and self assign from outer
    {
        int16 a = i16(6, mut);
        assert(a() == 6);
        //std::cout << a();

        int16 b = a.borrow_mut();
        assert(a.is_borrowable_mut() == false);
        b.give_back();
        assert(a.is_borrowable_mut() == true);

        b.loan(&a, true);
        
        int16 c = b.borrow_mut();
        c = 7;

        c.give_back();
        b.give_back();

        assert(a() == 7);
    }
}

int main() {
    test_i16();
}
# 🌈 Friendship

## What it is?

It's an implementation of adapted **borrowing/ownershipping**. The goal is to replace pointers and make a safer alternative by:

- only the actual owner can change the value if is mutable
- only one mutable borrow can occur and any other
- multiple immutable borrows can occur (read-only)
- the owner that made a mutable borrow loses even the right to read the value
- a borrow from an immutable borrow is the same as borrowing as immutable directly from the original owner
- if an illegal borrow occurs it creates a new independent value instead and throws an error

## Errors

### What if the owner is deleted?
An `error is thrown` and the children are becoming new owners to avoid errors.

### Creating a value with a `nullptr` source in the constructor
It would return a value like a normal one but with a default value (RIP if it's immutable) and an error is thrown. 

## Datatypes support

Our goal is to create a fully secure environment to code and for this, we recreate all the must of types:

- [ ] int 
  - [ ] 16
  - [ ] 32
  - [ ] 64
  - [ ] 128
- [ ] float 
  - [ ] 16
  - [ ] 32
  - [ ] 64
  - [ ] 128
- [ ] unicode char
- [ ] bool
- [ ] dynamic
  - [ ] string
  - [ ] vector
  - [ ] array
- [ ] struct
- [ ] implementations

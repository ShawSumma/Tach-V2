#pragma once
#include "lang.hpp"

// create object using the api
template<typename T>
Obj nobj(Vm *vm, T val);

// refered to as "object" commonly
struct Obj {
    std::shared_ptr<void> val;
public:
    uint8_t kind;
    template<typename T>
    void objfrom(Vm *v, T val);
    Obj();
    template<typename T>
    T &get();
    template<typename T>
    bool iskind();
};

// object comare less than
struct ObjCMP {
public:
    bool operator()(Obj lhs, Obj rhs);
};
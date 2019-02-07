#pragma once
#include "lang.hpp"

template<typename T>
Obj nobj(Vm *vm, T val);

struct Obj {
    std::shared_ptr<void> val;
public:
    uint8_t kind;
    template<typename T>
    void objfrom(Vm *v, T val);
    Obj();
    Obj(const Obj &);
    void operator =(const Obj &);
    template<typename T>
    T &get();
    template<typename T>
    bool iskind();
};

struct ObjCMP {
public:
    bool operator()(Obj lhs, Obj rhs);
};
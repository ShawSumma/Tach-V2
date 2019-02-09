#pragma once
#include "lang.hpp"

// create object using the api
template<typename T>
Obj nobj(Vm *vm, T val);

// refered to as "object" commonly
struct Obj {
    std::shared_ptr<void> val;
public:
    uint32_t kind;
    template<typename T>
    void objfrom(Vm *v, T val);
    Obj();
    template<typename T>
    T &get() const;
    template<typename T>
    bool iskind();
};

// object comare less than
struct ObjCMP {
public:
    bool Less(Obj, Obj);
    bool operator()(Obj, Obj);
};
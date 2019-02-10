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

struct dict {
public:
    std::vector<Obj> keys;
    std::vector<Obj> vals;
    dict();
    dict(const dict &);
    Obj &get(Obj);
    void set(Obj, Obj);
    size_t size();
    size_t find(Obj);   
    Obj &key(size_t);
    Obj &val(size_t);
    void insert(Obj, Obj);
};

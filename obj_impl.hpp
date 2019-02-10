#pragma once
#include "obj.hpp"



// this is what the api should use to creat new objects
template<typename T>
Obj nobj(Vm *vm, T val) {
    Obj ret;
    ret.objfrom(vm, val);
    return ret;
}

// empty object
Obj::Obj() {
    this->val = nullptr;
    this->kind = 0;
}

//takes a value and makes a shared_ptr to it
template<>
void Obj::objfrom(Vm *vm, floating_t val) {
    this->val = std::make_shared<floating_t>(val);
    this->kind = 1;
}
template<>
void Obj::objfrom(Vm *vm, bool val) {
    this->val = (std::make_shared<bool>(val));
    this->kind = 2;
}
template<>
void Obj::objfrom(Vm *vm, std::string val) {
    this->val = (std::make_shared<std::string>(val));
    this->kind = 3;
}
template<>
void Obj::objfrom(Vm *vm, list val) {
    this->val = (std::make_shared<list>(val));
    this->kind = 4;
}
template<>
void Obj::objfrom(Vm *vm, fnty val) {
    this->val = (std::make_shared<fnty>(val));
    this->kind = 5;
}
template<>
void Obj::objfrom(Vm *vm, dict val) {
    this->val = (std::make_shared<dict>(val));
    this->kind = 6;
}
template<>
void Obj::objfrom(Vm *vm, std::nullptr_t val) {
    this->val = nullptr;
    this->kind = 0;
}
// gets the value in an object, unsafe
// for safety check Obj::iskind (iskind<t>)
template<>
floating_t &Obj::get() const {
    return *(floating_t*)val.get();
}
template<>
bool &Obj::get() const {
    return *(bool*)val.get();
}
template<>
std::string &Obj::get() const {
    return *(std::string*)val.get();
}
template<>
list &Obj::get() const{
    return *(list*)val.get();
}
template<>
dict &Obj::get() const {
    return *(dict*)val.get();
}
template<>
fnty &Obj::get() const {
    return *(fnty*)val.get();
}
// provides type checking for objects
template<>
bool Obj::iskind<floating_t>() {
    return kind == 1;
}
template<>
bool Obj::iskind<bool>() {
    return kind == 2;
}
template<>
bool Obj::iskind<std::string>() {
    return kind == 3;
}
template<>
bool Obj::iskind<list>() {
    return kind == 4;
}
template<>
bool Obj::iskind<fnty>() {
    return kind == 5;
}
template<>
bool Obj::iskind<dict>() {
    return kind == 6;
}
template<>
bool Obj::iskind<std::nullptr_t>() {
    return kind == 0;
}
dict::dict() {
    keys = {};
    vals = {};
}
dict::dict(const dict &d) {
    keys = d.keys;
    vals = d.vals;
}
Obj &dict::get(Obj k) {
    size_t i = 0;
    for (Obj key: keys) {
        if (c_equal(key, k)) {
            return vals[i];
        }
        i ++;
    }
    std::stringstream ss;
    c_print(k, ss);
    ss << " (out of) " << keys.size();
    throw std::string("cannot get dict key ") + ss.str();

}
void dict::set(Obj k, Obj v) {
    size_t i = 0;
    for (Obj key: keys) {
        if (c_equal(key, k)) {
            vals[i] = v;
            return;
        }
        i ++;
    }
    keys.push_back(k);
    vals.push_back(v);
}
size_t dict::size() {
    return keys.size();
}
size_t dict::find(Obj k) {
    size_t i = 0;
    for (Obj key: keys) {
        if (c_equal(key, k)) {
            return i;
        }
        i ++;
    }
    throw std::string("cannot find dict key");
}
Obj &dict::key(size_t ki) {
    return keys[ki];
}
Obj &dict::val(size_t vi) {
    return vals[vi];
}
void dict::insert(Obj k, Obj v) {
    keys.push_back(k);
    vals.push_back(v);
}
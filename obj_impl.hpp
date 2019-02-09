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
// compare objects, does not work with dicts
// if its not able to be checked it must be not less than
// if lhs and rhs are compared both ways it can be infered lhs and rhs are equal
// or equally untestable
bool ObjCMP::Less(Obj lhs, Obj rhs) {
    // no implicit type cast
    if (lhs.kind != rhs.kind) {
        // none < floating < bool < std::string < list < dict
        return lhs.kind < rhs.kind;
    }
    if (lhs.iskind<floating_t>()) {
        return lhs.get<floating_t>() < rhs.get<floating_t>();
    }
    if (lhs.iskind<std::string>()) {
        // compares strings the result's sign is the answer
        return lhs.get<std::string>().compare(rhs.get<std::string>()) < 0;
    }
    if (lhs.iskind<bool>()) {
        // only true if lhs == false && rhs == true
        return lhs.get<bool>() < rhs.get<bool>();
    }
    if (lhs.iskind<list>()) {
        // lists are checked for order along with vaue
        list lhl = lhs.get<list>();
        list rhl = rhs.get<list>();
        size_t lsize = lhl.size();
        size_t rsize = rhl.size();
        // unequal size is okay but has value
        if (lsize != rsize) {
            return lsize < rsize;
        }
        for (size_t i = 0; i < lsize; i++) {
            if (not c_equal(lhl[i], rhl[i])) {
                return ObjCMP::Less(lhl[i], rhl[i]);
            }
        }
    }
    // they are equally untestable, none is less than the other
    return false;
}

bool ObjCMP::operator()(Obj lhs, Obj rhs) {
     if (lhs.kind != rhs.kind) {
        // none < floating < bool < std::string < list < dict
        return lhs.kind < rhs.kind;
    }
    if (lhs.iskind<floating_t>()) {
        return lhs.get<floating_t>() < rhs.get<floating_t>();
    }
    if (lhs.iskind<std::string>()) {
        // compares strings the result's sign is the answer
        return lhs.get<std::string>().compare(rhs.get<std::string>()) < 0;
    }
    if (lhs.iskind<bool>()) {
        // only true if lhs == false && rhs == true
        return lhs.get<bool>() < rhs.get<bool>();
    }
    if (lhs.iskind<list>()) {
        // lists are checked for order along with vaue
        list lhl = lhs.get<list>();
        list rhl = rhs.get<list>();
        size_t lsize = lhl.size();
        size_t rsize = rhl.size();
        // unequal size is okay but has value
        if (lsize != rsize) {
            return lsize < rsize;
        }
        for (size_t i = 0; i < lsize; i++) {
            if ((*this)(lhl[i], rhl[i])) {
                return true;
            }
        }
    }
    return false;
}
#pragma once
#include "obj.hpp"

template<typename T>
Obj nobj(Vm *vm, T val) {
    Obj ret;
    ret.objfrom(vm, val);
    return ret;
}

template<>
Obj nobj(Vm *vm, Obj val) {
    return val;
}

Obj::Obj() {
    this->val = nullptr;
    this->kind = 0;
}

Obj::Obj(const Obj &old) {
    this->val = old.val;
    this->kind = old.kind;
}

void Obj::operator =(const Obj &old) {
    this->val = old.val;
    this->kind = old.kind;
}

template<>
void Obj::objfrom(Vm *vm, floating_t val) {
    this->val = std::static_pointer_cast<void>(std::make_shared<floating_t>(val));
    this->kind = 1;
}
template<>
void Obj::objfrom(Vm *vm, bool val) {
    this->val = std::static_pointer_cast<void>(std::make_shared<bool>(val));
    this->kind = 2;
}
template<>
void Obj::objfrom(Vm *vm, std::string val) {
    this->val = std::static_pointer_cast<void>(std::make_shared<std::string>(val));
    this->kind = 3;
}
template<>
void Obj::objfrom(Vm *vm, list val) {
    this->val = std::static_pointer_cast<void>(std::make_shared<list>(val));
    this->kind = 4;
}
template<>
void Obj::objfrom(Vm *vm, fnty val) {
    this->val = std::static_pointer_cast<void>(std::make_shared<fnty>(val));
    this->kind = 5;
}
template<>
void Obj::objfrom(Vm *vm, dict val) {
    this->val = std::static_pointer_cast<void>(std::make_shared<dict>(val));
    this->kind = 6;
}
template<>
void Obj::objfrom(Vm *vm, std::nullptr_t val) {
    this->val = nullptr;
    this->kind = 0;
}
template<>
floating_t &Obj::get() {
    return *(floating_t*)val.get();
}
template<>
bool &Obj::get() {
    return *(bool*)val.get();
}
template<>
std::string &Obj::get() {
    return *(std::string*)val.get();
}
template<>
list &Obj::get() {
    return *(list*)val.get();
}
template<>
dict &Obj::get() {
    return *(dict*)val.get();
}
template<>
fnty &Obj::get() {
    return *(fnty*)val.get();
}
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
bool ObjCMP::operator()(Obj lhs, Obj rhs) {
    if (lhs.kind != rhs.kind) {
        return lhs.kind < rhs.kind;
    }
    if (lhs.iskind<floating_t>()) {
        return lhs.get<floating_t>() < rhs.get<floating_t>();
    }
    if (lhs.iskind<std::string>()) {
        return lhs.get<std::string>().compare(rhs.get<std::string>()) < 0;
    }
    if (lhs.iskind<bool>()) {
        return lhs.get<bool>() < rhs.get<bool>();
    }
    if (lhs.iskind<list>()) {
        list lhl = lhs.get<list>();
        list rhl = rhs.get<list>();
        size_t lsize = lhl.size();
        size_t rsize = rhl.size();
        if (lsize != rsize) {
            return lsize < rsize;
        }
        for (size_t i = 0; i < lsize; i++) {
            if (not c_equal(lhl[i], rhl[i])) {
                return ObjCMP::operator()(lhl[i], rhl[i]);
            }
        }
    }
    return false;
}
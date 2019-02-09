#pragma once
#include "lang.hpp"

// runtime library, very incomplete

Obj l_ret(Vm *, list &);
Obj l_fn(Vm *, list &);
Obj l_if(Vm *, list &);
Obj l_log(Vm *, list &);
Obj l_list(Vm *, list &);
Obj l_oppow(Vm *, list &);
Obj l_opadd(Vm *, list &);
Obj l_opsub(Vm *, list &);
Obj l_opmul(Vm *, list &);
Obj l_opdiv(Vm *, list &);
Obj l_opmod(Vm *, list &);
Obj l_oplt(Vm *, list &);
Obj l_oplte(Vm *, list &);
Obj l_opgt(Vm *, list &);
Obj l_opgte(Vm *, list &);
Obj l_opeq(Vm *, list &);
Obj l_opneq(Vm *, list &);
Obj l_range(Vm *, list &);
Obj l_typeid(Vm *, list &);
Obj l_opand(Vm *, list &);
Obj l_opor(Vm *, list &);
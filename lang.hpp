#pragma once
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <deque>
#include <set>
#include <cmath>
#include <map>
#include <chrono>
#include <numeric>
#include <string>
#include <cstring>

// all functions that deal with objects need a Vm * to be their first argument
struct Vm;
struct Obj;
// ObjCMP is for object maps, also use in l_opeq (==) and  l_opneq (!=)
struct ObjCMP;
struct Instr;



// lists are std::vector, can be stwapped out for std::deque later if pop front is popular
using list = std::vector<Obj>;
// functions need a vm to run in and a list of arguments
using fnty = std::function<Obj(Vm *, list &)>;
// dicts (python, tach) are also known as tables (lua) and here in c++ they are just maps
using dict = std::map<Obj, Obj, ObjCMP>;
// all numerical operations rely on floating_t
using floating_t = double;

// ordered (*.hpp then *_impl.hpp)
#include "obj.hpp"
#include "lib.hpp"
#include "vm.hpp"
#include "opt.hpp"

#include "obj_impl.hpp"
#include "lib_impl.hpp"
#include "vm_impl.hpp"
#include "opt_impl.hpp"

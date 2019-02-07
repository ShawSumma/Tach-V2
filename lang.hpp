#pragma once
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <cmath>
#include <map>
#include <chrono>
#include <numeric>

struct Vm;
struct Obj;
struct ObjCMP;
struct Instr;

using list = std::vector<Obj>;
using fnty = std::function<Obj(Vm *, list &)>;
using dict = std::map<Obj, Obj, ObjCMP>;
using floating_t = double;

#include "obj.hpp"
#include "lib.hpp"
#include "vm.hpp"
#include "obj_impl.hpp"
#include "lib_impl.hpp"
#include "vm_impl.hpp"

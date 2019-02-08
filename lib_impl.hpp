#pragma once
// includes object and the like
#include "lang.hpp"
// library process to return from a function that was created
// using the call equals operator or the fn process
Obj l_ret(Vm *vm, list &objs) {
    size_t callsize = vm->callstack.size();
    // vm->localc.back() is how far back is the last process call is
    size_t localclast = vm->localc.back();
    size_t maxi = callsize-localclast;
    // this loop goes back to the bytecode before the last process call
    for (size_t i = 0; i < maxi; i++) {
        vm->place = vm->callstack.back();
        vm->callstack.pop_back();
    }
    // locals and nlocals leave their namespace 
    vm->locals.pop_back();
    vm->nlocals.pop_back();
    vm->localc.pop_back();
    // make sure the stack is the same size as it was left before the call
    size_t ssize = vm->stack.size();
    for (size_t i = vm->stackd.back(); i < ssize; ssize--) {
        vm->stack.pop_back();
    }
    vm->stackd.pop_back();
    return objs[0];
}
// create function that has scope and takes args
Obj l_fn(Vm *vm, list &objs) {
    // only deals with bytecode functions
    floating_t fn = objs[0].get<floating_t>();
    // create a function that emulates a call
    return nobj<fnty>(vm, [fn](Vm *vm, list &objs) -> Obj {
        vm->localc.push_back(vm->callstack.size());
        vm->locals.push_back({});
        vm->nlocals.push_back({});
        vm->args = objs;
        vm->stackd.push_back(vm->stack.size());
        vm->stack.push_back(nobj(vm, fn));
        vm->callstack.push_back(vm->place);
        vm->place = fn;
        vm->noreturn = true;
        return nobj(vm, nullptr);
    });
}
// if is a function that deals with one condition and one function
// no else
Obj l_if(Vm *vm, list &objs) {
    if (c_bool(objs.back())) {
        // it will be time to call next time
        return nobj<fnty>(vm, [](Vm *vm, list &objs) -> Obj{
            Obj fn = objs[0];
            if (fn.iskind<fnty>()) {
                fnty cfn = fn.get<fnty>();
                list l;
                vm->stack.push_back(cfn(vm, l));
            }
            else {
                vm->callstack.push_back(vm->place);
                vm->place = fn.get<floating_t>();
            }
            return nobj(vm, nullptr);
        });
    }
    return nobj(vm, nullptr);
}
// displays the objects passed to cout
Obj l_log(Vm *vm, list &objs) {
    bool begin = true;
    std::cout << "log: ";
    // does for all objects
    for (Obj o: objs) {
        if (not begin) {
            // with a space between objects
            std::cout << " ";
        }
        c_print(o, std::cout);
        begin = false;
    }
    std::cout << std::endl;
    return nobj(vm, nullptr);
}

// returns arguments, because arguments are a list it makes a list
Obj l_list(Vm *vm, list &objs) {
    return nobj(vm, objs);
}

// the operator a->b makes a list from a up to but not including b
Obj l_range(Vm *vm, list &objs) {
    floating_t lhs, rhs;
    list ret;
    lhs = objs[0].get<floating_t>();
    rhs = objs[1].get<floating_t>();
    ret.reserve(rhs-lhs);
    for (floating_t d = lhs; d < rhs; d ++) {
        ret.push_back(nobj(vm, d));
    }
    return nobj(vm, ret);
}

// number and string addition, crash upon 0 or 1 args
Obj l_opadd(Vm *vm, list &objs) {
    if (objs[0].kind != objs[1].kind) {
        throw matcherr(objs);
    }
    if (objs[0].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() + objs[1].get<floating_t>());
    }
    if (objs[0].iskind<std::string>()) {
        return nobj(vm, objs[0].get<std::string>() + objs[1].get<std::string>());
    }
    throw typeerr();
}

// number multiplation, crash upon 0 or 1 args
Obj l_opmul(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() * objs[1].get<floating_t>());
    }
    throw typeerr();
}

// number subtraction, crash upon 0 or 1 args
Obj l_opsub(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() - objs[1].get<floating_t>());
    }
    throw typeerr();
}

// number devision, crash upon 0 or 1 args
Obj l_opdiv(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() / objs[1].get<floating_t>());
    }
    throw typeerr();
}

// number modulo, crash upon 0 or 1 args
Obj l_opmod(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, floating_t(fmod(objs[0].get<floating_t>(), objs[1].get<floating_t>())));
    }
    throw typeerr();
}

// number less than, crash upon 0 or 1 args
Obj l_oplt(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() < objs[1].get<floating_t>());
    }
    throw typeerr();
}

// number greater than, crash upon 0 or 1 args
Obj l_opgt(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() > objs[1].get<floating_t>());
    }
    throw typeerr();
}

// number less than or equal, crash upon 0 or 1 args
Obj l_oplte(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() <= objs[1].get<floating_t>());
    }
    throw typeerr();
}

// number greater than or equal, crash upon 0 or 1 args
Obj l_opgte(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() >= objs[1].get<floating_t>());
    }
    throw typeerr();
}

// any 2 values: if they are the same type and value are equal, crash upon 0 or 1 args
Obj l_opeq(Vm *vm, list &objs) {
    // uses c_equal, same as maps internally
    return nobj(vm, c_equal(objs[0], objs[1]));
}

// any 2 values: if they are the same type and value are not equal, crash upon 0 or 1 args
Obj l_opneq(Vm *vm, list &objs) {
    // uses c_equal, perfect opposite of l_opeq (==)
    return nobj(vm, not c_equal(objs[0], objs[1]));
}

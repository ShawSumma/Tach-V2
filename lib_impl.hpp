#pragma once
#include "lang.hpp"
Obj l_ret(Vm *vm, list &objs) {
    size_t callsize = vm->callstack.size();
    size_t localclast = vm->localc.back();
    size_t maxi = callsize-localclast;
    for (size_t i = 0; i < maxi; i++) {
        vm->place = vm->callstack.back();
        vm->callstack.pop_back();
    }
    vm->locals.pop_back();
    vm->nlocals.pop_back();
    vm->localc.pop_back();
    size_t ssize = vm->stack.size();
    for (size_t i = vm->stackd.back(); i < ssize; ssize--) {
        vm->stack.pop_back();
    }
    vm->stackd.pop_back();
    return objs[0];
}

Obj l_fn(Vm *vm, list &objs) {
    floating_t fn = objs[0].get<floating_t>();
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
Obj l_if(Vm *vm, list &objs) {
    if (c_bool(objs.back())) {
        return nobj<fnty>(vm, [](Vm *vm, list &objs) -> Obj{
            vm->store("else", nobj(vm, false));
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
    return nobj<fnty>(vm, [](Vm *vm, list &objs) -> Obj{
        vm->store("else", nobj(vm, true));
        return nobj(vm, nullptr);
    });
}
Obj l_log(Vm *vm, list &objs) {
    bool begin = true;
    std::cout << "log: ";
    for (Obj o: objs) {
        if (not begin) {
            std::cout << " ";
        }
        c_print(o, std::cout);
        begin = false;
    }
    std::cout << std::endl;
    return nobj(vm, nullptr);
}

Obj l_list(Vm *vm, list &objs) {
    return nobj(vm, objs);
}

Obj l_range(Vm *vm, list &objs) {
    double lhs, rhs;
    list ret;
    lhs = objs[0].get<double>();
    rhs = objs[1].get<double>();
    ret.reserve(rhs-lhs);
    for (double d = lhs; d < rhs; d ++) {
        ret.push_back(nobj(vm, d));
    }
    return nobj(vm, ret);
}

Obj l_opadd(Vm *vm, list &objs) {
    // if (objs.size() != 2) {
    //     throw argumenterr(2);
    // }
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

Obj l_opmul(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() * objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_opsub(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() - objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_opdiv(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() / objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_opmod(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, floating_t(fmod(objs[0].get<floating_t>(), objs[1].get<floating_t>())));
    }
    throw typeerr();
}

Obj l_oplt(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() < objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_opgt(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() > objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_oplte(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() <= objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_opgte(Vm *vm, list &objs) {
    if (objs[0].iskind<floating_t>() and objs[1].iskind<floating_t>()) {
        return nobj(vm, objs[0].get<floating_t>() >= objs[1].get<floating_t>());
    }
    throw typeerr();
}

Obj l_opeq(Vm *vm, list &objs) {
    return nobj(vm, c_equal(objs[0], objs[1]));
}

Obj l_opneq(Vm *vm, list &objs) {
    return nobj(vm, not c_equal(objs[0], objs[1]));
}

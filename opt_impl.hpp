#pragma once
#include "lang.hpp"

void opt_required(Vm *vm) {
    opt_names(vm);
}

void opt_floval(Vm *vm) {
    for (uint64_t pl = 0; pl < vm->instrs.size(); pl ++) {
        if (vm->instrs[pl].val.iskind<floating_t>()) {
            vm->instrs[pl].floval = vm->instrs[pl].val.get<floating_t>();
        }
        else {
            vm->instrs[pl].floval = 0;
        }
    }
}

void opt_loadargs(Vm *vm) {
    std::vector<Instr> instrs = vm->instrs;
    vm->instrs = {};
    int64_t offs = 0;
    for (uint64_t pl = 0; pl < instrs.size(); pl++) {
        Instr i = instrs[pl];
        if (i.type == Instr::JUMP) {
            size_t size = vm->instrs.size();
            vm->instrs.push_back(i);
            pl ++;
            while (instrs[pl].type == Instr::ARGS) {
                offs -= 4;
                Instr i;
                i.type = Instr::GARG;
                i.val = instrs[pl+3].val;
                vm->instrs.push_back(i);
                pl += 5;
            }
            vm->instrs[size].val = nobj<floating_t>(vm, vm->instrs[size].val.get<floating_t>()+offs);
        }
        vm->instrs.push_back(instrs[pl]);            
    }
}

void opt_names(Vm *vm) {
    size_t max = vm->instrs.size();
    for (uint64_t pl = 0; pl < max; pl++) {
        Instr i = vm->instrs[pl];
        if (i.type == Instr::LOAD || i.type == Instr::STORE || i.type == Instr::OPER
            || i.type == Instr::PROC) {
            std::string sv = i.val.get<std::string>();
            std::map<std::string, uint64_t>::iterator it = vm->conv.find(sv);
            if (it == vm->conv.end()) {
                vm->conv[sv] = vm->conv.size();
            }
        }
        if (i.type == Instr::LOAD) {
            if (i.val.get<std::string>() == "args") {
                vm->instrs[pl].type = Instr::ARGS;
            }
            // else if (optliteral && vm->locals.back().find(i.val.get<std::string>()) != vm->locals.back().end()) {
            //     vm->instrs[pl].type = Instr::OLOAD;
            //     vm->instrs[pl].val = vm->locals.back()[vm->instrs[pl].val.get<std::string>()];
            // }
            else {
                vm->instrs[pl].type = Instr::ILOAD;
                vm->instrs[pl].val = nobj(vm, floating_t(vm->conv[vm->instrs[pl].val.get<std::string>()]));
            }
        }
        if (i.type == Instr::STORE) {
            vm->instrs[pl].type = Instr::ISTORE;
            vm->instrs[pl].val = nobj(vm, floating_t(vm->conv[vm->instrs[pl].val.get<std::string>()]));
        }
        if (i.type == Instr::OPER) {
            vm->instrs[pl].type = Instr::IOPER;
            vm->instrs[pl].val = nobj(vm, floating_t(vm->conv[vm->instrs[pl].val.get<std::string>()]));
        }
        if (i.type == Instr::PROC) {
            vm->instrs[pl].val = nobj(vm, floating_t(vm->conv[vm->instrs[pl].val.get<std::string>()]));
        }
    }
    vm->nlocals.push_back({});
    for (std::pair<std::string, Obj> pr: vm->locals.back()) {
        std::map<std::string, uint64_t>::iterator it = vm->conv.find(pr.first);
        if (it == vm->conv.end()) {
            continue;
        }
        vm->nlocals.back()[it->second] = pr.second;
    }
}
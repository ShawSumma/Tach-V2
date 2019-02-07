#pragma once
#include "lang.hpp"

uint64_t time();
std::string argumenterr(size_t);
std::string matcherr(list);
std::string typeerr();
bool c_equal(Obj, Obj);
void c_print(Obj, std::ostream &);
bool c_bool(Obj);

std::map<std::string, Obj> glob(Vm *);
std::vector<Instr> readlns(Vm *, std::istream &); 


class Instr {
public:
    uint64_t count = 0;
    uint64_t time = 0;
    Obj val;
    enum {
        INDEX = 0,

        CALL = 1,
        ARGS = 2,
        RETN = 3,

        JUMP = 4,

        PUSH = 5,
        POP = 6,

        OPER = 7,
        LOAD = 8,
        STORE = 9,

        ILOAD = 10,
        ISTORE = 11,

        OLOAD = 12,
        OOPER = 13,

        DEFS = 14,
        PROC = 15,

        END = 16,

    } type;
    Instr(Vm *vm, std::string name, std::string val);
};

class Vm{
public:
    void opIndex();
    void opCall();
    void opArgs();
    void opRetn();
    void opJump();
    void opPush();
    void opPop();
    void opOper();
    void opLoad();
    void opStore();
    void opILoad();
    void opIStore();
    void opOLoad();
    void opOOper();
    void opDefs();
    void opProc();
    void call(Obj, list);
    using local_t = std::map<std::string, Obj>;
    using locals_t = std::vector<local_t>;
    locals_t locals;
    list args{0};
    using nlocal_t = std::map<uint64_t, Obj>;
    using nlocals_t = std::vector<nlocal_t>;
    nlocals_t nlocals;
    std::vector<size_t> localc;
    list stack{0};
    std::vector<size_t> stackd;
    std::vector<Instr> instrs;
    std::vector<size_t> callstack;
    size_t place;
    bool noreturn = false;
    Vm();
    Obj get(std::string val);
    Obj get(uint64_t val);
    void store(std::string k, Obj v);
    void store(uint64_t k, Obj v);
    void printstack();
    void vmrun(size_t pl);
    std::string ccompile();
};
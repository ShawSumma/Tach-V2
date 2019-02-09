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
    floating_t floval;
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
        IOPER = 12,


        DEFS = 13,
        PROC = 14,

        GARG = 15,

        DUP = 16,
        SWAP = 17,

    } type;
    using TypeEnum = decltype(type);
    Instr(Vm *vm, std::string name, std::string val);
    Instr();
};

class Vm{
public:
    void savestate(std::ostream &);
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
    void opIOper();
    void opDefs();
    void opProc();
    void opArg();
    void opDup();
    void opSwap();
    void call(Obj, list);
    using local_t = std::map<std::string, Obj>;
    using locals_t = std::vector<local_t>;
    locals_t locals;
    list args{0};
    uint16_t argp;
    using nlocal_t = std::map<uint64_t, Obj>;
    using nlocals_t = std::vector<nlocal_t>;
    nlocals_t nlocals;
    std::vector<size_t> localc;
    list stack{0};
    std::vector<size_t> stackd;
    std::vector<Instr> instrs;
    std::vector<size_t> callstack;
    std::map<std::string, uint64_t> conv;
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

Instr::TypeEnum instrTypeFrom(std::string);
std::string instrTypeTo(Instr::TypeEnum);
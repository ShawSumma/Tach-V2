#pragma once
#include "vm.hpp"

// config
bool timeit = false;
bool timeinstr = true;
bool optnames = true;
bool optliteral = true;

// get unix time in nanosecs
uint64_t time() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

// errors
// bad argument count
std::string argumenterr(size_t c) {
    return "function needs " + std::to_string(c) + " args";
}

// type mismatch
std::string matcherr(list objs) {
    return "types do not match";
}

// something else is wrong with types
std::string typeerr() {
    return "type error";
}

// numbrs can have _ as their first char
bool isnumber(std::string str) {
    for (char c: str) {
        if (not isdigit(c) and c != '_' and c != '.') {
            return false;
        }
    }
    return true;
}

bool c_equal(Obj a, Obj b) {
    // std::cout << a.kind << "\t" <<
    if (a.kind != b.kind) {
        return false;
    }
    if (a.iskind<bool>()) {
        return a.get<bool>() == b.get<bool>();
    }
    if (a.iskind<floating_t>()) {
        return a.get<floating_t>() == b.get<floating_t>();
    }
    if (a.iskind<std::string>()) {
        return a.get<std::string>() == b.get<std::string>();
    }
    if (a.iskind<list>()) {
        list lhl = a.get<list>();
        list rhl = b.get<list>();
        size_t lsize = lhl.size();
        size_t rsize = rhl.size();
        if (lsize != rsize) {
            return false;
        }
        for (size_t i = 0; i < lsize; i++) {
            if (not c_equal(lhl[i], rhl[i])) {
                return false;
            }
        }
        return true;
    }
    if (a.iskind<dict>()) {
        dict lhl = a.get<dict>();
        dict rhl = b.get<dict>();
        if (lhl.size() != rhl.size()) {
            return false;
        }
        size_t place = 0;
        for (uint64_t i = 0; i < lhl.size(); i++) {
            if (not c_equal(lhl.keys[i], rhl.keys[i])) {
                return false;
            }
            if (not c_equal(lhl.vals[i], rhl.vals[i])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool c_equal_types(Vm *vm, Obj a, Obj b) {
    if (a.iskind<dict>()) {
        dict ad = a.get<dict>();
        return c_equal(ad.get(nobj<std::string>(vm, "id")) , b);
    }
    return c_equal(a, b);
}

Instr::Instr(Vm *vm, std::string instr, std::string val) {
    if (instr == "PUSH  ") {
        if (isnumber(val)) {
            if (val[0] != '_') {
                floating_t v;
                std::stringstream ss;
                ss << val;
                ss >> v;
                this->val = nobj(vm, v);
            }
            else {
                floating_t v;
                std::stringstream ss;
                ss << val.substr(1);
                ss >> v;
                v *= -1;
                this->val = nobj(vm, v);
            }
        }
        else {
            this->val = nobj(vm, nullptr);
        }
        this->type = PUSH;
    }
    else if (instr == "STR   ") {
        this->val = nobj(vm, val);
        this->type = PUSH;
    }
    else if (instr == "OPER  ") {
        this->val = nobj(vm, val);
        this->type = OPER;
    }
    else if (instr == "LOAD  ") {
        if (val == "args") {
            this->type = ARGS;
        }
        else {
            this->val = nobj(vm, val);
            this->type = LOAD;
        }
    }
    else if (instr == "STORE ") {
        this->val = nobj(vm, val);
        this->type = STORE;
    }
    else if (instr == "PROC  ") {
        this->val = nobj(vm, val);
        this->type = PROC;
    }
    else if (instr == "DEFS  ") {
        floating_t v;
        std::stringstream ss;
        ss << val;
        ss >> v;
        this->val = nobj(vm, v);
        this->type = DEFS;
    }
    else if (instr == "INDEX ") {
        floating_t v;
        std::stringstream ss;
        ss << val;
        ss >> v;
        this->val = nobj(vm, v);
        this->type = INDEX;
    }
    else if (instr == "POP   ") {
        this->val = nobj(vm, nullptr);
        this->type = POP;
    }
    else if (instr == "DUP   ") {
        this->val = nobj(vm, nullptr);
        this->type = DUP;
    }
    else if (instr == "MODIF ") {
        this->val = nobj(vm, nullptr);
        this->type = MODIF;
    }
    else if (instr == "SWAP  ") {
        this->val = nobj(vm, nullptr);
        this->type = SWAP;
    }
    else if (instr == "RETN  ") {
        floating_t v;
        std::stringstream ss;
        ss << val;
        ss >> v;
        this->val = nobj(vm, v);
        this->type = RETN;
    }
    else if (instr == "JUMP  ") {
        floating_t v;
        std::stringstream ss;
        ss << val;
        ss >> v;
        this->val = nobj(vm, v);
        this->type = JUMP;
    }
    else if (instr == "CALL  ") {
        floating_t v;
        std::stringstream ss;
        ss << val;
        ss >> v;
        this->val = nobj(vm, v);
        this->type = CALL;
    }
    else {
        throw std::string("no operator ") + instr;
    }
}

void c_print(Obj o, std::ostream &out) {
    if (o.iskind<bool>()) {
        out << (o.get<bool>() ? "true" : "false");
        return;
    }
    if (o.iskind<floating_t>()) {
        out << o.get<floating_t>();
        return;
    }
    if (o.iskind<std::string>()) {
        out << o.get<std::string>();
        return;
    }
    if (o.iskind<fnty>()) {
        out << "<fn>";
        return;
    }
    if (o.iskind<list>()) {
        list v = o.get<list>();
        size_t size = v.size();
        out << "list(";
        for (size_t i = 0; i < size; i++) {
            if (i != 0) {
                out << " ";
            }
            c_print(v[i], out);
        }
        out << ")";
        return;
    }
    if (o.iskind<dict>()) {
        out << "dict(";
        bool beginning = true;
        dict d = o.get<dict>();
        for (size_t i = 0; i < d.size(); i++) {
            if (not beginning) {
                out << " ";
            }
            c_print(d.key(i), out);
            out << ": ";
            c_print(d.val(i), out);
            beginning = false;
        }
        out << ")";
        return;
    }
    if (o.iskind<std::nullptr_t>()) {
        out << "none";
        return;
    }
    out << "<obj " << o.kind << ">";
}
bool c_bool(Obj o) {
    if (o.iskind<bool>()) {
        return o.get<bool>();
    }
    if (o.iskind<floating_t>()) {
        return o.get<floating_t>() != 0;
    }
    if (o.iskind<std::string>()) {
        return o.get<std::string>() != "";
    }
    if (o.iskind<list>()) {
        return o.get<list>().size() != 0;
    }
    return false;
}

std::map<std::string, Obj> glob(Vm *vm) {
    std::map<std::string, Obj> ret = {
        {"+",     nobj<fnty>(vm, l_opadd)},
        {"-",     nobj<fnty>(vm, l_opsub)},
        {"*",     nobj<fnty>(vm, l_opmul)},
        {"/",     nobj<fnty>(vm, l_opdiv)},
        {"%",     nobj<fnty>(vm, l_opmod)},
        {"==",    nobj<fnty>(vm, l_opeq)},
        {"!=",    nobj<fnty>(vm, l_opneq)},
        {"<",     nobj<fnty>(vm, l_oplt)},
        {"<=",    nobj<fnty>(vm, l_oplte)},
        {">",     nobj<fnty>(vm, l_opgt)},
        {">=",    nobj<fnty>(vm, l_opgte)},
        {"->",    nobj<fnty>(vm, l_range)},
        {"**",     nobj<fnty>(vm, l_oppow)},
        {"&&",     nobj<fnty>(vm, l_opand)},
        {"||",     nobj<fnty>(vm, l_opor)},
        {"log",   nobj<fnty>(vm, l_log)},
        {"out",   nobj<fnty>(vm, l_out)},
        {"newline",   nobj<fnty>(vm, l_newline)},
        {"fn",    nobj<fnty>(vm, l_fn)},
        {"ret",   nobj<fnty>(vm, l_ret)},
        {"if",    nobj<fnty>(vm, l_if)},
        {"list",  nobj<fnty>(vm, l_list)},
        {"dict",  nobj<fnty>(vm, l_dict)},
        {"true",  nobj<bool>(vm, true)},
        {"false", nobj<bool>(vm, false)},
        {"typeid", nobj<fnty>(vm, l_typeid)},
        {"class", nobj<fnty>(vm, l_class)},
        {"none", nobj<floating_t>(vm, 0)},
        {"Number", nobj<floating_t>(vm, 1)},
        {"Boolean", nobj<floating_t>(vm, 2)},
        {"String", nobj<floating_t>(vm, 3)},
        {"List", nobj<floating_t>(vm, 4)},
        {"Proc", nobj<floating_t>(vm, 5)},
        {"Dict", nobj<floating_t>(vm, 6)},
    };
    return ret;
}

std::vector<Instr> readlns(Vm *vm, std::istream &f) {
    std::vector<Instr> ret;
    while (f.peek() != EOF) {
        std::string fr;
        getline(f, fr);
        if (fr != "") {
            vm->instrs.push_back(Instr(vm, fr.substr(0, 6), fr.substr(6)));
        }
    }
    return ret;
}
Vm::Vm() {
    locals = {glob(this)};
}
Obj Vm::get(std::string val) {
    size_t size = locals.size();
    while (size != 0) {
        size -= 1;
        local_t::iterator it = locals[size].find(val);
        if (it != locals[size].end()) {
            return it->second;
        }
    }
    throw std::string("no such local: ") + val;
}
void Vm::store(std::string k, Obj v) {
    locals.back()[k] = v;
}
Obj Vm::get(uint64_t val) {
    size_t size = nlocals.size();
    while (size != 0) {
        size -= 1;
        nlocal_t::iterator it = nlocals[size].find(val);
        if (it != nlocals[size].end()) {
            return it->second;
        }
    }
    for (std::pair<std::string, uint64_t> pr: conv) {
        if (pr.second == val) {
            throw std::string("no such local ") + pr.first;
        }
    }
    throw std::string("no such local with unknown name");
}
void Vm::store(uint64_t k, Obj v) {
    nlocals.back()[k] = v;
}
void Vm::printstack() {
    std::cout << "stack " << stack.size() << ": [";
    for (Obj sv: stack) {
        std::cout << " ";
        c_print(sv, std::cout);

    }
    std::cout << " ]" << std::endl;
}
void Vm::opIndex() {
    Instr i = instrs[place];
    Obj ind = stack.back();
    stack.pop_back();
    Obj lis = stack.back();
    if (lis.iskind<list>()) {
        stack.back() = lis.get<list>()[ind.get<floating_t>()];
    }
    else if (lis.iskind<dict>()) {
        dict ld = lis.get<dict>();
        stack.back() = ld.get(ind);
    }
    else {
        throw std::string("cannot index");
    }
}
void Vm::opModif() {
    Instr i = instrs[place];
    Obj v = stack.back();
    stack.pop_back();
    Obj k = stack.back();
    stack.pop_back();
    Obj iv = stack.back();
    stack.pop_back();
    if (iv.iskind<list>()) {
        list ls = iv.get<list>();
        ls[k.get<floating_t>()] = v;
        stack.push_back(nobj(this, ls));
    }
    else if (iv.iskind<dict>()) {
        dict d = iv.get<dict>();
        d.set(k, v);
        stack.push_back(nobj(this, d));
    }
    else {
        throw std::string("cannot index");
    }
}
void Vm::opJump() {
    Instr i = instrs[place];
    stack.push_back(nobj(this, floating_t(place)));
    place = i.val.get<floating_t>();
}
void Vm::opPush() {
    Instr i = instrs[place];
    stack.push_back(i.val);
}
void Vm::opDup() {
    Instr i = instrs[place];
    stack.push_back(stack.back());
}
void Vm::opSwap() {
    Instr i = instrs[place];
    std::iter_swap(stack.end()-1, stack.end()-2);
}
void Vm::opRetn() {
    Instr i = instrs[place];
    place = callstack.back();
    callstack.pop_back();
    if (localc.size() > 0 and callstack.size() == localc.back()) {
        locals.pop_back();
        localc.pop_back();
        Obj last = stack.back();
        size_t ssize = stack.size();
        for (size_t i = stackd.back(); i < ssize; ssize--) {
            stack.pop_back();
        }
        stackd.pop_back();
    }
}
void Vm::call(Obj fn, list args) {
    int16_t argc = args.size();
    if (fn.iskind<fnty>()) {
        fnty cfn = fn.get<fnty>();
        stack.back() = cfn(this, args);
    }
    else if (fn.iskind<dict>()) {
        dict fnd = fn.get<dict>();
        Obj fn;
        int16_t best = -1;
        for (size_t kv = 0; kv < fnd.size(); kv++) {
            list ls = fnd.key(kv).get<list>();
            if (ls.size() == argc) {
                int16_t bc = 0;
                bool okay = true;
                for (int16_t i = 0; i < argc; i++) {
                    Obj o = ls[i];
                    if (not o.iskind<std::nullptr_t>()) {
                        if (c_equal_types(this, o, args[i])) {
                            bc ++;
                        }
                        else {
                            okay = false;
                            break;
                        }
                    }
                }
                if (okay && bc >= best) {
                    best = bc;
                    fn = fnd.val(kv);;
                }
            }
        }
        if (fn.iskind<std::nullptr_t>()) {
            std::stringstream ss;
            c_print(nobj(this, args), ss);
            c_print(nobj(this, fnd), std::cout);
            std::cout << std::endl;
            throw std::string("cannot match function with args ") + ss.str();
        }
        call(fn, args);
    }
    else if (fn.iskind<floating_t>()) {
        stack.pop_back();
        callstack.push_back(place);
        place = fn.get<floating_t>();
        for (size_t i = 0; i < argc; i++) {
            stack.push_back(args[i]);
        }
    }
    else {
        throw std::string("uncallable");
    }
}
void Vm::opCall() {
    Instr i = instrs[place];
    size_t argc = i.val.get<floating_t>();
    list args(argc);
    for (int i = 0; i < argc; i++) {
        args[argc-1-i] = stack.back();
        stack.pop_back();
    }
    Obj fn = stack.back();
    call(fn, args);
}
void Vm::opOper() {
    Instr i = instrs[place];
    Obj rhs = stack.back();
    stack.pop_back();
    Obj lhs = stack.back();
    stack.pop_back();
    Obj fn = get(i.val.get<std::string>());
    if (not fn.iskind<fnty>()) {
        throw std::string("can only use funcs as opers not ") + std::to_string(fn.kind);
    }
    list args = {lhs, rhs};
    stack.push_back(fn.get<fnty>()(this, args));
}
void Vm::opStore() {
    Instr i = instrs[place];
    store(i.val.get<std::string>(), stack.back());
}
void Vm::opLoad() {
    Instr i = instrs[place];
    stack.push_back(get(i.val.get<std::string>()));
}
void Vm::opArgs() {
    Instr i = instrs[place];
    stack.push_back(nobj(this, args));
}
void Vm::opPop() {
    Instr i = instrs[place];
    stack.pop_back();
}
void Vm::opIStore() {
    Instr i = instrs[place];
    store(i.val.get<floating_t>(), stack.back());
}
void Vm::opILoad() {
    Instr i = instrs[place];
    stack.push_back(get(i.val.get<floating_t>()));
}
void Vm::opIOper() {
    Instr i = instrs[place];
    Obj rhs = stack.back();
    stack.pop_back();
    Obj lhs = stack.back();
    stack.pop_back();
    Obj fn = get(i.val.get<floating_t>());
    if (not fn.iskind<fnty>()) {
        throw std::string("can only use funcs as opers not ") + std::to_string(fn.kind);
    }
    list args = {lhs, rhs};
    stack.push_back(fn.get<fnty>()(this, args));
}
void Vm::opDefs() {
    Instr i = instrs[place];
    uint64_t maxp = i.val.get<floating_t>();
    list args(maxp);
    for (uint64_t p = 0; p < maxp; p++) {
        if (stack.back().iskind<dict>()) {
            dict d = stack.back().get<dict>();
            size_t it = d.find(nobj<std::string>(this, "id"));
            if (it == d.size()) {
                args[maxp-1-p] = stack.back();
            }
            else {
                args[maxp-1-p] = d.val(it);
            }
        }
        else {
            args[maxp-1-p] = stack.back();
        }
        stack.pop_back();
    }
    stack.push_back(nobj(this, args));
}
void Vm::opArg() {
    Instr i = instrs[place];
    store(i.val.get<floating_t>(), args[argp]);
    argp += 1;
}
void Vm::opProc() {
    Instr i = instrs[place];
    Obj args = stack.back();
    stack.pop_back();
    uint64_t s = i.val.get<floating_t>();
    nlocal_t::iterator it = nlocals.back().find(s);
    dict d;
    if (it != nlocals.back().end()) {
        d = it->second.get<dict>();
        d.set(args, stack.back());
        store(s, nobj(this, d));
    }
    else {
        d.insert(args, stack.back());
        nlocals.back()[s] = nobj(this, d);
    }
    stack.back() = nobj(this, d);
}

void Vm::vmrun(size_t pl) {
    std::vector<void(Vm::*)()> funcs = {
        &Vm::opIndex,
        &Vm::opCall,
        &Vm::opArgs,
        &Vm::opRetn,
        &Vm::opJump,
        &Vm::opPush,
        &Vm::opPop,
        &Vm::opOper,
        &Vm::opLoad,
        &Vm::opStore,
        &Vm::opILoad,
        &Vm::opIStore,
        &Vm::opIOper,
        &Vm::opDefs,
        &Vm::opProc,
        &Vm::opArg,
        &Vm::opDup,
        &Vm::opSwap,
        &Vm::opModif
    };
    
    size_t max = instrs.size();
    for (place = pl; place < max; place++) {
        if (noreturn) {
            stack.pop_back();
            noreturn = false;
        }
        Instr i = instrs[place];;
        uint64_t begint;
        if (timeit) {
            begint = time();
        }
        uint64_t placeb = place;
        std::invoke(funcs[i.type], this);
        if (timeit) {
            instrs[placeb].time += time()-begint;
            instrs[placeb].count += 1;
        }
    }
    if (timeit) {
        if (timeinstr) {
            std::map<uint64_t, std::pair<uint64_t, uint64_t>> bank;
            std::pair<uint64_t, uint64_t> tot;
            for (Instr i: instrs) {
                bank[i.type].first += i.time;
                bank[i.type].second += i.count;
                tot.first += i.time;
                tot.second += i.count;
            }
            std::cout << std::endl;
            for (std::pair<uint64_t, std::pair<uint64_t,uint64_t>> pr: bank) {
                std::string out = std::to_string(floating_t(pr.second.second)/tot.second);
                std::cout << pr.first << "\t" << out;
                for (uint64_t i = out.size(); i < 32; i++) {
                    std::cout << " ";
                }
                std::cout << uint16_t(1000*floating_t(pr.second.first)/tot.first) << std::endl;
            }
        }
        else {
            uint64_t pl = 0;
            for (Instr i: instrs) {
                uint64_t iv = floating_t(i.time)/1000000;
                if (iv != 0) {
                    std::cout << pl << "\t" << iv << std::endl;
                }
                pl ++;
            }
        }
    }
}

Instr::Instr() {}

void Vm::savestate(std::ostream &os) {
    for (std::pair<std::string, uint64_t> pr: conv) {
        os << ".set " << pr.second;
        for (uint16_t i = std::to_string(pr.second).size(); i < 7; i++) {
            os << " ";
        }
        os << " ";
        os << pr.first;
        os << std::endl;
    }
    for (Instr i: instrs) {
        std::string name = instrTypeTo(i.type);
        os << instrTypeTo(i.type);
        int16_t spaces = 12-name.size();
        for (int16_t i = spaces; i >= 0; i--) {
            os << " ";
        }
        c_print(i.val, os);
        os << std::endl;
    }
}

Instr::TypeEnum instrTypeFrom(std::string str) {
    std::map<std::string, Instr::TypeEnum> ret = {
        {"INDEX", Instr::INDEX},
        {"CALL", Instr::CALL},
        {"ARGS", Instr::ARGS},
        {"RETN", Instr::RETN},
        {"JUMP", Instr::JUMP},
        {"PUSH", Instr::PUSH},
        {"POP", Instr::POP},
        {"OPER", Instr::OPER},
        {"LOAD", Instr::LOAD},
        {"STORE", Instr::STORE},
        {"ILOAD", Instr::ILOAD},
        {"ISTORE", Instr::ISTORE},
        {"IOPER", Instr::IOPER},
        {"DEFS", Instr::DEFS},
        {"PROC", Instr::PROC},
        {"GARG", Instr::GARG},
        {"DUP", Instr::DUP},
        {"SWAP", Instr::SWAP},
        {"MODIF", Instr::MODIF},
    };
    return ret[str];
}
std::string instrTypeTo(Instr::TypeEnum en) {
    std::map<Instr::TypeEnum, std::string> ret = {
        {Instr::INDEX, "INDEX"},
        {Instr::CALL, "CALL"},
        {Instr::ARGS, "ARGS"},
        {Instr::RETN, "RETN"},
        {Instr::JUMP, "JUMP"},
        {Instr::PUSH, "PUSH"},
        {Instr::POP, "POP"},
        {Instr::OPER, "OPER"},
        {Instr::LOAD, "LOAD"},
        {Instr::STORE, "STORE"},
        {Instr::ILOAD, "ILOAD"},
        {Instr::ISTORE, "ISTORE"},
        {Instr::IOPER, "IOPER"},
        {Instr::DEFS, "DEFS"},
        {Instr::PROC, "PROC"},
        {Instr::GARG, "GARG"},
        {Instr::DUP, "DUP"},
        {Instr::SWAP, "SWAP"},
        {Instr::MODIF, "MODIF"},
    };
    return ret[en];
}
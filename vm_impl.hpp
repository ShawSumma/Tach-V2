#pragma once
#include "vm.hpp"

bool timeit = false;
bool timeinstr = false;
bool optnames = true;
bool optliteral = true;

uint64_t time() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

std::string argumenterr(size_t c) {
    return "function needs " + std::to_string(c) + " args";
}

std::string matcherr(list objs) {
    return "types do not match";
}

std::string typeerr() {
    return "type error";
}


bool isnumber(std::string str) {
    for (char c: str) {
        if (not isdigit(c) and c != '_') {
            return false;
        }
    }
    return true;
}

bool c_equal(Obj a, Obj b) {
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
            return lsize < rsize;
        }
        for (size_t i = 0; i < lsize; i++) {
            if (not c_equal(lhl[i], rhl[i])) {
                return true;
            }
        }
    }
    return false;
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
        out << "<dict>";
        return;
    }
    if (o.iskind<std::nullptr_t>()) {
        out << "None";
        return;
    }
    out << "<obj " << o.kind << ">";
}
bool c_bool(Obj o) {
    if (o.iskind<bool>()) {
        return o.get<bool>();
    }
    if (o.iskind<floating_t>()) {
        return o.get<floating_t>() == 0;
    }
    if (o.iskind<std::string>()) {
        return o.get<std::string>() == "";
    }
    if (o.iskind<list>()) {
        return o.get<list>().size() == 0;
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
        {"log",   nobj<fnty>(vm, l_log)},
        {"fn",    nobj<fnty>(vm, l_fn)},
        {"ret",   nobj<fnty>(vm, l_ret)},
        {"if",    nobj<fnty>(vm, l_if)},
        {"list",  nobj<fnty>(vm, l_list)},
        {"true",  nobj<bool>(vm, true)},
        {"false", nobj<bool>(vm, false)},
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
    throw std::string("no such local ") + std::to_string(val);
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
    if (lis.iskind<list>() && ind.iskind<floating_t>()) {
        stack.back() = lis.get<list>()[ind.get<floating_t>()];
    }
    else if (lis.iskind<dict>()) {
        stack.back() = lis.get<dict>()[ind];
    }
    else {
        throw std::string("cannot index ");
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
void Vm::opPop() {
    Instr i = instrs[place];
    stack.pop_back();
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
        for (std::pair<Obj, Obj> pr: fnd) {
            list ls = pr.first.get<list>();
            if (ls.size() == argc) {
                int16_t bc = 0;
                bool okay = true;
                for (int16_t i = 0; i < argc; i++) {
                    Obj o = ls[i];
                    if (not o.iskind<std::nullptr_t>()) {
                        if (c_equal(o, args[i]) && c_equal(args[i], o)) {
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
                    fn = pr.second;;
                }
            }
        }
        call(fn, args);
    }
    else {
        stack.pop_back();
        callstack.push_back(place);
        place = fn.get<floating_t>();
        for (size_t i = 0; i < argc; i++) {
            stack.push_back(args[i]);
        }
    }
}
void Vm::opCall() {
    Instr i = instrs[place];
    size_t argc = i.val.get<floating_t>();
    list args(argc);
    for (int i = 0; i < argc; i++) {
        args[i] = stack.back();
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
void Vm::opIStore() {
    Instr i = instrs[place];
    store(i.val.get<floating_t>(), stack.back());
}
void Vm::opILoad() {
    Instr i = instrs[place];
    stack.push_back(get(i.val.get<floating_t>()));
}
void Vm::opOLoad() {
    Instr i = instrs[place];
    stack.push_back(i.val);
}
void Vm::opOOper() {
    Instr i = instrs[place];
    Obj rhs = stack.back();
    stack.pop_back();
    Obj lhs = stack.back();
    stack.pop_back();
    Obj fn = i.val;
    if (not fn.iskind<fnty>()) {
        throw std::string("can only use funcs as opers not ") + std::to_string(fn.kind);
    }
    list args = {lhs, rhs};
    stack.push_back(fn.get<fnty>()(this, args));
}
void Vm::opDefs() {
    Instr i = instrs[place];
    list args;
    uint64_t maxp = i.val.get<floating_t>();
    for (uint64_t p = 0; p < maxp; p++) {
        args.push_back(stack.back());
        stack.pop_back();
    }
    stack.push_back(nobj(this, args));
}
void Vm::opProc() {
    Instr i = instrs[place];
    Obj args = stack.back();
    stack.pop_back();
    if (optnames) {
        uint64_t s = i.val.get<floating_t>();
        nlocal_t::iterator it = nlocals.back().find(s);
        if (it == nlocals.back().end()) {
            dict d;
            d.insert({args, stack.back()});
            nlocals.back()[s] = nobj(this, d);
            stack.push_back(nobj(this, d));
        }
        else {
            dict d = it->second.get<dict>();
            d[args] = stack.back();
            store(s, nobj(this, d));
            stack.push_back(nobj(this, d));
        }
    }
    else {
        std::string s = i.val.get<std::string>();
        local_t::iterator it = locals.back().find(s);
        if (it == locals.back().end()) {
            dict d;
            d.insert({args, stack.back()});
            locals.back()[s] = nobj(this, d);
            stack.push_back(nobj(this, d));
        }
        else {
            dict d = it->second.get<dict>();
            d[args] = stack.back();
            store(s, nobj(this, d));
            stack.push_back(nobj(this, d));
        }
    }
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
        &Vm::opOLoad,
        &Vm::opOOper,
        &Vm::opDefs,
        &Vm::opProc
    };
    size_t max = instrs.size();
    if (optnames) {
        std::map<std::string, uint64_t> conv;
        for (uint64_t pl = 0; pl < max; pl++) {
            Instr i = instrs[pl];
            if (i.type == Instr::LOAD || i.type == Instr::STORE || i.type == Instr::OPER
                || i.type == Instr::PROC) {
                std::string sv = i.val.get<std::string>();
                std::map<std::string, uint64_t>::iterator it = conv.find(sv);
                if (it == conv.end()) {
                    conv[sv] = conv.size();
                }
            }
            if (i.type == Instr::LOAD) {
                if (i.val.get<std::string>() == "args") {
                    instrs[pl].type = Instr::ARGS;
                }
                else if (optliteral && locals.back().find(i.val.get<std::string>()) != locals.back().end()) {
                    instrs[pl].type = Instr::OLOAD;
                    instrs[pl].val = nobj(this, locals.back()[instrs[pl].val.get<std::string>()]);
                }
                else {
                    instrs[pl].type = Instr::ILOAD;
                    instrs[pl].val = nobj(this, floating_t(conv[instrs[pl].val.get<std::string>()]));
                }
            }
            if (i.type == Instr::STORE) {
                instrs[pl].type = Instr::ISTORE;
                instrs[pl].val = nobj(this, floating_t(conv[instrs[pl].val.get<std::string>()]));
            }
            if (i.type == Instr::OPER) {
                instrs[pl].type = Instr::OOPER;
                instrs[pl].val = nobj(this, locals.back()[instrs[pl].val.get<std::string>()]);
            }
            if (i.type == Instr::PROC) {
                instrs[pl].val = nobj(this, floating_t(conv[instrs[pl].val.get<std::string>()]));
            }
        }
        nlocals.push_back({});
        for (std::pair<std::string, Obj> pr: locals.back()) {
            std::map<std::string, uint64_t>::iterator it = conv.find(pr.first);
            if (it == conv.end()) {
                continue;
            }
            nlocals.back()[it->second] = pr.second;
        }
    }
    
    for (place = pl; place < max; place++) {
        if (noreturn) {
            stack.pop_back();
            noreturn = false;
        }
        Instr i = instrs[place];
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
                uint64_t iv = double(i.time)/1000000;
                if (iv != 0) {
                    std::cout << pl << "\t" << iv << std::endl;
                }
                pl ++;
            }
        }
    }
}

import sys
import time
import os
from opkinds import kind as opkind
import copy
try:
    import __pypy__
    pypy = True
except ImportError:
    pypy = False

numerics = "0123456789._"
def isnumeric(strv):
    return all(i in numerics for i in strv)

class NoReturn(object):
    pass


class Code_point(object):
    def __init__(self, ln, col):
        self.ln = ln
        self.col = col
    def __lt__(self, other):
        if self.ln < other.ln:
            return True
        if self.ln > other.ln:
            return False
        if self.col < other.col:
            return True
        if self.col > other.col:
            return False
        return False
    def __gt__(self, other):
        return not self.__lt__(other)
    def __repr__(self):
        return str(self)
    def __str__(self):
        return "Point(ln={},col={})".format(self.ln, self.col)

class Token(object):
    class Kind(object):
        INTEGER = 1
        FLOATING = 2
        NAME = 3
        KEYWORD = 4
        OPERATOR = 5
        BRACE = 6
        STRING = 7
    def __init__(self, kind, tok, begin, end=None):
        self.kind = kind
        self.tok = tok
        self.begin = begin
        if end == None:
            end = begin
        self.end = end
    def extend(self, sub_tok, end):
        self.tok += sub_tok 
        self.end = end
    def __repr__(self):
        return str(self)
    def __str__(self):
        return self.tok
    def conv(self, vm):
        vm.lines[len(vm.ops)] = self.begin
        if self.kind == Token.Kind.INTEGER:
            val = float(self.tok) if self.tok[0] != '_' else -float(self.tok[1:])
            # val = int(val) if val % 1 == 0 else float(val)
            vm.append(Opcode(opkind["PUSH"], val))
        elif self.kind == Token.Kind.FLOATING:
            vm.append(Opcode(opkind["PUSH"], float(self.tok)))
        elif self.kind == Token.Kind.STRING:
            vm.append(Opcode(opkind["STR"], self.tok))
        elif self.kind == Token.Kind.NAME:
            vm.append(Opcode(opkind["LOAD"], self.tok))
    def export(self):
        return '(token {})'.format(self.tok)
class Tokens(object):
    def __init__(self, tokens=[]):
        self.tokens = []
        self.begin = Code_point(0,0)
        self.end = Code_point(0,0)
    def extend_last(self, sub_tok, end):
        self.tokens[-1].extend(sub_tok, end)
    def append(self, token):
        if self.begin == None or token.begin < self.begin:
            self.begin = token.begin
        if self.end == None or token.end > self.end:
            self.end = token.end
        self.tokens.append(token)
    def sub(self, begin, end):
        ret = Tokens()
        for i in range(begin, end):
            ret.append(self.tokens[i])
        return ret
    def __len__(self):
        return len(self.tokens)
    def __getitem__(self, ind):
        return self.tokens[ind]
    def __repr__(self):
        return str(self)
    def __str__(self):
        return "[{}]".format([', '.join(str(i) for i in self.tokens)])
    def conv(self, vm, base=True, poplast=True):
        first = True
        for i in self.tokens:
            if base and not first:
                vm.append(Opcode(opkind["POP"]))
            i.conv(vm)
            first = False
        if poplast:
            vm.append(Opcode(opkind["POP"]))
    def export(self):
        return "(tokens " + " ".join(i.export() for i in self.tokens) + ")"

operators_ordered = [
    ['.', ':'],
    ['**'],
    ['*', '/', '%'],
    ['+', '-'],
    ['->'],
    ['==', '!=', '<=', '>=', '<', '>'],
    ['&&', '||'],
    ['='],
]

escape_seqs = {
    'n': '\n',
    't': '\t',
    's': ' ',
    '\\': '\\',
    '\'': '\'',
    '\"': '\"',
}

word_chars = [
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '_', '$'
]

operators_flat = []
for i in operators_ordered:
    operators_flat += i

def starts_operator(testing):
    for oper in operators_flat:
        if oper[:len(testing)] == testing:
            return True
    return False

def read_tokens(code):
    class ret:
        def __init__(self):
            self.col = 1
            self.ln = 1
            self.place = 0
            self.maxplace = len(code)
            self.last_char = ''
            self.return_tokens = Tokens()
    ret = ret()
    def read():
        if ret.place < ret.maxplace:
            ret.last_char = code[ret.place]
            if ret.last_char == '\n':
                ret.ln += 1
                ret.col = 1
            else:
                ret.col += 1
            ret.place += 1
        else:
            ret.last_char = ''
        return ret.last_char

    def open_token(kind):
        ret.return_tokens.append(Token(kind, "", Code_point(ret.ln, ret.col-1)))
    def extend_last_token(sub_tok):
        ret.return_tokens.extend_last(sub_tok, Code_point(ret.ln, ret.col))
    read()
    while True:
        if ret.last_char == '':
            break
        elif ret.last_char == '.':
            open_token(Token.Kind.OPERATOR)
            extend_last_token(ret.last_char)
            read()
        elif isnumeric(ret.last_char):
            open_token(Token.Kind.INTEGER)
            while isnumeric(ret.last_char) and ret.last_char != '':
                extend_last_token(ret.last_char)
                read()
            if ret.return_tokens[-1].tok[-1] == '.':
                ret.return_tokens[-1].tok = ret.return_tokens[-1].tok[:-1]
                open_token(Token.Kind.OPERATOR)
                extend_last_token('.')
        elif ret.last_char in ('\t', '\n', ' ', '\r'):
            read()
        elif starts_operator(ret.last_char):
            open_token(Token.Kind.OPERATOR)
            beginning_char = ret.last_char
            extend_last_token(ret.last_char)
            read()
            while starts_operator(beginning_char + ret.last_char):
                extend_last_token(ret.last_char)
                read()                
        elif ret.last_char == '`':
            open_token(Token.Kind.NAME)   
            read()
            while ret.last_char != '`':
                extend_last_token(ret.last_char)
                read()
            read()
        elif ret.last_char in ('\'', '\"'):
            str_type = ret.last_char
            read()
            open_token(Token.Kind.STRING)
            while (ret.last_char != '"' and str_type == '"') or (ret.last_char in word_chars + list(numerics) and str_type == '\''):
                if ret.last_char == '\\':
                    read()
                    extend_last_token(escape_seqs[ret.last_char])
                else:
                    extend_last_token(ret.last_char)
                read()
            if str_type == '"':
                read()
        elif ret.last_char in word_chars:
            open_token(Token.Kind.NAME)
            while ret.last_char in word_chars or isnumeric(word_chars):
                extend_last_token(ret.last_char)
                read()
        elif ret.last_char in ('(', ')', '{', '}', '[', ']'):
            open_token(Token.Kind.BRACE)
            extend_last_token(ret.last_char)
            read()
        else:
            raise SyntaxError("cannot understand token at {}:{}".format(ret.ln, ret.col))
    return ret.return_tokens

def code_ref(code, helper):
    code_lines = code.split('\n')
    colb = helper.begin.col-1
    cole = helper.end.col-1
    linno = helper.begin.ln-1
    return code_lines[linno][colb:cole]

matching = {
    '(': ')',
    '{': '}',
    '[': ']',
}

class Opcode(object):
    def __init__(self, kind, value=None, place=None):
        self.kind = kind
        self.value = value
        self.target = False
        self.place = place
    def rep(self):
        if self.target:
            return "{}:\t{} {}".format(self.place, self.kind, self.value)
        else:
            return "\t{} {}".format(str(self.kind), self.value)
    def __str__(self):
        return self.rep()
    def __repr__(self):
        return self.rep()
class Operator(object):
    def __init__(self, kind, lhs, rhs):
        self.kind = kind
        self.lhs = lhs
        self.rhs = rhs
        self.begin = lhs.begin if lhs != None else kind.begin
        self.end = rhs.end
    def __str__(self):
        return "(op {} {} {})".format(self.kind, self.lhs, self.rhs)
    def __repr__(self):
        return "(op {} {} {})".format(self.kind, self.lhs, self.rhs)
    def conv(self, vm):
        if self.kind.tok == ':':
            self.rhs.fn.conv(vm)
            self.lhs.conv(vm)
            vm.append(Opcode(opkind["DUP"]))
            vm.append(Opcode(opkind["LOAD"], "typeid"))
            vm.append(Opcode(opkind["SWAP"]))
            if isinstance(self.rhs, Call):
                vm.append(Opcode(opkind["CALL"], 1))
                vm.append(Opcode(opkind["SWAP"]))
                # self.lhs.conv(vm)
                self.rhs.args.conv(vm)
                vm.append(Opcode(opkind["CALL"], len(self.rhs.args.val.tokens)+2))
            else:
                raise Exception("colon operator must be called after use")
        elif self.kind.tok == '.':
            self.lhs.conv(vm)
            vm.append(Opcode(opkind["STR"], self.rhs.tok))
            vm.append(Opcode(opkind["INDEX"]))
        elif self.kind.tok == '=':
            if isinstance(self.lhs, (Call, Braced, Operator)):
                islambda = isinstance(self.lhs, Braced)
                isoperator =  isinstance(self.lhs, Operator)
                ismethod = isoperator and self.lhs.kind.tok == ':'
                isdotop = isoperator and self.lhs.kind.tok == '.'
                # ismethod = ismethod and isinstance(self.lhs.rhs, Call)
                # ismethod = ismethod and self.lhs.fn.kind == '.'
                if ismethod:
                    args = self.lhs.rhs.args
                elif isdotop:
                    name = self.lhs.lhs
                    rhs = self.rhs
                    ind = self.lhs.rhs
                    vm.append(Opcode(opkind["LOAD"], name.tok))
                    vm.append(Opcode(opkind["STR"], ind.tok))
                    rhs.conv(vm)
                    vm.append(Opcode(opkind["MODIF"]))
                    vm.append(Opcode(opkind["STORE"], name))
                else:
                    args = self.lhs if islambda else self.lhs.args
                if isdotop:
                    pass
                elif self.lhs.kind == '[]':
                    name = self.lhs.fn
                    rhs = self.rhs
                    ind = self.lhs.args.val[0]
                    vm.append(Opcode(opkind["LOAD"], name.tok))
                    ind.conv(vm)
                    rhs.conv(vm)
                    vm.append(Opcode(opkind["MODIF"]))
                    vm.append(Opcode(opkind["STORE"], name))
                elif self.lhs.kind == '()' or ismethod:
                    vm.append(Opcode(opkind["LOAD"], "fn"))
                    jmpind = len(vm.ops)
                    vm.append(Opcode(opkind["JUMP"]))
                    if ismethod:
                        vm.append(Opcode(opkind["LOAD"], "args"))
                        vm.append(Opcode(opkind["PUSH"], 0))
                        vm.append(Opcode(opkind["INDEX"]))
                        vm.append(Opcode(opkind["STORE"], "self"))
                        vm.append(Opcode(opkind["POP"]))  
                        vm.append(Opcode(opkind["LOAD"], "args"))
                        vm.append(Opcode(opkind["PUSH"], 1))
                        vm.append(Opcode(opkind["INDEX"]))
                        vm.append(Opcode(opkind["STORE"], "this"))
                        vm.append(Opcode(opkind["POP"]))                        
                    for pl, i in enumerate(args.val):
                        vm.append(Opcode(opkind["LOAD"], "args"))
                        vm.append(Opcode(opkind["PUSH"], pl + ismethod*2))
                        vm.append(Opcode(opkind["INDEX"]))
                        vm.append(Opcode(opkind["STORE"], i.tok))
                        vm.append(Opcode(opkind["POP"]))
                    vm.append(Opcode(opkind["LOAD"], "ret"))
                    self.rhs.conv(vm)
                    # vm.marknex = True
                    vm.append(Opcode(opkind["CALL"], 1))
                    # vm.ops[jmpind+1].target = True
                    vm.ops[jmpind].value = len(vm.ops)-1
                    # vm.marknex = True
                    vm.append(Opcode(opkind["CALL"], 1))
                    # vm.ops[-1].target = True
                    if ismethod:
                        vm.append(Opcode(opkind["LOAD"], self.lhs.lhs.tok))
                        vm.append(Opcode(opkind["PUSH"], None))
                    for pl, i in enumerate(args.val):
                        if i.kind == Token.Kind.NAME and i.tok not in ["true", "false"]:
                            vm.append(Opcode(opkind["PUSH"], None))
                        else:
                            i.conv(vm)
                    if ismethod:
                        vm.append(Opcode(opkind["DEFS"], len(args.val)+2))
                        vm.append(Opcode(opkind["PROC"], self.lhs.rhs.fn.tok))
                    elif not islambda:
                        vm.append(Opcode(opkind["DEFS"], len(args.val)))
                        vm.append(Opcode(opkind["PROC"], self.lhs.fn.tok))
            else:
                self.rhs.conv(vm)
                vm.append(Opcode(opkind["STORE"], self.lhs.tok))
        else:
            if self.lhs is not None:
                self.lhs.conv(vm)
            self.rhs.conv(vm)
            vm.append(Opcode(opkind["OPER"], self.kind.tok))
    def export(self):
        return "(op {} {} {})".format(self.kind.export(), self.lhs.export(), self.rhs.export())

class Braced(object):
    def __init__(self, kind, val):
        self.kind = kind
        self.val = val
        self.begin = val.begin
        self.end = val.end
        self.name = {
            '{}': 'curly',
            '[]': 'square',
            '()': 'paren'
        }[kind]
    def __str(self):
        return "{} {} {}".format(self.kind[0], self.val, self.kind[1])
    def __repr__(self):
        return "{} {} {}".format(self.kind[0], self.val, self.kind[1])
    def conv(self, vm, fromcall=False):
        if self.kind == '()':
            self.val.conv(vm, base=False, poplast=False)
        elif self.kind == '{}':
            jmpind = len(vm.ops)
            vm.append(Opcode(opkind["JUMP"]))
            self.val.conv(vm, base=True, poplast=True)
            # vm.append(Opcode(opkind["PUSH"], None))
            vm.append(Opcode(opkind["PUSH"], None))
            vm.append(Opcode(opkind["RETN"]))
            vm.ops[jmpind].value = len(vm.ops)-1
            # vm.ops[jmpind+1].target = True
            vm.marknex = True
            if not fromcall:
                vm.append(Opcode(opkind["CALL"], 0))
        else:
            if not fromcall:
                self.val.conv(vm, base=False, poplast=False)
            else:
                self.val.conv(vm, base=False, poplast=False)
    def export(self):
        return "(braced " + self.name + " " + self.val.export() + ")"

class Call(object):
    def __init__(self, fn, args):
        self.fn = fn
        self.args = args
        self.begin = fn.begin
        self.end = args.end
        self.kind = self.args.kind
    def __str__(self):
        return "(call {self.fn} {self.args})"
    def __repr__(self):
        return "(call {self.fn} {self.args})"
    def conv(self, vm):
        if self.kind == '()':
            self.fn.conv(vm)
            self.args.conv(vm)
            vm.append(Opcode(opkind["CALL"], len(self.args.val.tokens)))
            vm.marknex = True
        elif self.kind == '[]':
            self.fn.conv(vm)
            self.args.conv(vm, fromcall=True)
            vm.append(Opcode(opkind["INDEX"]))
        elif self.kind == '{}':
            self.fn.conv(vm)
            self.args.conv(vm, fromcall=True)
            # vm.append(Opcode(opkind["POP"]))
            vm.append(Opcode(opkind["CALL"], 1))
            vm.marknex = True
    def export(self):
        return "(call {} {})".format(self.fn.export(), self.args.export())

def indexfn(lis, ind):
    if isinstance(ind, (int, float)):
        return lis[int(ind)]
    if isinstance(ind, str):
        return lis[ind]
    return [indexfn(lis, i) for i in ind]

operfns = {
    '+': lambda x, y: x + y,
    '*': lambda x, y: x * y,
    '/': lambda x, y: x / y,
    '-': lambda x, y: x - y,
    '%': lambda x, y: x % y,
    '**': lambda x, y: x ** y,
    '==': lambda x, y: x == y,
    '<=': lambda x, y: x <= y,
    '>=': lambda x, y: x >= y,
    '!=': lambda x, y: x != y,
    '<': lambda x, y: x < y,
    '>': lambda x, y: x > y,
    '||': lambda x, y: x or y,
    '&&': lambda x, y: x and y,
    '->': lambda x, y: list(range(x, y)),
    ':': indexfn,
}

def strfn(arg):
    if isinstance(arg, dict):
        return "dict(%s)" % ' '.join(strfn(i) + " " + strfn(j) for i, j in zip(arg.keys(), arg.values()))
    if isinstance(arg, (list, tuple)):
        return '['+' '.join(strfn(i) for i in arg)+']'
    if callable(arg):
        # if hasattr(arg, '__name__'):
        #     return "<fn {}>".format(arg.__name__)
        return "<fn>"
    return str(arg)

instrv = {}
class Vm(object):
    def op_push(self, value):
        self.stack.append(value)
    def op_pop(self, value):
        self.stack.pop()
    def op_oper(self, value):
        self.stack[-2] = operfns[value](self.stack[-2], self.stack[-1])
        self.stack.pop()
    def op_load(self, value):
        for i in self.locals[::-1]:
            if value in i:
                self.stack.append(i[value])
                break
        else:
            raise NameError("no such name %s" % value)
    def op_store(self, value):
        self.locals[-1][value] = copy.deepcopy(self.stack[-1])
    def op_jump(self, value):
        self.stack.append(self.place)
        self.jmp(value)
    def op_dup(self, value):
        self.stack.append(self.stack[-1])
    def op_modif(self, value):
        v = self.stack[-1]
        self.stack.pop()
        k = self.stack[-1]
        self.stack.pop()
        cont = self.stack[-1]
        self.stack.pop()
        cont[k] = v
        self.stack.append(cont)
    def op_swap(self, value):
        self.stack[-1], self.stack[-2] = self.stack[-2], self.stack[-1]
    def op_call(self, value):
        if isinstance(value, (tuple, list)):
            args = value
        elif value == 0:
            args = ()
        else:
            args = self.stack[-value:]
            self.stack = self.stack[:-value]
        if callable(self.stack[-1]):
            fn = self.stack[-1]
            self.stack.pop()
            result = fn(*args)
            if not isinstance(result, NoReturn):
                self.stack.append(result)
            else:
                return True
        elif isinstance(self.stack[-1], list):
            cans = self.stack[-1]
            fn = None
            best = -1
            for i in cans:
                curargs, curfn = i
                bc = 0
                for pl, (a, b) in enumerate(zip(curargs, args)):
                    if pl == 0 and isinstance(a, dict):
                        continue
                    if not a is None and a != b:
                        break
                    if a == b:
                        bc += 1
                else:
                    if len(curargs) == len(args) and bc >= best:
                        best = bc
                        fn = curfn
            if fn is None:
                # print(cans)
                # print(args)
                raise NameError("cannot call with args " + str(args))
            else:
                self.stack[-1] = fn
                self.op_call(args)
        else:
            self.callstack.append(self.place)
            fn = self.stack[-1]
            self.stack.pop()
            self.stack += args
            self.jmp(fn)
        return False
    def op_retn(self, value):
        self.jmp(self.callstack[-1])
        self.callstack.pop()
        if len(self.callstack) == self.localc[-1]:
            self.locals.pop()
            self.localc.pop()
            self.stack = self.stack[:self.stackd[-1]] + [self.stack[-1]]
            self.stackd.pop()
    def op_index(self, value):
        lis = self.stack[-2]
        ind = self.stack[-1]
        self.stack.pop()
        self.stack.pop()
        self.stack.append(indexfn(lis, ind))        
    def op_list(self, value):
        midind = len(self.stack)-value
        self.stack = self.stack[:midind] + [self.stack[midind:]]
    def op_proc(self, value):
        argv = self.stack[-1]
        self.stack.pop()
        if value in self.locals[-1]:                
            self.locals[-1][value].append((argv, self.stack[-1]))
        else:
            self.locals[-1][value] = [(argv, self.stack[-1])]
        self.stack[-1] = self.locals[-1][value]
    def op_defs(self, value):
        args = []
        for i in range(value):
            args.append(self.stack[-1])
            self.stack.pop()
        self.stack.append(tuple(args[::-1]))

    opfns = {
        opkind["PUSH"]: op_push,
        opkind["POP"]: op_pop,
        opkind["OPER"]: op_oper,
        opkind["LOAD"]: op_load,
        opkind["CALL"]: op_call,
        opkind["JUMP"]: op_jump,
        opkind["RETN"]: op_retn,
        opkind["STORE"]: op_store,
        opkind["INDEX"]: op_index,
        opkind["STR"]: op_push,
        opkind["PROC"]: op_proc,
        opkind["DEFS"]: op_defs,
        opkind["DUP"]: op_dup,
        opkind["SWAP"]: op_swap,
        opkind["MODIF"]: op_modif,
    }
    def glob(self):
        def loadfn(place, *args):
            self.stack.append(place)
            self.place = place[0]
            self.stack = self.stack[:place[1]+1]
            self.callstack = self.callstack[:place[2]]
            self.localc = self.localc[:place[3]]
            return place
        def savefn():
            return (self.place, len(self.stack), len(self.callstack), len(self.localc))
        def passfn(*args):
            return passfn
        def iffn(val, ret=False):
            def retf_if(fn):
                self.locals[-1]["else"] = False
                self.stack.append(fn)
                self.op_call(0)
            def ret_else(fn):
                self.locals[-1]["else"] = True
            if val:
                return retf_if
            return ret_else
        def retfn(arg=None):
            for i in range(len(self.callstack)-self.localc[-1]-1): 
                self.op_retn(None)
                self.stack.pop()
            self.op_retn(None)
            self.stack.pop()
            return arg
        def haltfn():
            return loadfn(len(self.ops))
        def newline():
            print()
        def out(*args):
            print(*(strfn(i) for i in args), sep='',end='')
        def logfn(*args):
            print('log: ', *(strfn(i) for i in args), sep='')
        def fnfn(fn):
            def retf_fn(*args):
                self.localc.append(len(self.callstack))
                self.locals.append({
                    'args' if self.namec == 0 else self.nameconv['args']: list(args)
                })
                self.stackd.append(len(self.stack))
                self.stack.append(fn)
                self.op_call(args)
                return NoReturn()
            return retf_fn
        def call_fn(fn, *args):
            self.stack.append(fn)
            self.op_call(args)
        def dictof_fn(*args):
            ret = {}
            pl = 0
            mpl = len(args)
            while pl < mpl:
                ret[args[pl]] = args[pl+1]
                pl += 2
            return ret
        listof = lambda *args: list(args)
        
        def typeid_fn(arg):
            if callable(arg):
                return fnfn
            if isinstance(arg, list):
                return listof
            if isinstance(arg, dict):
                return dictof_fn
            return type(arg)
        def class_fn(arg=None):
            if arg == None:
                arg = {}
            arg['id'] = self.classno
            self.classno += 1
            return arg
        ret = {
            'log': logfn,
            'out': out,
            'newline': newline,
            # 'save': savefn,
            # 'load': loadfn,
            'halt': haltfn,
            'true': True,
            'false': False,
            'if': iffn,
            'ret': retfn,
            'fn': fnfn,
            'len': len,
            'list': listof,
            'dict': dictof_fn,
            'List': listof,
            'Dict': dictof_fn,
            'Proc': fnfn,
            'String': str,
            'Boolean': bool,
            'Number': float,
            'typeid': typeid_fn,
            'class': class_fn,
            'none': None,
        }
        for i in operfns:
            ret[i] = operfns[i]
        return ret
    def __init__(self):

        self.ops = []
        self.lines = {}
        self.place = 0
        self.max = 0
        self.stack = []
        self.locals = [self.glob()]
        self.localc = [None]
        self.stackd = []
        self.callstack = []
        self.marknex = False
        self.namec = 0
        self.classno = 256
    def opt(self):
        self.nameconv = {}
        self.numconv = []
        self.nameconv["args"] = self.namec
        self.numconv.append("args")
        self.namec += 1
        for name in self.locals[0]:
            self.nameconv[name] = self.namec            
            self.numconv.append(name)
            self.namec += 1
        for i in self.ops:
            if i.kind in (opkind["STORE"], opkind["LOAD"]):
                self.nameconv[i.value] = self.namec
                self.numconv.append(i.value)
                self.namec += 1
        for pl, i in enumerate(self.ops):
            if i.kind in (opkind["STORE"], opkind["LOAD"]):
                self.ops[pl].value = self.nameconv[i.value]
        for d, i in enumerate(self.locals):
            self.locals[d] = {}
            for j in i:
                self.locals[d][self.nameconv[j]] = i[j]

    def append(self, op):
        if not isinstance(op, list):
            self.ops.append(op)
            self.ops[-1].place = len(self.ops)-1
            # if self.marknex:
            #     self.ops[-1].target = True
            self.max += 1
        else:
            for i in op:
                self.append(i)
        # self.marknex = False
    def jmp(self, val):
        self.place = int(val)
    def run(self):
        while self.place < self.max:
            begs = len(self.stack)
            op = self.ops[self.place]
            # print(self.place)
            # print(self.place, op.kind, op.value, strfn(self.stack))
            self.opfns[op.kind](self, op.value)
            self.place += 1
            diff = len(self.stack)-begs
            instrv[op.kind] = diff
            # print(op.kind, diff)
        self.place = 0
    def lasterrln(self):
        place = self.place
        while place >= 0:
            if place in self.lines:
                return "{} opcode: {}".format(self.lines[place].ln, self.place)
            place -= 1
        return "of opcode: {}".format(self.place)
    def prun(self):
        self.place = 0
        try:
            self.run()
        except:
            print('error on line:', self.lasterrln())
            raise
    def fformat(self, *args):
        lines = []
        for i in self.ops:
            lines.append("")
            # if i.target:
            #     lines[-1] += str(i.place)
            #     lines[-1] += ':'
           
            lines[-1] += str(i.kind)
            lines[-1] += ' '*(6-len(lines[-1]))
            if i.value != None:
                lines[-1] += str(i.value)
            else:
                lines[-1] += 'None'
            if 'len' in args:
                lenv = str(len(lines[-1]))
                lenv = '0'*(4-len(lenv))+lenv
                lines[-1] = lenv + ' ' + lines[-1]
            # if 'c' in args:
                # lines[-1] = '0'*(4-len(str(len(lines[-1]))))+str(len(lines[-1]))+' '+lines[-1]
        return '\n'.join(lines)+'\n'
    def __str__(self):
        return "Code({', '.join(str(i) for i in self.ops)})"
    def __repr__(self):
        return "Code({', '.join(str(i) for i in self.ops)})"

def read_tree_braces(tokens):
    ret = []
    place = 0
    maxplace = len(tokens)
    while place < maxplace:
        if tokens[place].kind == Token.Kind.BRACE:
            orig = tokens[place].tok
            start = place
            depth = 1
            while depth > 0:
                place += 1
                if tokens[place].tok == orig:
                    depth += 1
                if tokens[place].tok == matching[orig]:
                    depth -= 1
            end = place
            ret.append(
                Braced(orig + matching[orig], read_tree(tokens.sub(start+1, end))))
        else:
            ret.append(tokens[place])
        place += 1
    return ret

def read_tree_level(tokens, order):
    ret = Tokens()
    inds = {}
    for pl, token in enumerate(tokens):
        if isinstance(token, Token) and token.kind == Token.Kind.OPERATOR:
            if token.tok in order:
                inds[pl] = token.tok
    pl = 0
    maxpl = len(tokens)
    lastop = False
    while pl < maxpl:
        if pl in inds:
            if pl == -1:
                ret.append(Operator(tokens[pl], None, tokens[pl+1]))
            else:
                if lastop:
                    ret.tokens[-1] = Operator(tokens[pl], ret.tokens[-1], tokens[pl+1])
                else:
                    ret.tokens.pop()
                    ret.append(Operator(tokens[pl], tokens[pl-1], tokens[pl+1]))
            lastop = True
            pl += 1
        else:
            ret.append(tokens[pl])
            lastop = False
        pl += 1
    return ret

def read_tree_functions(tokens):
    ret = []
    for i in tokens:
        if isinstance(i, Braced):
            if len(ret) != 0 and ret[-1].kind != Token.Kind.OPERATOR:
                last = ret[-1]
                ret.pop()
                ret.append(Call(last, i))
                continue
        ret.append(i)
    return ret

def read_tree_operators(tokens):
    tokens = read_tree_functions(tokens)
    for order in operators_ordered:
        tokens = read_tree_level(tokens, order)
    return tokens

def read_tree(tokens):
    parensized = read_tree_braces(tokens)
    tree = read_tree_operators(parensized)
    return tree

def dostring(code_string):
    tokens = read_tokens(code_string)
    tree = read_tree(tokens)
    vm = Vm()
    tree.conv(vm, base=True)
    vm.prun()

def importfile(name):
    if os.path.isfile(name):
        name = name
    elif os.path.exists('include') and os.path.isfile('include/'+name):
        name = 'include/'+name

    code_file = open(name)
    code_string = code_file.read()
    code_file.close()
    return preprocess(code_string)

def preprocess(code):
    spl = code.split('\n')
    ret = []
    for i in spl:
        if len(i) > 0 and i[0] == '#':
            i = i[1:]
            if i.startswith('import'):
                ret.append(importfile(i[7:]))
        else:
            ret.append(i)
    return '\n'.join(ret)

def dof(name="auto.txt", mode="run", *rest):
    if '--opcode' in rest:
        for pl in opkind:
            opkind[pl] = pl
    code_file = open(name)
    code_string = code_file.read()
    code_file.close()
    code_string = preprocess(code_string)
    tokens = read_tokens(code_string)
    tree = read_tree(tokens)
    exp = tree.export()
    ft = open("cache/tree.txt", 'w')
    ft.write(exp)
    ft.close()
    vm = Vm()
    tree.conv(vm, base=True, poplast=False)
    if '--version' in rest:
        if pypy:
            print('PyPy3')
        else:
            print('CPython3')
        
    if mode == "run":
        fo = open('cache/code.txt', 'w')
        fo.write(vm.fformat(*[i[4:] for i in rest if i[:4] == '--op']))
        fo.close()
    if mode == "opt":
        vm.opt()
        fo = open('cache/code.txt', 'w')
        fo.write(vm.fformat(*rest))
        fo.close()
    if '--perf' in rest:
        import cProfile
        cProfile.runctx('vm.prun()', None, locals())
    elif '--opcode' not in rest:
        vm.prun()
if __name__ == "__main__":
    dof(*sys.argv[1:])
# cProfile.run('vm.run()')

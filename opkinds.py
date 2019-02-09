kind_names = [
    'INDEX',
    
    'CALL',
    'RETN',

    'JUMP',
    
    'PUSH',
    'POP',
    'DUP',
    'SWAP',

    'OPER',
    'LOAD',
    'STORE',

    'STR',

    'PROC',
    'DEFS',

    'ANYA',
]
kind = {}
for pl, i in enumerate(kind_names):
    kind[i] = i
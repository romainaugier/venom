from typing import Dict, Optional

from ._type import *
from ._symbols import FunctionBuiltin

_builtins = {
    "print": FunctionBuiltin("print",
                             FunctionType("print",
                                          { "...": Type },
                                          TypeVoid)),
    "range": FunctionBuiltin("range", 
                             FunctionType("range",
                                          { "x": Type },
                                          TypeInt64)),
    "len": FunctionBuiltin("len", 
                           FunctionType("len",
                                        { "x": Type },
                                        TypeInt64)),
    "float": FunctionBuiltin("float", FunctionType("float",
                                                   { "x": Type },
                                                   TypeFloat64)),
    "int": FunctionBuiltin("int", FunctionType("int",
                                               { "x": Type },
                                               TypeFloat64)),
    "bool": FunctionBuiltin("bool", FunctionType("bool",
                                                 { "x": Type },
                                                 TypeBool)),
}

def get_builtin_functions() -> Dict[str, FunctionBuiltin]:
    return _builtins

# Handle each function carefully 

def get_builtin_function_specialization(name: str, args: List[Type]) -> Optional[FunctionType]:
    if not name in _builtins:
        return None

    builtin = _builtins[name]

    args_mapping = { argname: argtype for argname, argtype in zip(builtin.type.args.keys(), args) }

    return FunctionType(name, args_mapping, builtin.type.return_type)
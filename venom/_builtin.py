from typing import Dict, Set, Union, Optional, Any
from dataclasses import dataclass

from ._type import *

@dataclass 
class Builtin():
    """
    Base class for all builtin stuff, like modules, functions, variables
    """
    
    name: str

@dataclass
class BuiltinConstant(Builtin):
    """
    Builtin constants are holding constant values
    """
    
    value: Any
    type: Type

@dataclass
class BuiltinFunction(Builtin):
    """
    Builtin function
    """

    func: FunctionType

@dataclass
class BuiltinModule(Builtin):
    """
    Builtin module holding classes, functions, constants, like math, sys...
    """
    
    constants: Dict[str, BuiltinConstant]
    functions: Dict[str, BuiltinFunction]

_builtins = {
    "range": BuiltinFunction("range", 
                             FunctionType("range",
                                          { "x": Type },
                                          PrimitiveType(Primitive.Int64))),
    "len": BuiltinFunction("len", 
                           FunctionType("len",
                                        { "x": Type },
                                        PrimitiveType(Primitive.Int64))),
    "float": BuiltinFunction("float", FunctionType("float",
                                                   { "x": Type },
                                                   PrimitiveType(Primitive.Float64))),
    "int": BuiltinFunction("int", FunctionType("int",
                                               { "x": Type },
                                               PrimitiveType(Primitive.Float64))),
    "bool": BuiltinFunction("bool", FunctionType("bool",
                                                 { "x": Type },
                                                 PrimitiveType(Primitive.Bool))),
}

def is_builtin_function(name: str) -> bool:
    return name in _builtins

def get_builtin_function(name: str) -> Optional[BuiltinFunction]:
    return _builtins.get(name)
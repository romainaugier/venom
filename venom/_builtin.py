# from typing import Dict, Set, Union, Optional, Any
# from dataclasses import dataclass

# from ._type import *


# @dataclass 
# class Builtin():
#     """
#     Base class for all builtin stuff, like modules, functions, variables
#     """
    
#     name: str

# @dataclass
# class BuiltinConstant(Builtin):
#     """
#     Builtin constants are holding constant values
#     """
    
#     value: Any
#     type: Type

# @dataclass
# class BuiltinFunction(Builtin):
#     """
#     Builtin function
#     """

# @dataclass
# class BuiltinModule(Builtin):
#     """
#     Builtin module holding classes, functions, constants, like math, sys...
#     """
    
#     constants: Dict[str, BuiltinConstant]
#     functions: Dict[str, BuiltinFunction]

# @dataclass
# class Func():
    
#     name: str
#     return_type: Union[Type, Set]

# @dataclass
# class BuiltinFunc(Func):
#     pass

# _builtins = {
#     "range": BuiltinFunc("range", Type.Int),
#     "len": BuiltinFunc("len", Type.Int),
#     "float": BuiltinFunc("float", Type.Float),
#     "int": BuiltinFunc("int", Type.Int),
#     "bool": BuiltinFunc("bool", Type.Bool),
# }

# def is_builtin_function(name: str) -> bool:
#     return name in _builtins

# def get_builtin_function(name: str) -> Optional[Func]:
#     return _builtins.get(name)
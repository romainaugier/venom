import ast 

from dataclasses import dataclass, field
from typing import Dict, Optional, List

from ._type import Type, FunctionType

@dataclass
class Symbol():

    name: str
    type: Optional[Type]
    
    def __str__(self) -> str:
        return f"SYMBOL(\"{self.name}\")"

    def __repr__(self) -> str:
        return self.__str__()

@dataclass
class Variable(Symbol):
    
    def __str__(self) -> str:
        return f"VARIABLE(\"{self.name}\", {self.type})"

@dataclass
class Parameter(Symbol):
    
    type: Optional[Type]

    def __str__(self) -> str:
        return f"PARAMETER(\"{self.name}\", {self.type})"

@dataclass
class FunctionBuiltin(Symbol):
    
    parameters: List[str] = field(default_factory=list)

    specializations: Dict[str, FunctionType] = field(default_factory=dict)

    def __str__(self) -> str:
        num_parameters = len(self.parameters)
        num_specializations = len(self.specializations)
        return f"BUILTINFUNC(\"{self.name}\", {num_parameters} parameter{'s' if num_parameters > 1 else ''}, {num_specializations} specializations)"

@dataclass
class FunctionDef(Symbol):

    ast_node: ast.FunctionDef
    parameters: List[str] = field(default_factory=list)

    specializations: Dict[str, FunctionType] = field(default_factory=dict)

    def __str__(self) -> str:
        return f"FUNCTIONDEF(\"{self.name}\", {len(self.parameters)} parameter{'s' if len(self.parameters) > 1 else ''}, {len(self.specializations)} specializations)"

@dataclass
class ClassDef(Symbol):

    ast_node: ast.ClassDef
    var_members: List[str] = field(default_factory=list)
    func_members: List[str] = field(default_factory=list)

    def __str__(self) -> str:
        return f"CLASSDEF(\"{self.name}\")"
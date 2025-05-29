from __future__ import annotations

import ast

from dataclasses import dataclass, field
from typing import List, Optional, Union, Dict, Any

from ._op import *
from ._type import Type
from ._symtable import SymbolTable

@dataclass
class IRStatement():
    """
    Base class for all the statements (moves, ops, calls...)
    """

    version: int

@dataclass
class IRTerminator():
    """
    Base class for all terminators (final block value)
    """

@dataclass
class IRBlock():
    """
    Base class for an IRBlock, consisting of parameters, statements and a terminator
    """
    
    name: str
    parameters: List[str]
    statements: List[IRStatement] = field(default_factory=list)
    terminator: Optional[IRTerminator] = None

@dataclass
class IRFunction():
    """
    Function composed of multiple IR blocks
    """

    name: str
    parameters: Dict[str, Type] = field(default_factory=dict)
    blocks: List[IRBlock] = field(default_factory=list)

@dataclass
class IRClass():
    """
    Class is composed of multiple Functions, along with all its member variables
    """

    variables: Dict[str, Type] = field(default_factory=dict)
    functions: Dict[str, IRFunction] = field(default_factory=dict)

# IR Types

@dataclass
class IRVariable(IRStatement):
    """
    Variable
    """

    name: str
    type: Type

@dataclass
class IRLiteral(IRStatement):
    """
    Constant literal value
    """

    name: str
    type: Type
    value: Any

# IR Ops

@dataclass
class UnaryOp(IRStatement):

    op: UnaryOpType
    operand: int

@dataclass
class BinaryOp(IRStatement):

    op: BinaryOpType
    left: int
    right: int

@dataclass
class TernaryOp(IRStatement):

    op: CompareOpType
    cond: int
    true_val: int
    false_val: int

@dataclass
class FuncOp(IRStatement):

    func: int
    args: List[int]

# IR Terminators

@dataclass
class Return(IRTerminator):
    
    value: Optional[int]

# IR AST Visitor

class IRBuilder(ast.NodeVisitor):
    
    def __init__(self, ir: "IR", symtable: SymbolTable) -> None:
        self._ir = ir
        self._symtable = symtable

        self._blocks = list()
        self._current_block = None

        self._functions = list()
        self._current_function = None

        self._classes = list()
        self._current_class = None

    def new_block(self, name: str, parameters: Optional[List[int]] = None) -> IRBlock:
        block = IRBlock(name, parameters)

        # No IRBlocks inside classes
        if self._current_function is not None:
            self._current_function.blocks.append(block)
        else:
            self._blocks.append(block)

        self._current_block = block

        return block

    def emit(self, statement: IRStatement) -> None:
        self._current_block.body.append(statement)

    def visit_FunctionDef(self, node: ast.FunctionDef) -> None:
        func = IRFunction(node.name)

        self._current_function = func

        self._functions.append(func)

        args = [arg.arg for arg in node.args.args]

        entry_block = self.new_block(f"{node.name}_entry")

        for stmt in node.body:
            self.visit(stmt)

        self._current_function = None

# IR 

class IR():
    
    def __init__(self) -> None:
        self._version_counter = 0
        self._variables_versions = dict()

    def new_version(self, variable_name: str) -> int:
        version = self._version_counter
        self._version_counter += 1
        self._variables_versions[variable_name] = version
        return version

    def get_version(self, variable_name) -> Optional[int]:
        return self._variables_versions.get(variable_name)

    def build(self, tree: ast.expr) -> bool:
        pass

    def print(self) -> None:
        pass
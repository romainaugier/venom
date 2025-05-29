from __future__ import annotations

import ast

from dataclasses import dataclass, field
from typing import List, Optional, Union, Dict, Any

from ._op import *
from ._type import Type, pytype_to_type, type_to_string
from ._symtable import SymbolTable

@dataclass
class IRStatement():
    """
    Base class for all the statements (moves, ops, calls...)
    """

    version: int

    def print(self, indent_size: int, depth: int) -> None:
        raise NotImplementedError

@dataclass
class IRTerminator():
    """
    Base class for all terminators (final block value)
    """

    def print(self, indent_size: int, depth: int) -> None:
        raise NotImplementedError

@dataclass
class IRBlock():
    """
    Base class for an IRBlock, consisting of parameters, statements and a terminator
    """
    
    name: str
    parameters: List[str] = field(default_factory=list)
    statements: List[IRStatement] = field(default_factory=list)
    terminator: Optional[IRTerminator] = None

    def print(self, indent_size: int, depth: int) -> None:
        parameters_str = ', '.join(self.parameters) if self.parameters is not None else ""

        print(" " * indent_size * depth, f"BLOCK {self.name} ({parameters_str})")

        for stmt in self.statements:
            stmt.print(indent_size, depth + 1)

        if self.terminator is not None:
            self.terminator.print(indent_size, depth + 1)

@dataclass
class IRFunction():
    """
    Function composed of multiple IR blocks
    """

    name: str
    return_type: Type
    parameters: Dict[str, Type] = field(default_factory=dict)
    blocks: List[IRBlock] = field(default_factory=list)

    def print(self, indent_size: int, depth: int) -> None:
        parameters_str = ', '.join([f"{name}: {type_to_string(type)}" for name, type in self.parameters.items()])

        print(" " * indent_size * depth, f"FUNCTION {self.name} ({parameters_str}) -> {type_to_string(self.return_type)}")

        for block in self.blocks:
            block.print(indent_size, depth + 1)

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

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, f"%{self.version} = {self.name}")

@dataclass
class IRLiteral(IRStatement):
    """
    Constant literal value
    """

    name: str
    type: Type
    value: Any

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, f"%{self.version} = {self.name}")

# IR Ops

@dataclass
class IRUnaryOp(IRStatement):

    op: UnaryOpType
    operand: int

    def print(self, indent_size: int, depth: int) -> None:
        pass

@dataclass
class IRBinaryOp(IRStatement):

    op: BinaryOpType
    left: int
    right: int

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, f"%{self.version} = %{self.left} {binop_to_string(self.op)} %{self.right}")

@dataclass
class IRTernaryOp(IRStatement):

    op: CompareOpType
    cond: int
    true_val: int
    false_val: int

    def print(self, indent_size: int, depth: int) -> None:
        pass

@dataclass
class IRFuncOp(IRStatement):

    func: int
    args: List[int]

    def print(self, indent_size: int, depth: int) -> None:
        pass

# IR Terminators

@dataclass
class IRReturn(IRTerminator):
    
    value: Optional[int]

    def print(self, indent_size: int, depth: int) -> None:
        if self.value is None:
            print(" " * indent_size * depth, "return")
        else:
            print(" " * indent_size * depth, f"return %{self.value}")

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

    def get_blocks(self) -> List[IRBlock]:
        return self._blocks

    def get_functions(self) -> List[IRFunction]:
        return self._functions

    def get_classes(self) -> List[IRClass]:
        return self._classes

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
        self._current_block.statements.append(statement)

    def visit_FunctionDef(self, node: ast.FunctionDef) -> None:
        func_symbol = self._symtable.resolve_symbol(node.name)
        
        self._symtable.set_scope(node.name)

        parameters = dict()

        for arg in node.args.args:
            sym = self._symtable.resolve_symbol(arg.arg)

            if sym is not None:
                parameters[sym.name] = sym.type

        func = IRFunction(node.name, func_symbol.return_type, parameters)

        self._current_function = func

        self._functions.append(func)

        entry_block = self.new_block(f"{node.name}_entry")

        for stmt in node.body:
            self.visit(stmt)

        self._current_function = None

        self._symtable.pop_scope()

    def visit_Name(self, node: ast.Name) -> int:
        return self._ir.new_version(node.id)

    def visit_Constant(self, node: ast.Constant) -> int:
        version = self._ir.new_version("_const")
        stmt = IRLiteral(version, str(node.value), pytype_to_type(str(type(node.value))), node.value)
        self.emit(stmt)
        return version

    def visit_BinOp(self, node: ast.BinOp) -> int:
        left = self.visit(node.left)
        right = self.visit(node.right)

        op = ast_binop_to_binop(node)

        version = self._ir.new_version("_tmp")

        stmt = IRBinaryOp(version, op, left, right)
        self.emit(stmt)
        
        return version

    def visit_Return(self, node):
        value = self.visit(node.value)
        self._current_block.terminator = IRReturn(value)

# IR 

class IR():
    
    def __init__(self, symtable: SymbolTable) -> None:
        self._symtable = symtable
        self._version_counter = 0
        self._variables_versions = dict()

        self._blocks = list()
        self._functions = list()
        self._classes = list()

    def new_version(self, variable_name: str) -> int:
        version = self._version_counter
        self._version_counter += 1
        self._variables_versions[variable_name] = version
        return version

    def get_version(self, variable_name) -> Optional[int]:
        return self._variables_versions.get(variable_name)

    def build(self, tree: ast.expr) -> bool:
        self._version_counter = 0
        self._variables_versions.clear()

        ir_builder = IRBuilder(self, self._symtable)
        ir_builder.visit(tree)

        self._blocks = ir_builder.get_blocks()
        self._functions = ir_builder.get_functions()
        self._classes = ir_builder.get_classes()

    def print(self, indent_size: int = 4) -> None:
        print("IR")

        if len(self._classes) > 0:
            print("CLASSES")

        if len(self._functions) > 0:
            print("FUNCTIONS")

            for function in self._functions:
                function.print(indent_size, 1)

        if len(self._blocks) > 0:
            print("BLOCKS")
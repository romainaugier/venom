from __future__ import annotations

import ast

from dataclasses import dataclass, field
from typing import List, Optional, Union, Dict, Any, Tuple

from ._op import *
from ._type import *
from ._symtable import SymbolTable, FunctionDef

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
        parameters_str = ', '.join([f"{name}: {type.ir_repr()}" for name, type in self.parameters.items()])

        print(" " * indent_size * depth, f"FUNCTION {self.name} ({parameters_str}) -> {self.return_type.ir_repr()}")

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
        print(" " * indent_size * depth, f"%{self.version} = {self.type.ir_repr()} {self.name}")

@dataclass
class IRLiteral(IRStatement):
    """
    Constant literal value
    """

    name: str
    type: Type
    value: Any

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, f"%{self.version} = {self.type.ir_repr()} {self.name}")

# IR Ops

@dataclass
class IrMemLoadOp(IRStatement):
    
    base_ptr: int
    type: Type
    offset: int

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"%{self.version} = {self.type.ir_repr()} memload %{self.base_ptr}[%{self.offset}]")

@dataclass
class IRCastOp(IRStatement):
    
    operand: int
    type_from: Type
    type_to: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, 
              f"%{self.version} = {self.type_to.ir_repr()} cast %{self.operand}")

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
    type: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, 
              f"%{self.version} = {self.type.ir_repr()} {binop_to_string(self.op)} %{self.left} %{self.right}")

@dataclass
class IRCompareOp(IRStatement):
    
    left: int
    right: int
    type: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"cmp {self.type.ir_repr()} %{self.left}, %{self.right}")

@dataclass
class IRCMovOp(IRStatement):
    
    op: CompareOpType
    true_val: int
    false_val: int
    type: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"%{self.version} = {self.type.ir_repr()} cmov %{self.true_val}, %{self.false_val} {compareop_to_ir_string(self.op)}")

@dataclass
class IRTernaryOp(IRStatement):

    op: CompareOpType
    left: int
    right: int
    true_val: int
    false_val: int
    type: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"%{self.version} = {self.type.ir_repr()} cmov ")

@dataclass
class IRFuncOp(IRStatement):

    func: FunctionType
    args: List[int]

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"%{self.version} = {self.func.return_type.ir_repr()} call {self.func.mangled_name()}({','.join(f'%{arg}' for arg in self.args)})")

@dataclass
class IRIncOp(IRStatement):
    
    operand: int
    type: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"%{self.operand} = {self.type.ir_repr()} inc %{self.operand}")

@dataclass
class IRDecOp(IRStatement):
    
    operand: int
    type: Type

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth,
              f"%{self.operand} = {self.type.ir_repr()} dec %{self.operand}")

# IR Terminators

@dataclass
class IRReturn(IRTerminator):
    
    value: Optional[int]

    def print(self, indent_size: int, depth: int) -> None:
        if self.value is None:
            print(" " * indent_size * depth, "return")
        else:
            print(" " * indent_size * depth, f"return %{self.value}")

@dataclass
class IRJump(IRTerminator):
    
    block: IRBlock
    comp: CompareOpType

    def print(self, indent_size: int, depth: int) -> None:
        print(" " * indent_size * depth, f"jump {self.block.name} {compareop_to_ir_string(self.comp)}")

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

        self._has_error = False

    def _error(self, err: str) -> None:
        self._has_error = True

    def has_error(self) -> bool:
        return self._has_error

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

    # Helpers
    
    def _cast_types(self, version_left: int, version_right: int) -> Tuple[int, int, Type]:
        left_type = self._ir.get_version_type(version_left)
        right_type = self._ir.get_version_type(version_right)

        final_type = left_type

        if left_type != right_type:
            left_rank = type_rank(left_type)
            right_rank = type_rank(right_type)
            
            if left_rank > 0 and right_rank > 0:
                if left_rank > right_rank:
                    cast_version = self._ir.new_version("_cast", left_type)
                    cast_stmt = IRCastOp(cast_version, version_right, right_type, left_type)
                    self.emit(cast_stmt)
                    version_right = cast_version
                elif right_rank > left_rank:
                    cast_version = self._ir.new_version("_cast", right_type)
                    cast_stmt = IRCastOp(cast_version, version_left, left_type, right_type)
                    self.emit(cast_stmt)
                    version_left = cast_version
                    final_type = right_type
            # TODO: If types are incompatible for casting, error

        return version_left, version_right, final_type

    def visit_FunctionDef(self, node: ast.FunctionDef) -> None:
        func_symbol = self._symtable.resolve_symbol(node.name)

        if not isinstance(func_symbol, FunctionDef):
            self._error(f"Cannot find function symbol: {node.name}")
            return

        for name, func_type in func_symbol.specializations.items():
            self._symtable.set_scope(node.name)

            func = IRFunction(name, func_type.return_type, func_type.args)
            self._current_function = func

            self._functions.append(func)
            entry_block = self.new_block(f"body{node.lineno}")

            for stmt in node.body:
                self.visit(stmt)

            self._current_function = None

            self._symtable.pop_scope()

    def visit_Name(self, node: ast.Name) -> int:
        sym = self._symtable.resolve_symbol(node.id)
        
        version = self._ir.get_version(node.id)

        if version is None:
            version = self._ir.new_version(node.id, sym.type if sym is not None else TypeInvalid)
            stmt = IRVariable(version, str(node.id), sym.type if sym is not None else TypeInvalid)
            self.emit(stmt)
        
        return version 

    def visit_Constant(self, node: ast.Constant) -> int:
        node_type = pytype_to_type(type(node.value))
        version = self._ir.new_version("_const", node_type)

        stmt = IRLiteral(version, str(node.value), node_type, node.value)
        self.emit(stmt)

        return version

    def visit_UnaryOp(self, node: ast.UnaryOp) -> int:
        operand = self.visit(node.operand)
        operand_type = self._ir.get_version_type(operand)

        op = ast_unop_to_unop(node)
        version = self._ir.new_version("_tmp", operand_type)
        stmt = IRUnaryOp(version, op, operand)
        self.emit(stmt)

        return version

    def visit_BinOp(self, node: ast.BinOp) -> int:
        left = self.visit(node.left)
        right = self.visit(node.right)

        left, right, final_type = self._cast_types(left, right)
        
        op = ast_binop_to_binop(node)
        version = self._ir.new_version("_tmp", final_type)
        stmt = IRBinaryOp(version, op, left, right, final_type)
        self.emit(stmt)
        
        return version

    def visit_AugAssign(self, node: ast.AugAssign) -> int:
        target = self.visit(node.target)
        value = self.visit(node.value)

        target, value, final_type = self._cast_types(target, value)

        op = ast_binop_to_binop(node)

        stmt = IRBinaryOp(target, op, target, value, final_type)
        self.emit(stmt)

        return target

    def visit_IfExp(self, node: ast.IfExp) -> int:
        true_val = self.visit(node.body)
        false_val = self.visit(node.orelse)

        # For now, since the test should be a compare as verified when building the symbol table 
        # and running the semantic analysis, and only one compare op should be present
        op = ast_compareop_to_compareop(node.test)

        left = self.visit(node.test.left)
        right = self.visit(node.test.comparators[0])

        left, right, cmp_type = self._cast_types(left, right)

        cmp_version = self._ir.new_version("_tmp", cmp_type)
        stmt = IRCompareOp(cmp_version, left, right, cmp_type)
        self.emit(stmt)

        true_val, false_val, mov_type = self._cast_types(true_val, false_val)

        version = self._ir.new_version("_tmp", mov_type)
        stmt = IRCMovOp(version, op, true_val, false_val, mov_type)
        self.emit(stmt)
        
        return version

    def visit_Return(self, node):
        value = self.visit(node.value)
        self._current_block.terminator = IRReturn(value)

    def visit_Call(self, node: ast.Call) -> int:
        if not isinstance(node.func, ast.Name):
            return None

        arg_versions = list()

        for arg in node.args:
            arg_versions.append(self.visit(arg))            

        arg_types = [self._ir.get_version_type(version) for version in arg_versions]

        func_specializations = self._ir._symtable.get_builtin_specializations().get(node.func.id, list())

        func_specialization = None

        for specialization in func_specializations:
            if all(arg_type == spe_arg_type for arg_type, spe_arg_type in zip(arg_types, specialization.args.values())):
                func_specialization = specialization
                break

        if func_specialization is None:
            return None

        version = self._ir.new_version("_tmp", func_specialization.return_type)
        stmt = IRFuncOp(version, func_specialization, arg_versions)
        self.emit(stmt)

        return version

    def visit_Subscript(self, node: ast.Subscript) -> int:
        value = self.visit(node.value)
        value_type = self._ir.get_version_type(value)
        offset = self.visit(node.slice)

        if not isinstance(value_type, ArrayType):
            return

        version = self._ir.new_version("_tmp", value_type.element_type)
        stmt = IrMemLoadOp(version, value, value_type.element_type, offset)
        self.emit(stmt)

        return version

    def visit_For(self, node: ast.For) -> None:
        if isinstance(node.iter, ast.Call):
            loop_target = self.visit(node.target)
            loop_iter = self.visit(node.iter)

            loop_target, loop_iter, cmp_type = self._cast_types(loop_target, loop_iter)

            for_block = self.new_block(f"for{node.lineno}")

            for stmt_body in node.body:
                self.visit(stmt_body)

            stmt = IRIncOp(None, loop_target, self._ir.get_version_type(loop_target))
            self.emit(stmt)

            cmp_version = self._ir.new_version("_tmp", TypeBool)
            stmt = IRCompareOp(cmp_version, loop_target, loop_iter, cmp_type)
            self.emit(stmt)
            for_block.terminator = IRJump(for_block, CompareOpType.Lt)

            self.new_block(f"body{stmt_body.lineno + 1}")

# IR 

class IR():
    
    def __init__(self, symtable: SymbolTable) -> None:
        self._symtable = symtable
        self._version_counter = 0
        self._variables_versions = dict()
        self._version_types = dict()

        self._blocks = list()
        self._functions = list()
        self._classes = list()

    def new_version(self, variable_name: str, type: Type) -> int:
        version = self._version_counter
        self._version_counter += 1
        self._variables_versions[variable_name] = version
        self._version_types[version] = type
        return version

    def get_version(self, variable_name: str) -> Optional[int]:
        return self._variables_versions.get(variable_name)

    def get_version_type(self, version: int) -> Type:
        return self._version_types.get(version, TypeInvalid)

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
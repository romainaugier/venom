from __future__ import annotations

import ast
import enum

from dataclasses import dataclass, field
from typing import Optional, List, Set, Dict

from ._type import *
from ._builtin import is_builtin_function, get_builtin_function
from ._error import print_ast_error

class SymbolTable:
    pass

class SymbolTableVisitor(ast.NodeVisitor):

    def __init__(self, symbol_table: SymbolTable, source_code: str = None) -> None:
        self._symbol_table = symbol_table
        self._return_types = list()
        self._source_code = source_code
        self._has_error = False

    def has_error(self) -> bool:
        return self._has_error

    def _error(self, node: ast.expr, message: str) -> None:
        self._has_error = True

        print_ast_error(node, message, self._source_code)

    def get_return_types(self) -> List[Type]:
        return self._return_types

    def _deduce_expr_type(self, node: Optional[ast.expr]) -> Type:
        if node is None: # Return statement without a value
            return PrimitiveType(Primitive.Void)

        if isinstance(node, ast.Constant):
            if isinstance(node.value, int): 
                return PrimitiveType(Primitive.Int64)

            if isinstance(node.value, float):
                return PrimitiveType(Primitive.Float64)

            if isinstance(node.value, bool):
                return PrimitiveType(Primitive.Bool)

            if isinstance(node.value, str):
                return ArrayType(Primitive.Int16)

            if isinstance(node.value, bytes):
                return ArrayType(Primitive.Int8)

            if node.value is None:
                return PrimitiveType(Primitive.Void)

        elif isinstance(node, ast.Name):
            symbol = self._symbol_table.resolve_symbol(node.id)

            if symbol is not None and hasattr(symbol, "type"):
                sym_type = symbol.type
                return sym_type

            self._error(node, f"Cannot find symbol: {symbol}")

            return PrimitiveType(Primitive.Invalid) # Symbol not found or has no type
        elif isinstance(node, ast.BinOp):
            left_type = self._deduce_expr_type(node.left)
            right_type = self._deduce_expr_type(node.right)

            if left_type == PrimitiveType(Primitive.Invalid) or right_type == PrimitiveType(Primitive.Invalid): 
                self._error(node, f"Invalid binary Op")
                return PrimitiveType(Primitive.Invalid)
            
            # Arithmetic operations
            op_type = type(node.op)

            if op_type in (ast.Add, ast.Sub, ast.Mult):
                if left_type == PrimitiveType(Primitive.Float64) or right_type == PrimitiveType(Primitive.Float64): 
                    return PrimitiveType(Primitive.Float64)

                if left_type == PrimitiveType(Primitive.Int64) and right_type == PrimitiveType(Primitive.Int64):
                    return PrimitiveType(Primitive.Int64)

            # True division /
            elif op_type is ast.Div:
                if left_type in (PrimitiveType(Primitive.Int64), PrimitiveType(Primitive.Float64)) and right_type in (PrimitiveType(Primitive.Int64), PrimitiveType(Primitive.Float64)):
                    return PrimitiveType(Primitive.Float64)

            # Floor division //
            elif op_type is ast.FloorDiv: 
                if (left_type == PrimitiveType(Primitive.Int64) and right_type == PrimitiveType(Primitive.Int64)) or \
                   (left_type == PrimitiveType(Primitive.Float64) and right_type == PrimitiveType(Primitive.Float64)) or \
                   (left_type == PrimitiveType(Primitive.Float64) and right_type == PrimitiveType(Primitive.Int64)) or \
                   (left_type == PrimitiveType(Primitive.Int64) and right_type == PrimitiveType(Primitive.Float64)): 
                       return PrimitiveType(Primitive.Float64) if PrimitiveType(Primitive.Float64) in (left_type, right_type) else PrimitiveType(Primitive.Int64)

            # % and ** (simplified)
            elif op_type in (ast.Mod, ast.Pow):
                if left_type == PrimitiveType(Primitive.Float64) or right_type == PrimitiveType(Primitive.Float64):
                    return PrimitiveType(Primitive.Float64)

                if left_type == PrimitiveType(Primitive.Int64) and right_type == PrimitiveType(Primitive.Int64):
                    return PrimitiveType(Primitive.Int64)
            
            # Comparisons
            elif op_type in (ast.Eq, ast.NotEq, ast.Lt, ast.LtE, ast.Gt, ast.GtE):
                return PrimitiveType(Primitive.Bool)

            # Bitwise (&, |, ^)
            elif op_type in (ast.BitAnd, ast.BitOr, ast.BitXor):
                return PrimitiveType(Primitive.Int64)

            # Bitshifts (>>, <<)
            elif op_type in (ast.RShift, ast.LShift):
                return PrimitiveType(Primitive.Int64)

            self._error(node, f"unsupported Binary Op type: {op_type}")

            return PrimitiveType(Primitive.Invalid)
        elif isinstance(node, ast.Compare): # x < y < z
            return PrimitiveType(Primitive.Bool)
        elif isinstance(node, ast.BoolOp): # and, or
            return PrimitiveType(Primitive.Bool)
        elif isinstance(node, ast.UnaryOp):
            operand_type = self._deduce_expr_type(node.operand)

            if operand_type == PrimitiveType(Primitive.Invalid): 
                self._error = True
                return PrimitiveType(Primitive.Invalid)

            op_type = type(node.op)

            if op_type is ast.Not:
                return PrimitiveType(Primitive.Bool)

            if op_type is ast.UAdd:
                return operand_type # +x

            if op_type is ast.USub: # -x
                if operand_type == PrimitiveType(Primitive.Int64):
                    return PrimitiveType(Primitive.Int64)

                if operand_type == PrimitiveType(Primitive.Float64):
                    return PrimitiveType(Primitive.Float64)

            if op_type is ast.Invert: # ~x (bitwise not)
                 if operand_type == PrimitiveType(Primitive.Int64):
                     return PrimitiveType(Primitive.Int64)

            self._error = True

            return PrimitiveType(Primitive.Invalid)
        elif isinstance(node, ast.Subscript):
            # arr[i]
            if isinstance(node.value, ast.Name):
                sym = self._symbol_table.resolve_symbol(node.value.id)

                if sym is None or not isinstance(sym, (Variable, Parameter)):
                    self._error(node, f"invalid subscript op (symbol: {sym})")
                    return PrimitiveType(Primitive.Invalid)

                sym_type = sym.type

                if not isinstance(sym_type, ArrayType):
                    self._error(node, f"invalid subscript op on {sym_type} (symbol must be an array)")
                    return PrimitiveType(Primitive.Invalid)

                return sym_type.element_type
        elif isinstance(node, ast.List):
            # [1, 2, 3]
            if not node.elts: 
                self._error(node, "empty list are not supported")
                return PrimitiveType(Primitive.Invalid)
            
            element_types = { self._deduce_expr_type(e) for e in node.elts }

            if PrimitiveType(Primitive.Invalid) in element_types and len(element_types) > 1:
                self._error(node, "Invalid type in list")
                return PrimitiveType(Primitive.Invalid)

            if len(element_types) == 1:
                sole_type = element_types.pop()
                return sole_type

            self._error(node, "mixed-types list are not supported")

            return PrimitiveType(Primitive.Invalid) 
        elif isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name):
                func_name = node.func.id

                # TODO: refactor this to handle functions properly, each builtin function 
                # will need a specialization
                if is_builtin_function(func_name):
                    return get_builtin_function(func_name).func.return_type

                if func_name == "list":
                    if not node.args: 
                        self._error(node, "empty list are not supported")
                        return PrimitiveType(Primitive.Invalid)

                    arg_type = self._deduce_expr_type(node.args[0])

                    if arg_type == PrimitiveType(Primitive.Int64):
                        return ArrayType(Primitive.Int64)

                    if arg_type == PrimitiveType(Primitive.Float64):
                        return ArrayType(Primitive.Float64)

                    if arg_type == PrimitiveType(Primitive.Bool):
                        return ArrayType(Primitive.Bool)

                    self._error(node, f"unsupported type in list: {arg_type}")

                    return PrimitiveType(Primitive.Invalid)

                self._error(node, f"unsupported function: {func_name}")

                return PrimitiveType(Primitive.Invalid)
            elif isinstance(node.func, ast.Attribute):
                if isinstance(node.func.value, ast.Name):
                    func = node.func.value

                    func_name = func.id

                    self._error(node, f"unsupported function: {func_name}")

                    return PrimitiveType(Primitive.Invalid)
                else:

                    self._error(node, f"unsupported attribute: {type(node.func.value)}")

                    return PrimitiveType(Primitive.Invalid)

            print_ast_error(node, f"unsupported call: {type(node.func)}", self._source_code)

            return PrimitiveType(Primitive.Invalid)
        elif isinstance(node, ast.IfExp):
            if not isinstance(node.test, ast.Compare):
                self._error(node, f"unsupported IfExp test: {type(node.test)}")
                return PrimitiveType(Primitive.Invalid)

            if len(node.test.ops) > 1:
                self._error(node, f"IfExp does not support more than one comparison for now")
                return PrimitiveType(Primitive.Invalid)

            if_type = self._deduce_expr_type(node.body)
            else_type = self._deduce_expr_type(node.orelse)

            if if_type == PrimitiveType(Primitive.Invalid) or else_type == PrimitiveType(Primitive.Invalid):
                return PrimitiveType(Primitive.Invalid)
            elif if_type != else_type:
                self._error(node, f"IfExp has different types for if and else exprs: {if_type} or {else_type}")
                return PrimitiveType(Primitive.Invalid)
            else:
                return if_type
        
        self._error(node, f"unsupported expression type: {type(node)}")

        return PrimitiveType(Primitive.Invalid)

    def visit_AnnAssign(self, node: ast.AnnAssign):
        # Handles assignments with type annotations like x: int = 10
        if isinstance(node.target, ast.Name):
            var_name = node.target.id
            
            annotated_type = pytype_to_type(node.annotation)
            inferred_type = PrimitiveType(Primitive.Invalid)

            if node.value is not None:
                inferred_type = self._deduce_expr_type(node.value)

            chosen_type = PrimitiveType(Primitive.Invalid)

            if annotated_type != PrimitiveType(Primitive.Invalid):
                chosen_type = annotated_type

                if inferred_type != PrimitiveType(Primitive.Invalid) and annotated_type != inferred_type and inferred_type != PrimitiveType(Primitive.Void):
                    self._error(node, 
                                f"Inferred type {inferred_type} for \"{var_name}\" conflicts with annotated type {annotated_type}")
                    
                    chosen_type = PrimitiveType(Primitive.Invalid)

            elif inferred_type != PrimitiveType(Primitive.Invalid):
                chosen_type = inferred_type
            
            self._symbol_table.add_symbol(Variable(var_name, chosen_type))

        if node.value is not None:
            self.visit(node.value)

    def visit_Assign(self, node: ast.Assign):
        value_type = self._deduce_expr_type(node.value)

        for target in node.targets:
            if isinstance(target, ast.Name):
                var_name = target.id
                self._symbol_table.add_symbol(Variable(var_name, value_type))

            # TODO: Handle unpacking (x, y = some_tuple)
        
        self.visit(node.value)

    def visit_Return(self, node: ast.Return):
        # node.value is None if it's just "return"
        return_expr_type = self._deduce_expr_type(node.value) 

        self._return_types.append(return_expr_type)

        if node.value:
            self.visit(node.value)

    def visit_For(self, node: ast.For):
        if isinstance(node.iter, ast.Call):
            # for i in range(), list()
            if isinstance(node.iter.func, ast.Name):
                element_type = self._deduce_expr_type(node.iter.func)
        else:
            # for i in []
            # TODO: handle this better
            iter_expr_type = self._deduce_expr_type(node.iter)

            if not isinstance(iter_expr_type, ArrayType):
                self._error(node, f"expected list literal")
                return PrimitiveType(Primitive.Invalid)

            element_type = iter_expr_type.element_type

        if isinstance(node.target, ast.Name):
            self._symbol_table.add_symbol(Variable(node.target.id, element_type))
        else:
            self._error(node, "expected a symbol")
            return PrimitiveType(Primitive.Invalid)
        # TODO: Handle unpacking in for-loop target (for i, j in items:)
        
        self.visit(node.iter)

        for stmt_in_body in node.body:
            self.visit(stmt_in_body)

        for stmt_in_orelse in node.orelse:
            self.visit(stmt_in_orelse)

    def visit_Call(self, node: ast.Call):
        func = node.func

        if isinstance(func, ast.Name):
            if is_builtin_function(func.id):
                self._symbol_table.add_builtin_symbol(func.id)
            func.id

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

class ScopeType(enum.IntEnum):
    Module = 0
    Class = 1
    Function = 2
    Body = 3

@dataclass
class ScopeFrame():

    name: str
    type: ScopeType
    symbols: Dict[str, Symbol] = field(default_factory=dict)
    parent: "ScopeFrame" = field(default=None)
    children: List["ScopeFrame"] = field(default_factory=list)

    def print(self, depth: int, indent_size: int = 4) -> None:
        print(" " * indent_size * depth, end="")
        print(f"SCOPE \"{self.name}\" ({self.type.name})")

        for symbol in self.symbols.values():
            print(" " * indent_size * (depth + 1), end="")
            print(f"{symbol}")

        for child in self.children:
            child.print(depth + 1, indent_size)

class SymbolTable():
    
    def __init__(self, name: str = None) -> None:
        self._root = ScopeFrame(name if name is not None else "__module__", ScopeType.Module)
        self._current_scope = self._root

        self._builtins = set()

    def push_scope(self, name: str, scope_type: ScopeType) -> None:
        new_scope = ScopeFrame(name, scope_type, parent=self._current_scope)
        self._current_scope.children.append(new_scope)
        self._current_scope = new_scope

    def set_scope(self, name: str) -> Optional[ScopeType]:
        for scope in self._current_scope.children:
            if scope.name == name:
                self._current_scope = scope
                return scope.type
            
        return None

    def pop_scope(self) -> None:
        self._current_scope = self._current_scope.parent

    def add_symbol(self, symbol: Symbol) -> None:
        self._current_scope.symbols[symbol.name] = symbol

    def add_builtin_symbol(self, name: str) -> None:
        self._builtins.add(name)

    def resolve_symbol(self, name: str) -> Optional[Symbol]:
        scope = self._current_scope
        
        while scope is not None:
            if name in scope.symbols:
                return scope.symbols.get(name)

            scope = scope.parent

        return None

    def print(self, indent_size: int = 4) -> None:
        print("SYMBOL TABLE")

        if self._root is not None:
            self._root.print(0, indent_size)

    def collect_from_function(self, function_node: ast.FunctionDef, function_source_code: str = None) -> Optional[Type]:
        if not isinstance(function_node, ast.FunctionDef):
            print_ast_error(function_node, f"expected function definition, got: {type(function_node)}", function_source_code)
            return None

        visitor = SymbolTableVisitor(self, function_source_code)
        
        for stmt in function_node.body:
            visitor.visit(stmt)

        if visitor.has_error():
            return None

        hinted_return_type = PrimitiveType(Primitive.Invalid)

        # If "-> hint" is present
        if function_node.returns is not None:
            candidate_hint_type = pystrtype_to_type(function_node.returns.id)

            if candidate_hint_type != PrimitiveType(Primitive.Invalid):
                hinted_return_type = candidate_hint_type
        
        if hinted_return_type != PrimitiveType(Primitive.Invalid):
            return hinted_return_type

        # Infer from return statements if no valid annotation was found
        collected_return_statement_types = visitor.get_return_types()

        if len(collected_return_statement_types) == 0:
            # No return statements in the function
            return PrimitiveType(Primitive.Void)

        # A function might have "return 1" and "return None" and we do not support this for now, we need a single type
        unique_types = set(collected_return_statement_types)
        
        # If PrimitiveType(Primitive.Invalid) is the only one, or remains after filtering, it's an issue
        valid_unique_types = { t for t in unique_types if t != PrimitiveType(Primitive.Invalid) }

        if len(valid_unique_types) == 0:
            # All return expressions were uninferable or explicitly returned an invalidly typed expression.
            return PrimitiveType(Primitive.Invalid)
        
        if len(valid_unique_types) == 1:
            return valid_unique_types.pop()
        else:
            # Multiple distinct return types so we return Invalid because we expect a single return type
            sorted_types_str = sorted([t for t in valid_unique_types])

            print(f"Error: Function \"{function_node.name}\" has different return types: {', '.join(sorted_types_str)}")

            return PrimitiveType(Primitive.Invalid)

    def collect_from_file(self, body: ast.expr, source_code: str = None) -> bool:
        if not isinstance(body, ast.Module):
            return False

        visitor = SymbolTableVisitor(self, source_code)

        visitor.visit(body)
from __future__ import annotations

import ast
import enum

from dataclasses import dataclass, field
from typing import Optional, List, Set, Dict
from collections import defaultdict

from ._type import *
from ._symbols import *
from ._builtin import get_builtin_functions, get_builtin_function_specialization
from ._log import print_ast_error, print_ast_info

class SymbolTable:
    pass

class SymbolTableVisitor(ast.NodeVisitor):

    def __init__(self, symbol_table: SymbolTable, source_code: str = None) -> None:
        self._symbol_table = symbol_table
        self._return_types = list()
        self._source_code = source_code
        self._has_error = False

    # Error and logging

    def has_error(self) -> bool:
        return self._has_error

    def _error(self, node: ast.expr, message: str) -> None:
        self._has_error = True

        print_ast_error(node, message, self._source_code)

    def _info(self, node: ast.expr, message: str) -> None:
        print_ast_info(node, message, self._source_code)

    def get_return_types(self) -> List[Type]:
        return self._return_types

    def _deduce_expr_type(self, node: Optional[ast.expr]) -> Type:
        if node is None: # Return statement without a value
            return TypeVoid

        if isinstance(node, ast.Constant):
            if isinstance(node.value, int): 
                return TypeInt64

            if isinstance(node.value, float):
                return TypeFloat64

            if isinstance(node.value, bool):
                return TypeBool

            if isinstance(node.value, str):
                return TypeString

            if isinstance(node.value, bytes):
                return TypeBytes

            if node.value is None:
                return TypeVoid

        elif isinstance(node, ast.Name):
            symbol = self._symbol_table.resolve_symbol(node.id)

            if symbol is not None and hasattr(symbol, "type"):
                sym_type = symbol.type
                return sym_type

            self._error(node, f"Cannot find symbol: {symbol}")

            return TypeInvalid # Symbol not found or has no type
        elif isinstance(node, ast.BinOp):
            left_type = self._deduce_expr_type(node.left)
            right_type = self._deduce_expr_type(node.right)

            if left_type == TypeInvalid or right_type == TypeInvalid: 
                self._error(node, f"Invalid binary Op")
                return TypeInvalid
            
            # Arithmetic operations
            op_type = type(node.op)

            if op_type in (ast.Add, ast.Sub, ast.Mult):
                if left_type == TypeFloat64 or right_type == TypeFloat64: 
                    return TypeFloat64

                if left_type == TypeInt64 and right_type == TypeInt64:
                    return TypeInt64

            # True division /
            elif op_type is ast.Div:
                if left_type in (TypeInt64, TypeFloat64) and right_type in (TypeInt64, TypeFloat64):
                    return TypeFloat64

            # Floor division //
            elif op_type is ast.FloorDiv: 
                if (left_type == TypeInt64 and right_type == TypeInt64) or \
                   (left_type == TypeFloat64 and right_type == TypeFloat64) or \
                   (left_type == TypeFloat64 and right_type == TypeInt64) or \
                   (left_type == TypeInt64 and right_type == TypeFloat64): 
                       return TypeFloat64 if TypeFloat64 in (left_type, right_type) else TypeInt64

            # % and ** (simplified)
            elif op_type in (ast.Mod, ast.Pow):
                if left_type == TypeFloat64 or right_type == TypeFloat64:
                    return TypeFloat64

                if left_type == TypeInt64 and right_type == TypeInt64:
                    return TypeInt64
            
            # Comparisons
            elif op_type in (ast.Eq, ast.NotEq, ast.Lt, ast.LtE, ast.Gt, ast.GtE):
                return TypeBool

            # Bitwise (&, |, ^)
            elif op_type in (ast.BitAnd, ast.BitOr, ast.BitXor):
                return TypeInt64

            # Bitshifts (>>, <<)
            elif op_type in (ast.RShift, ast.LShift):
                return TypeInt64

            self._error(node, f"unsupported Binary Op type: {op_type}")

            return TypeInvalid
        elif isinstance(node, ast.Compare): # x < y < z
            return TypeBool
        elif isinstance(node, ast.BoolOp): # and, or
            return TypeBool
        elif isinstance(node, ast.UnaryOp):
            operand_type = self._deduce_expr_type(node.operand)

            if operand_type == TypeInvalid: 
                self._error = True
                return TypeInvalid

            op_type = type(node.op)

            if op_type is ast.Not:
                return TypeBool

            if op_type is ast.UAdd:
                return operand_type # +x

            if op_type is ast.USub: # -x
                if operand_type == TypeInt64:
                    return TypeInt64

                if operand_type == TypeFloat64:
                    return TypeFloat64

            if op_type is ast.Invert: # ~x (bitwise not)
                 if operand_type == TypeInt64:
                     return TypeInt64

            self._error = True

            return TypeInvalid
        elif isinstance(node, ast.Subscript):
            # arr[i]
            if isinstance(node.value, ast.Name):
                sym = self._symbol_table.resolve_symbol(node.value.id)

                if sym is None or not isinstance(sym, (Variable, Parameter)):
                    self._error(node, f"invalid subscript op (symbol: {sym})")
                    return TypeInvalid

                sym_type = sym.type

                if not isinstance(sym_type, ArrayType):
                    self._error(node, f"invalid subscript op on {sym_type} (symbol must be an array)")
                    return TypeInvalid

                return sym_type.element_type
        elif isinstance(node, ast.List):
            # [1, 2, 3]
            if not node.elts: 
                self._error(node, "empty list are not supported")
                return TypeInvalid
            
            element_types = { self._deduce_expr_type(e) for e in node.elts }

            if TypeInvalid in element_types and len(element_types) > 1:
                self._error(node, "Invalid type in list")
                return TypeInvalid

            if len(element_types) == 1:
                sole_type = element_types.pop()
                return sole_type

            self._error(node, "mixed-types list are not supported")

            return TypeInvalid 
        elif isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name):
                # TODO: adapt to compile all functions, for now only builtins are supported
                func_name = node.func.id

                symbol = self._symbol_table.resolve_symbol(func_name)

                if not isinstance(symbol, FunctionBuiltin):
                    self._error(node, f"unsupported function in call: {func_name}")
                    return TypeInvalid

                arg_types = [self._deduce_expr_type(arg) for arg in node.args]

                if any(arg_type == TypeInvalid for arg_type in arg_types):
                    return TypeInvalid

                func_type = get_builtin_function_specialization(func_name, arg_types)

                if func_type.mangled_name() in symbol.specializations:
                    return func_type.return_type

                symbol.specializations[func_type.mangled_name()] = func_type

                self._info(node.func, f"compiling specialization: {func_type.name}({','.join(str(arg) for arg in arg_types)})")

                return func_type.return_type
            elif isinstance(node.func, ast.Attribute):
                if isinstance(node.func.value, ast.Name):
                    func = node.func.value

                    func_name = func.id

                    self._error(node, f"unsupported function: {func_name}")

                    return TypeInvalid
                else:

                    self._error(node, f"unsupported attribute: {type(node.func.value)}")

                    return TypeInvalid

            print_ast_error(node, f"unsupported call: {type(node.func)}", self._source_code)

            return TypeInvalid
        elif isinstance(node, ast.IfExp):
            if not isinstance(node.test, ast.Compare):
                self._error(node, f"unsupported IfExp test: {type(node.test)}")
                return TypeInvalid

            if len(node.test.ops) > 1:
                self._error(node, f"IfExp does not support more than one comparison for now")
                return TypeInvalid

            if_type = self._deduce_expr_type(node.body)
            else_type = self._deduce_expr_type(node.orelse)

            if if_type == TypeInvalid or else_type == TypeInvalid:
                return TypeInvalid
            elif if_type != else_type:
                self._error(node, f"IfExp has different types for if and else exprs: {if_type} or {else_type}")
                return TypeInvalid
            else:
                return if_type
        
        self._error(node, f"unsupported expression type: {type(node)}")

        return TypeInvalid

    def visit_AnnAssign(self, node: ast.AnnAssign):
        # Handles assignments with type annotations like x: int = 10
        if isinstance(node.target, ast.Name):
            var_name = node.target.id
            
            annotated_type = pytype_to_type(node.annotation)
            inferred_type = TypeInvalid

            if node.value is not None:
                inferred_type = self._deduce_expr_type(node.value)

            chosen_type = TypeInvalid

            if annotated_type != TypeInvalid:
                chosen_type = annotated_type

                if inferred_type != TypeInvalid and annotated_type != inferred_type and inferred_type != TypeVoid:
                    self._error(node, 
                                f"Inferred type {inferred_type} for \"{var_name}\" conflicts with annotated type {annotated_type}")
                    
                    chosen_type = TypeInvalid

            elif inferred_type != TypeInvalid:
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

    def visit_AugAssign(self, node: ast.AugAssign):
        target_symbol = self._symbol_table.resolve_symbol(node.target.id)
        target_type = target_symbol.type

        value_type = self._deduce_expr_type(node.value)

        if target_type != value_type:
            target_symbol.type = value_type

    def visit_Return(self, node: ast.Return):
        # node.value is None if it's just "return"
        return_expr_type = self._deduce_expr_type(node.value) 

        self._return_types.append(return_expr_type)

        if node.value:
            self.visit(node.value)

    def visit_For(self, node: ast.For):
        if isinstance(node.iter, ast.Call):
            # for i in range(), list()
            element_type = self._deduce_expr_type(node.iter)
        else:
            # for i in []
            # TODO: handle this better
            iter_expr_type = self._deduce_expr_type(node.iter)

            if not isinstance(iter_expr_type, ArrayType):
                self._error(node, f"expected list literal")
                return TypeInvalid

            element_type = iter_expr_type.element_type

        if isinstance(node.target, ast.Name):
            self._symbol_table.add_symbol(Variable(node.target.id, element_type))
        else:
            self._error(node, "expected a symbol")
            return TypeInvalid
        # TODO: Handle unpacking in for-loop target (for i, j in items:)
        
        self.visit(node.iter)

        for stmt_in_body in node.body:
            self.visit(stmt_in_body)

        for stmt_in_orelse in node.orelse:
            self.visit(stmt_in_orelse)

    def visit_Call(self, node: ast.Call):
        func = node.func

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

        self._builtins = dict()

        for name, func in get_builtin_functions().items():
            self._builtins[name] = func

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

    def resolve_symbol(self, name: str) -> Optional[Symbol]:
        if name in self._builtins:
            return self._builtins[name]

        scope = self._current_scope
        
        while scope is not None:
            if name in scope.symbols:
                return scope.symbols.get(name)

            scope = scope.parent

        return None

    def get_builtin_specializations(self) -> Dict[str, List[FunctionType]]:
        specializations = defaultdict(list)

        for name, builtin in self._builtins.items():
            if isinstance(builtin, FunctionBuiltin):
                if len(builtin.specializations) > 0:
                    for _, specialization in builtin.specializations.items():
                        specializations[name].append(specialization)

        return specializations

    def print(self, indent_size: int = 4) -> None:
        print("SYMBOL TABLE")

        if self._root is not None:
            self._root.print(0, indent_size)

        for _, specialization in self.get_builtin_specializations().items():
            print("BUILTIN SPECIALIZATION:", specialization)

    def collect_from_function(self, function_node: ast.FunctionDef, function_source_code: str = None) -> Optional[Type]:
        if not isinstance(function_node, ast.FunctionDef):
            print_ast_error(function_node, f"expected function definition, got: {type(function_node)}", function_source_code)
            return None

        visitor = SymbolTableVisitor(self, function_source_code)
        
        for stmt in function_node.body:
            visitor.visit(stmt)

        if visitor.has_error():
            return None

        hinted_return_type = TypeInvalid

        # If "-> hint" is present
        if function_node.returns is not None:
            candidate_hint_type = pystrtype_to_type(function_node.returns.id)

            if candidate_hint_type != TypeInvalid:
                hinted_return_type = candidate_hint_type
        
        if hinted_return_type != TypeInvalid:
            return hinted_return_type

        # Infer from return statements if no valid annotation was found
        collected_return_statement_types = visitor.get_return_types()

        if len(collected_return_statement_types) == 0:
            # No return statements in the function
            return TypeVoid

        # A function might have "return 1" and "return None" and we do not support this for now, we need a single type
        unique_types = set(collected_return_statement_types)
        
        # If TypeInvalid is the only one, or remains after filtering, it's an issue
        valid_unique_types = { t for t in unique_types if t != TypeInvalid }

        if len(valid_unique_types) == 0:
            # All return expressions were uninferable or explicitly returned an invalidly typed expression.
            return TypeInvalid
        
        if len(valid_unique_types) == 1:
            return valid_unique_types.pop()
        else:
            # Multiple distinct return types so we return Invalid because we expect a single return type
            sorted_types_str = sorted([t for t in valid_unique_types])

            print(f"Error: Function \"{function_node.name}\" has different return types: {', '.join(sorted_types_str)}")

            return TypeInvalid

    def collect_from_file(self, body: ast.expr, source_code: str = None) -> bool:
        if not isinstance(body, ast.Module):
            return False

        visitor = SymbolTableVisitor(self, source_code)

        visitor.visit(body)
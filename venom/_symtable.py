import ast

from typing import Optional, List, Set

from ._type import Type, type_to_string, pytype_to_type, pyast_annotation_to_string
from ._error import print_ast_error

class SymbolTable:
    pass

class SymbolTableVisitor(ast.NodeVisitor):

    def __init__(self, symbol_table: SymbolTable, source_code: str = None) -> None:
        self._symbol_table = symbol_table
        self._return_types = list()
        self._source_code = source_code

    def get_return_types(self) -> List[Type]:
        return self._return_types

    def _deduce_expr_type(self, node: Optional[ast.expr]) -> Type:
        if node is None: # Return statement without a value
            return Type.Void

        if isinstance(node, ast.Constant):
            if isinstance(node.value, int): 
                return Type.Int

            if isinstance(node.value, float):
                return Type.Float

            if isinstance(node.value, bool):
                return Type.Bool

            if isinstance(node.value, str):
                return Type.Str

            if isinstance(node.value, bytes):
                return Type.Bytes

            if node.value is None:
                return Type.Void
        elif isinstance(node, ast.Name):
            symbol = self._symbol_table.get_symbol(node.id)

            if symbol and hasattr(symbol, "get_type"):
                sym_type = symbol.get_type()
                return sym_type

            return Type.Invalid # Symbol not found or has no type
        elif isinstance(node, ast.BinOp):
            left_type = self._deduce_expr_type(node.left)
            right_type = self._deduce_expr_type(node.right)

            if left_type == Type.Invalid or right_type == Type.Invalid: 
                return Type.Invalid
            
            # Arithmetic operations
            op_type = type(node.op)

            if op_type in (ast.Add, ast.Sub, ast.Mult):
                if left_type == Type.Float or right_type == Type.Float: 
                    return Type.Float

                if left_type == Type.Int and right_type == Type.Int:
                    return Type.Int

                if left_type == Type.Str and right_type == Type.Str and op_type is ast.Add:
                    return Type.Str

                if left_type == Type.Bytes and right_type == Type.Bytes and op_type is ast.Add:
                    return Type.Bytes

            # True division /
            elif op_type is ast.Div:
                if left_type in (Type.Int, Type.Float) and right_type in (Type.Int, Type.Float):
                    return Type.Float

            # Floor division //
            elif op_type is ast.FloorDiv: 
                if (left_type == Type.Int and right_type == Type.Int) or \
                   (left_type == Type.Float and right_type == Type.Float) or \
                   (left_type == Type.Float and right_type == Type.Int) or \
                   (left_type == Type.Int and right_type == Type.Float): 
                       return Type.Float if Type.Float in (left_type, right_type) else Type.Int

            # % and ** (simplified)
            elif op_type in (ast.Mod, ast.Pow):
                if left_type == Type.Float or right_type == Type.Float:
                    return Type.Float

                if left_type == Type.Int and right_type == Type.Int:
                    return Type.Int
            
            # Comparisons
            elif op_type in (ast.Eq, ast.NotEq, ast.Lt, ast.LtE, ast.Gt, ast.GtE):
                return Type.Bool

            # Bitwise (&, |, ^)
            elif op_type in (ast.BitAnd, ast.BitOr, ast.BitXor):
                return Type.Int

            # Bitshifts (<<, >>)
            elif op_type in (ast.RShift, ast.LShift):
                return Type.Int

            print_ast_error(node, f"unsupported Binary Op type: {op_type}", self._source_code)

            return Type.Invalid
        elif isinstance(node, ast.Compare): # x < y < z
            return Type.Bool
        elif isinstance(node, ast.BoolOp): # and, or
            return Type.Bool
        elif isinstance(node, ast.UnaryOp):
            operand_type = self._deduce_expr_type(node.operand)

            if operand_type == Type.Invalid: 
                return Type.Invalid

            op_type = type(node.op)

            if op_type is ast.Not:
                return Type.Bool

            if op_type is ast.UAdd:
                return operand_type # +x

            if op_type is ast.USub: # -x
                if operand_type == Type.Int:
                    return Type.Int

                if operand_type == Type.Float:
                    return Type.Float

            if op_type is ast.Invert: # ~x (bitwise not)
                 if operand_type == Type.Int:
                     return Type.Int

            return Type.Invalid
        elif isinstance(node, ast.Subscript):
            # arr[i]
            if isinstance(node.value, ast.Name):
                sym = self._symbol_table.get_symbol(node.value.id)

                if sym is None or not isinstance(sym, (Variable, Parameter)):
                    return Type.Invalid

                sym_type = sym.get_type()

                if sym_type == Type.ListInt:
                    return Type.Int
                elif sym_type == Type.ListFloat:
                    return Type.Float
                elif sym_type == Type.ListBool:
                    return Type.Bool
                else:
                    return Type.Invalid
        elif isinstance(node, ast.List):
            # [1, 2, 3]
            if not node.elts: 
                print_ast_error(node, "empty list are not supported", self._source_code)
                return Type.Invalid
            
            element_types = { self._deduce_expr_type(e) for e in node.elts }

            if Type.Invalid in element_types and len(element_types) > 1:
                return Type.Invalid

            if len(element_types) == 1:
                sole_type = element_types.pop()

                if sole_type == Type.Int: 
                    return Type.ListInt

                if sole_type == Type.Float:
                    return Type.ListFloat

                if sole_type == Type.Bool:
                    return Type.ListBool

            print_ast_error(node, "mixed-types list are not supported", self._source_code)

            return Type.Invalid 
        elif isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name):
                func_name = node.func.id

                if func_name == "int":
                    return Type.Int

                if func_name == "float":
                    return Type.Float

                if func_name == "str":
                    return Type.Str

                if func_name == "bool":
                    return Type.Bool

                if func_name == "bytes":
                    return Type.Bytes

                if func_name == "list":
                    if not node.args: 
                        print_ast_error(node, "empty list are not supported", self._source_code)
                        return Type.Invalid

                    arg_type = self._deduce_expr_type(node.args[0])

                    if arg_type == Type.ListInt:
                        return Type.ListInt

                    if arg_type == Type.ListFloat:
                        return Type.ListFloat

                    if arg_type == Type.ListBool:
                        return Type.ListBool

                    return Type.Invalid

                if func_name == "len":
                    return Type.Int

                print_ast_error(node, f"unsupported function: {func_name}", self._source_code)

                return Type.Invalid
            elif isinstance(node.func, ast.Attribute):
                if isinstance(node.func.value, ast.Name):
                    func = node.func.value

                    func_name = func.id

                    print_ast_error(node, f"unsupported function: {func_name}", self._source_code)

                    return Type.Invalid
                else:

                    print_ast_error(node, f"unsupported attribute: {type(node.func.value)}", self._source_code)

                    return Type.Invalid

            print_ast_error(node, f"unsupported call: {type(node.func)}", self._source_code)

            return Type.Invalid
        elif isinstance(node, ast.IfExp):
            if_type = self._deduce_expr_type(node.body)
            else_type = self._deduce_expr_type(node.orelse)

            if if_type == Type.Invalid or else_type == Type.Invalid:
                return Type.Invalid
            elif if_type != else_type:
                print_ast_error(node, f"IfExp has different types for if and else exprs: {type_to_string(if_type)} or {type_to_string(else_type)}")
                return Type.Invalid
            else:
                return if_type
        
        print_ast_error(node, f"unsupported expression type: {type(node)}", self._source_code)

        return Type.Invalid

    def visit_AnnAssign(self, node: ast.AnnAssign):
        # Handles assignments with type annotations like x: int = 10
        if isinstance(node.target, ast.Name):
            var_name = node.target.id
            
            annotated_type = Type.Invalid
            type_str_hint = pyast_annotation_to_string(node.annotation)

            if type_str_hint:
                annotated_type = pytype_to_type(type_str_hint)

            inferred_type = Type.Invalid

            if node.value is not None:
                inferred_type = self._deduce_expr_type(node.value)

            chosen_type = Type.Invalid

            if annotated_type != Type.Invalid:
                chosen_type = annotated_type

                if inferred_type != Type.Invalid and annotated_type != inferred_type and inferred_type != Type.Void:
                    print_ast_error(node, 
                                    f"Inferred type {type_to_string(inferred_type)} for \"{var_name}\" conflicts with annotated type {type_to_string(annotated_type)}",
                                    self._source_code)
                    
                    chosen_type = Type.Invalid

            elif inferred_type != Type.Invalid:
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
            # for i in range()
            if isinstance(node.iter.func, ast.Name) and node.iter.func.id == "range":
                element_type = Type.Int
        else:
            # for i in []
            iter_expr_type = self._deduce_expr_type(node.iter)
            element_type = Type.Invalid

            if iter_expr_type == Type.ListInt: 
                element_type = Type.Int
            elif iter_expr_type == Type.ListFloat:
                element_type = Type.Float
            elif iter_expr_type == Type.ListBool:
                element_type = Type.Bool
            elif iter_expr_type == Type.Str:
                element_type = Type.Str
            elif iter_expr_type == Type.Bytes:
                element_type = Type.Int # TODO: add a char type

        if isinstance(node.target, ast.Name):
            self._symbol_table.add_symbol(Variable(node.target.id, element_type))
        # TODO: Handle unpacking in for-loop target (for i, j in items:)
        
        self.visit(node.iter)

        for stmt_in_body in node.body:
            self.visit(stmt_in_body)

        for stmt_in_orelse in node.orelse:
            self.visit(stmt_in_orelse)

class Symbol():
    
    def __init__(self, name: str) -> None:
        self._name = name

    def get_name(self) -> str:
        return self._name

class Variable(Symbol):
    
    def __init__(self, name: str, type: Type = None) -> None:
        super().__init__(name)

        self._type = type

    def get_type(self) -> Optional[Type]:
        return self._type

    def set_type(self, type: Type) -> None:
        self._type = type

class Parameter(Symbol):
    
    def __init__(self, name: str, type: Type = None) -> None:
        super().__init__(name)

        self._type = type

    def get_type(self) -> Optional[Type]:
        return self._type

class SymbolTable():
    
    def __init__(self) -> None:
        self._symbols = dict()

    def add_symbol(self, symbol: Symbol) -> None:
        self._symbols[symbol.get_name()] = symbol

    def get_symbol(self, name: str) -> Optional[Symbol]:
        return self._symbols.get(name)

    def print(self, indent_size: int = 4) -> None:
        print("SYMBOL TABLE")

        for symbol in self._symbols.values():
            print(" " * indent_size + "-" + f" {symbol.get_name()}", end="")

            if isinstance(symbol, (Variable, Parameter)):
                type = symbol.get_type()

                print(f" ({type_to_string(type)})", end="")

            print()

    def collect_from_function(self, function_node: ast.FunctionDef, function_source_code: str = None) -> Type:
        if not isinstance(function_node, ast.FunctionDef):
            return Type.Invalid

        visitor = SymbolTableVisitor(self, function_source_code)
        
        for stmt in function_node.body:
            visitor.visit(stmt)

        hinted_return_type = Type.Invalid

        # If "-> hint" is present
        if function_node.returns:
            type_str_hint = pyast_annotation_to_string(function_node.returns)

            if type_str_hint:
                candidate_hint_type = pytype_to_type(type_str_hint)

                if candidate_hint_type != Type.Invalid:
                    hinted_return_type = candidate_hint_type
        
        if hinted_return_type != Type.Invalid:
            return hinted_return_type

        # Infer from return statements if no valid annotation was found
        collected_return_statement_types = visitor.get_return_types()

        if len(collected_return_statement_types) == 0:
            # No return statements in the function
            return Type.Void

        # A function might have "return 1" and "return None" and we do not support this for now, we need a single type
        unique_types = set(collected_return_statement_types)
        
        # If Type.Invalid is the only one, or remains after filtering, it's an issue
        valid_unique_types = { t for t in unique_types if t != Type.Invalid }

        if len(valid_unique_types) == 0:
            # All return expressions were uninferable or explicitly returned an invalidly typed expression.
            return Type.Invalid
        
        if len(valid_unique_types) == 1:
            return valid_unique_types.pop()
        else:
            # Multiple distinct return types so we return Invalid because we expect a single return type
            sorted_types_str = sorted([type_to_string(t) for t in valid_unique_types])

            print(f"Error: Function \"{function_node.name}\" has different return types: {', '.join(sorted_types_str)}")

            return Type.Invalid
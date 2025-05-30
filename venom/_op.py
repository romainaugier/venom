import ast
import enum

from typing import Optional, List, Union

class UnaryOpType(enum.IntEnum):
    Add = 0    # +x
    Sub = 1    # -x
    Not = 2    # not x
    Invert = 3 # ~x

_ast_unop_to_unop = {
    ast.UAdd: UnaryOpType.Add,
    ast.USub: UnaryOpType.Sub,
    ast.Not: UnaryOpType.Not,
    ast.Invert: UnaryOpType.Invert,
}

def ast_unop_to_unop(op: ast.UnaryOp) -> Optional[UnaryOpType]:
    op_type = type(op.op)

    return _ast_unop_to_unop.get(op_type)

class BinaryOpType(enum.IntEnum):
    Add = 0      # a + b
    Sub = 1      # a - b
    Mul = 2      # a * b
    Div = 3      # a / b
    FloorDiv = 4 # a // b
    Mod = 5      # a % b
    Pow = 6      # a ** b
    BitAnd = 7   # a & b
    BitOr = 8    # a | b
    BitXor = 9   # a ^ b 
    RShift = 10  # a >> b
    LShift = 11  # a << b

_ast_binop_to_binop = {
    ast.Add: BinaryOpType.Add,
    ast.Sub: BinaryOpType.Sub,
    ast.Mult: BinaryOpType.Mul,
    ast.Div: BinaryOpType.Div,
    ast.FloorDiv: BinaryOpType.FloorDiv,
    ast.Mod: BinaryOpType.Mod,
    ast.Pow: BinaryOpType.Pow,
    ast.BitAnd: BinaryOpType.BitAnd,
    ast.BitOr: BinaryOpType.BitOr,
    ast.BitXor: BinaryOpType.BitXor,
    ast.RShift: BinaryOpType.RShift,
    ast.LShift: BinaryOpType.LShift,
}

def ast_binop_to_binop(op: ast.BinOp) -> Optional[BinaryOpType]:
    op_type = type(op.op)

    return _ast_binop_to_binop.get(op_type)

_binop_to_string = {
    BinaryOpType.Add: "add",
    BinaryOpType.Sub: "sub",
    BinaryOpType.Mul: "mul",
    BinaryOpType.Div: "div",
    BinaryOpType.FloorDiv: "fdiv",
    BinaryOpType.Mod: "mod",
    BinaryOpType.Pow: "pow",
    BinaryOpType.BitAnd: "band",
    BinaryOpType.BitOr: "bor",
    BinaryOpType.BitXor: "bxor",
    BinaryOpType.RShift: "rsh",
    BinaryOpType.LShift: "lsh",
}

def binop_to_string(op: BinaryOpType) -> str:
    return _binop_to_string.get(op, "?")

class CompareOpType(enum.IntEnum):
    Eq = 0    # a == b
    NotEq = 1 # a != b
    Lt = 2    # a < b
    LtEq = 3  # a <= b
    Gt = 4    # a > b
    GtEq = 5  # a >= b

_ast_compareop_to_compareop = {
    ast.Eq: CompareOpType.Eq,
    ast.NotEq: CompareOpType.NotEq,
    ast.Lt: CompareOpType.Lt,
    ast.LtE: CompareOpType.LtEq,
    ast.Gt: CompareOpType.Gt,
    ast.GtE: CompareOpType.GtEq,
}

def ast_compareop_to_compareop(op: ast.Compare) -> Union[CompareOpType, List[CompareOpType]]:
    if len(op.ops) == 1:
        return _ast_compareop_to_compareop.get(type(op.ops[0]))
    else:
        return [_ast_compareop_to_compareop.get(type(o)) for o in op.ops]

_compareop_to_ir_string = {
    CompareOpType.Eq: "eq",
    CompareOpType.NotEq: "neq",
    CompareOpType.Lt: "lt",
    CompareOpType.LtEq: "lteq",
    CompareOpType.Gt: "gt",
    CompareOpType.GtEq: "gteq",
}

def compareop_to_ir_string(op: CompareOpType) -> str:
    return _compareop_to_ir_string.get(op, "?")

class BoolOp(enum.IntEnum):
    And = 0 # a and b
    Or = 1  # a or b
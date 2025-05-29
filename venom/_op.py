import ast
import enum

from typing import Optional

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
    BinaryOpType.Add: "+",
    BinaryOpType.Sub: "-",
    BinaryOpType.Mul: "*",
    BinaryOpType.Div: "/",
    BinaryOpType.FloorDiv: "//",
    BinaryOpType.Mod: "%",
    BinaryOpType.Pow: "**",
    BinaryOpType.BitAnd: "&",
    BinaryOpType.BitOr: "|",
    BinaryOpType.BitXor: "^",
    BinaryOpType.RShift: ">>",
    BinaryOpType.LShift: "<<",
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

class BoolOp(enum.IntEnum):
    And = 0 # a and b
    Or = 1  # a or b
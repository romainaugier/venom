import ast
import enum

class UnaryOpType(enum.IntEnum):
    Add = 0    # +x
    Sub = 1    # -x
    Not = 2    # not x
    Invert = 3 # ~x

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
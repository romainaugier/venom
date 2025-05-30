import enum
import ctypes
import ast
import hashlib

from dataclasses import dataclass
from typing import Any, Tuple, Union, Optional, Dict, List, get_args, get_origin

from ._error import print_generic_error

class Primitive(enum.IntEnum):
    Invalid = -1
    Void = 0
    Bool = 1
    Int64 = 2
    Int32 = 3
    Int16 = 4
    Int8 = 5
    Float64 = 6
    Float32 = 7

_primitive_to_ir_string = {
    Primitive.Void: "void",
    Primitive.Bool: "i32",
    Primitive.Int64: "i64",
    Primitive.Int32: "i32",
    Primitive.Int16: "i16",
    Primitive.Int8: "i8",
    Primitive.Float64: "f64",
    Primitive.Float32: "f32",
}

_primitive_to_ctypes = {
    Primitive.Void: None,
    Primitive.Bool: ctypes.c_int32,
    Primitive.Int64: ctypes.c_int64,
    Primitive.Int32: ctypes.c_int32,
    Primitive.Int16: ctypes.c_int16,
    Primitive.Int8: ctypes.c_int8,
    Primitive.Float64: ctypes.c_double,
    Primitive.Float32: ctypes.c_float,
}

@dataclass
class Type():

    def __repr__(self) -> str:
        return self.beautiful_repr()

    def __str__(self) -> str:
        return self.beautiful_repr()

    def beautiful_repr(self) -> str:
        raise NotImplementedError

    def ir_repr(self) -> str:
        raise NotImplementedError

    def to_ctypes(self) -> Any:
        raise NotImplementedError

@dataclass
class PrimitiveType(Type):

    type: Primitive

    def beautiful_repr(self) -> str:
        return self.type.name

    def ir_repr(self) -> str:
        return _primitive_to_ir_string.get(self.type, "???")

    def to_ctypes(self) -> Optional[Any]:
        return _primitive_to_ctypes.get(self.type, None)

@dataclass
class ArrayType(Type):

    element_type: Type
    size: Optional[int]  # None means dynamic size

    def beautiful_repr(self) -> str:
        size_str = "dynamic" if self.size is None else str(self.size)
        return f"Array({self.element_type.beautiful_repr()}, {size_str})"

    def ir_repr(self) -> str:
        if self.size is None:
            return f"{self.element_type.ir_repr()}*"
        return f"{self.element_type.ir_repr()}[{self.size}]"

@dataclass
class PointerType(Type):

    pointee_type: Type

    def beautiful_repr(self) -> str:
        return f"{self.pointee_type.beautiful_repr()}*"

    def ir_repr(self) -> str:
        return f"{self.pointee_type.ir_repr()}*"

@dataclass
class StructType(Type):

    name: str
    fields: Dict[str, Type]

    def beautiful_repr(self) -> str:
        field_strs = [f"{name}: {typ.beautiful_repr()}" for name, typ in self.fields.items()]
        return f"struct {self.name} {{ " + ", ".join(field_strs) + " }}"

    def ir_repr(self) -> str:
        # Example: { i32, i32 }
        field_strs = [typ.ir_repr() for typ in self.fields.values()]
        return f"{{ {', '.join(field_strs)} }}"

@dataclass
class FunctionType(Type):
    
    name: str
    args: Dict[str, Type]
    return_type: Optional[Type]

    def beautiful_repr(self) -> str:
        if self.return_type is None:
            return f"{self.name}({','.join(f'{t.beautiful_repr()} {name}' for name, t in self.args.items())})"
        else:
            return f"{self.name}({','.join(f'{t.beautiful_repr()} {name}' for name, t in self.args.items())}) -> {self.return_type.beautiful_repr()}"

    def ir_repr(self) -> str:
        if self.return_type is None:
            return f"{self.name}({','.join(f'{t.ir_repr()} {name}' for name, t in self.args.items())})"
        else:
            return f"{self.name}({','.join(f'{t.ir_repr()} {name}' for name, t in self.args.items())}) -> {self.return_type.ir_repr()}"

    def mangled_name(self) -> str:
        sig_hash = hashlib.md5(self.ir_repr().endswith()).hexdigest()

        return f"{self.name}__{sig_hash}"

# Utils

def types_from_function_args(args: Tuple[Any, ...]) -> Optional[List[Type]]:
        types = []

        for arg in args:
            t = pytype_to_type(type(arg))

            if t is None:
                return None
            
            types.append(t)

        return types

def type_rank(t: Type) -> int:
    if isinstance(t, PrimitiveType):
        if t.type in (Primitive.Float64, Primitive.Float32):
            return 3
        elif t.type in (Primitive.Int64, Primitive.Int32, Primitive.Int16, Primitive.Int8):
            return 2
        elif t.type == Primitive.Bool:
            return 1
    return 0

def type_to_ir_string(t: Type) -> str:
    return t.ir_repr()

def type_to_ctypes_type(t: Type) -> Any:
    if isinstance(t, PrimitiveType):
        return _primitive_to_ctypes.get(t.type)
    elif isinstance(t, PointerType):
        base = type_to_ctypes_type(t.pointee_type)
        return ctypes.POINTER(base) if base else None
    elif isinstance(t, ArrayType):
        base = type_to_ctypes_type(t.element_type)

        if base is None:
            return None

        if t.size is None:
            return ctypes.POINTER(base)

        return base * t.size

    return None

def pytype_to_type(py_type: Any) -> Optional[Type]:
    """
    """
    origin = get_origin(py_type)
    args = get_args(py_type)

    # Optional are treated as nullable pointers
    if origin is Optional:
        inner = args[0]
        inner_type = pytype_to_type(inner)

        # Avoid double wrapping: Optional[List[int]] → Array[...] (it's already pointer-like)
        if isinstance(inner_type, PointerType) or isinstance(inner_type, ArrayType):
            return inner_type

        return PointerType(inner_type)

    # Union are supported only if it's a Union[T, None], like Optional
    if origin is Union:
        non_none_args = [a for a in args if a is not type(None)]

        if len(non_none_args) == 1:
            return pytype_to_type(non_none_args[0])
        
        print_generic_error("Unions other than Optional[T] are not supported")

        return None

    # List[T] to array
    if origin is list or origin is List:

        if not args:
            print_generic_error("List[T] must specify an element type")
            return None

        element_type = pytype_to_type(args[0])

        return ArrayType(element_type=element_type, size=None)

    # Primitive Types
    if py_type is int:
        return PrimitiveType(Primitive.Int64)

    if py_type is float:
        return PrimitiveType(Primitive.Float64)

    if py_type is bool:
        return PrimitiveType(Primitive.Bool)

    if py_type is bytes:
        return PointerType(PrimitiveType(Primitive.Int8))

    if py_type is str:
        return PointerType(PrimitiveType(Primitive.Int16))

    if py_type is type(None):
        return PrimitiveType(Primitive.Void)

    print_generic_error(f"Unsupported Python type: {py_type}")

    return None
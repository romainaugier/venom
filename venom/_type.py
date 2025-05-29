import enum
import ctypes
import ast

from typing import Any, Optional

class Type(enum.IntEnum):
    Invalid = -1
    Void = 0
    Int = 1
    Float = 2
    Bool = 3
    Bytes = 4
    Str = 5

    ListInt = 6
    ListFloat = 7
    ListBool = 8

def type_to_string(type: Type) -> str:
    return type.name

_pytype_to_type = {
    "None": Type.Void,
    "int": Type.Int,
    "float": Type.Float,
    "bool": Type.Bool, 
    "bytes": Type.Bytes,
    "str": Type.Str,
    "List[int]": Type.ListInt,
    "List[float]": Type.ListFloat,
    "List[bool]": Type.ListBool,
}

def pytype_to_type(pytype: str) -> Type:
    pytype = pytype.lstrip("typing.")

    return _pytype_to_type.get(pytype, Type.Invalid)    

_type_to_ctypes_type = {
    Type.Void: None,
    Type.Int: ctypes.c_longlong,
    Type.Float: ctypes.c_double,
    Type.Bool: ctypes.c_long,
    Type.Bytes: ctypes.POINTER(ctypes.c_byte),
    Type.Str: ctypes.POINTER(ctypes.c_uint16),
    Type.ListInt: ctypes.POINTER(ctypes.c_longlong),
    Type.ListFloat: ctypes.POINTER(ctypes.c_double),
    Type.ListBool: ctypes.POINTER(ctypes.c_long),
}

def type_to_ctypes_type(type: Type) -> Any:
    return _type_to_ctypes_type.get(type)

def pyast_annotation_to_string(annotation_node: ast.expr) -> Optional[str]:
    if isinstance(annotation_node, ast.Name):
        return annotation_node.id

    if isinstance(annotation_node, ast.Constant) and isinstance(annotation_node.value, str):
        return annotation_node.value

    if isinstance(annotation_node, ast.Subscript):
        value_str = pyast_annotation_to_string(annotation_node.value)
        slice_node = annotation_node.slice
        actual_slice_node = getattr(slice_node, "value", slice_node)
        slice_str = pyast_annotation_to_string(actual_slice_node)

        if value_str and slice_str:
            return f"{value_str}[{slice_str}]"

    return None
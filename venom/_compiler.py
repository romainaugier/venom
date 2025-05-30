import ast
import ctypes
import hashlib
import inspect
import os

from typing import Dict, Any, Callable, Tuple, List, Optional

from ._type import *
from ._execmem import ExecMemory
from ._symtable import SymbolTable, Parameter, FunctionDef, ScopeType
from ._ir import IR

DEBUG = 1

class _JITFunc():
    
    def __init__(self, bytecode: bytes, argtypes: Tuple, restype: Any) -> None:
        self._exec_mem = ExecMemory(len(bytecode))
        self._exec_mem.write(bytecode)

        self._func_type = ctypes.CFUNCTYPE(restype, *argtypes)
        self._func = self._func_type(self._exec_mem.address())

    def __call__(self, *args):
        return self._func(*args)

class _JITFile():
    
    def __init__(self, code: str) -> None:
        
        self._code = code

class _JITCompiler():

    _cache: Dict[str, _JITFunc]

    def __init__(self) -> None:
        self._cache = dict()

    def _fix_source_indentation(self, source: str) -> str:
        i = 0

        while source[i] == ' ':
            i += 1

        lines = list()

        for line in source.splitlines():
            lines.append(line[i:])

        return '\n'.join(lines)

    def _get_type_signature(self, args: Tuple[Any, ...]) -> str:
        return str('_'.join(t.beautiful_repr() for t in types_from_function_args(args)))
    
    def jit_func(self, func: Callable, args: Tuple[Any, ...]) -> Optional[_JITFunc]:
        func_source = inspect.getsource(func)

        type_sig = self._get_type_signature(args)

        if type_sig is None:
            return None
        
        cache_key = hashlib.md5(f"{func_source}_{type_sig}".encode()).hexdigest()
        
        if cache_key in self._cache:
            return self._cache[cache_key]
        else:
            source = self._fix_source_indentation(inspect.getsource(func))
            tree = ast.parse(source)
            func_node = tree.body[0]

            if not isinstance(func_node, ast.FunctionDef):
                print(f"Error: cannot compile \"{type(func_node)}\", it is not a function definition")
                return None

            args = { arg.arg: t for arg, t in zip(func_node.args.args, types_from_function_args(args)) }

            func_type = FunctionType(func_node.name, args, None)

            symtable = SymbolTable("__jitmodule__")
            symtable.push_scope(func_node.name, ScopeType.Function)

            for name, type in args.items():
                symtable.add_symbol(Parameter(name, type))

            # Build the symtable and run semantic analysis for the jit function
            func_return_type = symtable.collect_from_function(func_node, source)

            if func_return_type is None:
                print(f"Error: error caught during parse of function \"{func.__name__}\", aborting jit-compilation")
                return None
            elif func_return_type == PrimitiveType(Primitive.Invalid):
                print(f"Error: cannot deduce return type for function \"{func.__name__}\", aborting jit-compilation")
                return None

            func_type.return_type = func_return_type

            # Back to module scope
            symtable.pop_scope()

            # Add the function to the module scope
            symtable.add_symbol(FunctionDef(func_node.name, None, func_node, list(args.keys()), { func_type.mangled_name(): func_type }))

            # Generate the Intermediate Representation
            ir = IR(symtable)
            ir.build(func_node)

            if DEBUG:
                print("SOURCE")
                print(source)

                print()

                symtable.print()

                print()
                ir.print()

                print()

            return None

    def jit_file(self, filepath: str) -> Optional[_JITFile]:
        """
        Compiles the given file, as it is currently being worked on the function will change

        Args:
            filepath (str): Path to the file that will be compiled

        Returns:
            Optional[_JITFile]: _JITFile if the compilation is successful, None otherwise
        """
        if not os.path.exists(filepath):
            return None

        with open(filepath, "r", encoding="utf-8") as file:
            source = file.read()

        tree = ast.parse(source)

        # Build the symtable for the jit module
        symtable = SymbolTable()
        symtable.collect_from_file(tree, source)

        symtable.print()
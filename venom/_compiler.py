import ast
import ctypes
import hashlib
import inspect
import os

from typing import Dict, Any, Callable, Tuple, List, Optional

from ._type import Type, pytype_to_type, pyast_annotation_to_string
from ._execmem import ExecMemory
from ._symtable import SymbolTable, Parameter, FunctionDef, ScopeType

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
    
    def _get_type_signature(self, args: Tuple[Any, ...]) -> List[Type]:
        types = []

        for arg in args:
            if isinstance(arg, int):
                types.append(Type.Int)
            elif isinstance(arg, float):
                types.append(Type.Float)
            elif isinstance(arg, list):
                if len(arg) == 0:
                    print("Error: Empty lists not supported")
                    return None

                first_type = type(arg[0])

                if not all(isinstance(x, first_type) for x in arg):
                    print("Error: Mixed-type lists not supported")
                    return None
                
                if isinstance(arg[0], int):
                    types.append(Type.ListInt)
                elif isinstance(arg[0], float):
                    types.append(Type.ListFloat)
                else:
                    print(f"Error: Unsupported list element type: {type(arg[0])}")
                    return None
            else:
                print(f"Error: Unsupported type: {type(arg)}")
                return None

        return types

    def _get_arg_types(self, func_node: ast.FunctionDef, arg_types: List[Type]) -> Dict[str, Type]:
        annotations = [arg.annotation for arg in func_node.args.args]

        for i, ann in enumerate(annotations):
            if ann is not None:
                str_ann = pyast_annotation_to_string(ann)

                if str_ann is not None:
                    arg_types[i] = pytype_to_type(str_ann)
        
        names = [arg.arg for arg in func_node.args.args]

        mapping = dict()

        for name, type in zip(names, arg_types):
            mapping[name] = type

        return mapping
    
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

            symtable = SymbolTable("__jitmodule__")

            symtable.push_scope(func_node.name, ScopeType.Function)

            param_names = list()

            for name, type in self._get_arg_types(func_node, type_sig).items():
                symtable.add_symbol(Parameter(name, type))

                param_names.append(name)

            # Build the symtable for the jit function
            func_return_type = symtable.collect_from_function(func_node, source)

            if func_return_type is None or func_return_type == Type.Invalid:
                print(f"Error: cannot deduce return type for function \"{func.__name__}\", aborting jit-compilation")
                return None

            # Back to module scope
            symtable.pop_scope()

            # Add the function to the module scope
            symtable.add_symbol(FunctionDef(func_node.name, func_node, param_names, func_return_type))

            symtable.print()

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
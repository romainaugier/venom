import functools
import os

from typing import Callable

from ._compiler import _JITCompiler

_compiler = _JITCompiler()

def jit(func: Callable) -> Callable:
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        if kwargs:
            print(f"Error: kwargs not supported for now, disabling jit-compilation for \"{func.__name__}\"")
            return func(*args, **kwargs)
        
        jit_func = _compiler.jit_func(func, args)

        if jit_func is not None:
            return jit_func(*args)
        
        print(f"Error: jit compilation failed for \"{func.__name__}\", check the log for more information")
        
        return func(*args, **kwargs)
    
    return wrapper

def compile_file(filepath: str) -> None:
    _compiler.jit_file(filepath)
# venom

venom is a tiny and simple just-in-time compiler for Python. For now, it only provides jit-compilation for function by using the `@venom.jit` decorator like so:
```py
@venom.jit
def add(a, b):
    return a + b
```

For each instance of the function, venom will compile a version (think like templates in C++) by infering types. If venom cannot fully infer types, an error will be raised and the function will be called as it is, without jit-compilation. Under the hood, venom compiles the function to assembly bytecode and calls it directly from Python.
```py
x = add(3.0, 1.0) # Would compile to
def add(a: float, b: float) -> float: return a + b
# Generated assembly (linux x64):
# add_f_f:
#     addsd xmm0, xmm1
#     ret

x = add(3.0, 4) # Would compile to
def add(a: float, b: int) -> float: return a + float(b)
# Generated assembly (linux x64):
# add_f_i:
#     cvtsi2sd xmm1, rdi
#     addsd xmm0, xmm1
#     ret

x = add(6, 4) # Would compile to
def add(a: int, b: int) -> int: return a + b
# Generated assembly (linux x64):
# add_i_i:
#     mov rax, rdi
#     add rax, rsi
#     ret
```

For now, only a very limited subset of Python is supported:
 - int, float, bool, List[int], List[float], List[bool]
 - Unary ops (-, ~)
 - Binary ops (+, -, *, /, //, %, **)
 - Bit ops (&, |, ^, <<, >>)
 - Ternary ops (x if cond else y)
 - Comparisons (==, !=, <, >, <=, =>)
 - Boolean ops (and, or, not)
 - For Loops with range

The long-term goal is to cover more and more Python features, incrementally, until it becomes a fully working optimizing compiler, along specialized libraries, especially for maths, statistics, and computationally-demanding tasks.

No dependencies are required and everything is written from scratch in pure Python.
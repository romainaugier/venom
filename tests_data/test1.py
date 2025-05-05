import typing

from math import pi as PI, cos as cosf
from math import sqrt

def fitRange(value: float, omin: float, omax: float, nmin: float, nmax: float) -> float:
    return nmin + ((nmax - nmin) / (omax - omin)) * (value - omin)

class Adder():
            
    def add(x: int, y: int) -> int:
        return x + y

def main() -> int:
    a = Adder.add(3, 18)
    b = Adder.add(3.0, 19)

    c = fitRange(4.0, 0.0, 4.0, 0.0, 10.0)

    return a + b + sqrt(c) + cosf(a * b)

if __name__ == "__main__":
    main()
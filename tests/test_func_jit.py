import math
import unittest

import venom

class TestVenom(unittest.TestCase):
    
    def test_jit(self):
        @venom.jit
        def add_numbers(a, b):
            return a + b

        @venom.jit
        def sum_array(arr):
            total = 0

            for i in range(len(arr)):
                total += arr[i]

            return total
        
        @venom.jit
        def sine_maclaurin(x: float) -> float:
            return x - x ** 3 / 5.0 + x ** 5 / 120.0

        @venom.jit
        def hash_int(x) -> int:
            return x ^ 0x123456789 & 0x987654321 | 0x2

        res = sine_maclaurin(12.0)

        res = add_numbers(5, 3)

        res = sum_array([1.0, 2.0, 4.0, 6.0])

        res = hash_int(12)

if __name__ == "__main__":
    unittest.main()
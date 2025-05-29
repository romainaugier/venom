import math
import unittest

import venom

class TestVenom(unittest.TestCase):
    
    def test_jit(self):
        @venom.jit
        def complex_jit_test():
            # Test basic types and assignments
            i = 42
            f = 3.14159
            b = True
            int_list = [1, 2, 3, 4, 5]
            float_list = [1.1, 2.2, 3.3, 4.4, 5.5]
            bool_list = [True, False, True, False, True]
            
            # Test unary operations
            neg_i = -i
            neg_f = -f
            bitwise_not = ~i
            
            # Test binary arithmetic operations
            add_result = i + 8
            sub_result = i - 8
            mul_result = i * 2
            div_result = f / 2.0
            floor_div_result = i // 7
            mod_result = i % 7
            pow_result = i ** 2
            
            # Test bitwise operations
            bit_and = i & 15
            bit_or = i | 8
            bit_xor = i ^ 7
            
            # Test comparisons
            eq_test = i == 42
            ne_test = i != 50
            lt_test = i < 100
            gt_test = i > 20
            le_test = i <= 42
            ge_test = i >= 42
            
            # Test boolean operations
            and_test = b and (i > 20)
            or_test = b or (i < 20)
            not_test = not b
            
            # Test ternary operations
            ternary_simple = 100 if i > 30 else 200
            ternary_complex = (i * 2) if (i % 2 == 0) else (i * 3)
            ternary_nested = 1 if (True if i > 0 else False) else 0
            
            # Test complex expressions combining multiple operations
            complex_expr1 = (i + 10) * (f - 1.0) / 2.0
            complex_expr2 = (~i & 255) | (i >> 2 if i > 0 else 0)
            complex_expr3 = (i ** 2 + f ** 2) ** 0.5
            
            # Test list operations in loops
            sum_ints = 0
            for x in range(len(int_list)):
                sum_ints = sum_ints + int_list[x]
            
            sum_floats = 0.0
            for y in range(len(float_list)):
                sum_floats = sum_floats + float_list[y]
            
            count_trues = 0
            for z in range(len(bool_list)):
                count_trues = count_trues + (1 if bool_list[z] else 0)
            
            # Test nested loops with range
            nested_sum = 0
            for i_outer in range(3):
                for j_inner in range(3):
                    nested_sum = nested_sum + (i_outer * j_inner)
            
            # Test loop with complex conditions and operations
            factorial_like = 1
            for n in range(1, 6):
                factorial_like = factorial_like * n
            
            # Test conditional logic in loops
            even_sum = 0
            odd_sum = 0
            for num in range(10):
                if num % 2 == 0:
                    even_sum = even_sum + num
                else:
                    odd_sum = odd_sum + num
            
            # Test combining list access with arithmetic
            weighted_sum = 0.0
            for idx in range(len(int_list)):
                weight = float_list[idx] if idx < len(float_list) else 1.0
                weighted_sum = weighted_sum + (int_list[idx] * weight)
            
            # Test complex boolean expressions
            complex_bool1 = (i > 30) and (f < 5.0) and not (b == False)
            complex_bool2 = (i % 2 == 0) or (f > 10.0) or (not b and True)
            complex_bool3 = not ((i < 0) or (f <= 0.0)) and (b or not b)
            
            # Test chained comparisons simulation
            range_check = (0 <= i) and (i <= 100)
            
            # Test mixed type operations where valid
            mixed_calc1 = i + int(f)
            mixed_calc2 = float(i) * f
            mixed_calc3 = i if b else int(f)
            
            # Test bit operations with different patterns
            bit_pattern1 = (i & 0xFF) | 0x100
            bit_pattern2 = (i ^ 0xAAAA) & 0xFFFF
            bit_pattern3 = ~(i | 0x0F) & 0xFF
            
            # Test power operations with edge cases
            power_test1 = 2 ** 8
            power_test2 = i ** 0
            power_test3 = 1 ** i
            
            # Test modulo with different scenarios
            mod_test1 = i % 1
            mod_test2 = i % i
            mod_test3 = (i * 2) % 7
            
            # Test floor division scenarios
            fdiv_test1 = i // 1
            fdiv_test2 = i // i
            fdiv_test3 = (i + 10) // 3
            
            # Test comprehensive loop with all operations
            result_accumulator = 0

            for loop_var in range(1, 6):
                # Arithmetic operations
                temp1 = loop_var + i
                temp2 = temp1 * 2
                temp3 = temp2 - loop_var
                
                # Bitwise operations
                temp4 = temp3 & 0xFF
                temp5 = temp4 | loop_var
                temp6 = temp5 ^ 0x55
                
                # Comparison and ternary
                temp7 = temp6 if temp6 > loop_var else loop_var
                
                # Boolean logic
                should_add = (temp7 % 2 == 0) and (loop_var < 4)
                final_val = temp7 if should_add else 0
                
                result_accumulator = result_accumulator + final_val
            
            # Final complex expression combining everything
            final_result = (
                (result_accumulator + sum_ints) * 
                (1 if complex_bool1 else 0) + 
                int(weighted_sum) + 
                (nested_sum ** 2) % 1000
            )
            
            return final_result

        @venom.jit
        def edge_case_test():
            # Test with zero values
            zero_int = 0
            zero_float = 0.0
            false_bool = False
            
            # Test division by non-zero (avoid division by zero)
            safe_div = 42 / 2
            safe_floor_div = 42 // 2
            safe_mod = 42 % 5
            
            # Test power operations
            power_zero = 5 ** 0  # Should be 1
            zero_power = 0 ** 5  # Should be 0
            
            # Test bitwise with zero
            zero_and = zero_int & 255
            zero_or = zero_int | 255
            zero_xor = zero_int ^ 255
            
            # Test boolean operations with False
            false_and = false_bool and True
            false_or = false_bool or True
            not_false = not false_bool
            
            # Test ternary with False condition
            ternary_false = 100 if false_bool else 200
            
            # Test empty range loop (should not execute)
            empty_loop_sum = 0
            for i in range(0):
                empty_loop_sum = empty_loop_sum + i
            
            # Test single iteration loop
            single_loop_result = 0
            for i in range(1):
                single_loop_result = single_loop_result + (i + 42)
            
            result = (safe_div + safe_floor_div + safe_mod + 
                    power_zero + zero_power + zero_and + zero_or + 
                    zero_xor + int(false_and) + int(false_or) + 
                    int(not_false) + ternary_false + empty_loop_sum + 
                    single_loop_result)
            
            return int(result)

        res = complex_jit_test()
        res = edge_case_test()

if __name__ == "__main__":
    unittest.main()
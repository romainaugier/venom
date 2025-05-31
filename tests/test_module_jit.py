import unittest
import os

import venom

class TestVenom(unittest.TestCase):
    
    def test_module_jit(self):
        pass
        # file_path = f"{os.path.dirname(__file__)}/data/test_module.py"

        # venom.compile_file(file_path)

if __name__ == "__main__":
    unittest.main()

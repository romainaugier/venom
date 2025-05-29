import ast
import sys

from typing import Optional

def print_ast_error(node: ast.expr, err: str, source_code: Optional[str] = None):
    """
    Print a beautiful error message showing where the error occurred in the AST
    
    Args:
        node: The AST node where the error occurred
        err: The error message to display
        source_code: Optional source code string. If not provided, will try to read from file_path
    """
    lineno = getattr(node, "lineno", None)
    col_offset = getattr(node, "col_offset", None)
    end_col_offset = getattr(node, "end_col_offset", None)
    
    if lineno is None:
        print(f"Error: {err}")
        return
    
    lines = list()

    if source_code is not None:
        lines = source_code.splitlines()
    else:
        print(f"Error: {err} (line {lineno})")
        return
    
    if lineno > len(lines) or lineno < 1:
        print(f"Error: {err} (line {lineno})")
        return
    
    error_line = lines[lineno - 1]
    
    location_info = f"line {lineno}"

    if col_offset is not None:
        location_info += f", column {col_offset + 1}"
    
    print(f"Error: {err} ({location_info})")
    print()
    
    line_num_width = len(str(lineno))
    print(f"{lineno:>{line_num_width}} | {error_line}")
    
    if col_offset is not None:
        prefix_len = line_num_width + 3
        
        tab_count = error_line[:col_offset].count('\t')
        adjusted_col = col_offset + (tab_count * 3)
        
        pointer_line = " " * (prefix_len + adjusted_col) + "^"
        
        if end_col_offset is not None and end_col_offset > col_offset:
            range_len = end_col_offset - col_offset - 1
            if range_len > 0:
                tab_count_in_range = error_line[col_offset:end_col_offset].count('\t')
                adjusted_range_len = range_len + (tab_count_in_range * 3)
                pointer_line += "~" * min(adjusted_range_len, 50)
        
        print(pointer_line)
    else:
        prefix_len = line_num_width + 3
        print(" " * prefix_len + "^")
    
    print()
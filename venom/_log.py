import ast
import sys

from typing import Optional

def print_generic_error(err: str) -> None:
    """
    Print a generic error message

    Args:
        err (str): error to print
    """
    print(f"Error: {err}")

def _print_ast(node: ast.expr, msg: str, source_code: Optional[str] = None) -> None:
    """
    Print a beautiful message showing where the event occurred in the AST
    
    Args:
        node (ast.expr): The AST node where the error occurred
        msg (str): The message to display
        source_code (Optional[str]): Source code string. If not provided, no details will be provided in the error message. Defaults to None.
    """
    lineno = getattr(node, "lineno", None)
    col_offset = getattr(node, "col_offset", None)
    end_col_offset = getattr(node, "end_col_offset", None)
    
    if lineno is None:
        print(msg)
        return
    
    lines = list()

    if source_code is not None:
        lines = source_code.splitlines()
    else:
        print(f"{msg} (line {lineno})")
        return
    
    if lineno > len(lines) or lineno < 1:
        print(f"{msg} (line {lineno})")
        return
    
    error_line = lines[lineno - 1]
    
    location_info = f"line {lineno}"

    if col_offset is not None:
        location_info += f", column {col_offset + 1}"
    
    print(f"{msg} ({location_info})")
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

def print_ast_info(node: ast.expr, info: str, source_code: Optional[str] = None) -> None:
    _print_ast(node, f"Info: {info}", source_code)

def print_ast_error(node: ast.expr, err: str, source_code: Optional[str] = None) -> None:
    _print_ast(node, f"Error: {err}", source_code)

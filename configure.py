import os
import sys
import re
import typing
import datetime
import shutil

LANGUAGE_PATTERN = re.compile(r"(CP?P?)(\d+)")

FORMATTERS = {
    "default": lambda s: s,
    "lower": lambda s: s.lower(),
    "upper": lambda s: s.upper(),
}

def build_variable_pattern(variable_name: str) -> re.Pattern:
    return re.compile(rf"!({variable_name})(\=[a-z]+)?!")

def replace_variables_in_file(file_path: str, variables: typing.List[typing.Tuple[re.Pattern, str]]) -> bool:
    if not os.path.exists(file_path):
        return False

    with open(file_path, "r", encoding="utf-8") as file:
        content = file.read()
        
    while True:
        any_match = False

        for pattern, value in variables:
            if (m := pattern.search(content)) is not None:
                fmt = m.group(2)

                if fmt is not None:
                    fmt = fmt.replace('=', '')

                new_value = FORMATTERS.get(fmt, lambda s: s)(value)

                content = content.replace(m.group(0), new_value)

                any_match = True

        if not any_match:
            break

    with open(file_path, "w", encoding="utf-8") as file:
        file.write(content)
        
    return True

def find_variables_in_path(file_path: str, variables: typing.List[typing.Tuple[re.Pattern, str]]) -> bool:
    if not os.path.exists(file_path):
        return False

    for pattern, _ in variables:
        if pattern.search(file_path) is not None:
            return True

    return False

def replace_variables_in_path(file_path: str, variables: typing.List[typing.Tuple[re.Pattern, str]]) -> bool:
    if not os.path.exists(file_path):
        return False

    original_file_path = file_path

    any_rename = False

    while True:
        any_match = False
        
        for pattern, value in variables:
            if (m := pattern.search(file_path)) is not None:
                fmt = m.group(2)

                if fmt is not None:
                    fmt = fmt.replace(':', '')

                new_value = FORMATTERS.get(fmt, lambda s: s)(value)

                file_path = file_path.replace(m.group(0), new_value)

                any_match = True
                any_rename = True

        if not any_match:
            break

    if any_rename:
        os.rename(original_file_path, file_path)

    return True

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python configure.py <project_name> <language_standard>")
        print("Exemple: python configure.py libromano C90")
        print("         python configure.py stdromano CPP11")

        sys.exit(1)

    project_name = sys.argv[1]
    lang_match = LANGUAGE_PATTERN.match(sys.argv[2])

    if lang_match is None:
        print(f"Invalid language version entered: {sys.argv[2]}")
        sys.exit(1)

    lang, lang_version = lang_match.groups()

    print(f"Configuring project")
    print(f"Name: {project_name}")
    print(f"Language: {lang}")
    print(f"Language Version: {lang_version}")

    variables = [
        (build_variable_pattern("PROJECT_NAME"), project_name),
        (build_variable_pattern("YEAR"), str(datetime.datetime.now().year)),
        (build_variable_pattern("AUTHOR"), "Romain Augier"),
        (build_variable_pattern("LANG_SOURCE_EXT"), lang.lower()),
        (build_variable_pattern("LANG_CMAKE_ALIAS"), "C" if lang == "C" else "CXX"),
        (build_variable_pattern("LANG_STANDARD"), str(lang_version)),
    ]

    git_dir = f"{os.curdir}/.git"
    
    if os.path.exists(git_dir):
        print("Removing .git directory")
        shutil.rmtree(git_dir)

    paths_to_replace = list()

    for root, dirs, files in os.walk(os.curdir):
        for file in files:
            path = os.path.join(root, file)

            if not replace_variables_in_file(path, variables):
                print(f"Error during content replacement of file: {path}")
                sys.exit(1)

    while True:
        any_change = False

        for root, dirs, files in os.walk(os.curdir):
            for dir in dirs:
                dir_path = os.path.join(root, dir)

                if find_variables_in_path(dir_path, variables):
                    any_change = True

                    if not replace_variables_in_path(dir_path, variables):
                        print(f"Error during path replacement of path: {dir_path}")
                        sys.exit(1)

        for root, dirs, files in os.walk(os.curdir):
            for file in files:
                file_path = os.path.join(root, file)

                if find_variables_in_path(file_path, variables):
                    any_change = True

                    if not replace_variables_in_path(file_path, variables):
                        print(f"Error during path replacement of path: {file_path}")
                        sys.exit(1)

        if not any_change:
            break

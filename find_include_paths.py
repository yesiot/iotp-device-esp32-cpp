#!/usr/bin/python

import sys
import re
import os.path

project_root_dir = sys.argv[1]
source_dir = sys.argv[1]
idf_dir = sys.argv[2]
search_dir = idf_dir + "/components/"


def find_all_by_name(name, path):
    result = []
    for root, dirs, files in os.walk(path):
        if name in files:
            result.append(os.path.join(root, name))
    return result

def find_all_by_extention(ext_list, path):
    result = []
    for root, dirs, files in os.walk(path):
        for file in files:
            if any(ext in file for ext in ext_list):
                result.append(os.path.join(root, file))

    return result

cmake_file = "CMakeLists.txt"
if not os.path.isfile(cmake_file):
    print("Error: cannot find {}".format(cmake_file))
    exit(-1)

project_name = ""
with open(cmake_file) as file:
    for line in file:
        if "project" in line:
            project_name = line[line.index('(') + 1 : line.index(')')]
            break

if project_name == "":
    print("Error: cant fide project name inside {}".format(cmake_file))

source_files = find_all_by_extention([".c", ".cpp", ".h", ".hpp"], source_dir)

reg_pattern_comp = re.compile(r"#include [\"|<](.*\.h)[\">]")
results = []

for source_file in source_files:
    with open(source_file) as file:
        for line in file:
            line = line.strip()
            result = re.match(reg_pattern_comp, line)
            if result:
                initial_path = result.group(1)
                file_name = os.path.basename(initial_path)
                all_path_list = find_all_by_name(file_name, search_dir)

                if(len == 0):
                    print("Error: cant find path for ", initial_path)
                    continue

                idx = 0
                if (len(all_path_list) > 1):
                    for i, file in enumerate(all_path_list):
                        print(i, file)

                    while True:
                        idx = input("Please select the header: ")
                        if idx >= 0 and idx < len(all_path_list):
                            break

                include_path = all_path_list[idx].replace(initial_path, "")
                if include_path not in results:
                    results.append(include_path)


print("set(SOURCES ")
for source_file in source_files:
    print("    {}".format(source_file))
print(")\n")

print("add_executable(dont_build_this ${SOURCES})")
for path in results:
    path = path.replace("/.", "/")
    print("target_include_directories(dont_build_this PUBLIC {})".format(path))

absolute_idf_path = os.path.abspath(idf_dir)
print("\nadd_custom_target({0} COMMAND make IDF_PATH={1} -C ${{{0}_SOURCE_DIR}} CLION_EXE_DIR=${{PROJECT_BINARY_DIR}})".format(project_name, absolute_idf_path))
print("add_custom_target({0}_Flash COMMAND make flash IDF_PATH={1} -C ${{{0}_SOURCE_DIR}} CLION_EXE_DIR=${{PROJECT_BINARY_DIR}})".format(project_name, absolute_idf_path))

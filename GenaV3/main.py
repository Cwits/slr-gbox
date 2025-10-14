#SPDX-FileCopyrightText: 2025 Cwits
#SPDX-License-Identifier: GPL-3.0-or-later

import sys
import re
from pathlib import Path
import os
import parser
import grammar
import generator as gena
import json
from paths import *

STATE_FILE = "file_state.json"

arg_include_re = re.compile(r'\#include\s\"(\S+?)\"', re.S)
include_del_re = r"^\s*#include.*\n?"

def get_time(filepath):
    return os.path.getmtime(filepath)

SRC_DIR = Path(LOOKUP_PATH)

module_files = []
#find files
for file in SRC_DIR.rglob("*Gen.h"):
	module_files.append(file)

print("Found files to parse: " + str(module_files))
print(" ")


files_to_parse = []
print("looking for updated files")
if not os.path.exists(STATE_FILE):
    #build from scratch
    state = {}

    for f in module_files:
        state[str(f)] = get_time(str(f))
        files_to_parse.append(f)
        #parse includes as well
        data = f.read_text()
        includes = arg_include_re.findall(data)
        for inc in includes:
            i = str(inc)
            path = i
            path = "../" + path
            state[path] = get_time(path)

    target = open(STATE_FILE, "w")
    json.dump(state, target, sort_keys=True, indent=4)
    target.close()
else:
    #1. check if ModuleNameGen.h changed - if yes rebuild only that module
    state = {}
    with open(STATE_FILE, 'r') as js:
        state = json.load(js)

    for f in module_files:
        testme = str(f)
        current_time = get_time(testme)
        last_time = state.get(testme)

        if last_time is None or current_time != last_time:
            state[testme] = current_time
            with open(STATE_FILE, 'w') as js:
                json.dump(state, js, sort_keys=True, indent=4)

            files_to_parse.append(f)
    
        #2. if not than check if some events.h changed -> rebuild module
        data = f.read_text()
        includes = arg_include_re.findall(data)
        for inc in includes:
            i = str(inc)
            path = i
            path = "../" + path
            current_time = get_time(path)
            last_time = state.get(path)

            if last_time is None or current_time != last_time:
                state[path] = current_time
                with open(STATE_FILE, 'w') as js:
                    json.dump(state, js, sort_keys=True, indent=4)

                files_to_parse.append(f)

files_to_parse = list(set(files_to_parse))

if len(files_to_parse) != 0:
    files_to_parse = module_files
    print("regenerate all because generator can't make diff rn")

print("Parsing next files: " + str(files_to_parse))
modules = {}
event_sources = []
#parse
for file in files_to_parse:
    #aggregate if necessary
    data = file.read_text()
    agrdata = ""
    includes = arg_include_re.findall(data)
    for inc in includes:
        f = str(file)
        pathpre = f[:f.rfind("/")+1]
        path = "./../" + str(inc)
        event_sources.append(path)
        with open(path, 'r') as f:
            agrdata += f.read()
            agrdata += "\n\n"
    
    #remove includes
    
    data = re.sub(include_del_re, "", data, flags=re.MULTILINE)
    
    agrdata += data
    #check grammar
    if(grammar.check(agrdata) is False):
        print("in file: " + str(file))
        grammar.print_errors()
        sys.exit(1)
    
    #parse
    modules[str(file)] = parser.parse(agrdata)


if len(modules) != 0:
    # print("Generate .cmake file")

    # target = open(GENERATED_SOURCES_LIST, "w")
    # target.write("// This file generated automatically! don't edit manually!\n")
    # target.write("set(ALL_GENERATED_SOURCES \n")
    # for src in event_sources:
    #     target.write(r'"${CMAKE_SOURCE_DIR}/' + src + '"\n')

    # target.write(")\n")
    # target.close()

    #generate
    print("Generating")
    gena.generate(modules)

# print("Generated:")
# print(" ")
# for key, ev in modules.items():
#     print("for file: \n" + key + "\n\nevents:")
#     for e in ev.events:
#         print(e.name)
#     print(" ")


print("Generation complete.")
# os.exit(1)
#SPDX-FileCopyrightText: 2025 Cwits
#SPDX-License-Identifier: GPL-3.0-or-later

from typing import List, Tuple, Optional
import re

event_re = re.compile(r'EV.*?[\r\n]+([\s\S]*?};)', re.S)

flatreq_re = re.compile(r'FLAT_REQ.*?[\r\n]+([\s\S]*?};)', re.S)

flatresp_re = re.compile(r'FLAT_RESP.*?[\r\n]+([\s\S]*?};)', re.S)

forward_names = re.compile(r'\s+(\w+)\s\*\s\w+;\s\/\/->\s\w+\s(\w+);', re.S)

ev_handle_re = re.compile(r'EV_HANDLE\s([\s\S]*?)END_HANDLE', re.S)
rt_handle_re = re.compile(r'RT_HANDLE\s([\s\S]*?)END_HANDLE', re.S)
resp_handle_re = re.compile(r'RESP_HANDLE\s([\s\S]*?)END_HANDLE', re.S)

event_handle_pattern = r'void\s*handleEvent\(\s*const\s+ControlContext\s+&\s*\w+\s*,\s*const\s+Events::(\w+)\s+&\s*\w+\s*\)'

resp_handle_pattern = r'void\s*(handle\w+)\(\s*const\s+ControlContext\s+&\s*\w+\s*,\s*const\s+FlatEvents::FlatResponse\s+&\s*\w+\s*\)'

grammar_version = "v0.1"

class error:
    def __init__(self, line: str, message: str):
        self.line = line
        self.message = message
        
    def __str__(self):
        return f"Error in line {self.line}\n {self.message}"



FAKE_DATA = """

"""

MAIN_KEYWORDS = ("START_BLOCK", "END_BLOCK", "EV", "FLAT_REQ", "FLAT_RESP", "INCLUDE", "EV_HANDLE", "RT_HANDLE", "RESP_HANDLE", "END_HANDLE", "//->")

errors = []

events_names = []

def print_errors():
    for err in errors:
        print(err)

def get_block_name(block) -> str:
    nameline = block[:block.find("\n")]
    name = nameline[nameline.find(" ")+1:]
    return name

def check_include(line, linenum):
    st = -1
    ed = -1
    failed = False
    if "\"" in line:
        st = line.find("\"")
        ed = line[st+1:].find("\"")
    elif "<" in line:
        st = line.find("<")
        ed = line[st+1:].find(">")
    else:
        failed = True
        errors.append(error(line, f"Can't find symbols used for include(either \" or < > on line: {linenum}"))
    #print(str(st) + " " + str(ed))
    if not failed and (st == -1 or ed == -1):
        errors.append(error(line, f"Did you forget properly open or close include on line: {linenum}?"))
        
def find_next_block(lines, startline) -> Optional[Tuple[int, int]]:
    start = -1
    end = -1
    for i in range(startline, len(lines)):
        line = lines[i]
        
        if "START_BLOCK" in line:
            if start != -1:
                errors.append(error(line, "Found START_BLOCK without closing previous START_BLOCK" + str(i+startline)))
                #print(lines)
                return None
            
            start = i
        
        elif "END_BLOCK" in line:
            if start == -1:
                errors.append(error(line, "Found END_BLOCK without START_BLOCK in: " + str(i+startline)))
                #print(lines)
                return None
            end = i
            return (start, end)
            
    if start != -1:
        errors.append(error(line, "Reached EOF without closing START_BLOCK"))
        #print(lines)
        return None
    
    return None

def check_forward_declaration(line, lineNum) -> bool:
    if not "//->" in line:
        errors.append(error(line, f'Did you forget forward declaration hint on line: {lineNum} in block?'))
    
    if not "class" in line:
        errors.append(error(line, f'Missing \'class\' keyword in forward declaration on line: {lineNum}'))
    
    
    names = re.search(r'\s+(\w+)\s\*\s\w+;\s\/\/->\s\w+\s(\w+);', line)
    
    if not names:
        errors.append(error(line, "Invalid forward declaration syntax"))
        return False
    
    name = names.group(1)
    nameDecl = names.group(2)
    
    if name != nameDecl:
        print(name)
        print(nameDecl)
        errors.append(error(line, f'class name doesn\'t match: {name} != {nameDecl} on line: {lineNum}'))
    
    return True

def check_evs(block) -> int:
    count = 0
    
    #better use regex here i guess
    events = event_re.findall(block)
    for ev in events:
        count += 1
        #analyze event
        event_lines = ev.splitlines()
        for i, line in enumerate(event_lines):
            if "INCLUDE" in line:
                check_include(line, i)
                    
            if "*" in line:
                check_forward_declaration(line, i)
                
            if "struct" in line:
                name = line[line.find(" ")+1:]
                name = name[:name.find(" ")]
                events_names.append(name)
    
    print(f"found {count} EV blocks")
    return count

def check_flats(block)-> set:
    reqCount = 0
    respCount = 0
    
    count = 0
    requests = flatreq_re.findall(block)
    for req in requests:
        count += 1
        splitted = req.splitlines()
        for i, line in enumerate(splitted):
            if "INCLUDE" in line:
                check_include(line, i)
                
            if "*" in line:
                check_forward_declaration(line, i)
    
    reqCount = count
    
    count = 0
    responses = flatresp_re.findall(block)
    for resp in responses:
        count += 1
        splitted = resp.splitlines()
        for i, line in enumerate(splitted):
            if "INCLUDE" in line:
                check_include(line, i)
                
            if "*" in line:
                check_forward_declaration(line, i)
    
    respCount = count
    
    if respCount > reqCount:
        errors.append(error(get_block_name(block), f"Number of found FLAT_RESP blocks({respCount}) is bigger than FLAT_REQ blocks({reqCount})"))
    
    print(f'found {reqCount} FLAT_REQ blocks')
    print(f'found {respCount} FLAT_RESP blocks')
    
    return (reqCount, respCount)

def check_evhandles(block)-> int:
    count = 0
    events = ev_handle_re.findall(block)
    for ev in events:
        count += 1
        
        lines = ev.splitlines()
        for i, line in enumerate(lines):
            if "INCLUDE" in line:
                check_include(line, i)
                
        res = re.search(event_handle_pattern, ev)
        if res:
            name = res.group(1)
            match = False
            for n in events_names:
                if n == name:
                    match = True
            
            if not match:
                errors.append(error(line, f"Name {name} of event doesn\'t match any of {events_names} provided in structs"))
        else:
            errors.append(error(get_block_name(block), f"Couldn't find correct handleEvent function signature"))
    print(f"found {count} EV_HANDLE blocks")
    
    return count
    
def check_rthandle(block) -> int:
    count = 0
    handles = rt_handle_re.findall(block)
    for hnd in handles:
        count += 1
        lines = hnd.splitlines()
        for i, line in enumerate(lines):
            if not line.strip():
                continue
            
            if 'INCLUDE' in line:
                check_include(line, i)
            else:
                if "&" != line[0]:
                    errors.append(error(line, "RT handle must be reference"))
                
        
    if count > 1:
        errors.append(error(get_block_name(block), f"Can\'t be more than 1 RT_HANDLE block, found {count}"))
    
    print(f"found {count} RT_HANDLE blocks")
    
    return count
    
def check_resphandle(block) -> int:
    count = 0
    handles = resp_handle_re.findall(block)
    for hnd in handles:
        count += 1
        
        lines = hnd.splitlines()
        for i, line in enumerate(lines):
            if "INCLUDE" in line:
                check_include(line, i)
        
        res = re.search(resp_handle_pattern, hnd)
        if not res:
            errors.append(error(get_block_name(block), f"Couldn't find correct response handler signature"))
    if count > 1:
        errors.append(error(get_block_name(block), f"Found {count} of RESP_HANDLE blocks, must be 0 or 1"))
        
    print(f"found {count} RESP_HANDLE blocks")
    
    return count
    
def check_block(block):
    evcnt = check_evs(block)
    flatcnt = check_flats(block)
    ehndcnt = check_evhandles(block)
    rthndcnt = check_rthandle(block)
    resphndcnt = check_resphandle(block)
    
    if evcnt != ehndcnt:
        errors.append(error(get_block_name(block), f"EV and EV_HANDLE blocks count must be equal, but found {evcnt} EVs and {ehndcnt} EV_HANDLE"))
        
    if flatcnt[0] != rthndcnt:
        errors.append(error(get_block_name(block), f"FLAT_REQ and RT_HANDLE blocks count must be equal, but found {flatcnt[1]} FLAT_REQ and {rthndcnt} RT_HANDLE"))
        
    if flatcnt[0] != resphndcnt:
        errors.append(error(get_block_name(block), f"RESP_HANDLE for FLAT_REQ must be provided, found {flatcnt[1]} FLAT_REQ and {resphndcnt} RESP_HANDLE"))

def check(data: str) -> bool:
    #data contains all includes concatinated in one string
    errors.clear()
    
    cursor = 0
    running = True
    #lines = data.splitlines()
    lines = FAKE_DATA.splitlines(True)
    print("total len: " + str(len(lines)))
    
    while(running):
        events_names.clear()
        res = find_next_block(lines, cursor)
        if res == None and len(errors) != 0:
            break
        
        if res == None:
            break
        
        blockName = "".join(lines[res[0]:res[0]+1])
        blockName = blockName[blockName.find(" ")+1:]
        print("")
        print("Cursor at: " + str(cursor))
        print("")
        print(f"Found block {blockName} at range: {res[0]} till {res[1]}")
        
        block = lines[res[0]:res[1]]
        cursor = res[1]+1
        
        conc_block = "".join(block)
        #print(conc_block)
        #print(res)
        
        check_block(conc_block)
    
    #del this, errors would be printed in main
    if(len(errors)):
        print("")
        for err in errors:
            print(err)
        print("")
        return False
    else:
        print("")
        print("No errors found!")
        print("")
    return True
    
    
if check("") == False:
    print("failed")
else:
    print("success")
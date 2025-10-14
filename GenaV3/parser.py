#SPDX-FileCopyrightText: 2025 Cwits
#SPDX-License-Identifier: GPL-3.0-or-later

import regexes
from collections import defaultdict


class event:
    def __init__(self):
        self.name = ""
        self.ev_includes = []
        self.ev_forwards = []
        self.ev_structs = []
        self.flat_req_includes = []
        self.flat_req_forwards = []
        self.flat_req_struct = []
        self.flat_resp_includes = []
        self.flat_resp_forwards = []
        self.flat_resp_struct = []
        self.ce_includes = []
        # self.ce_forwards = []
        self.ce_handlers = []
        self.rt_include = ""
        # self.rt_forwards = []
        self.rt_handler = ""
        self.resp_includes = []
        self.resp_handler = ""

    def parse(self, data):
        #name
        words = regexes.name_re.findall(data)
        self.name = words[0]
        
        #event structs
        events = regexes.event_re.findall(data)
        for e in events:
            includes = regexes.include_re.findall(e)
            fwdecl = regexes.forward_decl_re.findall(e)
            structs = regexes.struct_re.findall(e)

            for i in includes:
                self.ev_includes.append(i)
            for f in fwdecl:
                self.ev_forwards.append(f)
            for s in structs:
                self.ev_structs.append(s)

        #flat requests
        flatreq = regexes.flat_req_re.findall(data)
        for f in flatreq:
            includes = regexes.include_re.findall(f)
            fwdecl = regexes.forward_decl_re.findall(f)
            structs = regexes.struct_re.findall(f)

            for i in includes:
                self.flat_req_includes.append(i)
            for f in fwdecl:
                self.flat_req_forwards.append(f)
            for s in structs:
                self.flat_req_struct.append(s)
        
        #flat responses
        flatresp = regexes.flat_resp_re.findall(data)
        if not flatresp:
            self.flat_resp_includes = self.flat_req_includes
            self.flat_resp_forwards = self.flat_req_forwards
            self.flat_resp_struct = self.flat_req_struct
        else:
            for f in flatresp:
                includes = regexes.include_re.findall(f)
                fwdecl = regexes.forward_decl_re.findall(f)
                structs = regexes.struct_re.findall(f)

                for i in includes:
                    self.flat_resp_includes.append(i)
                for f in fwdecl:
                    self.flat_resp_forwards.append(f)
                for s in structs:
                    self.flat_resp_struct.append(s)
        
        #event handlers
        ev_handles = regexes.ev_handle_re.findall(data)
        for e in ev_handles:
            includes = regexes.include_re.findall(e)
            for i in includes:
                self.ce_includes.append(i)
            # #get rid of includes
            handle = e[e.find("void"):]
            self.ce_handlers.append(handle)

        #rt handler
        rt_handle_block = regexes.rt_handle_re.findall(data)
        if len(rt_handle_block) > 0:
            for i in rt_handle_block: #ugh...
                rt_handle_block = i

            rt_include = rt_handle_block[:rt_handle_block.find("&")]
            self.rt_include = rt_include[rt_include.find("\""):]

            rt_handle = rt_handle_block[rt_handle_block.find("&"):]
            self.rt_handler = rt_handle

        #response handler
        resp_handle_block = regexes.resp_handle_re.findall(data)
        if len(resp_handle_block) > 0:
            for i in resp_handle_block:
                resp_handle_block = i
                
            resp_includes = regexes.include_re.findall(resp_handle_block)
            for i in resp_includes:
                self.resp_includes.append(i)

            resp_handle = resp_handle_block[resp_handle_block.find("void"):]
            self.resp_handler = resp_handle


class parser:
    def __init__(self, data):
        self.events = []
        #text = filepath.read_text()
        for block in regexes.block_re.findall(data):
            ev = event()
            ev.parse(block)   
            self.events.append(ev)

def print_debug(e):
    print("START DEBUG")
    print("NAME: " + e.name)
    print("EVENTS:")
    print("INCLUDES: ")
    for i in e.ev_includes:
        print(i)
    print("")
    print("FORWARDS: ")
    for f in e.ev_forwards:
        print(f)
    print("")
    
    print("STRUCTURES: ")
    for s in e.ev_structs:
        print(s)
    print("")
    print("FLAT EVENTS:")
    print("INCLUDES:")
    for i in e.flat_req_includes:
        print(i)
    print("")
    print("FORWARDS: ")
    for f in e.flat_req_forwards:
        print(f)
    print("")
    print("STRUCTURES: ")
    for s in e.flat_req_struct:
        print(s)
    print("")
    print("FLAT RESPONSE:")
    print("INCLUDES:")
    for i in e.flat_resp_includes:
        print(i)
    print("")
    print("FORWARDS: ")
    for f in e.flat_resp_forwards:
        print(f)
    print("")
    print("STRUCTURES: ")
    for s in e.flat_resp_struct:
        print(s)
    print("")
    print("EVENTS HANDLERS:")
    print("INCLUDES:")
    for ev in e.ce_includes:
        print(ev)
    print("")
    print("HANDLERS:")
    for hl in e.ce_handlers:
        print(hl)
    print("")
    print("RT HANDLER")
    print("INCLUDES:")
    print(e.rt_include)
    # for i in e.rt_include:
    #     print(i)
    print("")
    print("HANDLER:")
    print(e.rt_handler)
    print("")
    print("RESP HANDLER")
    print("INCLUDES")
    for i in e.resp_includes:
        print(i)
    print("")
    print("HANDLER:")
    print(e.resp_handler)
    print("")
    print("END DEBUG")

def parse(data) -> []:
    res = parser(data)
    
    # for e in res.events:
    #     print_debug(e)
        
    return res

#SPDX-FileCopyrightText: 2025 Cwits
#SPDX-License-Identifier: GPL-3.0-or-later

import re
import os
from paths import *

AUTO_STR = "/* This file is generated automatically, do not edit manually */\n"
PRAGMA_STR = "#pragma once\n"

response_struct_pattern = r'(void\s*handle\w+Response\(const\s*ControlContext\s*&\s*ctx,\s*const\s*FlatEvents::FlatResponse\s*&\s*resp\))'

def getPath(name) -> str:
    # path = name[:name.rfind("/")+1]

    path = "../inc/generated/"
    path += name[name.rfind("/")+1:]
    return path

def getPath2(name) -> str:
    path = "generated/"
    path += name[name.rfind("/")+1:]
    return path

def generate_module_events_file(data):
    for key, evs in data.items():
        name = key[:key.find("Gen.h")]
        if "Events" not in name:
            name += "Events.h"
        else:
            name += ".h"

        path = getPath(name)
        print("Gena name: " + name)

        target = open(path, 'w')
        target.write(AUTO_STR)
        target.write(PRAGMA_STR)

        #includes
        includes = []
        for ev in evs.events:
            for inc in ev.ev_includes:
                includes.append(inc)
            for inc in ev.flat_req_includes:
                includes.append(inc)
            for inc in ev.flat_resp_includes:
                includes.append(inc)

        includes = list(set(includes))
        for inc in includes:
            #inc = inc[inc.find("/")+1:]
            print(inc)
            target.write("#include " + inc + "\n")
        target.write("\n")

        target.write("namespace slr {\n")
        target.write("\n")
        
        #forward declarations
        forwards = []
        for ev in evs.events:
            for fwd in ev.ev_forwards:
                forwards.append(fwd)
            for fwd in ev.flat_req_forwards:
                forwards.append(fwd)
            for fwd in ev.flat_resp_forwards:
                forwards.append(fwd)

        forwards = list(set(forwards)) #get rid of duplicates
        for fw in forwards:
            target.write(fw + "\n")
        target.write("\n")

        target.write("namespace Events {\n")
        target.write("\n")
        
        #structures
        for ev in evs.events:
            for st in ev.ev_structs:
                target.write(st)
                target.write("\n")
        
        target.write("\n")
        target.write("} //namespace Events\n\n")

        target.write("namespace FlatEvents {\n")
        target.write("\n")

        target.write("namespace FlatControls {\n")
        target.write("\n")

        for ev in evs.events:
            for fl in ev.flat_req_struct:
                target.write(fl)
                target.write("\n")

        target.write("\n")
        target.write("} //namespace FlatControls\n")
        target.write("\n")
        target.write("namespace FlatResponses {\n")
        target.write("\n")
        
        for ev in evs.events:
            for flr in ev.flat_resp_struct:
                target.write(flr)
                target.write("\n")

        target.write("\n")
        target.write("} //namespace FlatResponses\n")

        # #flat events
        # for ev in evs.events:
        #     for fl in ev.flat_req_struct:
        #         target.write(fl)
        #         target.write("\n")
        #     for flr in ev.flat_resp_struct:
        #         structname = re.search(r'struct\s+(\w+)\s*\{', flr)
        #         res = "struct "
        #         res += structname.group(1)
        #         res += "Response "
        #         res += flr[flr.find("{\n"):]
        #         target.write(res)
        #         target.write("\n")

        target.write("\n")
        target.write("} //namespace FlatEvents\n\n")
        target.write("} //namespace slr\n")
        target.close()

def generate_module_h_file(data):
    for key, evs in data.items():
        name = key[:key.find("Gen.h")]
        if "Events" in name:
            name = name[:name.find("Events")]
        name += "Handles"

        name_h = name + ".h"
        name_cpp = name + ".cpp"
        
        #write h file
        path = getPath(name_h)

        target = open(path, 'w')

        target.write(AUTO_STR)
        target.write(PRAGMA_STR)

        #includes
        includes = []
        for ev in evs.events:
            for inc in ev.ce_includes:
                includes.append(inc)

            for inc in ev.resp_includes:
                includes.append(inc)
       
        includes = list(set(includes))
        includes.append("\"core/primitives/ControlContext.h\"")

        for inc in includes:
            target.write("#include " + inc + "\n")

        target.write("\n")
        target.write("namespace slr {\n")
        target.write("\n")

        #TODO: souldn't be like this, but to be able to 
        #call response handles in handlers you need to declare them first
        resphandlers = []
        for ev in evs.events:
            if ev.resp_handler != "":
                target.write("inline " + ev.resp_handler + "\n")

        #handles
        handlers = []
        for ev in evs.events:
            for hnd in ev.ce_handlers:
                handlers.append(hnd)

        for hnd in handlers:
            target.write("inline " + hnd + "\n")


        target.write("} //namespace slr\n")
        target.close()

def generate_module_cpp_file(data):
    pass

def generate_events_file(data):
    #check if exists
    # if not os.path.exists(EVENTS_PATH):
    #     print("Events.h file doesn't exists, trying to create one")
    #     #find template file
    #     files = os.listdir(TEMPLATES_PATH)
    #     if "Events_template.h" not in files:
    #         print("Failed to find Events_template.h file")
    #         os.exit(1)
        
    #     with open(TEMPLATES_PATH + "Events_template.h", "r") as ev_f:
    #         ev_file = ev_f.read()
    #         target = open(EVENTS_PATH, "w")
    #         target.write(ev_file)
    #         target.close()
    #         print("Events.h file created successfully")
    # else:
    #     print("Events.h file exists, updating it")

    split_events_data = []
    # with open(EVENTS_PATH, "r") as ev_f:
    with open(TEMPLATES_PATH + "Events_template.h", "r") as ev_f:
        split_events_data = ev_f.readlines()

    cursor_includes_line = 0
    cursor_variant_line = 0

    for i in range(0, len(split_events_data)):
        line = split_events_data[i]
        if "//GENERATE_INCLUDES" in line:
            cursor_includes_line = i
        elif "//GENERATE_STRUCTS" in line:
            cursor_variant_line = i

    if (cursor_includes_line == 0 or
        cursor_variant_line == 0):
        print("Failed to find generation flags in Events_template.h")
        os.exit(1)

    target = open(EVENTS_PATH, "w")
    target.write(AUTO_STR)
    #write beginning
    for i in range(0, cursor_includes_line):
        target.write(split_events_data[i])
        # target.write("\n")

    #aggregate includes
    includes = []
    for key, evs in data.items():
        name = key[:key.find("Gen.h")]
        if "Events" not in name:
            name += "Events.h"
        else:
            name += ".h"

        
        # result = 
        # name = name[name.rfind("/")+1:]
        fullinc = getPath2(name)
        # fullinc = name[:name.rfind("/")+1]
        # fullinc += TARGET_DIR
        # fullinc += name[name.rfind("/")+1:]
        # print("FULL INC: " + fullinc)
        includes.append("#include \"" + fullinc + "\"\n")

    #includes
    for inc in includes:
        target.write(inc)

    #write middle
    for i in range(cursor_includes_line+1, cursor_variant_line):
        target.write(split_events_data[i])
        # target.write("\n")

    #write events
    structs = []
    for key, evs in data.items():
        for ev in evs.events:
            for e in ev.ev_structs:
                structname = re.search(r'struct\s+(\w+)\s*\{', e)
                structs.append(structname.group(1))
                
    for i, st in enumerate(structs):
        target.write("\t\t")
        if i != len(structs)-1:
            target.write(st + ",\n")
        else:
            target.write(st + "\n")

    #write end
    for i in range(cursor_variant_line+1, len(split_events_data)):
        target.write(split_events_data[i])
        # target.write("\n")

    target.close()

def generate_flat_events_file(data):
    # if not os.path.exists(FLAT_EVENTS_PATH):
    #     print("FlatEvents.h file doesn't exists, trying to create one")
    #     #find template file
    #     files = os.listdir(TEMPLATES_PATH)
    #     if "FlatEvents_template.h" not in files:
    #         print("Failed to find FlatEvents_template.h file")
    #         os.exit(1)
        
    #     with open(TEMPLATES_PATH + "FlatEvents_template.h", "r") as ev_f:
    #         ev_file = ev_f.read()
    #         target = open(FLAT_EVENTS_PATH, "w")
    #         target.write(ev_file)
    #         target.close()
    #         print("FlatEvents.h file created successfully")
    # else:
    #     print("FlatEvents.h file exists, updating it")

    split_flats_data = []
    # with open(FLAT_EVENTS_PATH, "r") as fl_f:
    with open(TEMPLATES_PATH + "FlatEvents_template.h", "r") as fl_f:
        split_flats_data = fl_f.readlines()

    cursor_includes_line = 0
    cursor_ctl_type_line = 0
    cursor_ctl_union_line = 0
    cursor_resp_type_line = 0
    cursor_resp_union_line = 0

    for i in range(0, len(split_flats_data)):
        line = split_flats_data[i]
        # print(str(i) + line)
        if "//GENERATE_INCLUDES" in line:
            cursor_includes_line = i
        elif "//GENERATE_CTL_TYPES" in line:
            cursor_ctl_type_line = i
        elif "//GENERATE_CTL_UNIONS" in line:
            cursor_ctl_union_line = i
        elif "//GENERATE_RESP_TYPES" in line:
            cursor_resp_type_line = i
        elif "//GENERATE_RESP_UNIONS" in line:
            cursor_resp_union_line = i

    if (cursor_includes_line == 0 or 
        cursor_ctl_type_line == 0 or 
        cursor_ctl_union_line == 0 or
        cursor_resp_type_line == 0 or
        cursor_resp_union_line == 0):
        print("Failed to find generation flags in FlatEvents_template.h")
        os.exit(1)

    target = open(FLAT_EVENTS_PATH, "w")
    target.write(AUTO_STR)

    #write before includes
    for i in range(0, cursor_includes_line):
        target.write(split_flats_data[i])
        # target.write("\n")

    #write includes
    #aggregate includes
    includes = []
    for key, evs in data.items():
        name = key[:key.find("Gen.h")]
        if "Events" not in name:
            name += "Events.h"
        else:
            name += ".h"
        
        # name = name[name.find("/")+1:]
        fullinc = getPath2(name)
        # fullinc = name[:name.rfind("/")+1]
        # fullinc += TARGET_DIR
        # fullinc += name[name.rfind("/")+1:]
        includes.append("#include \"" + fullinc + "\"\n")

    #includes
    for inc in includes:
        target.write(inc)

    #write after includes, before control type
    for i in range(cursor_includes_line+1, cursor_ctl_type_line):
        target.write(split_flats_data[i])
        # target.write("\n")

    #write types
    ctlstructnames = []
    for key, evs in data.items():
        for ev in evs.events:
            for flt in ev.flat_req_struct:
                structname = re.search(r'struct\s+(\w+)\s*\{', flt)
                ctlstructnames.append(structname.group(1))
    
    for i, ctl in enumerate(ctlstructnames):
        target.write("\t\t")
        if i != len(ctlstructnames)-1:
            target.write(ctl + ",\n")
        else:
            target.write(ctl)
            
    target.write("\n")
    #write after control type, before control union
    for i in range(cursor_ctl_type_line+1, cursor_ctl_union_line):
        target.write(split_flats_data[i])
        # target.write("\n")
    
    #write structs in control union
    for i, ctl in enumerate(ctlstructnames):
        target.write("\t\t")
        target.write("FlatControls::" + ctl + " ")
        ctl = ctl[:1].lower() + ctl[1:]
        target.write(ctl + ";\n")


    #write after control union, before response type
    for i in range(cursor_ctl_union_line+1, cursor_resp_type_line):
        target.write(split_flats_data[i])
        # target.write("\n")

    #write types
    # target.write("testtyperesp")
    respstructnames = []
    for key, evs in data.items():
        for ev in evs.events:
            for rsp in ev.flat_resp_struct:
                structname = re.search(r'struct\s+(\w+)\s*\{', rsp)
                respstructnames.append(structname.group(1))

    for i, rsp in enumerate(respstructnames):
        target.write("\t\t")
        if i != len(respstructnames)-1:
            target.write(rsp + ",\n")
        else:
            target.write(rsp)

    target.write("\n")
    #write after response type, before resp union
    for i in range(cursor_resp_type_line+1, cursor_resp_union_line):
        target.write(split_flats_data[i])
        # target.write("\n")

    #write structs to response union
    # target.write("testunionresp")
    
    for i, rsp in enumerate(respstructnames):
        target.write("\t\t")
        target.write("FlatResponses::" + rsp + " ")
        rsp = rsp[:1].lower() + rsp[1:]
        target.write(rsp + ";\n")
        
    #write after resp union, till end
    for i in range(cursor_resp_union_line+1, len(split_flats_data)):
        target.write(split_flats_data[i])
        # target.write("\n")

    target.close()

def generate_ce_table(data):
    ce_handle_data = ""
    with open(TEMPLATES_PATH + "CEHandlerTables_template.h", "r") as ce:
        ce_handle_data = ce.readlines()
    
    cursor_include_line = 0
    cursor_control_line = 0
    cursor_response_line = 0

    for i, line in enumerate(ce_handle_data):
        if "//GENERATE_INCLUDES" in line:
            cursor_include_line = i
        elif "//GENERATE_CONTROL_HANDLES" in line:
            cursor_control_line = i
        elif "//GENERATE_RESPONSE_HANDLES" in line:
            cursor_response_line = i

    if (cursor_include_line == 0 or 
        cursor_control_line == 0 or 
        cursor_response_line == 0):
        print("Failed to find generation flags in CEHandlerTables_template.h")
        os.exit(1)

    target = open(CE_TABLE_PATH, "w")
    target.write(AUTO_STR)

    #write before includes
    for i in range(0, cursor_include_line):
        target.write(ce_handle_data[i])

    #generate includes
    # target.write("testinc")
    includes = []
    for key, evs in data.items():
        name = key[:key.find("Gen.h")]
        
        if "Events" in name:
            name = name[:name.find("Events")]
        # else:
        #     name += ".h"
        
        # name = name[name.find("/")+1:]
        # fullinc = name[:name.rfind("/")+1]
        # fullinc += TARGET_DIR
        # fullinc += name[name.rfind("/")+1:]
        fullinc = getPath2(name)
        includes.append("#include \"" + fullinc + "Events.h\"\n")
        includes.append("#include \"" + fullinc + "Handles.h\"\n")


    for inc in includes:
        target.write(inc)


    # target.write("\n")
    #write before control table
    for i in range(cursor_include_line+1, cursor_control_line):
        target.write(ce_handle_data[i])

    #generate control handles
    # target.write("testhnd")
    handlers = []
    for key, evs in data.items():
        for ev in evs.events:
            for hnd in ev.ce_handlers:
                name = re.search(r'void\s+\w+\(const\s+ControlContext\s+\&ctx,\s+const\s+(\w+\:\:\w+)\s+\&\w+\)\s+{', hnd)
                handlers.append(name.group(1))

    for i, hnd in enumerate(handlers):
        target.write("\t")
        if i != len(handlers)-1:
            target.write("&dispatchHelper<" + hnd + ">,\n")
        else:
            target.write("&dispatchHelper<" + hnd + ">\n")

    # target.write("\n")
    #write before response table
    for i in range(cursor_control_line+1, cursor_response_line):
        target.write(ce_handle_data[i])

    #generate response handles
    # target.write("testresp")
    responses = []
    for key, evs in data.items():
        for ev in evs.events:
            name = re.search(r'void\s+(\w+)', ev.resp_handler)
            if name is not None:
                responses.append(name.group(1))

    for i, rsp in enumerate(responses):
        target.write("\t")
        if i != len(responses)-1:
            target.write("&" + rsp + ",\n")
        else:
            target.write("&" + rsp + "\n")
    
    # target.write("\n")
    #write till the end
    for i in range(cursor_response_line+1, len(ce_handle_data)):
        target.write(ce_handle_data[i])

    target.close()
    
def generate_rt_table(data):
    rt_data = []
    with open(TEMPLATES_PATH + "RTHandlerTable_template.h", "r") as rt_t:
        rt_data = rt_t.readlines()

    cursor_includes_line = 0
    cursor_rt_handles_line = 0

    for i, line in enumerate(rt_data):
        if "//GENERATE_INCLUDES" in line:
            cursor_includes_line = i
        elif "//GENERATE_RT_HANDLES" in line:
            cursor_rt_handles_line = i

    if (cursor_includes_line == 0 or
        cursor_rt_handles_line == 0):
        print("Failed to find generation flags in RTHandlerTable_template.h")
        os.exit(1)

    target = open(RT_TABLE_PATH, "w")
    target.write(AUTO_STR)

    #write till includes
    for i in range(0, cursor_includes_line):
        target.write(rt_data[i])

    #generate includes
    # target.write("includes")
    includes = []
    for key, evs in data.items():
        for ev in evs.events:
            if(ev.rt_include != ""):
                includes.append(ev.rt_include)

    includes = list(set(includes))
    # print(includes)
    for inc in includes:
        target.write("#include " + inc)

    #write middle
    for i in range(cursor_includes_line+1, cursor_rt_handles_line):
        target.write(rt_data[i])

    #generate handles
    # target.write("handles")
    handles = []
    for key, evs in data.items():
        for ev in evs.events:
            if ev.rt_handler != "":
                handles.append(ev.rt_handler)
    
    for i, hnd in enumerate(handles):
        hnd = hnd[:len(hnd)-1] #get rid of \n
        target.write("\t")
        if i != len(handles)-1:
            target.write(hnd + ",\n")
        else:
            target.write(hnd + "\n")

    # target.write("\n")
    #write end
    for i in range(cursor_rt_handles_line+1, len(rt_data)):
        target.write(rt_data[i])

    target.close()

def generate(data):
    # i = 1
    # for key, ev in modules.items():
    # print("for file: \n" + key + "\n\nevents:")
    # for e in ev.events:
    #     print(e.name)
    # print(" ")
    generate_module_events_file(data)
    generate_module_h_file(data)
    generate_module_cpp_file(data)

    generate_events_file(data)
    generate_flat_events_file(data)
    generate_ce_table(data)
    generate_rt_table(data)
#SPDX-FileCopyrightText: 2025 Cwits
#SPDX-License-Identifier: GPL-3.0-or-later

import re

# Patterns
block_re = re.compile(r'START_BLOCK.*?([\s\S]*?)\bEND_BLOCK\b', re.S)

name_re = re.compile(r'\b([\w]+)\b')

include_re = re.compile(r'INCLUDE[\s](.*?)[\r\n]+', re.S)
forward_decl_re = re.compile(r'\/\/->\s(.*?)[\r\n]', re.S)
struct_re = re.compile(r'(struct\s+\w+\s*\{[\s\S]*?\};)', re.S)

event_re = re.compile(r'EV.*?[\r\n]+([\s\S]*?};)', re.S)
flat_req_re = re.compile(r'FLAT_REQ.*?[\r\n]+([\s\S]*?};)', re.S)
flat_resp_re = re.compile(r'FLAT_RESP.*?[\r\n]+([\s\S]*?};)', re.S)

ev_handle_re = re.compile(r'EV_HANDLE\s([\s\S]*?)END_HANDLE', re.S)
rt_handle_re = re.compile(r'RT_HANDLE\s([\s\S]*?)END_HANDLE', re.S)
resp_handle_re = re.compile(r'RESP_HANDLE\s([\s\S]*?)END_HANDLE', re.S)
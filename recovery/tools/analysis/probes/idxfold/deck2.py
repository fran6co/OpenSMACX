S = 'parse_say(0, Facility[FacilitySPStart + project].name, -1, -1);'
DECKS = [('src/basewin.cpp', '0x0041BBC0', [
    ('fid_nobrace', [(S, 'int fid = FacilitySPStart + project; parse_say(0, Facility[fid].name, -1, -1);')]),
    ('fid_const', [(S, 'const int fid = FacilitySPStart + project; parse_say(0, Facility[fid].name, -1, -1);')]),
    ('brace_only', [(S, '{ parse_say(0, Facility[FacilitySPStart + project].name, -1, -1); }')]),
    ('fid_brace_swap', [(S, '{ int fid = project + FacilitySPStart; parse_say(0, Facility[fid].name, -1, -1); }')]),
    ('fid_top', [('    const int project = popup.exec();', '    const int project = popup.exec();\n    int fid;'), (S, 'fid = FacilitySPStart + project; parse_say(0, Facility[fid].name, -1, -1);')]),
])]

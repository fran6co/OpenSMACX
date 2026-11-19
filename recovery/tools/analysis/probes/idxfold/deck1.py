S = 'parse_say(0, Facility[FacilitySPStart + project].name, -1, -1);'
DECKS = [('src/basewin.cpp', '0x0041BBC0', [
    ('base', []),
    ('swap', [(S, 'parse_say(0, Facility[project + FacilitySPStart].name, -1, -1);')]),
    ('ptr', [(S, '{ RulesFacility *f = &Facility[FacilitySPStart + project]; parse_say(0, f->name, -1, -1); }')]),
    ('fid', [(S, '{ int fid = FacilitySPStart + project; parse_say(0, Facility[fid].name, -1, -1); }')]),
    ('ptrarith', [(S, 'parse_say(0, (Facility + FacilitySPStart)[project].name, -1, -1);')]),
])]

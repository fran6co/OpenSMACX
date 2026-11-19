A = '        RulesFacility *project = &Facility[FacilitySPStart + project_id];'
B = '                    parse_say(1, Facility[FacilitySPStart + related_id].name, -1, -1);'
A1 = '        const int project_facility = FacilitySPStart + project_id;\n        RulesFacility *project = &Facility[project_facility];'
B1 = '                    const int related_facility = FacilitySPStart + related_id;\n                    parse_say(1, Facility[related_facility].name, -1, -1);'
DECKS = [('src/netdaemon.cpp', '0x00534400', [
    ('base', []),
    ('A', [(A, A1)]),
    ('B', [(B, B1)]),
    ('AB', [(A, A1), (B, B1)]),
])]

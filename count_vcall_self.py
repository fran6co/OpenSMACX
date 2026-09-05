import re, pathlib

SELF_CAST = re.compile(
    r"reinterpret_cast<[^>]*>\s*\(\s*this\s*\)"
    r"|\(\s*[A-Za-z_][\w :]*\*+\s*\)\s*this\b(?!\s*(?:->|\.))"
    r"|\bmem(?:cpy|set|move)\s*\([^;)]*\bthis\b")
VCALL = re.compile(r"\bVCall\b")

root = pathlib.Path('src')
for name, rx in (("SELF", SELF_CAST), ("VCALL", VCALL)):
    print("===", name)
    for p in sorted(root.glob('*.cpp')) + sorted(root.glob('*.h')):
        if {'recovered', 'unrecovered', 'vendor'} & set(p.parts):
            continue
        in_block = False
        for i, line in enumerate(p.read_text(errors='replace').splitlines(), 1):
            s = line.lstrip()
            if in_block:
                if '*/' in s:
                    in_block = False
                continue
            if s.startswith('//'):
                continue
            if s.startswith('/*'):
                if '*/' not in s:
                    in_block = True
                continue
            code = line.split('//', 1)[0]
            if rx.search(code):
                print(" ", p.name, i, code.strip()[:70])

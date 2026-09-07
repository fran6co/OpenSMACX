"""Storage-identity and scaled-offset hints; not C++ ownership or layout proof.

The legacy spelling lens recognizes conventional six-or-more-digit address
comments/bindings and uppercase class-pointer types. It is not a C++ parser:
macros, scopes, aliases and actual element sizes require independent evidence.
Compact RECOVERY_OBJECT bindings retain their full configured identity syntax.
"""

from collections import Counter, defaultdict
import re

from .object_source import parse_objects
from .includes import COMMENTS_OR_STRINGS
from .source.mangled import qualified_name


STORAGE_RULES = {
    "duplicate global identities": "Distinct names refer to one original address; reconcile actual storage and aliasing before renaming.",
    "scaled class-pointer sum": "A pointer cast wraps class-pointer arithmetic; investigate whether the intended offset is bytes or elements.",
}
BINDING = re.compile(r"^\s*(?:static\s+)?[\w:]+[\s\*&]+const\s+(\w+)\s*=\s*"
                     r"\(?\s*\([\w:\s\*]+\)\s*0[xX]([0-9A-Fa-f]{6,16})\b")
ANNOTATED = re.compile(r"^\s*(?:extern\s+)?[\w:]+[\s\*&]+(\w+)\s*"
                       r"(?:\[[^\]]*\])?\s*;\s*//\s*0[xX]([0-9A-Fa-f]{6,16})\b")
DEFN = re.compile(r"^[\w:][\w:\*&<>,\s]*?\b\w+\s*\([^;{)]*\)\s*\{")
SUM = re.compile(r"(?:reinterpret_cast<[^>]*\*[^>]*>|\(\s*[\w:]+\s*\*+\s*\))"
                 r"\s*\(\s*(\w+)\s*\+\s*0x[0-9A-Fa-f]{2,}\s*\)")
CLASS_PTR = re.compile(r"\b([A-Z]\w+)\s*\*\s*(?:const\s+)?(\w+)\s*[=;,)]")
BYTE_PTR = re.compile(r"\b(?:char|void|unsigned char|uint8_t|int8_t)\s*"
                      r"\*\s*(?:const\s+)?(\w+)\s*[=;,)]")


def class_member_lines(text):
    """Exclude conventional class bodies, whose address notes describe members.

    An embedded first member can share its containing object's original address
    without creating another global. This lexical boundary is not layout proof.
    """
    code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    members = set()
    for match in re.finditer(r"\b(?:class|struct|union)\s+(?:\w+(?:\s+final)?\s*)?(?::[^;{}]+)?\{"
                             r"|\b(?:struct|union)\s*\{", code):
        depth, end = 1, match.end()
        while end < len(code) and depth:
            depth += (code[end] == "{") - (code[end] == "}")
            end += 1
        if depth:
            raise ValueError("unterminated class scope in storage audit")
        members.update(range(code.count("\n", 0, match.start()) + 1,
                             code.count("\n", 0, end) + 2))
    return members


def scaled_sums(text):
    """Retain the historical per-function lens and expose original locations."""
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        if not DEFN.match(lines[i]):
            i += 1
            continue
        j = i + 1
        while j < len(lines) and not lines[j].startswith("}"):
            j += 1
        # Preserve newlines so comments do not shift diagnostic locations.
        body = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m[0]),
                      "\n".join(lines[i:j + 1]), flags=re.S)
        body = "\n".join(line.split("//", 1)[0] for line in body.splitlines())
        typed = {m[2] for m in CLASS_PTR.finditer(body)}
        byte_names = {m[1] for m in BYTE_PTR.finditer(body)}
        for match in SUM.finditer(body):
            if match[1] in typed and match[1] not in byte_names:
                yield i + 1 + body.count("\n", 0, match.start()), match[0], match[1]
        i = j + 1


def storage_findings(files, *, pin, address_space, symbol_format):
    names, sites = defaultdict(set), defaultdict(list)
    mapped_symbols, mapped_addresses = set(), set()
    counts = Counter({name: 0 for name in STORAGE_RULES})
    per_file, findings = defaultdict(Counter), []

    def binding(address, symbol, path, line, kind):
        names[address].add(symbol)
        sites[address].append({"path": path, "line": line, "symbol": symbol, "kind": kind})

    for path, text in sorted(files.items()):
        members = class_member_lines(text)
        for line, value in enumerate(text.splitlines(), 1):
            if line in members:
                continue
            for pattern, kind in ((BINDING, "address_pointer"), (ANNOTATED, "address_comment")):
                match = pattern.match(value)
                if match:
                    binding(int(match[2], 16), match[1], path, line, kind)
        for obj in parse_objects(text, path, pin, address_space=address_space):
            if obj.symbol in mapped_symbols or obj.origin.address in mapped_addresses:
                raise ValueError("duplicate original object symbol or start address")
            mapped_symbols.add(obj.symbol)
            mapped_addresses.add(obj.origin.address)
            name = qualified_name(obj.symbol) if symbol_format == "msvc" else obj.symbol
            if not name:
                raise ValueError(f"cannot identify mapped object {obj.symbol}")
            binding(obj.origin.address, name, path, obj.line, "source_object")
        for line, expression, symbol in scaled_sums(text):
            rule = "scaled class-pointer sum"
            counts[rule] += 1
            per_file[path][rule] += 1
            findings.append({"rule": rule, "path": path, "line": line, "symbols": [symbol],
                             "expression": expression, "occurrences": 1, "basis": "source_heuristic"})
    for address, group in sorted(names.items()):
        if len(group) < 2:
            continue
        rule = "duplicate global identities"
        counts[rule] += 1
        for site in sites[address]:
            per_file[site["path"]][rule] += 1
            findings.append({"rule": rule, "path": site["path"], "line": site["line"],
                             "address": address, "symbols": sorted(group), "related": sites[address],
                             "occurrences": 1, "basis": "source_heuristic"})
    return counts, {path: dict(values) for path, values in per_file.items()}, findings

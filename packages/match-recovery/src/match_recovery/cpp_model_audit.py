"""Source-model smells and their locations; hints, never structural proofs.

The v1 lens recognizes conventional decompiler names and source annotations.
Reference counts are whole-product textual observations, not linker reachability.
Neither an orphan hint nor an EVIDENCE-FREE note authorizes deletion or renaming.
"""

from bisect import bisect_right
from collections import Counter, defaultdict
from pathlib import PurePosixPath
import re


UNK = re.compile(r"\bUNK\d+\b")
ADDRESS_CAST = re.compile(
    r"(?:\(\s*[A-Za-z_][\w :]*?\*\s*\)|reinterpret_cast<[^>]*\*\s*>\s*\()"
    r"\s*0x(00[0-9A-Fa-f]{6})\b")
REDIRECT = re.compile(r"\b\w+_redirect\b")
POINTER_AS_INT = re.compile(
    r"return\s+reinterpret_cast<\s*int\s*>\s*\(\s*this\s*\)"
    r"|return\s*\(\s*int\s*\)\s*new\b|\b__(?:as|ct|dt)\b")
DOCUMENTED = re.compile(r"Purpose:|Verification note", re.I)
SCAFFOLD_NAME = re.compile(r"\ba\d+\b")
PARAM_CAST = re.compile(
    r"(?:reinterpret_cast<[^>]*\*[^>]*>|\(\s*[A-Za-z_][\w :]*\*+\s*\))"
    r"\s*\(?\s*a\d+\b")
SELF_CAST = re.compile(
    r"reinterpret_cast<[^>]*>\s*\(\s*this\s*\)"
    r"|\(\s*[A-Za-z_][\w :]*\*+\s*\)\s*this\b(?!\s*(?:->|\.))"
    r"|\bmem(?:cpy|set|move)\s*\([^;)]*\bthis\b")

MODEL_RULES = {
    "unk-method": "Unresolved method name; investigate behavior and callers without inventing an original name.",
    "evidence-free unk": "An existing note explicitly refuses a name; preserve the finding and revisit only with new evidence.",
    "function-address binding": "A pointer cast targets a catalogued function span; investigate a disguised function reference.",
    "orphan redirect": "At most two textual references in product scope; establish actual reachability before deleting anything.",
    "pointer-as-int": "A pointer/lifecycle spelling may conceal the C++ type or ownership model.",
    "undocumented trivial body": "A short historical matching claim lacks nearby or family-level purpose documentation.",
    "raw self-access": "A raw this cast or memory operation may conceal a member or inheritance edge; inspect ownership and layout.",
    "pointer-parameter as int": "A scaffold parameter is cast to a pointer; establish its type from independent use-site evidence.",
    "scaffold name": "A provisional parameter/local name needs evidence; an unread argument need not receive an invented purpose.",
}


def excused_names(files):
    """Pair headers/units only within their own relative parent and stem.

    Preserve the consecutive-// evidence lens; a similarly named method in
    another directory or UNK10 beside an excused UNK1 does not inherit the refusal.
    """
    result = defaultdict(set)
    for path, text in files.items():
        lines = text.split("\n")
        for index, line in enumerate(lines):
            if "EVIDENCE-FREE" not in line:
                continue
            while index < len(lines) and lines[index].lstrip().startswith("//"):
                result[PurePosixPath(path).with_suffix("").as_posix()].update(UNK.findall(lines[index]))
                index += 1
    return result


def model_findings(files, records, lines_for):
    """Count selected product files using all source records as address hints.

    `files` is a text snapshot, `records` pairs of snapshot path and parsed mapping
    (including seeds), and
    `lines_for` supplies original line numbers through the versioned comment lens.
    """
    # Union intervals first: a short nested span must not hide a containing
    # function when bisect selects the latest start address.
    records = tuple(records)
    spans = []
    for start, end in sorted(span for _, record in records if record.address for span in record.image_spans or ()):
        if spans and start <= spans[-1][1]:
            spans[-1] = (spans[-1][0], max(spans[-1][1], end))
        else:
            spans.append((start, end))
    starts = [start for start, _ in spans]
    counts = Counter({name: 0 for name in MODEL_RULES})
    per_file = defaultdict(Counter)
    findings, redirect_sites = {}, defaultdict(list)
    excused = excused_names(files)

    def add(rule, path, line, *, symbol=None, address=None):
        counts[rule] += 1
        per_file[path][rule] += 1
        key = (rule, path, line, address)
        item = findings.setdefault(key, {"rule": rule, "path": path, "line": line,
                                         "basis": "source_heuristic", "occurrences": 0, "symbols": []})
        item["occurrences"] += 1
        if symbol is not None:
            item["symbols"] = sorted(set(item["symbols"]) | {symbol})
        if address is not None:
            item["address"] = address

    for path, text in files.items():
        refusals = excused.get(PurePosixPath(path).with_suffix("").as_posix(), set())
        for number, line in lines_for(text):
            unknowns = UNK.findall(line)
            for symbol in unknowns:
                add("evidence-free unk" if symbol in refusals else "unk-method", path, number, symbol=symbol)
            for hexpart in ADDRESS_CAST.findall(line):
                address = int(hexpart, 16)
                index = bisect_right(starts, address) - 1
                if index >= 0 and address < spans[index][1]:
                    add("function-address binding", path, number, address=address)
            for symbol in REDIRECT.findall(line):
                redirect_sites[symbol].append((path, number, line))
            if POINTER_AS_INT.search(line):
                add("pointer-as-int", path, number)
            for rule, pattern in (("raw self-access", SELF_CAST), ("pointer-parameter as int", PARAM_CAST)):
                for _ in pattern.findall(line):
                    add(rule, path, number)
            for symbol in SCAFFOLD_NAME.findall(line):
                add("evidence-free unk" if refusals.intersection(unknowns) else "scaffold name",
                    path, number, symbol=symbol)
    orphans = {symbol: sites for symbol, sites in redirect_sites.items() if len(sites) <= 2}
    for symbol, sites in orphans.items():
        for path, number, line in sites:
            add("orphan redirect", path, number, symbol=symbol)
    # The ratchet counts unique names; per-file locations count mentions. Keep
    # this distinction explicit rather than silently changing the old obligation.
    counts["orphan redirect"] = len(orphans)
    for path, record in records:
        if path not in files or not record.byte_exact or (record.size or 99) > 4:
            continue
        text = files[path].splitlines()
        nearby = "\n".join(text[max(0, record.line - 30):record.line + 3])
        if not DOCUMENTED.search(nearby) and not DOCUMENTED.search("\n".join(text[:40])):
            add("undocumented trivial body", path, record.line, address=record.address)
    return counts, {path: dict(values) for path, values in per_file.items()}, list(findings.values())

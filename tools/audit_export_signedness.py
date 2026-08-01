#!/usr/bin/env python3
"""Report recovered functions whose parameter signedness contradicts the
original's own mangled name.

This is the shape of the defect byte-matching found in `bitmask`: the original
divides signed, the recovery declared `uint32_t`, and the difference is
invisible to every differential oracle here because both answers agree on
non-negative inputs. `tools/mutate_and_verify.py` cannot see it either - no
mutation of a correct-looking body reproduces it.

Two independent sources are compared, which is the point: neither is derived
from the other.

  original  - the catalogue's decorated name in docs/recovery/functions.csv,
              read out of the SP3 IDB. `H` is int, `I` is unsigned.
  recovered - the GCC symbol each src/OpenSMACX.def alias maps to, which the
              compiler produced from the committed source. `i` is int,
              `j` is unsigned.

A disagreement is a CANDIDATE, not a defect. It only changes behaviour where
the original does something signedness-sensitive with that parameter, so
candidates are ranked by what the original's own bytes contain:

  idiv/cdq - signed division. `bitmask` was here.
  sar      - arithmetic shift right, usually a signed divide by a power of two.
  neither  - bounded arithmetic; the two spellings almost certainly agree.

Ranking reads the original's instructions, so it is evidence rather than a
guess - but it flags a function for containing signed arithmetic ANYWHERE, not
necessarily on the mismatched parameter. Confirming one still means reading it.
"""
import argparse
import csv
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_DEF = REPO_ROOT / "src" / "OpenSMACX.def"
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"

# Recorded so a new recovery cannot quietly add to the population. Lower these
# when candidates are resolved; the check refuses any increase.
BASELINE = {"disagreements": 199, "signed_divide": 44}

MSVC = {'H': 'int', 'I': 'uint', 'J': 'long', 'K': 'ulong',
        'F': 'short', 'G': 'ushort', 'D': 'char', 'E': 'uchar',
        'C': 'schar', 'M': 'float', 'N': 'double', 'X': 'void'}
GCC = {'i': 'int', 'j': 'uint', 'l': 'long', 'm': 'ulong',
       's': 'short', 't': 'ushort', 'c': 'char', 'h': 'uchar',
       'a': 'schar', 'f': 'float', 'd': 'double', 'v': 'void', 'b': 'bool'}

SIGNED_PAIRS = ({'int', 'uint'}, {'int*', 'uint*'})


def msvc_types(name):
    """Decorated MSVC name -> [return, arg, ...], or None if not understood."""
    at = name.find('@@')
    if at < 0 or not name.endswith('@Z'):
        return None
    body = name[at + 2:-2]
    # Free and static functions are Y?/S?; members carry access + cv + call.
    body = body[2:] if body[:1] in ('Y', 'S') else body[3:]
    out = []
    index = 0
    while index < len(body):
        if body.startswith('_N', index):
            out.append('bool')
            index += 2
            continue
        char = body[index]
        if char == 'P':
            if not body.startswith('PA', index) or index + 2 >= len(body):
                return None
            pointee = body[index + 2]
            if pointee not in MSVC:
                return None
            out.append(MSVC[pointee] + '*')
            index += 3
            continue
        if char == '0':                       # back-reference to a prior arg
            if not out:
                return None
            out.append(out[-1])
            index += 1
            continue
        if char == '@':                       # end of the argument list
            break
        if char not in MSVC:
            return None
        out.append(MSVC[char])
        index += 1
    return out


def gcc_types(symbol):
    """Itanium symbol -> [arg, ...]. The return type is not encoded."""
    if not symbol.startswith('_Z'):
        return None
    rest = symbol[2:]
    if rest.startswith('N'):                  # _ZN5Class6methodEii
        rest = rest[1:]
        while rest[:1].isdigit():
            match = re.match(r'\d+', rest)
            rest = rest[match.end() + int(match.group(0)):]
        if not rest.startswith('E'):
            return None
        rest = rest[1:]
    else:
        match = re.match(r'\d+', rest)
        if not match:
            return None
        rest = rest[match.end() + int(match.group(0)):]
    out = []
    index = 0
    while index < len(rest):
        char = rest[index]
        if char == 'P':
            if index + 1 >= len(rest):
                return None
            pointee = rest[index + 1]
            if pointee == 'K':                # pointer to const
                if index + 2 >= len(rest) or rest[index + 2] not in GCC:
                    return None
                out.append(GCC[rest[index + 2]] + '*')
                index += 3
                continue
            if pointee not in GCC:
                return None
            out.append(GCC[pointee] + '*')
            index += 2
            continue
        if char == 'S':                       # substitution: repeats a type
            end = rest.find('_', index)
            if end < 0 or not out:
                return None
            out.append(out[-1])
            index = end + 1
            continue
        if char not in GCC:
            return None
        out.append(GCC[char])
        index += 1
    return [one for one in out if one != 'void']


def base_key(name):
    """The undecorated stem, which is what survives a wrong decoration."""
    match = re.match(r'^\?(\??[\w@]+?)@@', name)
    return match.group(1) if match else None


def read_definitions(def_path):
    pairs = []
    for line in def_path.read_text(encoding="utf-8").splitlines():
        match = re.match(r'\s*"([^"]+)"\s*=\s*(\S+)', line)
        if match:
            pairs.append(match.groups())
    return pairs


def read_catalogue(functions_path):
    catalogue = defaultdict(list)
    with functions_path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            key = base_key(row["name"])
            if key:
                catalogue[key].append(row)
    return catalogue


def signed_arithmetic(exe_path):
    """address -> the signed-only mnemonics the original body contains."""
    try:
        import capstone
        import pefile
    except ImportError:
        return None
    if not exe_path.is_file():
        return None
    image = pefile.PE(str(exe_path))
    base = image.OPTIONAL_HEADER.ImageBase

    def body(start, end):
        for section in image.sections:
            begin = base + section.VirtualAddress
            if begin <= start < begin + section.Misc_VirtualSize:
                offset = start - begin
                return section.get_data()[offset:offset + (end - start)]
        return b""

    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    return body, decoder


def audit(functions_path, def_path, exe_path):
    catalogue = read_catalogue(functions_path)
    disassembler = signed_arithmetic(exe_path)
    findings = []
    compared = 0
    for decorated, symbol in read_definitions(def_path):
        rows = catalogue.get(base_key(decorated) or "", [])
        if len(rows) != 1:
            continue
        row = rows[0]
        original = msvc_types(row["name"])
        recovered = gcc_types(symbol)
        if original is None or recovered is None:
            continue
        original = [one for one in original[1:] if one != "void"]
        if len(original) != len(recovered):
            continue
        compared += 1
        diffs = [(index, was, now)
                 for index, (was, now) in enumerate(zip(original, recovered))
                 if was != now and {was, now} in SIGNED_PAIRS]
        if not diffs:
            continue
        kind = "unranked"
        if disassembler:
            body, decoder = disassembler
            try:
                start = int(row["address"], 16)
                end = int(row["end_address"], 16)
                mnemonics = {one.mnemonic
                             for one in decoder.disasm(body(start, end), start)}
                if {"idiv", "cdq"} & mnemonics:
                    kind = "signed_divide"
                elif "sar" in mnemonics:
                    kind = "signed_shift"
                else:
                    kind = "bounded"
            except (ValueError, KeyError):
                kind = "unranked"
        findings.append({"alias": decorated, "original": row["name"],
                         "address": row["address"], "kind": kind,
                         "arguments": [{"index": index, "original": was,
                                        "recovered": now}
                                       for index, was, now in diffs]})
    return compared, findings


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--def-file", type=Path, default=DEFAULT_DEF)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--check", action="store_true",
                        help="fail if the candidate population has grown")
    arguments = parser.parse_args()

    compared, findings = audit(arguments.functions, arguments.def_file,
                               arguments.exe)
    counts = defaultdict(int)
    for one in findings:
        counts[one["kind"]] += 1

    if arguments.json:
        print(json.dumps({"compared": compared, "counts": dict(counts),
                          "findings": findings}, indent=2))
    else:
        print(f"exports compared: {compared}")
        print(f"signedness disagreements: {len(findings)}")
        for kind in ("signed_divide", "signed_shift", "bounded", "unranked"):
            if counts[kind]:
                print(f"  {kind:>14}: {counts[kind]}")
        for one in sorted(findings, key=lambda f: (f["kind"] != "signed_divide",
                                                   f["original"])):
            arguments_text = ", ".join(
                f"arg{a['index']} {a['original']}->{a['recovered']}"
                for a in one["arguments"])
            print(f"\n  [{one['kind']}] {one['address']} {one['original']}")
            print(f"      alias {one['alias']}")
            print(f"      {arguments_text}")

    if arguments.check:
        grew = []
        if len(findings) > BASELINE["disagreements"]:
            grew.append(f"disagreements {len(findings)} > "
                        f"{BASELINE['disagreements']}")
        if counts["signed_divide"] > BASELINE["signed_divide"]:
            grew.append(f"signed_divide {counts['signed_divide']} > "
                        f"{BASELINE['signed_divide']}")
        if grew:
            print("\naudit-export-signedness: FAILED - " + "; ".join(grew),
                  file=sys.stderr)
            print("A new recovery declared a parameter with the opposite "
                  "signedness to the original's mangled name.", file=sys.stderr)
            return 1
        print("\naudit-export-signedness: within baseline "
              f"({len(findings)} <= {BASELINE['disagreements']}, "
              f"{counts['signed_divide']} <= {BASELINE['signed_divide']})")
    return 0


if __name__ == "__main__":
    sys.exit(main())

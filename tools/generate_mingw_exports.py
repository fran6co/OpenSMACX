#!/usr/bin/env python3

import argparse
from collections import defaultdict
from pathlib import Path
import subprocess

import pefile
import undname


def split_parameters(parameters):
    if parameters.strip() in ("", "void"):
        return []

    result = []
    start = 0
    depth = 0
    for index, character in enumerate(parameters):
        if character in "(<[":
            depth += 1
        elif character in ")>]":
            depth -= 1
        elif character == "," and depth == 0:
            result.append(parameters[start:index].strip())
            start = index + 1
    result.append(parameters[start:].strip())
    return result


def signature(value):
    open_paren = value.find("(")
    close_paren = value.rfind(")")
    if open_paren < 0 or close_paren < open_paren:
        raise ValueError(f"cannot parse signature: {value}")

    prefix = value[:open_paren].strip()
    name = prefix.split()[-1].replace("[abi:cxx11]", "")
    parameters = split_parameters(value[open_paren + 1:close_paren])
    normalized = tuple(
        parameter.replace("class ", "").replace("struct ", "")
        .replace("enum ", "").replace(" ", "")
        for parameter in parameters
    )
    return name, normalized


def load_imports(path):
    pe = pefile.PE(str(path))
    try:
        entry = next(
            entry for entry in pe.DIRECTORY_ENTRY_IMPORT
            if entry.dll.lower() == b"opensmacx.dll")
        return [item.name.decode("ascii") for item in entry.imports]
    finally:
        pe.close()


def load_symbols(path, nm, cxxfilt):
    nm_result = subprocess.run(
        [nm, "--defined-only", str(path)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    names = []
    for line in nm_result.stdout.splitlines():
        fields = line.split()
        if len(fields) == 3 and fields[1] in ("T", "t") and fields[2].startswith("__Z"):
            names.append(fields[2])

    result = subprocess.run(
        [cxxfilt],
        input="\n".join(names),
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    demangled = result.stdout.splitlines()
    if len(names) != len(demangled):
        raise RuntimeError("C++ demangler returned an unexpected number of lines")
    return list(zip(names, demangled))


def main():
    parser = argparse.ArgumentParser(
        description="Generate MSVC export aliases for an i686 MinGW OpenSMACX DLL")
    parser.add_argument("--patched-exe", required=True,
                        help="Executable processed by ImportAdder.exe")
    parser.add_argument("--dll", required=True, help="MinGW OpenSMACX DLL")
    parser.add_argument("--output", required=True, help="Output module definition file")
    parser.add_argument("--nm", default="i686-w64-mingw32-nm")
    parser.add_argument("--cxxfilt", default="i686-w64-mingw32-c++filt")
    args = parser.parse_args()

    imports = load_imports(Path(args.patched_exe))
    exports = load_symbols(Path(args.dll), args.nm, args.cxxfilt)

    exact_candidates = defaultdict(list)
    arity_candidates = defaultdict(list)
    for mangled, demangled in exports:
        try:
            key = signature(demangled)
        except ValueError:
            continue
        exact_candidates[key].append((mangled, demangled))
        arity_candidates[(key[0], len(key[1]))].append((mangled, demangled))

    aliases = []
    failures = []
    for imported in imports:
        demangled = undname.undname(imported)
        key = signature(demangled)
        matches = exact_candidates[key]
        if len(matches) != 1:
            matches = arity_candidates[(key[0], len(key[1]))]
        if len(matches) > 1 and key[0].split("::")[-1].lstrip("~") in key[0]:
            preferred = [
                match for match in matches
                if "C1E" in match[0] or "D1E" in match[0]
            ]
            if len(preferred) == 1:
                matches = preferred
        if len(matches) != 1:
            failures.append((imported, demangled, matches))
            continue
        aliases.append((imported, matches[0][0], demangled, matches[0][1]))

    if failures:
        for imported, demangled, matches in failures:
            print(f"No unique match for {imported}: {demangled}")
            for mangled, candidate in matches:
                print(f"  {mangled}: {candidate}")
        raise RuntimeError(f"could not uniquely map {len(failures)} exports")

    output = Path(args.output)
    with output.open("w", encoding="ascii", newline="\n") as file:
        file.write("LIBRARY OpenSMACX\nEXPORTS\n")
        for imported, exported, _imported_name, _exported_name in aliases:
            file.write(f'    "{imported}" = {exported[1:]}\n')

    print(f"Mapped {len(aliases)} MSVC imports to MinGW exports in {output}")


if __name__ == "__main__":
    main()

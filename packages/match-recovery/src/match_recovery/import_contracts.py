"""Stack contracts of imported Windows functions, from their header-derived import names.

User decision (2026-11-16): "Take the Windows's functions stack effect from
it's documented signature". A call to an imported Windows API function is
proved by that function's documented signature, not by its bytes (they are not
in the image). The documented signature lives in the SDK headers the build
compiles against; the compiler and the SDK import libraries encode it in the
decorated import name, which this module reads mechanically:

* `_Name@N`  - `__stdcall` (WINAPI/APIENTRY): the callee pops N argument bytes.
* `_Name`    - `__cdecl` (e.g. `wsprintfA`, declared WINAPIV): pops nothing.

Anything else (fastcall `@Name@N`, C++ `?Name@@...`, data/const imports, an
undecorated symbol whose export name equals the symbol, conflicting
decorations) gives no contract, and the caller keeps refusing.

Evidence source: the Microsoft short-format import objects
(https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-library-format)
inside the project's declared toolchain import libraries (for VC6,
`VC98/LIB/KERNEL32.LIB` and the others). Each object maps one decorated public
symbol to a DLL and export name (or ordinal); the export name is derived by
the object's own name-type rule and must equal the image's own import name.
The image's import directory (`PeImage.imports`) selects the loader slot. The
same documented contract also states that the callee hands EBX, ESI, EDI and
EBP back unchanged (the Win32 x86 calling convention for every documented API
function); `callee_preservation` applies that only to slots carrying a contract
from this module. No caller code is consulted to infer a signature.
"""

from __future__ import annotations

from dataclasses import dataclass
import os
from pathlib import Path
import re
import struct
import weakref

from .coff_archive import CoffArchive
from .pe_coff import PeImage


@dataclass(frozen=True)
class ImportContract:
    module: str
    name: str | None
    ordinal: int | None
    symbol: str          # decorated public symbol, e.g. `_WritePrivateProfileStringA@20`
    convention: str      # "stdcall" or "cdecl"
    popped: int          # argument bytes the callee removes on return
    evidence: str        # import library label


_STDCALL = re.compile(r"_([A-Za-z_][A-Za-z0-9_]*)@([0-9]+)")
_CDECL = re.compile(r"_([A-Za-z_][A-Za-z0-9_]*)")
_NAME_TYPES = {0: "ordinal", 1: "name", 2: "noprefix", 3: "undecorate"}


def _decorated(symbol: str) -> tuple[str, str, int] | None:
    """(undecorated name, convention, popped bytes) from a C import symbol."""
    match = _STDCALL.fullmatch(symbol)
    if match is not None:
        popped = int(match.group(2))
        return (match.group(1), "stdcall", popped) if popped % 4 == 0 and popped <= 0x400 else None
    match = _CDECL.fullmatch(symbol)
    if match is not None:
        return match.group(1), "cdecl", 0
    return None


def library_contracts(data: bytes, label: str) -> list[ImportContract]:
    """Every code import object of one import library that carries a C decoration."""
    contracts = []
    for bodies in CoffArchive(data).members.values():
        for body in bodies:
            if len(body) < 20 or body[:4] != b"\0\0\xff\xff":
                continue  # long-format members (descriptors, thunks) name no export
            _, machine, _, size, hint, kind = struct.unpack_from("<HHIIHH", body, 4)
            strings = body[20:20 + size].split(b"\0")
            if machine != 0x14C or len(strings) < 2 or (kind & 3) != 0:
                continue  # not i386 code (data/const imports have no call contract)
            try:
                symbol, module = strings[0].decode("ascii"), strings[1].decode("ascii")
            except UnicodeDecodeError:
                continue
            decoded = _decorated(symbol)
            name_type = _NAME_TYPES.get((kind >> 2) & 7)
            if decoded is None or name_type is None or not module:
                continue
            undecorated, convention, popped = decoded
            if name_type == "ordinal":
                name, ordinal = None, hint
            elif name_type == "name":
                continue  # the export would be the decorated text itself
            elif name_type == "noprefix":
                name, ordinal = symbol[1:], None
            else:
                name, ordinal = undecorated, None
            contracts.append(ImportContract(module, name, ordinal, symbol, convention, popped, label))
    return contracts


def image_import_contracts(image: PeImage, libraries) -> dict[int, ImportContract]:
    """IAT slot -> contract for the image imports some library uniquely decorates.

    `libraries` is an iterable of (label, archive bytes). An import named with
    different contracts by different objects/libraries gets none.
    """
    by_key: dict[tuple, set] = {}
    for label, data in libraries:
        for contract in library_contracts(data, label):
            key = (contract.module.casefold(), contract.name, contract.ordinal)
            by_key.setdefault(key, set()).add(contract)
    result = {}
    for imported in image.imports():
        found = by_key.get((imported.module.casefold(), imported.name, imported.ordinal), ())
        if len({(item.convention, item.popped) for item in found}) == 1:
            result[imported.address] = min(found, key=lambda item: (item.evidence, item.symbol))
    return result


_LIBRARY_CACHE: dict[tuple, bytes] = {}


def toolchain_import_libraries(root: Path, image: PeImage) -> list[tuple[str, bytes]]:
    """The import libraries in the declared toolchain directories that name an image DLL.

    Only top-level `*.lib` files of the project's declared toolchain
    directories are read (their contents are part of the measurement's
    toolchain identity). A library is kept when its bytes mention one of the
    image's import DLL names; library file names are not trusted to match DLL
    names (MSVFW32.dll is served by VFW32.LIB). A DLL no library names simply
    has no contracts.
    """
    from .toolchain import declared_directories
    modules = sorted({item.module for item in image.imports()})
    if not modules:
        return []
    needles = [module.casefold().encode("ascii") + b"\0" for module in modules]
    found = []
    for directory in declared_directories(root):
        try:
            entries = sorted(os.scandir(directory), key=lambda entry: entry.name)
        except OSError:
            continue
        for entry in entries:
            if not entry.name.casefold().endswith(".lib") or not entry.is_file():
                continue
            stat = entry.stat()
            key = (entry.path, stat.st_size, stat.st_mtime_ns, stat.st_ino, tuple(needles))
            data = _LIBRARY_CACHE.get(key)
            if data is None:
                data = Path(entry.path).read_bytes()
                if not data.startswith(b"!<arch>\n"):
                    data = b""
                else:
                    lowered = data.lower()
                    data = data if any(needle in lowered for needle in needles) else b""
                _LIBRARY_CACHE[key] = data
            if data:
                found.append((entry.path, data))
    return found


_DECLARED: "weakref.WeakKeyDictionary[PeImage, dict[int, ImportContract]]" = weakref.WeakKeyDictionary()


def declare_import_contracts(image: PeImage, contracts) -> None:
    """Bind import contracts (IAT slot -> ImportContract) to one parsed image.

    Replaces any earlier declaration; `callee_preservation` rebuilds its
    cached verdicts for the image when the declaration changes.
    """
    if type(image) is not PeImage:
        raise ValueError("import contracts require a pinned parsed PE image")
    slots = {imported.address for imported in image.imports()}
    checked = {}
    for slot, contract in contracts.items():
        if (type(slot) is not int or slot not in slots or type(contract) is not ImportContract
                or type(contract.popped) is not int or contract.popped < 0 or contract.popped % 4):
            raise ValueError(f"invalid import contract for slot {slot!r}")
        checked[slot] = contract
    _DECLARED[image] = checked


def declared_import_contracts(image: PeImage) -> dict[int, ImportContract]:
    return dict(_DECLARED.get(image, {}))


def declare_toolchain_import_contracts(root: Path, image: PeImage) -> dict[int, ImportContract]:
    """Read the toolchain's import libraries and bind their contracts to `image`."""
    try:
        contracts = image_import_contracts(image, toolchain_import_libraries(root, image))
    except ValueError:
        contracts = {}  # unreadable evidence: every import call keeps refusing
    declare_import_contracts(image, contracts)
    return contracts

"""Linker-generated storage assigns only verified import-thunk recipes."""

import struct

from match_recovery.linker_generated_storage import assign_linker_contributions


class StubImage:
    def __init__(self, imports, blobs):
        self._imports = imports
        self._blobs = blobs

    def imports(self):
        return self._imports

    def read(self, span):
        for (start, end), data in self._blobs:
            if start <= span.start and span.end <= end:
                return data[span.start - start:span.end - start]
        raise ValueError("span outside stub")


class StubImport:
    def __init__(self, module, name, ordinal, address):
        self.module, self.name, self.ordinal, self.address = module, name, ordinal, address


def modules(*rows):
    return [{"index": index, "name": name, "object_name": obj, "kind": kind}
            for index, (name, obj, kind) in enumerate(rows)]


def contributions(*rows):
    return [{"index": index, "module": module, "address": address, "size": size}
            for index, (module, address, size) in enumerate(rows)]


def test_verified_thunk_recipe_assigns_to_its_import():
    slot = 0x405000
    thunk = b"\xff\x25" + struct.pack("<I", slot)
    image = StubImage([StubImport("KERNEL32.dll", "ReadFile", None, slot)],
                      [((0x401000, 0x401010), thunk + b"\xcc" * 10)])
    report = assign_linker_contributions(
        contributions((0, 0x401000, 6)), modules(("* Linker *", "", "linker_generated")), image)
    record, = report["records"]
    assert record["result"] == "pass" and report["assigned"] == 1
    assert record["assignment"]["kind"] == "import_thunk"
    assert record["assignment"]["symbol"] == "ReadFile"
    assert record["assignment"]["slot"] == "0x00405000"


def test_unbound_operand_and_wrong_shape_stay_unmatched():
    image = StubImage([StubImport("KERNEL32.dll", "ReadFile", None, 0x405000)],
                      [((0x401000, 0x401020), b"\x90" * 0x20)])
    wrong_slot = b"\xff\x25" + struct.pack("<I", 0x406000)
    report = assign_linker_contributions(
        contributions((0, 0x401000, 6), (0, 0x401008, 5), (0, 0x401010, 6)),
        modules(("* Linker *", "", "linker_generated")), image)
    assert report["assigned"] == 0 and report["unassigned"] == 3
    assert all(record["assignment"] is None for record in report["records"])
    assert report["result"] == "unknown"


def test_ordinary_modules_are_never_considered():
    image = StubImage([], [])
    report = assign_linker_contributions(
        contributions((0, 0x401000, 6)), modules(("unit.obj", "unit.obj", "standalone_object")), image)
    assert report["records"] == [] and report["result"] == "pass"

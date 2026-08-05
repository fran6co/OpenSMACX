#!/usr/bin/env python3

import struct
import unittest

import emit_target_object as eto


class CallRelocationsTest(unittest.TestCase):
    FUNCTIONS = {
        0x00500000: {"name": "?known@@YAXXZ"},
        0x00600000: {"name": ""},
    }

    def test_catalogued_call_becomes_a_relocation_and_is_zeroed(self):
        # call 0x00500000 from 0x00400000: E8 rel32, rel32 = 0x000FFFFB
        rel32 = 0x00500000 - (0x00400000 + 5)
        text = bytearray(b"\xe8" + struct.pack("<i", rel32) + b"\xc3")
        relocs, externals = eto.call_relocations(text, 0x00400000, self.FUNCTIONS)

        self.assertEqual([(1, "?known@@YAXXZ", eto.IMAGE_REL_I386_REL32)], relocs)
        self.assertEqual(["?known@@YAXXZ"], externals)
        self.assertEqual(b"\xe8\x00\x00\x00\x00\xc3", bytes(text))

    def test_uncatalogued_call_keeps_its_bytes(self):
        rel32 = 0x00700000 - (0x00400000 + 5)
        original = b"\xe8" + struct.pack("<i", rel32) + b"\xc3"
        text = bytearray(original)
        relocs, externals = eto.call_relocations(text, 0x00400000, self.FUNCTIONS)

        self.assertEqual([], relocs)
        self.assertEqual([], externals)
        self.assertEqual(original, bytes(text))

    def test_nameless_catalogue_row_is_treated_as_uncatalogued(self):
        rel32 = 0x00600000 - (0x00400000 + 5)
        text = bytearray(b"\xe8" + struct.pack("<i", rel32) + b"\xc3")
        relocs, _ = eto.call_relocations(text, 0x00400000, self.FUNCTIONS)

        self.assertEqual([], relocs)

    def test_self_reference_is_not_relocated(self):
        # Infinite-loop jmp to itself must stay a raw branch.
        text = bytearray(b"\xeb\xfe")
        relocs, _ = eto.call_relocations(text, 0x00400000, self.FUNCTIONS)

        self.assertEqual([], relocs)


class SymbolResolverTest(unittest.TestCase):
    """The relocation has to name what the RECOVERED SOURCE will emit.

    A `call sub_401520` relocated against the catalogued label pairs with
    nothing, because the recovered unit declares that callee `extern "C"` and
    CL writes `_sub_401520@4`. The mismatch shows up as a `call` diff inside a
    body that is otherwise byte-exact, and no edit to the call site fixes it.
    """

    LABEL = {"address": "0x00401520", "name": "sub_401520",
             "prototype": "int (__stdcall sub_401520)(int)",
             "size": "16", "body_ranges": "0x00401520-0x00401530"}
    MANGLED = {"address": "0x00500000", "name": "?known@@YAXXZ",
               "prototype": "void (__cdecl ?known@@YAXXZ)()",
               "size": "16", "body_ranges": "0x00500000-0x00500010"}

    def test_a_label_resolves_to_its_c_decoration(self):
        resolver = eto.SymbolResolver({})
        self.assertEqual("_sub_401520@4",
                         resolver.of(0x00401520, self.LABEL))

    def test_a_mangled_name_resolves_to_itself(self):
        resolver = eto.SymbolResolver({})
        self.assertEqual("?known@@YAXXZ", resolver.of(0x00500000, self.MANGLED))

    def test_an_unsettled_row_gets_the_declaration_the_emitter_writes(self):
        # `declare_callee` falls back to `extern "C" int name();` - __cdecl,
        # so no argument count. Both sides have to make the same guess.
        resolver = eto.SymbolResolver({})
        row = dict(self.LABEL, prototype="")
        self.assertEqual("_sub_401520", resolver.of(0x00401520, row))

    def test_the_relocation_carries_the_resolved_symbol(self):
        rel32 = 0x00401520 - (0x00400000 + 5)
        text = bytearray(b"\xe8" + struct.pack("<i", rel32) + b"\xc3")
        relocs, externals = eto.call_relocations(
            text, 0x00400000, {0x00401520: self.LABEL},
            eto.SymbolResolver({}))

        self.assertEqual(["_sub_401520@4"], externals)
        self.assertEqual([(1, "_sub_401520@4", eto.IMAGE_REL_I386_REL32)],
                         relocs)


class BuildCoffTest(unittest.TestCase):
    def test_layout_header_section_text_relocs_symtab_strings(self):
        text = b"\xe8\x00\x00\x00\x00\xc3"
        relocs = [(1, "?known@@YAXXZ", eto.IMAGE_REL_I386_REL32)]
        coff = eto.build_coff(text, "?fn@@YAXXZ", relocs, ["?known@@YAXXZ"])

        machine, nsec, _, symptr, nsym, opthdr, _ = struct.unpack_from("<HHIIIHH", coff, 0)
        self.assertEqual(0x014C, machine)
        self.assertEqual(1, nsec)
        self.assertEqual(2, nsym)  # function + one external
        self.assertEqual(0, opthdr)

        # Section header: .text, raw data right after the headers.
        name, _, _, raw_size, raw_ptr, reloc_ptr, _, nreloc, _, chars = \
            struct.unpack_from("<8sIIIIIIHHI", coff, 20)
        self.assertEqual(b".text\x00\x00\x00", name)
        self.assertEqual(len(text), raw_size)
        self.assertEqual(60, raw_ptr)
        self.assertEqual(1, nreloc)
        self.assertEqual(0x60500020, chars)

        # Text bytes follow the headers verbatim.
        self.assertEqual(text, coff[raw_ptr:raw_ptr + raw_size])

        # One REL32 relocation at offset 1 targeting symbol index 1.
        r_off, r_sym, r_type = struct.unpack_from("<IIH", coff, reloc_ptr)
        self.assertEqual((1, 1, eto.IMAGE_REL_I386_REL32), (r_off, r_sym, r_type))

        # Symbol table: function symbol in section 1, external in section 0.
        _, fn_sec, fn_type, fn_class, _ = struct.unpack_from("<8xIhHBB", coff, symptr)
        self.assertEqual((1, eto.SYM_FUNCTION, eto.SYM_CLASS_EXTERNAL),
                         (fn_sec, fn_type, fn_class))

        # String table starts with its own size and holds both long names.
        str_size = struct.unpack_from("<I", coff, symptr + nsym * 18)[0]
        strings = coff[symptr + nsym * 18: symptr + nsym * 18 + str_size]
        self.assertIn(b"?fn@@YAXXZ\x00", strings)
        self.assertIn(b"?known@@YAXXZ\x00", strings)

    def test_short_symbol_name_is_inlined_not_stringed(self):
        coff = eto.build_coff(b"\xc3", "_free", [], [])
        _, _, _, symptr, _, _, _ = struct.unpack_from("<HHIIIHH", coff, 0)
        self.assertEqual(b"_free\x00\x00\x00", coff[symptr:symptr + 8])


if __name__ == "__main__":
    unittest.main()

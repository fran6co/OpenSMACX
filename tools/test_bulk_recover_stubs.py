#!/usr/bin/env python3

import unittest
from unittest import mock

import bulk_recover_stubs as bulk


class ClassifyTest(unittest.TestCase):
    """Only a provable constant return with agreeing arity becomes a Stub.

    These are the two checks that keep the bulk path from emitting something
    plausible and wrong - the same pair that reject Win::on_redraw across the
    whole image."""

    def row(self, name, size):
        return {"name": name, "address": "0x0045C170", "size": str(size)}

    def classify(self, name, size, body):
        pe = mock.Mock()
        with mock.patch.object(bulk.const, "read_bytes", return_value=b""), \
             mock.patch.object(bulk.const, "constant_body", return_value=body):
            return bulk.classify(self.row(name, size), pe)

    def test_accepts_a_void_bare_return(self):
        stub = self.classify("?close@BaseWin@@QAEXXZ", 1, (None, 0))
        self.assertEqual(("close", "thiscall", 0, None, "void"),
                         (stub.method, stub.convention, stub.param_count,
                          stub.returns, stub.return_type))

    def test_accepts_a_constant_return_with_parameters(self):
        stub = self.classify("?UNK1@Font@@QAEHHHHH@Z", 6, (1, 16))
        self.assertEqual(("UNK1", 4, 1, "int"),
                         (stub.method, stub.param_count, stub.returns,
                          stub.return_type))

    def test_rejects_when_arity_and_cleanup_disagree(self):
        # Win::on_redraw: declares no parameters, body pops eight bytes.
        self.assertIsNone(
            self.classify("?on_redraw@Win@@QAEHXZ", 8, (0, 8)))

    def test_rejects_a_non_constant_body(self):
        self.assertIsNone(
            self.classify("?close@BaseWin@@QAEXXZ", 40, None))

    def test_rejects_a_body_that_is_too_large(self):
        # Guarded by the size window before the body is even read.
        pe = mock.Mock()
        self.assertIsNone(bulk.classify(self.row("?x@Y@@QAEXXZ", 64), pe))

    def test_reads_a_cdecl_static(self):
        stub = self.classify("?close_class@CheckButton@@QAAXXZ", 1, (None, 0))
        self.assertEqual("cdecl", stub.convention)
        self.assertEqual(0, stub.param_count)


class RenderTest(unittest.TestCase):
    def stub(self, **kw):
        base = dict(method="close", address="0x0045C170", convention="thiscall",
                    param_count=0, returns=None, return_type="void")
        base.update(kw)
        return bulk.Stub(**base)

    def test_declaration_of_a_void_thiscall(self):
        text = bulk.render_declarations([self.stub()])
        self.assertEqual("  void close();", text)

    def test_declaration_of_an_int_return_with_params(self):
        text = bulk.render_declarations(
            [self.stub(method="UNK1", param_count=2, returns=1, return_type="int")])
        self.assertEqual("  int UNK1(int, int);", text)

    def test_declaration_of_a_cdecl_static(self):
        text = bulk.render_declarations(
            [self.stub(method="close_class", convention="cdecl")])
        self.assertEqual("  static void close_class();", text)

    def test_definition_carries_its_own_original_offset(self):
        text = bulk.render_definitions("BaseWin", [self.stub(address="0x00408710")])
        # The exporter reads one annotation per function; every stub must have
        # its own, or it lands unrecovered despite being implemented.
        self.assertIn("Original Offset: 00408710", text)
        self.assertIn("void BaseWin::close()", text)

    def test_definition_of_a_constant_return_forwards_the_value(self):
        text = bulk.render_definitions(
            "Font", [self.stub(method="UNK1", param_count=4, returns=1,
                               return_type="int", address="0x00618F30")])
        self.assertIn("return 1;", text)
        self.assertIn("return self->UNK1(a1, a2, a3, a4);", text)

    def test_redirect_of_a_cdecl_static_takes_no_self(self):
        text = bulk.render_redirect_declarations(
            "CheckButton", [self.stub(method="close_class", convention="cdecl")])
        self.assertIn("void __cdecl check_button_close_class_redirect();", text)

    def test_redirect_of_a_thiscall_takes_self_and_args(self):
        text = bulk.render_redirect_declarations(
            "Font", [self.stub(method="UNK1", param_count=2, returns=1,
                               return_type="int")])
        self.assertIn(
            "int __fastcall font_unk1_redirect(Font *self, void *, int a1, int a2);",
            text)


class SnakeTest(unittest.TestCase):
    def test_inserts_underscores_at_word_boundaries(self):
        self.assertEqual("base_win", bulk.snake("BaseWin"))

    def test_collapses_a_pre_existing_underscore(self):
        # Midi_Device would otherwise become midi__device.
        self.assertEqual("midi_device", bulk.snake("Midi_Device"))


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3
"""The two controls that caught this tool being wrong, both ways.

A detector for "the catalogue calls this void but it returns a status" is only
worth having if it finds the known case and does not find the known non-case.
This one failed each of those in turn during development, so both are pinned.
"""
from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match                                             # noqa: E402
import verify_void_returns as tool                            # noqa: E402

HAVE_IMAGE = byte_match.DEFAULT_EXE.is_file()


class NameDecodingTests(unittest.TestCase):
    """Read through the same decoder the emitter uses, or the two disagree."""

    def test_a_void_return_is_recognised(self):
        self.assertTrue(tool.returns_void("?editor_fast@Console@@QAEXXZ"))
        self.assertTrue(
            tool.returns_void("?init@Cursor@@QAEXPAUSprite@@HH@Z"))

    def test_a_non_void_return_is_not(self):
        self.assertFalse(tool.returns_void("?immune@SocialWin@@QAEHHHH@Z"))

    def test_an_undecorated_name_is_skipped_not_guessed(self):
        for name in ("sub_63b2e0", "", "j_something"):
            self.assertFalse(tool.returns_void(name), name)


@unittest.skipUnless(HAVE_IMAGE, "needs the shipped terranx.exe")
class ControlTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
        cls.data, cls.base = tool.image()
        cls.engine = Cs(CS_ARCH_X86, CS_MODE_32)

    def constants(self, spans):
        return tool.exit_constants(self.data, self.base, self.engine, spans)[0]

    def test_the_positive_control_is_found(self):
        # `?init@Cursor@@QAEX...` returns 0/1/2/3/7 - read off the disassembly
        # by an agent recovering it, before this tool existed.
        #
        # THE FIRST VERSION SCORED THIS ZERO. It looked for the last write to
        # EAX before each `ret`, and cl 12.00.8168 at /O2 emits ONE shared
        # epilogue: every `return N` is `mov eax, N; jmp <epilogue>`, so the
        # whole function has a single `ret` and a ret-only walk sees one value
        # no matter how many the source returned.
        found = self.constants([(0x0063B2E0, 0x0063B8CA),
                                (0x00663910, 0x0066393E)])
        self.assertEqual(set(found), {0, 1, 2, 3, 7})

    def test_the_noise_control_is_not_found(self):
        # `?base_production@@YAXXZ` genuinely returns nothing. Counting every
        # `mov eax, N; jmp` without following the jump reported SIXTEEN
        # constants for it, four of them addresses - because that is also how
        # a switch arm hands a value to shared code that stores it.
        self.assertEqual(self.constants([(0x004F07E0, 0x004F47AE)]), {})

    def test_an_address_sized_constant_does_not_survive(self):
        # The same walk used to report 6816888 (0x680478) as one of
        # Cursor::init's return values. It is a string address loaded for a
        # call, and following the jump removes it.
        found = self.constants([(0x0063B2E0, 0x0063B8CA),
                                (0x00663910, 0x0066393E)])
        self.assertTrue(all(value < 256 for value in found), found)

    def test_the_answer_stays_small(self):
        # 93 candidates at two constants was the pre-discriminator number and
        # was mostly noise. If this climbs back toward it, the discriminator
        # has stopped discriminating.
        self.assertLess(len(tool.candidates(2)), 25)
        self.assertLessEqual(len(tool.candidates(3)), 3)


if __name__ == "__main__":
    unittest.main()

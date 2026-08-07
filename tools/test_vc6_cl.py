#!/usr/bin/env python3
"""Unit tests for the VC6 command-line wrapper.

CMake drives a compiler with POSIX paths and MSVC reads a leading `/` as the
start of an option, so every absolute path has to become `Z:\\...` before it
reaches CL. What makes that hard is that an option and a path cannot be told
apart by shape - `/c` and a file called `/c` look identical - so the wrapper
asks the filesystem, and the interesting cases are the ones where a path is
GLUED to an option prefix.

That gluing is what broke the build: `/FI`, `/Yc` and `/Yu` were missing from
the prefix list, the path went through untranslated, and VC6 stopped at the
very first object with `D2004: '/FI' requires an argument`.
"""

import tempfile
import unittest
from pathlib import Path

import vc6_cl


class GluedPathTest(unittest.TestCase):
    """An option prefix with an absolute path stuck to it."""

    def setUp(self):
        self.holder = tempfile.TemporaryDirectory()
        self.directory = Path(self.holder.name)
        self.file = self.directory / "pch.hxx"
        self.file.write_text("")

    def tearDown(self):
        self.holder.cleanup()

    def expect(self, prefix):
        translated = vc6_cl.translate(f"{prefix}{self.file}")
        self.assertTrue(translated.startswith(prefix), translated)
        self.assertTrue(translated[len(prefix):].startswith("Z:\\"), translated)
        self.assertNotIn("/", translated[len(prefix):], translated)

    def test_the_precompiled_header_trio_is_translated(self):
        # The three that were missing. Each is a build-stopping failure on
        # the FIRST object, so none of them can be caught late.
        for prefix in ("/FI", "/Yc", "/Yu"):
            self.expect(prefix)

    def test_the_output_prefixes_are_still_translated(self):
        for prefix in ("/Fo", "/Fe", "/Fd", "/Fp", "/Fa", "/I"):
            self.expect(prefix)

    def test_a_linker_prefix_is_translated(self):
        for prefix in ("/OUT:", "/IMPLIB:", "/PDB:", "/LIBPATH:"):
            self.expect(prefix)


class PlainArgumentTest(unittest.TestCase):
    def test_an_option_is_left_alone(self):
        for option in ("/nologo", "/O2", "/W4", "/GX", "/TP", "-MDd"):
            self.assertEqual(vc6_cl.translate(option), option)

    def test_a_relative_path_is_left_alone(self):
        # Wine resolves it against the working directory, and rewriting one
        # would break the response files CMake writes relative to the build
        # tree.
        self.assertEqual(vc6_cl.translate("CMakeFiles/x.obj"),
                         "CMakeFiles/x.obj")

    def test_a_define_that_looks_like_a_path_is_left_alone(self):
        self.assertEqual(vc6_cl.translate("/D_WINDOWS"), "/D_WINDOWS")

    def test_a_response_file_is_translated(self):
        with tempfile.TemporaryDirectory() as directory:
            rsp = Path(directory) / "cl.rsp"
            rsp.write_text("")
            self.assertEqual(vc6_cl.translate(f"@{rsp}"),
                             "@Z:" + str(rsp).replace("/", "\\"))


if __name__ == "__main__":
    unittest.main()

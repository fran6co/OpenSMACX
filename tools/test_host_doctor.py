#!/usr/bin/env python3
"""Unit tests for the host preflight check.

The doctor's whole job is to be believed on a machine nobody has debugged yet,
so the properties worth asserting are the ones where a wrong answer is still a
plausible-looking answer:

* The verdict is a WORST-of, not a majority. One missing dependency has to sink
  the whole report even when eight other rows are green, because the reader
  acts on the last line.
* An unknown image must not read as a matching one. The recorded sha256 is what
  ties every address in this repo to a specific build; a comparison that
  silently passed on a different binary would invalidate every number without
  saying anything.
* Optimism about the CPU is the one failure that cannot be recovered from
  later. If a translated host were reported as native, someone would drop the
  undefined-flag masks and start comparing a translator's arbitrary answers -
  so every uncertain case must resolve to "not native".
"""
from __future__ import annotations

import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import host_doctor as doctor  # noqa: E402


class ReportVerdict(unittest.TestCase):
    def test_all_ok_is_ok(self):
        report = doctor.Report()
        report.add(doctor.OK, "a", "fine")
        report.add(doctor.OK, "b", "fine")
        self.assertEqual(report.worst(), doctor.OK)

    def test_one_missing_sinks_a_green_report(self):
        report = doctor.Report()
        for name in "abcdefgh":
            report.add(doctor.OK, name, "fine")
        report.add(doctor.BAD, "wine", "not found")
        self.assertEqual(report.worst(), doctor.BAD)

    def test_bad_outranks_warn(self):
        report = doctor.Report()
        report.add(doctor.WARN, "a", "translated")
        report.add(doctor.BAD, "b", "absent")
        self.assertEqual(report.worst(), doctor.BAD)

    def test_warn_alone_is_warn_not_ok(self):
        report = doctor.Report()
        report.add(doctor.OK, "a", "fine")
        report.add(doctor.WARN, "b", "translated")
        self.assertEqual(report.worst(), doctor.WARN)

    def test_empty_report_does_not_crash(self):
        self.assertEqual(doctor.Report().worst(), doctor.OK)

    def test_fix_text_is_shown_for_problems_and_hidden_for_ok(self):
        report = doctor.Report()
        report.add(doctor.BAD, "wine", "not found", "apt install wine32:i386")
        report.add(doctor.OK, "cmake", "present", "never printed")
        rendered = report.render()
        self.assertIn("apt install wine32:i386", rendered)
        self.assertNotIn("never printed", rendered)


class ImageIdentity(unittest.TestCase):
    def _image(self, directory: str, payload: bytes) -> Path:
        path = Path(directory) / "terranx_original.exe"
        path.write_bytes(payload)
        return path

    def test_absent_image_is_bad(self):
        with tempfile.TemporaryDirectory() as directory:
            report = doctor.Report()
            doctor.check_image(report, Path(directory) / "nope.exe")
            self.assertEqual(report.worst(), doctor.BAD)

    def test_a_different_build_warns_and_never_claims_a_match(self):
        with tempfile.TemporaryDirectory() as directory:
            report = doctor.Report()
            doctor.check_image(report, self._image(directory, b"not the game"))
            self.assertEqual(report.worst(), doctor.WARN)
            state, _, detail, _ = report.rows[0]
            self.assertIn("differs", detail)
            self.assertNotIn("matches", detail)

    def test_the_recorded_hash_is_the_hash_of_what_it_claims(self):
        # Pins the constant against itself being edited to whatever some future
        # local file happens to hash to.
        with tempfile.TemporaryDirectory() as directory:
            payload = b"x" * 64
            path = self._image(directory, payload)
            report = doctor.Report()
            doctor.KNOWN, saved = None, doctor.KNOWN_IMAGE_SHA256
            try:
                doctor.KNOWN_IMAGE_SHA256 = hashlib.sha256(payload).hexdigest()
                doctor.check_image(report, path)
                self.assertEqual(report.worst(), doctor.OK)
                self.assertIn("matches", report.rows[0][2])
            finally:
                doctor.KNOWN_IMAGE_SHA256 = saved

    def test_the_shipped_constant_is_a_real_sha256(self):
        self.assertEqual(len(doctor.KNOWN_IMAGE_SHA256), 64)
        int(doctor.KNOWN_IMAGE_SHA256, 16)


class CpuHonesty(unittest.TestCase):
    """check_cpu must never return True on anything it is unsure about."""

    def setUp(self):
        self.platform = doctor.platform
        self.run = doctor.run
        self.addCleanup(setattr, doctor, "platform", self.platform)
        self.addCleanup(setattr, doctor, "run", self.run)

    def _fake_platform(self, system: str, machine: str):
        class Fake:
            @staticmethod
            def system():
                return system

            @staticmethod
            def machine():
                return machine
        doctor.platform = Fake

    def test_apple_silicon_is_not_native(self):
        self._fake_platform("Darwin", "arm64")
        doctor.run = lambda argv, timeout=20.0: (0, "M2 Pro")
        report = doctor.Report()
        self.assertFalse(doctor.check_cpu(report))
        self.assertEqual(report.worst(), doctor.WARN)

    def test_rosetta_translated_python_is_not_native(self):
        # uname says x86_64 because the interpreter itself is translated. The
        # kernel is the only honest source, and it says 1.
        self._fake_platform("Darwin", "x86_64")
        doctor.run = lambda argv, timeout=20.0: (
            (0, "1") if "sysctl.proc_translated" in argv else (0, "M2 Pro"))
        report = doctor.Report()
        self.assertFalse(doctor.check_cpu(report))

    def test_real_intel_mac_is_native(self):
        self._fake_platform("Darwin", "x86_64")
        doctor.run = lambda argv, timeout=20.0: (
            (0, "0") if "sysctl.proc_translated" in argv else (0, "Core i9"))
        report = doctor.Report()
        self.assertTrue(doctor.check_cpu(report))
        self.assertEqual(report.worst(), doctor.OK)

    def test_arm_linux_is_bad_not_merely_warned(self):
        self._fake_platform("Linux", "aarch64")
        report = doctor.Report()
        self.assertFalse(doctor.check_cpu(report))
        self.assertEqual(report.worst(), doctor.BAD)

    def test_an_unrecognised_system_is_never_native(self):
        self._fake_platform("SunOS", "x86_64")
        report = doctor.Report()
        self.assertFalse(doctor.check_cpu(report))


if __name__ == "__main__":
    unittest.main()

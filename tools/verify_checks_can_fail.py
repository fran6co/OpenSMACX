#!/usr/bin/env python3
"""Damage the real inputs and require each gate check to go red.

WHY THIS EXISTS. On 2026-08-01 an audit of the 13 mechanical checks added the
day before found three that could not fail:

  * `verify_no_load_time_addresses` matched one of the four ways this tree
    spells the construct it forbids, and never opened a header;
  * `audit_export_signedness` printed `0 <= 44` whenever the executable was
    absent, which is every checkout without the game;
  * `indirect_call_sites` read an edge list as a map, so its filter admitted
    every row it was given.

Every one of them passed its own unit tests, had a docstring asserting what it
guarded, and had a positive control. The controls were the problem: each tested
the single input shape its author already had in mind, so they demonstrated the
check fires on the bug that had already been found and nothing else.

WHAT IS DIFFERENT HERE. A unit test asks whether a function returns the right
value for a synthetic input. This asks whether the SHIPPED check, invoked the
way CMake invokes it, refuses REAL repository content that has been damaged. The
load-time defect is the proof of the distinction: its unit tests passed on
three-line fixtures while `src/console.cpp` had contained an undetected spelling
for weeks.

THE PROPERTY THAT MATTERS MOST is the last one, and it is not a case at all:
every gate check of this family must APPEAR in CASES. A new check with no damage
case fails this tool. That converts the standing rule - "every assertion needs a
positive control; damage the code, confirm it FAILS, revert, record the failure
text" - from something a person must remember into something the gate enforces.

A SKIP IS NOT A PASS. Cases needing artifacts a clean checkout lacks report as
skipped and do not satisfy the coverage requirement for their check.
"""
import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"
PYTHON = sys.executable


class Skip(Exception):
    """A precondition is absent, so this case proves nothing. Never a pass."""


def substitute(text, old, new, count=0):
    """Replace, and refuse if the pattern was not there to replace.

    THE FIRST RUN OF THIS TOOL CAUGHT ITS OWN AUTHOR WITH THIS. The wine-lock
    case removed `RESOURCE_LOCK wineprefix`, but CMake generates
    `RESOURCE_LOCK "wineprefix"` with quotes, so the substitution matched
    nothing, the "damaged" input was identical to the real file, and the check
    passed - which the harness reported as the check being unable to fail.

    A damage case that does not damage is precisely the vacuous control this
    tool exists to detect, so it must be impossible to write one by accident
    here. Every substitution goes through this.
    """
    if old not in text:
        raise Skip(f"damage pattern {old!r} is not present, so this case "
                   f"would prove nothing")
    return text.replace(old, new, count) if count else text.replace(old, new)


def copy_tree(source, destination, patterns):
    destination.mkdir(parents=True, exist_ok=True)
    copied = 0
    for pattern in patterns:
        for path in source.glob(pattern):
            if path.is_file():
                shutil.copy2(path, destination / path.name)
                copied += 1
    if not copied:
        raise Skip(f"nothing matched {patterns} under {source}")
    return destination


# ---------------------------------------------------------------- damage cases

def damage_load_time_address(workspace):
    """A file-scope load through a fixed game address, indented inside a
    namespace and written without the 0x00 padding - the two spellings the
    check used to miss, in a copy of the real src/."""
    source = copy_tree(REPO_ROOT / "src", workspace / "src", ("*.cpp", "*.h"))
    victim = source / "console.cpp"
    if not victim.is_file():
        raise Skip("src/console.cpp is absent")
    victim.write_text(
        victim.read_text(encoding="utf-8", errors="replace")
        + "\nnamespace {\n"
          "  int *DamageProbe = *reinterpret_cast<int **>(0x669304);\n"
          "}\n",
        encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_no_load_time_addresses.py"),
            "--src", str(source)]


def damage_unregistered_tool_test(workspace):
    """A tools/test_*.py that CMakeLists.txt never executes."""
    tools = copy_tree(TOOLS, workspace / "tools", ("test_*.py",))
    (tools / "test_zzz_damage_probe.py").write_text("", encoding="utf-8")
    cmake = workspace / "CMakeLists.txt"
    shutil.copy2(REPO_ROOT / "CMakeLists.txt", cmake)
    return [PYTHON, str(TOOLS / "verify_tool_test_registration.py"),
            "--cmake", str(cmake), "--tools", str(tools)]


def damage_dropped_gameplay_case(workspace):
    """A test defined and never registered, which is the silent merge loss."""
    suite = REPO_ROOT / "tests" / "recovery_gameplay_tests.cpp"
    if not suite.is_file():
        raise Skip("the gameplay suite is absent")
    text = suite.read_text(encoding="utf-8")
    match = re.search(r"^GAMEPLAY_CASE\(test_[A-Za-z0-9_]+\);\n", text,
                      re.MULTILINE)
    if not match:
        raise Skip("no GAMEPLAY_CASE registration to remove")
    copy = workspace / "suite.cpp"
    copy.write_text(text[:match.start()] + text[match.end():], encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_test_registration.py"),
            "--suite", str(copy)]


def damage_restated_count(workspace):
    """A hand-typed per-state count back in AGENTS.md."""
    document = REPO_ROOT / "AGENTS.md"
    if not document.is_file():
        raise Skip("AGENTS.md is absent")
    copy = workspace / "AGENTS.md"
    copy.write_text(
        document.read_text(encoding="utf-8")
        + "\n\nCurrent recovery state: 2,544 `source_complete` functions.\n",
        encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_documented_counts.py"),
            "--doc", str(copy)]


def damage_wine_lock(workspace):
    """A Wine-backed test that no longer holds the prefix lock."""
    generated = REPO_ROOT / "build" / "mingw-i686-debug" / "CTestTestfile.cmake"
    if not generated.is_file():
        raise Skip("no configured debug build directory")
    text = generated.read_text(encoding="utf-8")
    expected = len(re.findall(r"/run_windows_test\.py", text))
    if not expected:
        raise Skip("no Wine-backed tests in this configuration")
    copy = workspace / "CTestTestfile.cmake"
    copy.write_text(substitute(text, 'RESOURCE_LOCK "wineprefix"', ""),
                    encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_wine_test_locks.py"),
            "--ctest-file", str(copy), "--expect-at-least", "1"]


def damage_blinded_wine_check(workspace):
    """The runner renamed, so the check matches nothing. It used to call that
    'verified NOTHING' and return 0."""
    generated = REPO_ROOT / "build" / "mingw-i686-debug" / "CTestTestfile.cmake"
    if not generated.is_file():
        raise Skip("no configured debug build directory")
    text = generated.read_text(encoding="utf-8")
    if "--wine-prefix" not in text:
        raise Skip("no Wine-backed tests in this configuration")
    copy = workspace / "CTestTestfile.cmake"
    copy.write_text(substitute(text, "--wine-prefix", "--wineprefix"),
                    encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_wine_test_locks.py"),
            "--ctest-file", str(copy), "--expect-at-least", "1"]


def damage_stale_exclusions(workspace):
    """One number in the ```measured fence disagreeing with the image."""
    document = REPO_ROOT / "docs" / "EXCLUSIONS.md"
    exe = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
    if not document.is_file() or not exe.is_file():
        raise Skip("EXCLUSIONS.md or the pinned executable is absent")
    text = document.read_text(encoding="utf-8")
    match = re.search(r"^([a-z_.]+ = )(\d+)$", text, re.MULTILINE)
    if not match:
        raise Skip("no measured entry to perturb")
    copy = workspace / "EXCLUSIONS.md"
    copy.write_text(
        text[:match.start()] + match.group(1) + str(int(match.group(2)) + 7)
        + text[match.end():], encoding="utf-8")
    return [PYTHON, str(TOOLS / "measure_exclusions.py"), "--check",
            "--document", str(copy), "--exe", str(exe)]


def damage_verified_layouts(workspace):
    """A class dropped from the list of layouts proved against the image.

    Dropping rather than adding is deliberate. `test_class_layouts.py`'s
    VerificationGateTest already asserts `verified <= pinned`, so an INVENTED
    name is caught there; a name that quietly stops being verified is not, and
    it is the one that matters - the emitter reads this file to decide whether
    an agent gets real members or an opaque shell.
    """
    listing = REPO_ROOT / "docs" / "recovery" / "verified-layouts.txt"
    if not listing.is_file():
        raise Skip("docs/recovery/verified-layouts.txt is absent")
    lines = listing.read_text(encoding="utf-8").splitlines(keepends=True)
    body = [index for index, line in enumerate(lines)
            if line.strip() and not line.startswith("#")]
    if not body:
        raise Skip("no verified layout to drop")
    copy = workspace / "verified-layouts.txt"
    copy.write_text("".join(line for index, line in enumerate(lines)
                            if index != body[0]), encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_class_layouts.py"), "--check",
            "--verified", str(copy)]


def damage_layout_probe_header(workspace):
    """A header that does not compile, which is NOT a wrong layout.

    This case exists because the distinction was missing and it cost four
    classes. `src/buffer.h` referenced an undeclared `Vert`; `graphicwin.h`
    and `win.h` include it, so the probes for ButtonGroup, MenuEntry,
    PullDownItem and TutWin stopped producing an object. Their layouts were
    correct. The tool reported "verified-layouts.txt is stale", and the
    obvious response - regenerate it - would have dropped all four from the
    list, turned `supplyable()` False for each, and sent every recovered body
    in them to NO_COMPILE with nothing pointing at the typo.

    The damage is applied to a COPY of src/ passed via `--src`, because a gate
    check may not edit the tree it is checking. What must go red is the build
    control specifically, so the expected text is the header-defect wording
    and not "is stale" - the two outcomes are exactly what this separates.
    """
    source = REPO_ROOT / "src"
    header = source / "buffer.h"
    if not header.is_file():
        raise Skip("src/buffer.h is absent")
    copy = workspace / "src"
    shutil.copytree(source, copy)
    (copy / "buffer.h").write_text(
        substitute(header.read_text(encoding="utf-8"),
                   "struct Vert;", "// struct Vert;"),
        encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_class_layouts.py"), "--check",
            "--src", str(copy)]


def damage_byte_match_ratchet(workspace):
    """A body whose annotation claims BYTE_EXACT, edited so it no longer is.

    THE CLAIM IS THE FLOOR, so this is the case that matters most here: if a
    claimed body can be changed and the check still passes, the ratchet is
    decorative. The damage is a CONSTANT, not a deleted statement - the body
    still compiles and still produces a function, so a pass could not be
    excused as "it failed to build".

    THE LINE SPAN COMES FROM THE TREE'S OWN EXTRACTOR. Two earlier attempts
    failed in ways worth recording, because both LOOKED like a working case:
    perturbing the first `return N;` after the marker landed in a later,
    unclaimed function, and `text.replace(annotation.region, ...)` matched
    nothing at all - `extract_body` re-opens a mid-comment extract with a
    synthetic `/*`, so the region is not a substring of the file. Both left
    the file unchanged and the check passing.
    """
    sys.path.insert(0, str(TOOLS))
    import annotation_scan
    import byte_match_census as census

    for source in sorted((REPO_ROOT / "src").glob("*.cpp")):
        if "BYTE_EXACT" not in source.read_text(errors="ignore"):
            continue
        for annotation in annotation_scan.scan_file(source):
            if not annotation.matched or annotation.mode != annotation_scan.MODE_BODY:
                continue
            try:
                _, lines, first, last = census.body_span(annotation.location)
            except (ValueError, OSError):
                continue
            for index in range(first, last + 1):
                damaged, count = re.subn(
                    r"\breturn (\d+);",
                    lambda m: f"return {int(m.group(1)) + 1};",
                    lines[index], count=1)
                if not count:
                    continue
                tree = workspace / "src"
                tree.mkdir(parents=True, exist_ok=True)
                lines = list(lines)
                lines[index] = damaged
                (tree / source.name).write_text("\n".join(lines) + "\n",
                                                encoding="utf-8")
                return [PYTHON, str(TOOLS / "decomp_status.py"), "--check",
                        "--no-ledger", "--src", str(tree)]
    raise Skip("no claimed body with a constant return to perturb")


def damage_writing_test(workspace):
    """A test file that edits and deletes tracked files in its own repository.

    Built as a REAL git repository rather than by pointing the check at this
    one, because the check's verdict is "did the tree change" - so damaging the
    actual tree to prove it would be indistinguishable from the defect it
    hunts, and would leave the operator's work destroyed if the case aborted.

    Two test files, not one: the polite one proves the check does not simply
    report everything it runs. That is the failure mode the first draft had,
    where `__pycache__` made every test look like an offender.
    """
    repo = workspace / "writing-test-repo"
    (repo / "tools").mkdir(parents=True, exist_ok=True)
    (repo / "src").mkdir(parents=True, exist_ok=True)
    (repo / "tracked.txt").write_text("original\n")
    (repo / "src" / "proof.cpp").write_text("// ORIGINAL: 0x00401000 BYTE_EXACT\n")
    (repo / "tools" / "test_polite.py").write_text(
        "import unittest\n"
        "class T(unittest.TestCase):\n"
        "    def test_ok(self): self.assertTrue(True)\n")
    (repo / "tools" / "test_rude.py").write_text(
        "import pathlib, unittest\n"
        "ROOT = pathlib.Path(__file__).resolve().parent.parent\n"
        "class T(unittest.TestCase):\n"
        "    def test_writes(self):\n"
        "        (ROOT / 'tracked.txt').write_text('clobbered\\n')\n"
        "        (ROOT / 'src' / 'proof.cpp').unlink()\n"
        "        self.assertTrue(True)\n")
    for command in (["git", "init", "-q", "."],
                    ["git", "config", "user.email", "damage@example.invalid"],
                    ["git", "config", "user.name", "damage"],
                    ["git", "add", "-A"],
                    ["git", "commit", "-qm", "base"]):
        subprocess.run(command, cwd=repo, capture_output=True)
    return [PYTHON, str(TOOLS / "verify_tests_do_not_write.py"),
            "--repo", str(repo)]


def damage_cast_classification(workspace):
    """A cast reclassified, so the committed measurement stops describing src/.

    Dropping a ROW rather than editing a field, because a dropped row is the
    failure mode this classifier is built against: it is the shape a
    classifier takes when it quietly emits only what it understood, and a
    clean sheet produced by discarding evidence looks exactly like a clean
    sheet produced by there being nothing wrong.
    """
    report = REPO_ROOT / "docs" / "recovery" / "cast-classification.csv"
    if not report.is_file():
        raise Skip("docs/recovery/cast-classification.csv is absent")
    lines = report.read_text(encoding="utf-8").splitlines(keepends=True)
    if len(lines) < 3:
        raise Skip("no classified cast to drop")
    copy = workspace / "cast-classification.csv"
    copy.write_text("".join(lines[:1] + lines[2:]), encoding="utf-8")
    return [PYTHON, str(TOOLS / "classify_casts.py"), "--check",
            "--report", str(copy)]


def damage_pinned_class_size(workspace):
    """Every pinned `sizeof` moved, so the image-derived sizes disagree.

    Perturbing ALL of them rather than one chosen by name: the derivation
    reproduces only 9 of the 40 and ABSTAINS on the rest, and abstention is a
    correct outcome that this check must not treat as a refusal. Moving them
    all guarantees the perturbation lands on a class the tool actually answers
    for, whichever those turn out to be.
    """
    exe = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
    if not exe.is_file():
        raise Skip("the pinned executable is absent")
    source = copy_tree(REPO_ROOT / "src", workspace / "src", ("*.h",))
    pattern = re.compile(
        r"(static_assert\(\s*sizeof\(\s*\w+\s*\)\s*==\s*)(0[xX][0-9a-fA-F]+|\d+)")
    moved = 0

    def bump(match):
        nonlocal moved
        moved += 1
        return f"{match.group(1)}0x{int(match.group(2), 0) + 7:X}"

    for header in sorted(source.glob("*.h")):
        text = header.read_text(encoding="utf-8", errors="replace")
        damaged = pattern.sub(bump, text)
        if damaged != text:
            header.write_text(damaged, encoding="utf-8")
    if not moved:
        raise Skip("no pinned sizeof to perturb")
    return [PYTHON, str(TOOLS / "derive_class_layout.py"), "--check-pinned",
            "--src", str(source), "--exe", str(exe)]


def damage_pipeline_golden(workspace):
    """A pinned pipeline fact that no longer matches what the tools derive.

    Damaging the golden rather than a tool is deliberate: it exercises the
    comparison without needing to break and restore a module the rest of this
    run imports.
    """
    golden = REPO_ROOT / "docs" / "recovery" / "pipeline-golden.json"
    exe = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
    if not golden.is_file() or not exe.is_file():
        raise Skip("the golden or the pinned executable is absent")
    text = golden.read_text(encoding="utf-8")
    match = re.search(r'"span_bytes": (\d+)', text)
    if not match:
        raise Skip("no span_bytes entry to perturb")
    copy = workspace / "pipeline-golden.json"
    copy.write_text(
        text[:match.start()] + f'"span_bytes": {int(match.group(1)) + 1}'
        + text[match.end():], encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_recovery_pipeline.py"),
            "--golden", str(copy)]


def damage_observability_census(workspace):
    """A recovery of >=200 B that the census does not record.

    This is the shape a new recovery hits: land the function, forget to measure
    whether anything observes it. Damaging the CATALOGUE rather than the census
    is deliberate - it is the direction a real recovery arrives from.
    """
    census = REPO_ROOT / "docs" / "recovery" / "observability.json"
    functions = REPO_ROOT / "docs" / "recovery" / "functions.csv"
    if not census.is_file() or not functions.is_file():
        raise Skip("the census or the catalogue is absent")
    import csv as _csv
    rows = list(_csv.DictReader(functions.open(newline="", encoding="utf-8-sig")))
    if not rows:
        raise Skip("the catalogue is empty")
    planted = dict(rows[0])
    planted.update({"address": "0x00ABCDEF", "name": "?planted@@YAXXZ",
                    "size": "999", "recovery_state": "source_complete",
                    "binary_kind": "game"})
    copy = workspace / "functions.csv"
    with copy.open("w", newline="", encoding="utf-8") as handle:
        writer = _csv.DictWriter(handle, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows + [planted])
    return [PYTHON, str(TOOLS / "verify_observability_ratchet.py"),
            "--census", str(census), "--functions", str(copy)]


def damage_exception_object(workspace):
    """An object carrying exception unwind data in the swept directory.

    recovery-abi scored 77% survivors under mutation with no damage case at all,
    which made its green the weakest on the board. Its actual job - refusing a
    translation unit that lost -fno-exceptions - is testable in seconds by
    hard-linking the real object directory and dropping one -fexceptions object
    into the copy.
    """
    import subprocess as _sp
    build = REPO_ROOT / "build" / "mingw-i686-debug"
    objects = build / "CMakeFiles" / "OpenSMACX.dir" / "src"
    alphanet = objects / "alphanet.cpp.obj"
    if not objects.is_dir() or not alphanet.is_file():
        raise Skip("no configured debug build directory")
    compiler = shutil.which("i686-w64-mingw32-g++")
    if not compiler:
        raise Skip("no i686 mingw compiler on PATH")
    copy = workspace / "objects"
    _sp.run(["cp", "-al", str(objects), str(copy)], check=True)
    source = workspace / "poison.cpp"
    source.write_text("struct E{};\nint poison(int x){ if(x) throw E(); return x; }\n",
                      encoding="utf-8")
    done = _sp.run([compiler, "-m32", "-c", "-fexceptions", str(source),
                    "-o", str(copy / "poison.cpp.obj")], capture_output=True)
    if done.returncode != 0:
        raise Skip("could not build an exception-carrying object")
    return [PYTHON, str(TOOLS / "verify_recovery_abi.py"),
            "--nm", "/usr/bin/i686-w64-mingw32-nm",
            "--objdump", "/usr/bin/i686-w64-mingw32-objdump",
            "--object", str(alphanet), "--object-dir", str(copy)]


def damage_absent_signedness_image(workspace):
    """No executable, so nothing can be ranked. It used to print `0 <= 44`."""
    return [PYTHON, str(TOOLS / "audit_export_signedness.py"), "--check",
            "--exe", str(workspace / "absent.exe")]


def damage_emptied_def(workspace):
    """An empty .def compares zero exports and used to report within baseline."""
    empty = workspace / "empty.def"
    empty.write_text("", encoding="utf-8")
    return [PYTHON, str(TOOLS / "audit_export_signedness.py"), "--check",
            "--def-file", str(empty),
            "--exe", str(REPO_ROOT / ".opensmacx" / "game"
                         / "terranx_original.exe")]


# Each entry: the gate check it defends, and a damage that must make it exit
# non-zero. `check` must match the name used in COVERED_CHECKS below.
# (check, description, damage, TEXT THE REFUSAL MUST CONTAIN).
#
# The fourth field is what stops a process failure counting as a refusal. It is
# not decoration: every case here passes at least one flag that no add_test in
# CMakeLists.txt passes, so an argparse usage error - exit 2, no work done -
# used to score identically to the check doing its job.
CASES = (
    ("load-time-addresses", "unpadded address at namespace scope",
     damage_load_time_address, "dereferenced at load time"),
    ("tool-test-registration", "a test file CMake never executes",
     damage_unregistered_tool_test, "never executed by CMake"),
    ("test-registration", "a gameplay case defined and not registered",
     damage_dropped_gameplay_case, "never registered"),
    ("documented-counts", "a per-state count restated by hand",
     damage_restated_count, "restates"),
    ("wine-test-lock-check", "a Wine test without the prefix lock",
     damage_wine_lock, "without RESOURCE_LOCK"),
    ("wine-test-lock-check", "the runner renamed, blinding the check",
     damage_blinded_wine_check, "expected at least"),
    ("exclusions-current", "a measured number disagreeing with the image",
     damage_stale_exclusions, "disagrees with the image"),
    ("recovery-abi", "an object carrying exception unwind data",
     damage_exception_object, "carries exception unwind data"),
    ("recovery-pipeline", "a pinned pipeline fact that no longer holds",
     damage_pipeline_golden, "answers changed"),
    ("export-signedness-audit", "no image, so nothing can be ranked",
     damage_absent_signedness_image, "verified NOTHING"),
    ("export-signedness-audit", "an empty .def comparing zero exports",
     damage_emptied_def, "below the floor"),
    ("verified-layouts-current", "a proved class layout quietly dropped",
     damage_verified_layouts, "is stale"),
    ("verified-layouts-current", "a header defect read as a wrong layout",
     damage_layout_probe_header, "header defect"),
    ("cast-classification-current", "a classified cast quietly dropped",
     damage_cast_classification, "is stale"),
    ("class-layout-derivation-pinned", "pinned sizes the image contradicts",
     damage_pinned_class_size, "answered wrongly"),
    ("byte-match-ratchet", "a claimed byte-exact body quietly changed",
     damage_byte_match_ratchet, "no longer reproduces"),
    ("tests-do-not-write", "a test that edits and deletes tracked files",
     damage_writing_test, "write into the source tree"),
)

# The gate checks this tool is responsible for. Adding a check here without
# adding a case fails the run, which is the point: a check ships with a proof
# that it can fail, or it does not ship.
COVERED_CHECKS = {
    "load-time-addresses",
    "tool-test-registration",
    "test-registration",
    "documented-counts",
    "wine-test-lock-check",
    "exclusions-current",
    "export-signedness-audit",
    "recovery-pipeline",
    "recovery-abi",
    "verified-layouts-current",
    "class-layout-derivation-pinned",
    "cast-classification-current",
    "byte-match-ratchet",
    "tests-do-not-write",
}


# A refusal is a check saying no. These are the process saying it never ran.
NOT_A_REFUSAL = ("usage:", "Traceback (most recent call last)")


def run_case(check, description, build_damage, workspace):
    command = build_damage(workspace)
    done = subprocess.run(command, capture_output=True, text=True, timeout=600)
    output = (done.stderr or done.stdout).strip()
    return done.returncode, output.splitlines(), output


def refusal_is_real(status, output, expected):
    """Did the check REFUSE, or did it merely fail to run?

    A NON-ZERO EXIT IS NOT EVIDENCE, and reading it as evidence is the exact
    defect this file exists to catch, committed inside this file. argparse exits
    2 on an unrecognised flag, and 7 of the 8 checks here are invoked with at
    least one flag no add_test in CMakeLists.txt passes - `--src`, `--cmake`,
    `--tools`, `--suite`, `--doc`, `--document`, `--golden`, `--def-file`.
    Renaming any of them would leave every ctest check green while its damage
    case quietly became `ok (exit 2)` on a usage error. Demonstrated:

        verify_no_load_time_addresses.py --renamed-flag /tmp
        -> exit 2, first line `usage: ...`, scored `ok (counts as REFUSED)`

    So a case now declares text its refusal must contain. A usage banner or a
    traceback does not contain it, and neither counts however the process died.
    """
    if status == 0:
        return False, "exited 0 on damaged input"
    for marker in NOT_A_REFUSAL:
        if output.startswith(marker) or f"\n{marker}" in output:
            return False, (f"did not refuse - it failed to run ({marker!r}), "
                           f"so this case proves nothing")
    if expected and expected not in output:
        return False, (f"exited {status} but its output does not contain "
                       f"{expected!r}, so it may not be the intended refusal")
    return True, ""


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--verbose", action="store_true",
                        help="print the refusal text each check produced")
    arguments = parser.parse_args()

    passed, skipped, failures = [], [], []
    for index, (check, description, build_damage, expected) in enumerate(CASES):
        with tempfile.TemporaryDirectory(prefix="cancanfail-") as directory:
            workspace = Path(directory)
            try:
                status, message, output = run_case(check, description,
                                                   build_damage, workspace)
            except Skip as reason:
                skipped.append((check, description, str(reason)))
                print(f"SKIP  {check}: {description} - {reason}")
                continue
            except Exception as error:  # a broken case is not a passing case
                failures.append((check, description,
                                 f"the damage case itself raised {error!r}"))
                print(f"ERROR {check}: {description} - {error!r}")
                continue
        real, why = refusal_is_real(status, output, expected)
        if not real:
            failures.append((check, description, why))
            print(f"FAIL  {check}: {description} - {why}")
        else:
            passed.append((check, description))
            print(f"ok    {check}: {description} (exit {status})")
            if arguments.verbose and message:
                print(f"        {message[0][:120]}")

    proved = {check for check, _ in passed}
    uncovered = sorted(COVERED_CHECKS - proved)
    if uncovered:
        print("\nchecks-can-fail: these checks have no damage case that "
              "actually ran, so nothing here shows they can fail:",
              file=sys.stderr)
        for name in uncovered:
            reason = next((why for check, _, why in skipped if check == name),
                          "no case is declared for it")
            print(f"    {name}  - {reason}", file=sys.stderr)
        print("  Add a case to CASES, or explain in COVERED_CHECKS why the "
              "check cannot be damaged mechanically.", file=sys.stderr)

    if failures or uncovered:
        print(f"\nchecks-can-fail: {len(failures)} check(s) did not refuse "
              f"damaged input, {len(uncovered)} unproven", file=sys.stderr)
        for check, description, why in failures:
            print(f"    {check}: {description} - {why}", file=sys.stderr)
        return 1

    print(f"\nchecks-can-fail: {len(passed)} damage case(s) refused across "
          f"{len(proved)} check(s), {len(skipped)} skipped")
    return 0


if __name__ == "__main__":
    sys.exit(main())

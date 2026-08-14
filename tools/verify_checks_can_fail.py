#!/usr/bin/env python3
"""Damage the real inputs and require each gate check to go red.

WHY THIS EXISTS. On 2026-08-01 an audit of the 13 mechanical checks added the
day before found three that could not fail:

  * `verify_no_load_time_addresses` matched one of the four ways this tree
    spells the construct it forbids, and never opened a header;
  * `audit_export_signedness` printed `0 <= 44` whenever the executable was
    absent, which is every checkout without the game (that tool was retired
    2026-08-13 with `src/OpenSMACX.def`, the second record it compared);
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

AND NEITHER IS AN EXEMPTION, for long. `EXEMPT` is the escape hatch, and an
escape hatch with no counter on it is how a requirement empties itself: it held
FOURTEEN of the twenty-nine registered checks on the morning of 2026-08-13,
which is to say the rule above was suspended for half the gate and the
suspension was invisible because it was a set of bare names. It now carries a
reason per entry, a ratchet that may only shrink, and a refusal if an entry
names a check CMake no longer registers - so an exemption cannot outlive its
subject. Thirteen of the fourteen turned out to be damageable in an afternoon.
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
sys.path.insert(0, str(TOOLS))

import byte_match  # noqa: E402  - needs TOOLS on the path first
PYTHON = sys.executable
EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"


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


def shadow_repo(workspace, trees=("tools", "src")):
    """A whole repository, hard-linked, so a tool can be pointed at a fake tree.

    THE LAST RESORT, and only for a check whose inputs are module constants.
    Four of the artifact-currency checks have no flag to redirect what they
    read: `emit_vtable_shim_header.py` compares against `src/vtable_shim.h`
    with nothing but `--check`, and `derive_class_size_bounds.py` reads the
    pinned sizes through `derive_class_layout`'s own module-level `SRC_DIR`.
    Damaging either in place would mean editing the tree the gate is checking,
    which is the one thing a gate check may never do.

    Every tool computes `REPO_ROOT` from `Path(__file__).resolve().parent
    .parent`, so a hard-linked copy of `tools/` beside a copy of `src/` gives
    a tool a different repository to answer about while running the SAME bytes
    the gate runs. Hard links rather than symlinks on purpose: `resolve()`
    follows a symlink straight back to the real tree, and the case would then
    quietly measure the repository it was supposed to be shielding.
    """
    root = workspace / "shadow"
    root.mkdir(parents=True, exist_ok=True)
    for name in trees:
        source = REPO_ROOT / name
        if not source.is_dir():
            raise Skip(f"{name}/ is absent, so no shadow tree can be built")
        subprocess.run(["cp", "-al", str(source), str(root / name)], check=True)
    return root


def configured_build(marker="CTestTestfile.cmake") -> Path | None:
    """Any configured build directory, found rather than named.

    This was hardcoded to `build/mingw-i686-debug` at three sites - a preset
    a673bf79 DELETED when it made VC6 the only compiler. There has been no
    `CMakePresets.json` since, so the path could never resolve and three damage
    cases reported "no configured debug build directory" forever. A skip that
    can never stop skipping is not a skip, it is a silently disabled case, and
    it read as environmental for months.

    Sorted so the choice is deterministic; any configured tree can carry these
    cases, because what they need is a generated CTestTestfile.cmake and not a
    particular toolchain.

    `marker` is a parameter for the same reason the hardcoded path was removed:
    the build-freshness cases need a tree with a `build.ninja`, which is not the
    same question as a tree with a generated CTestTestfile, and naming one
    directory for both is how the first version came to be permanently blind.
    """
    root = REPO_ROOT / "build"
    if not root.is_dir():
        return None
    for candidate in sorted(root.iterdir()):
        if (candidate / marker).is_file():
            return candidate
    return None




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


def damage_stranded_test_class(workspace):
    """A test file with a test class defined below its `unittest.main()`.

    The exact shape found on 2026-08-13 in test_emit_translation_unit.py, where
    four classes sat below the guard and the registered gate ran 57 of 73 tests
    while exiting 0. Twenty-five files are written because the check refuses a
    population under twenty rather than reporting a clean sweep over almost
    nothing - so a case with one file would prove the floor, not the rule.
    """
    root = workspace / "stranded-tests"
    root.mkdir(parents=True, exist_ok=True)
    for index in range(25):
        body = ("import unittest\n"
                f"class Fine{index}(unittest.TestCase):\n"
                "    def test_ok(self): pass\n"
                'if __name__ == "__main__":\n    unittest.main()\n')
        if index == 0:
            # The offender, and only one: a check that flagged all 25 would
            # pass this case while being useless.
            body += ("class Stranded(unittest.TestCase):\n"
                     "    def test_never_runs(self): pass\n")
        (root / f"test_{index}.py").write_text(body)
    return [PYTHON, str(TOOLS / "verify_tests_all_run.py"), "--tools", str(root)]


def damage_retired_cmake_path(workspace):
    """A CMakeLists naming a tool that was retired out from under it.

    The shape found in the tree on 2026-08-12, reproduced exactly: a live
    COMMAND pointing at `run_game (retired)`, the placeholder spelling left by
    rewriting a tool's name in place rather than removing the block. Twenty-one
    of those plus two DEPENDS on a deleted CSV survived a full configure and a
    173-step build, because a custom target outside `all` never resolves its own
    command line.

    A second, GOOD reference is written beside it so the case cannot pass by a
    check that simply refuses every CMakeLists it is handed - the vacuous shape
    this file exists to catch.
    """
    root = workspace / "retired-cmake"
    (root / "tools").mkdir(parents=True, exist_ok=True)
    (root / "tools" / "real.py").write_text("")
    (root / "CMakeLists.txt").write_text(
        'add_custom_target(alive\n'
        '    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/real.py"\n'
        ')\n'
        'add_custom_target(dead\n'
        '    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/run_game (retired)"\n'
        ')\n')
    return [PYTHON, str(TOOLS / "verify_cmake_paths_exist.py"),
            "--cmakelists", str(root / "CMakeLists.txt"), "--check"]


def damage_refuted_wall(workspace):
    """A RULED-OUT list left standing on a body that has since been PROVED.

    The refutation case for the lesson grammar, and the reason the tokens are
    allowed at all under "state is measured, not claimed": a RULED-OUT sits on a
    body that has not matched, so if one ever does, the list is refuted and must
    be promoted to a LEVER or deleted. A wall that can quietly outlive its
    reason is the `static_assert` trap in another costume - one marker carrying
    both "I stopped here" and "nobody should try".

    Offline: no VC6, no image. The check runs before every early return for
    exactly this reason.
    """
    sys.path.insert(0, str(TOOLS))
    import annotation_scan

    for source in sorted((REPO_ROOT / "src").glob("*.cpp")):
        text = source.read_text(errors="ignore")
        if "BYTE_EXACT" not in text:
            continue
        for annotation in annotation_scan.scan_file(source):
            if not annotation.matched or not annotation.line:
                continue
            lines = text.splitlines()
            marker = lines[annotation.line - 1]
            damaged = list(lines)
            damaged.insert(annotation.line,
                           "// RULED-OUT: ternary; do/while; counting down")
            tree = workspace / "src"
            tree.mkdir(parents=True, exist_ok=True)
            (tree / source.name).write_text("\n".join(damaged) + "\n",
                                            encoding="utf-8")
            assert "ORIGINAL" in marker
            return [PYTHON, str(TOOLS / "decomp_status.py"), "--check",
                    "--no-ledger", "--state-only", "--src", str(tree)]
    raise Skip("no BYTE_EXACT annotation in src/*.cpp to contradict")


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


def damage_exception_object(workspace):
    """An object carrying exception unwind data in the swept directory.

    recovery-abi scored 77% survivors under mutation with no damage case at all,
    which made its green the weakest on the board. Its actual job - refusing a
    translation unit that lost -fno-exceptions - is testable in seconds by
    hard-linking the real object directory and dropping one -fexceptions object
    into the copy.
    """
    import subprocess as _sp
    build = configured_build()
    objects = build / "CMakeFiles" / "OpenSMACX.dir" / "src"
    alphanet = objects / "alphanet.cpp.obj"
    if not objects.is_dir() or not alphanet.is_file():
        raise Skip("no configured debug build directory")
    # VC6 with /EHsc, not a cross g++. The gate reads objects THIS project
    # produces, so the poison has to come from the compiler that produces them -
    # a second toolchain's unwind tables are a different shape, and the case was
    # asserting on the wrong one.
    if not byte_match.VC6_CL.is_file():
        raise Skip("no VC6 to build an exception-carrying object with")
    copy = workspace / "objects"
    _sp.run(["cp", "-al", str(objects), str(copy)], check=True)
    try:
        obj = byte_match.compile_unit(
            "struct E{};\nint poison(int x){ if(x) throw E(); return x; }\n",
            workspace, "poison", "/c /O2 /EHsc")
    except Exception as error:                      # noqa: BLE001 - reported
        raise Skip(f"could not build an exception-carrying object: {error}")
    if not obj:
        raise Skip("could not build an exception-carrying object")
    (copy / "poison.cpp.obj").write_bytes(obj)
    # Plain binutils: objdump reads a cl 12.00.8168 object as pe-i386.
    return [PYTHON, str(TOOLS / "verify_recovery_abi.py"),
            "--nm", "/usr/bin/nm", "--objdump", "/usr/bin/objdump",
            "--object", str(alphanet), "--object-dir", str(copy)]


def _stub_ninja(workspace, script):
    stub = workspace / "ninja"
    stub.write_text("#!/bin/sh\n" + script, encoding="utf-8")
    stub.chmod(0o755)
    return str(stub)


def damage_blind_freshness_query(workspace):
    """A ninja that answers without parsing the target list.

    This is build-freshness's own check 1, and it is what makes the other two
    mean anything. It is not hypothetical: the real `ninja -n` IS blind here by
    default, because file(GLOB CONFIGURE_DEPENDS) leaves the manifest
    permanently dirty and ninja exits 0 after "Re-running CMake..." without ever
    reading the target list. A stub that always exits 0 is exactly that state.

    WHY THE CONTROL FOR THIS PAIR IS NOT AN EXIT 0. A stub that answers check 1
    correctly and says "no work to do" gets past checks 1 and 2 and then hits
    check 3 - `build/` here is genuinely older than CMakeLists.txt, so the
    honest run refuses with "CMake needs re-running". That is the control: it
    demonstrates the stub route reaches the tool's later checks, while these two
    cases stop at checkpoint 1 and checkpoint 2 respectively. The expected text
    is what pins each to its own checkpoint, and none of the three sentences is
    the same as either of the others.
    """
    build = configured_build("build.ninja")
    if build is None:
        raise Skip("no build directory with a build.ninja anywhere under build/")
    return [PYTHON, str(TOOLS / "verify_build_freshness.py"),
            "--build-dir", str(build),
            "--ninja", _stub_ninja(workspace, "exit 0\n"),
            "--also", "OpenSMACX.dll"]


def damage_stale_artifacts(workspace):
    """Pending work reported with exit 0, which is what a dry run really does.

    THE FINDING THIS CASE CARRIES: exit status is NOT the signal. A `ninja -n`
    with pending work also exits 0, having merely PRINTED the commands it would
    run - so the only honest reading is the absence of "no work to do". This
    stub fails the impossible target, satisfying check 1 so that check 1 cannot
    be what refuses here, and then prints a link line without ever saying it.
    """
    build = configured_build("build.ninja")
    if build is None:
        raise Skip("no build directory with a build.ninja anywhere under build/")
    script = (
        'for argument in "$@"; do\n'
        '  case "$argument" in\n'
        '    opensmacx--no-such-target*)\n'
        '      echo "ninja: unknown target" >&2; exit 1 ;;\n'
        '  esac\n'
        'done\n'
        'echo "[1/1] Linking CXX shared library OpenSMACX.dll"\n'
        'exit 0\n')
    return [PYTHON, str(TOOLS / "verify_build_freshness.py"),
            "--build-dir", str(build),
            "--ninja", _stub_ninja(workspace, script),
            "--also", "OpenSMACX.dll"]


def stale_derived_artifact(script, flag, artifact, needs_exe=False):
    """Factory: the committed derived output, perturbed, must be called stale.

    Six gate checks are `<generator>.py --check`; each regenerates its artifact
    in memory and compares it against the committed copy. All six sat outside
    the coverage requirement until that requirement started being derived, so
    nothing had ever shown the comparison can say no. The damage is identical
    for all six, so it is written once.

    THE UNDAMAGED RUN FIRST IS THE POINT, not ceremony. If `artifact` named the
    wrong file, the tool would call the copy stale for a reason that has nothing
    to do with the damage, and the case would score `ok` while proving only that
    the path is wrong. So the unmodified copy must PASS before the perturbed one
    is allowed to fail - a positive control for the positive control.
    """
    def build_damage(workspace):
        source = REPO_ROOT / artifact
        if not source.is_file():
            raise Skip(f"{artifact} is absent")
        if needs_exe and not EXE.is_file():
            raise Skip("the pinned executable is absent")
        text = source.read_text(encoding="utf-8")
        lines = text.splitlines(keepends=True)
        if len(lines) < 2:
            raise Skip(f"{artifact} has too few lines to perturb")

        def command_for(path):
            extra = ["--exe", str(EXE)] if needs_exe else []
            return [PYTHON, str(TOOLS / script), "--check", flag, str(path),
                    *extra]

        # THE CONTROL KEEPS THE BASENAME and changes only the directory. A
        # generator may derive part of its answer from `output_path.name` -
        # generate_signature_oracles matched it against the evidence column of
        # proven.csv - so a copy called `control-<name>` regenerates into
        # something else and is called stale for a reason that has nothing to do
        # with the damage. The first run of this factory did exactly that, and
        # the control is why it was an honest SKIP and not a false `ok`.
        control = workspace / "control" / source.name
        control.parent.mkdir(parents=True, exist_ok=True)
        control.write_text(text, encoding="utf-8")
        done = subprocess.run(command_for(control), capture_output=True,
                              text=True, timeout=900)
        if done.returncode != 0:
            raise Skip(
                f"{script} already calls an UNMODIFIED copy of {artifact} "
                f"stale, so a refusal here would not be caused by the damage: "
                f"{(done.stderr or done.stdout).strip().splitlines()[:1]}")

        damaged = workspace / source.name
        damaged.write_text("".join(lines[:-1]), encoding="utf-8")
        return command_for(damaged)

    return build_damage


def damage_stale_vtable_shim(workspace):
    """The committed VCall shim, one declaration short.

    `emit_vtable_shim_header.py` takes `--check` and nothing else: both the
    generated text and the file it compares against are module constants, so
    this is one of the four checks that can only be damaged through a shadow
    tree. The header the compiled tree includes is generated from the same
    function the verification scaffolding uses, and a drift between them lets a
    body verify against one slot layout and call another - so a shim that
    stopped being regenerated is the defect, and this proves the comparison can
    see it.

    The CONTROL runs first for the reason the factory above states: the shadow
    tree could be wrong in a way that makes an unmodified header stale.
    """
    header = REPO_ROOT / "src" / "vtable_shim.h"
    if not header.is_file():
        raise Skip("src/vtable_shim.h is absent")
    root = shadow_repo(workspace, ("tools",))
    (root / "src").mkdir(parents=True, exist_ok=True)
    shadow = root / "src" / "vtable_shim.h"
    text = header.read_text(encoding="utf-8")
    command = [PYTHON, str(root / "tools" / "emit_vtable_shim_header.py"),
               "--check"]

    shadow.write_text(text, encoding="utf-8")
    done = subprocess.run(command, capture_output=True, text=True, timeout=600)
    if done.returncode != 0:
        raise Skip("the shadow tree already calls an UNMODIFIED vtable_shim.h "
                   f"stale: {(done.stderr or done.stdout).strip()[:120]}")

    lines = text.splitlines(keepends=True)
    slot = next((index for index, line in reversed(list(enumerate(lines)))
                 if line.startswith("    virtual")), None)
    if slot is None:
        raise Skip("no virtual slot to remove from the shim")
    shadow.write_text("".join(lines[:slot] + lines[slot + 1:]),
                      encoding="utf-8")
    return command


def damage_underestimated_class_size(workspace):
    """A pinned size the derived upper bound sits BELOW.

    Not a staleness check despite its `*-current` name: `--check` here runs a
    SOUNDNESS control, and the direction it guards is the one that corrupts
    memory - a bound under the true size under-allocates a receiver. So the
    damage moves the pinned sizes UP, out from under every bound the image
    supports, rather than perturbing an artifact.

    Through a shadow tree, because the pinned sizes are read through
    `derive_class_layout`'s module-level SRC_DIR and no flag redirects it. The
    image is the real one: it is `--exe`, and the point is that the derivation
    against the true image now disagrees with what the tree claims.
    """
    if not EXE.is_file():
        raise Skip("the pinned executable is absent")
    root = shadow_repo(workspace)
    pattern = re.compile(
        r"(static_assert\(\s*sizeof\(\s*\w+\s*\)\s*==\s*)(0[xX][0-9a-fA-F]+|\d+)")
    moved = 0

    def inflate(match):
        nonlocal moved
        moved += 1
        # Past any plausible bound, so the refusal cannot depend on which
        # classes the derivation happens to answer for today.
        return f"{match.group(1)}0x{int(match.group(2), 0) + 0x40000:X}"

    for header in sorted((root / "src").glob("*.h")):
        text = header.read_text(encoding="utf-8", errors="replace")
        damaged = pattern.sub(inflate, text)
        if damaged != text:
            # The hard link is shared with the real tree; replace it rather
            # than writing through it.
            header.unlink()
            header.write_text(damaged, encoding="utf-8")
    if not moved:
        raise Skip("no pinned sizeof to inflate")
    return [PYTHON, str(root / "tools" / "derive_class_size_bounds.py"),
            "--check", "--exe", str(EXE)]


def damage_contradicted_yitzi_marker(workspace):
    """A `// yitzi` marker that no longer agrees with anything.

    The check correlates an outside human's notes with the marker and with the
    size the compiler pins, three ways, and it is the only gate over a claim
    whose source is not in this repository. Moving the marker's number alone
    contradicts BOTH of the others, so the case refuses on a checkout that has
    the fetched notes and on one that does not - the two produce different
    sentences, and `marked 0xBAD` is the part they share.
    """
    source = REPO_ROOT / "src"
    copy = copy_tree(source, workspace / "src", ("*.h",))
    pattern = re.compile(r"(//\s*yitzi\s+)(0x[0-9A-Fa-f]+|\d+)")
    for header in sorted(copy.glob("*.h")):
        text = header.read_text(encoding="utf-8", errors="replace")
        damaged, count = pattern.subn(r"\g<1>0xBAD", text, count=1)
        if count:
            header.write_text(damaged, encoding="utf-8")
            return [PYTHON, str(TOOLS / "correlate_yitzi_notes.py"), "--check",
                    "--src", str(copy)]
    raise Skip("no `// yitzi` marker in src/*.h to contradict")


def damage_overrun_member_access(workspace):
    """An observed access past the end of every class that declares a size.

    An overrun is the check's own worst finding: the image reaching past a
    declared `sizeof` means the tree's layout is wrong in the direction that
    truncates a member. One row is added per class already in the measurement
    rather than one row overall, because which classes the VC6 probe resolves a
    size for is not fixed, and a case that depends on that would rot into a
    permanent pass the first time the probe set moved.

    `--accesses` points at the copy, which is the tool's own offline route; the
    gate derives the same rows from the image instead. What is under test is the
    comparison against the declared boundary, and that half is identical.
    """
    published = REPO_ROOT / "docs" / "recovery" / "member-accesses.csv"
    if not published.is_file():
        raise Skip("docs/recovery/member-accesses.csv is absent")
    lines = published.read_text(encoding="utf-8").splitlines()
    if len(lines) < 2:
        raise Skip("no observed access to build on")
    classes = sorted({line.split(",", 1)[0] for line in lines[1:] if line})
    copy = workspace / "member-accesses.csv"
    copy.write_text("\n".join(lines + [
        f"{name},0x7FFF0,4,0,damage probe: past the end of any declared class"
        for name in classes]) + "\n", encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_member_offsets.py"), "--check",
            "--accesses", str(copy)]


def damage_unreachable_tool(workspace):
    """A tool no entry point names and nothing reachable imports.

    A retirement that leaves a tool behind leaves something nobody runs and
    nobody deletes, and the tree carried twenty-three such references at once.
    Two tools are written, not one: the reachable one is what stops this passing
    against a check that simply refuses every tree it is handed.
    """
    root = workspace / "reachability-root"
    (root / "tools").mkdir(parents=True, exist_ok=True)
    (root / "tools" / "reachable.py").write_text("", encoding="utf-8")
    (root / "tools" / "orphan.py").write_text("", encoding="utf-8")
    (root / "CMakeLists.txt").write_text(
        'add_test(NAME demo COMMAND "${DIR}/tools/reachable.py")\n',
        encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_tool_reachability.py"),
            "--root", str(root)]


# Each entry: the gate check it defends, and a damage that must make it exit
# non-zero. `check` must be a name CMakeLists.txt registers - a case naming
# anything else is reported and fails the run, because it defends nothing.
# (check, description, damage, TEXT THE REFUSAL MUST CONTAIN).
#
# The fourth field is what stops a process failure counting as a refusal. It is
# not decoration: every case here passes at least one flag that no add_test in
# CMakeLists.txt passes, so an argparse usage error - exit 2, no work done -
# used to score identically to the check doing its job.
def damage_truncated_span(workspace):
    """A catalogued span two bytes short of its closing `ret`.

    THE REAL DEFECT, reproduced: nine rows were in this state on 2026-08-14
    and 0x005D91D0 went BYTE_EXACT the moment its span was corrected. The
    damaged tree holds ONE annotation, and the tool reads it through
    `project_catalogue.from_source` - the same reader the unflagged
    invocation uses on `src/`, so this proves the gate's own check can fail
    and not a neighbouring one.
    """
    source = REPO_ROOT / "src" / "unrecovered" / "005d91d0.cpp"
    if not source.is_file():
        raise Skip("0x005D91D0 is not annotated in this tree")
    text = source.read_text(encoding="utf-8")
    intact, cut = "0x005D91D0-0x005D92BE", "0x005D91D0-0x005D92BC"
    if intact not in text:
        raise Skip("the span is not the one this case damages")
    tree = workspace / "src"
    tree.mkdir(parents=True, exist_ok=True)
    (tree / "005d91d0.cpp").write_text(text.replace(intact, cut),
                                       encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_span_termination.py"),
            "--src", str(tree)]


def damage_contradicted_return_type(workspace):
    """A name spelling `void` for a body whose prototype returns int.

    THE REAL DEFECT, reproduced: `?init@GraphicWin@@QAEX...` spelled `X` for a
    body that returns a failure code, and 47 callers inherited `void` from it
    because the emitter follows the name. The damaged tree holds ONE
    annotation and the tool reads it through `project_catalogue.from_source`,
    the same reader its unflagged invocation uses on `src/`.

    The tool's population floor is deliberately NOT applied under `--src`, or
    this case would "fail" for having one row rather than for the return type,
    and would pass just as well with the check deleted.
    """
    source = REPO_ROOT / "src" / "unrecovered" / "00614fe0.cpp"
    if not source.is_file():
        raise Skip("0x00614FE0 is not annotated in this tree")
    text = source.read_text(encoding="utf-8")
    intact, voided = "@@QAEHHHHHPAUWin@@H@Z", "@@QAEXHHHHPAUWin@@H@Z"
    if intact not in text:
        raise Skip("the mangled name is not the one this case damages")
    tree = workspace / "src"
    tree.mkdir(parents=True, exist_ok=True)
    (tree / "00614fe0.cpp").write_text(text.replace(intact, voided),
                                       encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_return_agreement.py"),
            "--src", str(tree)]


def damage_removed_span(workspace):
    """A span half deleted, which is how a body lowers its own bar.

    THE REAL DEFECT, reproduced: 0x00589B60's `// spans` lost the
    `0x0066026A-0x0066028A` half in batch 10 because the body had dropped
    that SEH funclet. A span is a fact about the ORIGINAL; taking one out
    shrinks what the comparison demands. The truncation test cannot see it -
    what remains decodes cleanly and ends on a terminator - so the size the
    annotation states beside it is the witness.
    """
    source = REPO_ROOT / "src" / "unrecovered" / "00402dd0.cpp"
    if not source.is_file():
        raise Skip("0x00402DD0 is not annotated in this tree")
    text = source.read_text(encoding="utf-8")
    intact = "0x00402DD0-0x00402ED2;0x006505A0-0x006505B5"
    if intact not in text:
        raise Skip("0x00402DD0 no longer carries the second span")
    tree = workspace / "src"
    tree.mkdir(parents=True, exist_ok=True)
    (tree / "00402dd0.cpp").write_text(
        text.replace(intact, "0x00402DD0-0x00402ED2"), encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_span_termination.py"),
            "--src", str(tree)]


def damage_deleted_call_edge(workspace):
    """An agent's rebuilt header dropping a call target its own bytes make.

    THE REAL DEFECT, reproduced: `Video_unk2` at 0x00636740 went from seven
    call targets to `(none)` in batch 9 and stayed that way through batch 10,
    because src/ is the catalogue and nothing held the header to the image.

    The damaged tree holds ONE annotation and the tool reads it through
    `project_catalogue.from_source`, the same reader its unflagged invocation
    uses on `src/` - so what is proved able to fail is the gate's own
    question, not a neighbouring one.
    """
    source = REPO_ROOT / "src" / "unrecovered" / "00636740.cpp"
    if not source.is_file():
        raise Skip("0x00636740 is not annotated in this tree")
    text = source.read_text(encoding="utf-8")
    intact = "// calls     0x005D7470"
    if intact not in text:
        raise Skip("0x00636740 no longer records the edge this case deletes")
    tree = workspace / "src"
    tree.mkdir(parents=True, exist_ok=True)
    (tree / "00636740.cpp").write_text(
        text.replace(intact, "// calls     "), encoding="utf-8")
    return [PYTHON, str(TOOLS / "verify_call_edges.py"), "--src", str(tree)]


CASES = (
    ("span-termination", "a span two bytes short of its closing ret",
     damage_truncated_span, "truncated span"),
    ("span-termination", "a second span deleted because the body dropped it",
     damage_removed_span, "beside spans covering"),
    ("call-edges", "a header dropping a call target its own bytes make",
     damage_deleted_call_edge, "which its own bytes call directly"),
    ("return-agreement", "a name spelling void where the prototype returns int",
     damage_contradicted_return_type, "the emitter follows the name"),
    ("tool-test-registration", "a test file CMake never executes",
     damage_unregistered_tool_test, "never executed by CMake"),
    ("test-registration", "a gameplay case defined and not registered",
     damage_dropped_gameplay_case, "never registered"),
    ("documented-counts", "a per-state count restated by hand",
     damage_restated_count, "restates"),
    ("exclusions-current", "a measured number disagreeing with the image",
     damage_stale_exclusions, "disagrees with the image"),
    ("recovery-abi", "an object carrying exception unwind data",
     # VC6 does not emit `.eh_frame` - that is GCC's DWARF section, and the
     # detector that fires on a cl object is the undefined `__CxxFrameHandler`
     # instead. Same defect, the other of the tool's two detectors; the case
     # named the one only a second compiler could trigger.
     damage_exception_object, "imports exception handling support"),
    ("recovery-pipeline", "a pinned pipeline fact that no longer holds",
     damage_pipeline_golden, "answers changed"),
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
    ("byte-match-ratchet", "a wall left standing on a body that was proved",
     damage_refuted_wall, "LESSON GRAMMAR"),
    ("cmake-paths", "a CMake command naming a tool that was retired",
     damage_retired_cmake_path, "does not exist"),
    ("tests-all-run", "a test class stranded below unittest.main()",
     damage_stranded_test_class, "never runs"),
    ("build-freshness", "a staleness query not parsing the target list",
     damage_blind_freshness_query, "the staleness query is BLIND"),
    ("build-freshness", "pending work reported with exit 0",
     damage_stale_artifacts, "the build is OUT OF DATE"),
    ("access-lower-bounds-current", "a row dropped from the committed bounds",
     stale_derived_artifact("derive_access_bounds.py", "--out",
                            "docs/recovery/access-lower-bounds.csv",
                            needs_exe=True),
     "is stale; regenerate it"),
    ("base-edges-current", "a row dropped from the committed base edges",
     stale_derived_artifact("derive_base_edges.py", "--out",
                            "docs/recovery/base-edges.csv", needs_exe=True),
     "is stale; regenerate it"),
    ("agreed-class-sizes-current", "a row dropped from the agreed sizes",
     stale_derived_artifact("derive_agreed_sizes.py", "--out",
                            "docs/recovery/agreed-class-sizes.csv"),
     "is stale; regenerate it"),
    ("idb-members-current", "a row dropped from the exported IDB members",
     stale_derived_artifact("export_idb_members.py", "--out",
                            "docs/recovery/idb-members.csv"),
     "is stale; regenerate it"),
    ("hypothesis-layouts-current", "a line dropped from the emitted header",
     stale_derived_artifact("emit_hypothesis_layouts.py", "--out",
                            "src/hypothesis_layouts.h"),
     "is stale; regenerate it"),
    ("derived-prototypes-current", "a prototype dropped from the derived table",
     stale_derived_artifact("derive_prototypes_from_names.py", "--out",
                            "docs/recovery/derived-prototypes.csv",
                            needs_exe=True),
     "is stale; regenerate it"),
    ("vtable-shim-header-current", "the committed shim one declaration short",
     damage_stale_vtable_shim, "is stale; regenerate it"),
    ("class-size-bounds-current", "a pinned size the derived bound sits below",
     damage_underestimated_class_size, "a bound below the true size"),
    ("yitzi-sizes-current", "a marker that agrees with neither source",
     damage_contradicted_yitzi_marker, "marked 0xBAD"),
    ("member-offsets-current", "an access past the end of a declared class",
     damage_overrun_member_access, "declared boundaries the image contradicts"),
    ("tool-reachability", "a tool no entry point names",
     damage_unreachable_tool, "no entry point names it"),
)

CMAKELISTS = REPO_ROOT / "CMakeLists.txt"

# A floor, so a parser that quietly matches nothing cannot report a clean sweep
# over an empty population - the exact shape this file exists to hunt. It sits
# just under the real count on purpose: a floor far below it is satisfied by a
# scan that has lost most of the gate, and a floor of ZERO satisfies the
# assertion that a floor exists at all. Measured 2026-08-13: 27 registered,
# after `export-signedness-audit` and `def-append-only` were retired with
# `src/OpenSMACX.def`. One check of headroom, matching the convention
# verify_tool_test_registration's --floor uses, so the next honest retirement
# does not take down every caller of gate_checks() before it can be reviewed;
# test_the_floor_is_not_slack refuses more slack than two.
#
# 26 -> 28 on 2026-08-14, with `return-agreement` and `call-edges` registered.
GATE_CHECK_FLOOR = 28


def without_comments(text):
    """CMake comments removed, so prose cannot confuse the paren balancing.

    Not hypothetical caution: this CMakeLists.txt carries long English notes and
    several contain an unbalanced `(`. One inside an add_test block would make
    the scan run past the closing paren and attribute a later tool to the wrong
    test - or swallow the rest of the file and lose every check after it, which
    the floor would then catch as a parser failure.

    A regex (`(?<!\\)#.*`) got the first half of this right and the second half
    wrong: it also strips a `#` inside a quoted argument, which drops the
    COMMAND line of any block that passes one and makes a real gate check
    invisible. Stripping too much and stripping too little are the same defect
    from opposite sides, so the scanner tracks the string state instead.
    """
    lines, quoted = [], False
    for line in text.split("\n"):
        kept = []
        for index, char in enumerate(line):
            if char == '"' and (index == 0 or line[index - 1] != "\\"):
                quoted = not quoted
            elif char == "#" and not quoted:
                break
            kept.append(char)
        lines.append("".join(kept))
    return "\n".join(lines)


TOOL_IN_COMMAND = re.compile(r"tools/([A-Za-z0-9_]+\.py)")
ADD_TEST_HEAD = re.compile(r"add_test\(\s*NAME\s+([A-Za-z0-9_-]+)")


def parse_gate_checks(text) -> dict:
    r"""{check name: the tools/ script it runs}, for one CMakeLists' text.

    The block extent is found by COUNTING PARENS, not by a regex. A non-greedy
    `(.*?)\n\s*\)` looks correct and silently loses any block a preceding
    mis-parse has already swallowed - it missed `recovery-abi` and
    `export-signedness-audit` on the first attempt, and the miss was invisible
    because both were in the hand-written set it was replacing and so could
    never show up as absent. The test that pins this DERIVES the casualties by
    running the naive regex over the real CMakeLists rather than naming them,
    because naming them dates: `export-signedness-audit` was retired 2026-08-13
    and the block that fell over after it went was a different one.

    `test_*.py` IS NOT A GATE CHECK. Its predecessor asked only whether the
    block mentioned `tools/`, which counts a directly registered unit suite as a
    check needing a damage case - a demand for a positive control on a positive
    control, forever. Whether CMake executes those is
    verify_tool_test_registration's job; a block whose only tools/ script is a
    test file is not a gate check at all.

    Returns the script as well as the name because the caller has to be able to
    tell the two apart, and because a reader looking at an uncovered check wants
    to know what to go and read.
    """
    body = without_comments(text)
    found = {}
    for match in ADD_TEST_HEAD.finditer(body):
        index, depth = match.end(), 1
        while index < len(body) and depth:
            depth += (body[index] == "(") - (body[index] == ")")
            index += 1
        scripts = [name for name in TOOL_IN_COMMAND.findall(body[match.end():index])
                   if not name.startswith("test_")]
        if scripts:
            found[match.group(1)] = scripts[0]
    return found


def gate_checks(cmakelists=None) -> dict:
    r"""Every `add_test` block that runs a `tools/` script, DERIVED from CMake.

    The requirement below used to be a hand-written set, which is
    a hand-written list of the things you must prove - the same defect as a
    hand-written list of the things you must check, one level up, and hiding in
    the one file whose whole subject is a check that reports success while
    verifying nothing. Measured 2026-08-13: CMake registered 28 such checks and
    the set named 14.

    The parsing is `parse_gate_checks`; what this adds is THE FLOOR, and the
    floor lives here rather than in main() so that it protects every caller.
    `verify_check_tests_observe.py` takes its population from this function, and
    a scan that quietly stopped matching would shrink that sweep while it
    printed the same shape - the same defect one level further out.
    """
    path = cmakelists or CMAKELISTS
    gates = parse_gate_checks(path.read_text(errors="replace"))
    if len(gates) < GATE_CHECK_FLOOR:
        raise SystemExit(f"gate_checks found only {len(gates)} checks in "
                         f"{path.name}, below the floor of {GATE_CHECK_FLOOR}; "
                         f"the scan is broken")
    return gates


# A gate check may sit outside the requirement only with its REASON RECORDED
# here. This is a hand-maintained list and it is meant to be one: it is the
# visible debt, it may only shrink, and an entry naming something CMakeLists.txt
# no longer registers fails the run - so an exemption cannot outlive its
# subject.
#
# It held FOURTEEN names on the morning of 2026-08-13, and the thirteen that
# left are the argument against the shape: every one of them was damageable, and
# none had been tried. Two needed a flag nobody had used (`--root`, `--src`),
# six needed the same six-line perturbation of a committed artifact, and two
# needed a hard-linked shadow tree because they read their inputs through module
# constants. What kept them exempt was that the list existed.
EXEMPT = {
    "checks-can-fail":
        "it is this harness. A damage case for it would be self-referential, "
        "and its own ability to fail is demonstrated by every other case here "
        "plus tools/test_verify_checks_can_fail.py.",
}

# A RATCHET. Raising this is a deliberate act that shows up in review; without
# it, "add a reason to EXEMPT" is an easier way out than writing a damage case,
# and the list grows instead of shrinking.
MAX_EXEMPT = 1


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
    registered = gate_checks()
    structural = []

    # A case naming a check CMake does not register guards nothing: the name
    # drifted or the check was retired, and either way it stopped being evidence
    # without anybody noticing. It still scores `ok`, which is why this is
    # counted rather than trusted.
    orphans = sorted({case[0] for case in CASES} - set(registered))
    if orphans:
        structural.append(
            f"these CASES name a check CMakeLists.txt does not register: "
            f"{', '.join(orphans)}. A case defending nothing scores `ok` and "
            f"covers no gate check.")

    # An exemption that outlives its subject is a hole nobody can see, because
    # the name it excuses no longer appears anywhere to be looked up.
    stale = sorted(set(EXEMPT) - set(registered))
    if stale:
        structural.append(
            f"EXEMPT names {', '.join(stale)}, which CMakeLists.txt no longer "
            f"registers. An exemption must not outlive its subject.")
    if len(EXEMPT) > MAX_EXEMPT:
        structural.append(
            f"{len(EXEMPT)} exemptions against a ratchet of {MAX_EXEMPT}. "
            f"Exemptions may only shrink; write the damage case instead.")

    uncovered = sorted(set(registered) - set(EXEMPT) - proved)
    if uncovered:
        print("\nchecks-can-fail: these checks have no damage case that "
              "actually ran, so nothing here shows they can fail:",
              file=sys.stderr)
        for name in uncovered:
            reason = next((why for check, _, why in skipped if check == name),
                          "no case is declared for it")
            print(f"    {name}  ({registered[name]})  - {reason}",
                  file=sys.stderr)
        print("  Add a case to CASES, or record in EXEMPT why the check "
              "cannot be damaged mechanically.", file=sys.stderr)
    for problem in structural:
        print(f"\nchecks-can-fail: {problem}", file=sys.stderr)

    print(f"checks-can-fail: {len(set(registered) - set(EXEMPT))} of "
          f"{len(registered)} registered check(s) must prove they can fail, "
          f"{len(EXEMPT)} exempt")
    if failures or uncovered or structural:
        print(f"\nchecks-can-fail: {len(failures)} check(s) did not refuse "
              f"damaged input, {len(uncovered)} unproven, "
              f"{len(structural)} structural problem(s)", file=sys.stderr)
        for check, description, why in failures:
            print(f"    {check}: {description} - {why}", file=sys.stderr)
        return 1

    print(f"\nchecks-can-fail: {len(passed)} damage case(s) refused across "
          f"{len(proved)} of {len(registered)} gate check(s), "
          f"{len(skipped)} skipped, {len(EXEMPT)} exempt")
    return 0


if __name__ == "__main__":
    sys.exit(main())

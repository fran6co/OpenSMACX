#!/usr/bin/env python3

import argparse
import concurrent.futures
import cpu
import os
import re
from pathlib import Path
import subprocess
import sys
from typing import NoReturn

# WHY THIS FILE PREFETCHES.
#
# Measured on the debug preset: this check was 49.36 s of a 47-second gate lane
# - the whole critical path, and every recovery pays it twice, once per preset.
# 46.2 s of that was user CPU inside serial subprocesses.
#
# The obvious suspect was wrong, and the wrong answer is worth recording. The
# 137-object -fno-exceptions sweep looks like the expensive part because it is
# 274 processes, but it measures 3.7 s. The cost is the THIRTY-THREE full
# `objdump -d` disassemblies below: each one disassembles an entire translation
# unit - up to 2.3 MB - so that a regex can pull one function body out of it,
# and they average 1.4 s.
#
# Those calls are pure functions of a file on disk, so they parallelise with no
# reordering hazard. Rather than restructure 2,400 lines of linear checks into
# concurrent blocks - the kind of refactor that silently drops a check, which is
# the exact defect class this file exists to catch - the checks stay linear and
# untouched, and `run()` becomes a cache that is filled in parallel first.
#
# TWO PROPERTIES MAKE THIS SAFE TO DO TO A VERIFIER:
#
#  1. A cache miss is not an error. Any command the plan failed to anticipate is
#     executed on demand exactly as before, so the verdict cannot change - only
#     the wall clock. Under-planning is slow, never wrong.
#  2. A prefetch failure is not a verdict. If a speculatively-issued command
#     fails, the exception is discarded and nothing is cached, so the real call
#     re-runs it and raises where it always did.
#
# The plan is read out of THIS FILE'S OWN SOURCE, so it cannot go stale: adding
# a `run([args.objdump, ...])` line anywhere below adds it to the prefetch
# automatically. A shape the scanner fails to recognise degrades to property 1.

_CACHE = {}
_EXECUTED_ON_DEMAND = 0


def _execute(command):
    return subprocess.run(command, check=True, text=True, capture_output=True).stdout


def run(command):
    """Return this command's stdout, from the prefetch cache when possible."""
    global _EXECUTED_ON_DEMAND
    key = tuple(command)
    if key in _CACHE:
        return _CACHE[key]
    _EXECUTED_ON_DEMAND += 1
    result = _execute(command)
    _CACHE[key] = result
    return result


# `run([args.objdump, "-d", "-r", "-C", args.autosound_object])`, tolerating the
# line breaks black-style wrapping puts inside the call.
_COMMAND_SHAPE = re.compile(
    r"run\(\s*\[\s*args\.(?P<tool>objdump|nm)\s*,\s*"
    r"(?P<flags>(?:\"[^\"]*\"\s*,\s*)*)"
    r"args\.(?P<name>[a-z_]+)\s*\]")


def plan_prefetch(args, swept):
    """Every command the checks below are going to ask for, in no order.

    Over-planning is harmless and under-planning only costs time, so this
    deliberately guesses rather than proving; see the note at the top.
    """
    commands = []
    source = Path(__file__).read_text(encoding="utf-8")
    for match in _COMMAND_SHAPE.finditer(source):
        name = match.group("name")
        if name == "object_dir":
            continue
        value = getattr(args, name, None)
        if not value:
            continue
        tool = args.objdump if match.group("tool") == "objdump" else args.nm
        flags = re.findall(r"\"([^\"]*)\"", match.group("flags"))
        commands.append([tool, *flags, str(value)])
    # The sweep loop reaches its files through a local, so no source shape names
    # them; it always asks these two questions of every object it visits.
    named = [value for name, value in vars(args).items()
             if name.endswith("object") and value and name != "object_dir"]
    for value in [str(p) for p in swept] + [str(v) for v in named]:
        commands.append([args.objdump, "-h", value])
        commands.append([args.nm, "-u", value])
    unique = {tuple(command): command for command in commands}
    return list(unique.values())


def prefetch(commands, workers):
    workers = cpu.worker_count(workers)
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        pending = {pool.submit(_execute, command): command
                   for command in commands}
        for future in concurrent.futures.as_completed(pending):
            try:
                _CACHE[tuple(pending[future])] = future.result()
            except Exception:
                # Property 2: speculation never decides anything. The real call
                # re-runs this command and raises there if it genuinely fails.
                pass


def fail(message) -> NoReturn:
    raise RuntimeError(message)


def returns_without_popping(body):
    """Thiscall bodies with no stack arguments must never pop on return.

    A tail jump satisfies the contract as well as a literal ret: the tail
    callee performs the return, and -fno-exceptions lets GCC pick that form
    for forwarding destructors. What must never appear is a popping return.
    """
    if re.search(r"\bret\s+\$", body):
        return False
    return bool(re.search(r"\bret\b", body) or re.search(r"\bjmp\b", body))


def main():
    parser = argparse.ArgumentParser(
        description="Verify the ABI of source-recovered redirect functions")
    parser.add_argument("--nm", required=True)
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--object", required=True)
    parser.add_argument("--object-dir",
                        help="sweep every *.obj here for exception machinery, "
                             "instead of relying on a hand-kept --*-object list")
    parser.add_argument("--object-floor", type=int, default=100,
                        help="refuse to report clean over fewer objects than "
                             "this; an empty directory sweeps nothing")
    # Parallel by default - an agent should not have to remember a flag to get
    # the fast path. 0 means every hardware core, resolved by
    # `byte_match.worker_count`. It used to be capped at 8 "because two gate
    # lanes run concurrently, each already at `ctest --parallel 8`" - a reason
    # that retired with the ctest suite on 2026-08-15, and the kind of stale
    # justification that outlives what made it true.
    parser.add_argument("--jobs", type=int,
                        default=0,
                        help="workers used to prefetch objdump/nm output; "
                             "1 disables prefetching entirely")
    # Brought under the -fno-exceptions sweep below, which iterates every
    # argument whose name ends in "object". The generated oracle gained a
    # setjmp/longjmp fault guard, and the guarantee that it introduces no
    # exception machinery should be gated rather than asserted - these two
    # were the only oracle objects the sweep did not see.
    parser.add_argument("--generated-signature-oracle-object")
    parser.add_argument("--oracle-fault-guard-object")
    parser.add_argument("--autosound-object")
    parser.add_argument("--scenario-object")
    parser.add_argument("--button-group-object")
    parser.add_argument("--basepop-font-object")
    parser.add_argument("--buffer-object")
    parser.add_argument("--base-button-object")
    parser.add_argument("--base-button-oracle-object")
    parser.add_argument("--dialog-object")
    parser.add_argument("--constructor-oracle-object")
    parser.add_argument("--font-object")
    parser.add_argument("--filemap-object")
    parser.add_argument("--flat-button-object")
    parser.add_argument("--heap-object")
    parser.add_argument("--graphicwin-object")
    parser.add_argument("--graphicwin-oracle-object")
    parser.add_argument("--log-object")
    parser.add_argument("--menu-object")
    parser.add_argument("--palette-object")
    parser.add_argument("--pulldown-object")
    parser.add_argument("--random-object")
    parser.add_argument("--scroll-object")
    parser.add_argument("--scroll-oracle-object")
    parser.add_argument("--runtime-oracle-object")
    parser.add_argument("--string-struct-object")
    parser.add_argument("--spot-object")
    parser.add_argument("--strings-object")
    parser.add_argument("--text-object")
    parser.add_argument("--text-index-object")
    parser.add_argument("--time-object")
    parser.add_argument("--vector-object")
    parser.add_argument("--win-object")
    parser.add_argument("--win-oracle-object")
    args = parser.parse_args()

    # The canonical executable has no C++ throw entry point: it contains no
    # _CxxThrowException, its operator new returns null instead of raising, and
    # every occurrence of the C++ exception magic sits inside CRT handling code
    # with no game callers. Recovered bodies therefore replace functions that
    # cannot unwind, and must not introduce exception machinery of their own.
    # Any object growing an .eh_frame or a personality/unwind import means a
    # translation unit lost -fno-exceptions and could raise where the original
    # could not.
    # Sweep the BUILD OUTPUT, not a hand-kept list. This loop used to visit
    # only the objects named by a `--*-object` flag in CMakeLists.txt - 35 of
    # them, against 137 the DLL actually compiles. The other 102 translation
    # units could lose -fno-exceptions and grow an .eh_frame with nothing
    # noticing, and adding a source file did not add it here, so the coverage
    # shrank as a proportion every time the project grew.
    swept = []
    if args.object_dir:
        directory = Path(args.object_dir)
        if not directory.is_dir():
            fail(f"--object-dir {directory} is not a directory, so this check "
                 f"verified NOTHING")
        swept = sorted(directory.rglob("*.obj"))
        if len(swept) < args.object_floor:
            fail(f"--object-dir {directory} holds {len(swept)} objects, fewer "
                 f"than the floor of {args.object_floor}. An empty or wrong "
                 f"directory sweeps nothing and reports clean.")
    if args.jobs > 1:
        prefetch(plan_prefetch(args, swept), args.jobs)

    named = [(name.replace("_", " "), value)
             for name, value in sorted(vars(args).items())
             if name.endswith("object") and value and name != "object_dir"]
    for described, value in named + [(str(p.name), str(p)) for p in swept]:
        # `.eh_frame`, `__gxx_personality` and `_Unwind_` were the other three
        # detectors here, and all three are GCC's. They came from the era of a
        # second compiler and could not fire once VC6 was the only one:
        # measured 2026-08-13 over 126 objects this build produces, zero carry
        # an `.eh_frame` section and none reference either symbol. A detector
        # that cannot fire is not defence in depth, it is a line that makes the
        # check look broader than it is - and this one had already cost a damage
        # case, which asserted on the GCC message and so proved nothing about
        # the compiler the tree actually uses.
        undefined = run([args.nm, "-u", value])
        if re.search(r"CxxFrameHandler", undefined):
            fail(f"{described} imports exception handling support")
    if swept:
        print(f"recovery-abi: swept {len(swept)} built objects for exception "
              f"machinery, plus {len(named)} named")

    headers = run([args.objdump, "-f", args.object])
    if "file format pe-i386" not in headers:
        fail("AlphaNet object is not a 32-bit PE COFF object")

    symbols = run([args.nm, "--defined-only", args.object])
    required_symbols = {
        "AlphaNet::pid_2_idx(unsigned int)": r"_ZN8AlphaNet9pid_2_idxEj",
        "AlphaNet::pid_2_who(unsigned int)": r"_ZN8AlphaNet9pid_2_whoEj",
        "AlphaNet::who_2_pid(int)": r"_ZN8AlphaNet9who_2_pidEi",
        "AlphaNet::who_2_idx(int)": r"_ZN8AlphaNet9who_2_idxEi",
        "alpha_net_pid_to_idx_redirect": r"@_Z29alpha_net_pid_to_idx_redirectP8AlphaNetPvj@12",
        "alpha_net_pid_to_who_redirect": r"@_Z29alpha_net_pid_to_who_redirectP8AlphaNetPvj@12",
        "alpha_net_who_to_pid_redirect": r"@_Z29alpha_net_who_to_pid_redirectP8AlphaNetPvi@12",
        "alpha_net_who_to_idx_redirect": r"@_Z29alpha_net_who_to_idx_redirectP8AlphaNetPvi@12",
    }
    for description, symbol in required_symbols.items():
        if symbol not in symbols:
            fail(f"missing required AlphaNet symbol: {description}")

    disassembly = run([args.objdump, "-d", "-C", args.object])
    for description, label in (
            ("AlphaNet::pid_2_idx", "AlphaNet::pid_2_idx(unsigned int)"),
            ("AlphaNet::pid_2_who", "AlphaNet::pid_2_who(unsigned int)"),
            ("AlphaNet::who_2_pid", "AlphaNet::who_2_pid(int)"),
            ("AlphaNet::who_2_idx", "AlphaNet::who_2_idx(int)")):
        method = re.search(
            rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            disassembly, re.DOTALL)
        if not method:
            fail(f"could not locate {description} in disassembly")
        if not re.search(r"\bret\s+\$0x4\b", method.group("body")):
            fail(f"{description} does not use one-argument thiscall cleanup")
        if description != "AlphaNet::pid_2_idx" and re.search(
                r"\bcall\b", method.group("body")):
            fail(f"{description} unexpectedly contains a call")

    for description, label in (
            ("PID-to-index", "@_Z29alpha_net_pid_to_idx_redirectP8AlphaNetPvj@12"),
            ("PID-to-identity", "@_Z29alpha_net_pid_to_who_redirectP8AlphaNetPvj@12"),
            ("identity-to-PID", "@_Z29alpha_net_who_to_pid_redirectP8AlphaNetPvi@12"),
            ("identity-to-index", "@_Z29alpha_net_who_to_idx_redirectP8AlphaNetPvi@12")):
        adapter = re.search(
            rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            disassembly, re.DOTALL)
        if not adapter:
            fail(f"could not locate the AlphaNet {description} adapter in disassembly")
        if not re.search(r"\bret\s+\$0x4\b", adapter.group("body")):
            fail(f"AlphaNet {description} adapter does not pop its stack argument")

    if args.autosound_object:
        auto_headers = run([args.objdump, "-f", args.autosound_object])
        if "file format pe-i386" not in auto_headers:
            fail("AutoSound object is not a 32-bit PE COFF object")
        auto_symbols = run(
            [args.nm, "--defined-only", args.autosound_object])
        for description, symbol in (
                ("AutoSound constructor body", "__ZN9AutoSound9constructEv"),
                ("AutoSound constructor adapter",
                 "@_Z29auto_sound_construct_redirectP9AutoSoundPv@8")):
            if symbol not in auto_symbols:
                fail(f"missing required {description} symbol")
        auto_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.autosound_object])
        construct = re.search(
            r"<AutoSound::construct\(\)>:(?P<body>.*?)"
            r"(?=\n[0-9a-f]+ <|\Z)", auto_disassembly, re.DOTALL)
        if not construct or not returns_without_popping(construct.group("body")):
            fail("AutoSound constructor violates no-argument thiscall cleanup")
        # Debug addresses the first four fields directly from `this`; Release
        # may rebase the destination register to `this + 0x14`.  Both forms
        # retain the legacy +4,+C,+10,+8 write order.
        initial_stores = (
            r"\$0x66ff34.*?(?:"
            r"0x4.*?0xc.*?0x10.*?0x8|"
            r"-0x10.*?-0x8.*?-0x4.*?-0xc)"
        )
        if not re.search(
                initial_stores, construct.group("body"), re.DOTALL):
            fail("AutoSound constructor does not preserve its initial store order")
        adapter = re.search(
            r"<@_Z29auto_sound_construct_redirectP9AutoSoundPv@8>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            auto_disassembly, re.DOTALL)
        if not adapter or not returns_without_popping(adapter.group("body")):
            fail("AutoSound constructor adapter violates thiscall cleanup")

    if args.palette_object:
        palette_headers = run([args.objdump, "-f", args.palette_object])
        if "file format pe-i386" not in palette_headers:
            fail("Palette object is not a 32-bit PE COFF object")
        palette_symbols = run(
            [args.nm, "--defined-only", args.palette_object])
        for description, symbol in (
                ("Palette RGBQUAD body",
                 "__ZN7Palette11get_rgbquadEP10tagRGBQUADii"),
                ("Palette RGBQUAD adapter",
                 "@_Z28palette_get_rgbquad_redirectP7PalettePvP10tagRGBQUADii@20")):
            if symbol not in palette_symbols:
                fail(f"missing required {description} symbol")
        palette_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.palette_object])
        palette_bodies = {}
        for description, label in (
                ("Palette RGBQUAD body",
                 "Palette::get_rgbquad(tagRGBQUAD*, int, int)"),
                ("Palette RGBQUAD adapter",
                 "@_Z28palette_get_rgbquad_redirectP7PalettePvP10tagRGBQUADii@20")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                palette_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            palette_bodies[description] = match.group("body")
        if not re.search(
                r"\bret\s+\$0xc\b",
                palette_bodies["Palette RGBQUAD body"]):
            fail("Palette RGBQUAD body does not pop three stack arguments")
        adapter_body = palette_bodies["Palette RGBQUAD adapter"]
        if not (re.search(r"\bret\s+\$0xc\b", adapter_body)
                or re.search(
                    r"\bjmp\b[^\n]*<Palette::get_rgbquad\(", adapter_body)):
            fail("Palette RGBQUAD adapter does not pop three stack arguments")
        body = palette_bodies["Palette RGBQUAD body"]
        direct_access_order = (
            r"\(%e[a-z]{2}\).*?0x2\(%e[a-z]{2}\).*?"
            r"0x1\(%e[a-z]{2}\).*?0x1\(%e[a-z]{2}\).*?"
            r"0x2\(%e[a-z]{2}\).*?\(%e[a-z]{2}\).*?"
            r"0x3\(%e[a-z]{2}\)"
        )
        # Release advances the source by four bytes before each destination
        # write and folds the destination/source delta into an indexed
        # address.  Its -2,-3,-3,-2,-4,-1 displacements are the same
        # read-red/write-blue, read-green/write-green,
        # read-blue/write-red, write-reserved sequence.
        rebased_access_order = (
            r"\(%e[a-z]{2}\).*?-0x2\([^\n]+\).*?"
            r"-0x3\(%e[a-z]{2}\).*?-0x3\([^\n]+\).*?"
            r"-0x2\(%e[a-z]{2}\).*?-0x4\([^\n]+\).*?"
            r"-0x1\([^\n]+\)"
        )
        # A cross g++ emitted a THIRD shape with the same
        # semantics: it advances the source by four before the SECOND read
        # rather than before the first write, and keeps a separate destination
        # register, so the destination displacements are positive and the
        # source displacements are the -3/-2 of an already-advanced pointer.
        #
        #   movzbl (%eax),%ecx      -> mov %cl,0x2(%edx)   read red,   write blue
        #   movzbl -0x3(%eax),%ecx  -> mov %cl,0x1(%edx)   read green, write green
        #   movzbl -0x2(%eax),%ecx  -> mov %cl,(%edx)      read blue,  write red
        #   movb   $0x0,0x3(%edx)                          write reserved
        #
        # Written out pair by pair rather than as a looser pattern, because the
        # property being checked is an ORDER: a regex permissive enough to
        # match a permutation of these would pass the exact defect the check
        # exists to catch, and this is the third shape to arrive - there will
        # be a fourth.
        advanced_source_access_order = (
            r"movzbl\s+\(%e[a-z]{2}\),.*?mov\s+%[a-z]l,0x2\(%e[a-z]{2}\).*?"
            r"movzbl\s+-0x3\(%e[a-z]{2}\),.*?mov\s+%[a-z]l,0x1\(%e[a-z]{2}\).*?"
            r"movzbl\s+-0x2\(%e[a-z]{2}\),.*?mov\s+%[a-z]l,\(%e[a-z]{2}\).*?"
            r"movb\s+\$0x0,0x3\(%e[a-z]{2}\)"
        )
        if not (re.search(direct_access_order, body, re.DOTALL)
                or re.search(rebased_access_order, body, re.DOTALL)
                or re.search(advanced_source_access_order, body, re.DOTALL)):
            fail("Palette RGBQUAD conversion lost its alias-sensitive access order")

    if args.buffer_object:
        buffer_headers = run([args.objdump, "-f", args.buffer_object])
        if "file format pe-i386" not in buffer_headers:
            fail("Buffer object is not a 32-bit PE COFF object")
        buffer_symbols = run(
            [args.nm, "--defined-only", args.buffer_object])
        for description, symbol in (
                ("Buffer constructor body", "__ZN6Buffer9constructEv"),
                ("Buffer constructor adapter",
                 "@_Z25buffer_construct_redirectP6BufferPv@8")):
            if symbol not in buffer_symbols:
                fail(f"missing required {description} symbol")
        buffer_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.buffer_object])
        construct = re.search(
            r"<Buffer::construct\(\)>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            buffer_disassembly, re.DOTALL)
        if not construct or not returns_without_popping(construct.group("body")):
            fail("Buffer constructor violates no-argument thiscall cleanup")
        body = construct.group("body")
        if ("Spot::Spot()" not in body
                or "Palette::get_rgbquad(tagRGBQUAD*, int, int)" not in body):
            fail("Buffer constructor does not retain both source-owned calls")
        if not re.search(
                r"\$0x66fdbc.*?0x50c.*?0x520.*?0x52c.*?"
                r"0x53c.*?0x54c.*?0x55c.*?0x56c.*?"
                r"0x540.*?0x550.*?0x560.*?0x570",
                body, re.DOTALL):
            fail("Buffer constructor does not preserve its text-default order")
        adapter = re.search(
            r"<@_Z25buffer_construct_redirectP6BufferPv@8>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            buffer_disassembly, re.DOTALL)
        if not adapter or not returns_without_popping(adapter.group("body")):
            fail("Buffer constructor adapter violates thiscall cleanup")

    if args.win_object:
        win_headers = run([args.objdump, "-f", args.win_object])
        if "file format pe-i386" not in win_headers:
            fail("Win object is not a 32-bit PE COFF object")
        win_symbols = run([args.nm, "--defined-only", args.win_object])
        required_win_symbols = {
            "Win constructor body": "__ZN3Win9constructEv",
            "Win constructor adapter":
                "@_Z22win_construct_redirectP3WinPv@8",
            "Win move": "__ZN3Win4moveEii",
            "Win visibility": "__ZN3Win10is_visibleEv",
            "Win client translation": "__ZN3Win16client_to_screenEPiS0_",
            "Win vertical paging": "__ZN3Win15set_vert_pagingEi",
            "Win horizontal paging": "__ZN3Win15set_horz_pagingEi",
            "Win move adapter": "@_Z17win_move_redirectP3WinPvii@16",
            "Win visibility adapter":
                "@_Z23win_is_visible_redirectP3WinPv@8",
            "Win client-translation adapter":
                "@_Z29win_client_to_screen_redirectP3WinPvPiS2_@16",
            "Win vertical paging adapter":
                "@_Z28win_set_vert_paging_redirectP3WinPvi@12",
            "Win horizontal paging adapter":
                "@_Z28win_set_horz_paging_redirectP3WinPvi@12",
            "RECT construction helper": "__Z9make_rectP7tagRECTiiii",
            "six-argument in_box": "__Z6in_boxiiiiii",
            "rectangle-center helper": "__Z11rect_centerP7tagRECTPiS1_",
            "TutWin rectangle-center adapter":
                "@_Z27tutwin_rect_center_redirectPvS_P7tagRECTPiS2_@20",
        }
        for description, symbol in required_win_symbols.items():
            if symbol not in win_symbols:
                fail(f"missing required Win symbol: {description}")
        win_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.win_object])
        win_construct = re.search(
            r"<Win::construct\(\)>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            win_disassembly, re.DOTALL)
        if not win_construct or not returns_without_popping(
                win_construct.group("body")):
            fail("Win constructor violates no-argument thiscall cleanup")
        if ("AutoSound::construct()" not in win_construct.group("body")
                or not re.search(
                    r"\$0x66ff30.*?\$0x66fdd0.*?0xa8.*?0x3fc.*?"
                    r"0x130.*?0xfc.*?0x100.*?0x114.*?0x104",
                    win_construct.group("body"), re.DOTALL)):
            fail("Win constructor does not preserve its call/default order")
        win_construct_adapter = re.search(
            r"<@_Z22win_construct_redirectP3WinPv@8>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            win_disassembly, re.DOTALL)
        if not win_construct_adapter or not returns_without_popping(
                win_construct_adapter.group("body")):
            fail("Win constructor adapter violates thiscall cleanup")
        for description, label, stack_bytes in (
                ("Win move", "Win::move(int, int)", "8"),
                ("Win client translation",
                 "Win::client_to_screen(int*, int*)", "8"),
                ("Win vertical paging", "Win::set_vert_paging(int)", "4"),
                ("Win horizontal paging", "Win::set_horz_paging(int)", "4"),
                ("Win move adapter", "@_Z17win_move_redirectP3WinPvii@16", "8"),
                ("Win client-translation adapter",
                 "@_Z29win_client_to_screen_redirectP3WinPvPiS2_@16", "8"),
                ("Win vertical paging adapter",
                 "@_Z28win_set_vert_paging_redirectP3WinPvi@12", "4"),
                ("Win horizontal paging adapter",
                 "@_Z28win_set_horz_paging_redirectP3WinPvi@12", "4")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                win_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            returns = re.findall(
                r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            if not returns or any(value != stack_bytes for value in returns):
                fail(f"{description} does not pop {stack_bytes} stack bytes")

        for description, label in (
                ("Win visibility", "Win::is_visible()"),
                ("Win visibility adapter",
                 "@_Z23win_is_visible_redirectP3WinPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                win_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            if not returns_without_popping(match.group("body")):
                fail(f"{description} unexpectedly pops stack arguments")

        for description, label in (
                ("RECT construction helper",
                 "make_rect(tagRECT*, int, int, int, int)"),
                ("six-argument in_box",
                 "in_box(int, int, int, int, int, int)"),
                ("rectangle-center helper",
                 "rect_center(tagRECT*, int*, int*)")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                win_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            if not re.search(r"\bret\s*$", body, re.MULTILINE) or re.search(
                    r"\bret\s+\$", body):
                fail(f"{description} is not cdecl")
        center_adapter = re.search(
            r"<@_Z27tutwin_rect_center_redirectPvS_P7tagRECTPiS2_@20>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            win_disassembly, re.DOTALL)
        if not center_adapter or not re.search(
                r"\bret\s+\$0xc\b", center_adapter.group("body")):
            fail("TutWin rectangle-center adapter does not pop three stack arguments")

    if args.win_oracle_object:
        oracle_headers = run([args.objdump, "-f", args.win_oracle_object])
        if "file format pe-i386" not in oracle_headers:
            fail("Win runtime oracle object is not a 32-bit PE COFF object")
        oracle_symbols = run([args.nm, "--defined-only", args.win_oracle_object])
        if "__Z20run_win_oracle_suitev" not in oracle_symbols:
            fail("missing Win runtime oracle suite entry point")
        oracle_disassembly = run(
            [args.objdump, "-d", "-C", args.win_oracle_object])
        for description, address in (
                ("visibility", 0x005F7E90),
                ("client translation", 0x005ED240)):
            call = re.search(
                rf"\bmov\s+\$0x{address:x},%(?P<register>e[a-z]{{2}})"
                rf"(?:[^\n]*\n){{0,8}}?[^\n]*\bcall\s+\*%(?P=register)",
                oracle_disassembly)
            if not call:
                fail(
                    f"Win runtime oracle lacks raw original {description} "
                    f"call at 0x{address:08X}")

    if args.graphicwin_object:
        graphic_headers = run([args.objdump, "-f", args.graphicwin_object])
        if "file format pe-i386" not in graphic_headers:
            fail("GraphicWin object is not a 32-bit PE COFF object")
        graphic_symbols = run(
            [args.nm, "--defined-only", args.graphicwin_object])
        for description, symbol in (
                ("GraphicWin constructor body",
                 "__ZN10GraphicWin9constructEv"),
                ("GraphicWin constructor adapter",
                 "@_Z30graphic_win_construct_redirectP10GraphicWinPv@8"),
                ("GraphicWin close", "__ZN10GraphicWin5closeEv"),
                ("GraphicWin close adapter",
                 "@_Z26graphic_win_close_redirectP10GraphicWinPv@8")):
            if symbol not in graphic_symbols:
                fail(f"missing required {description} symbol")
        graphic_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.graphicwin_object])
        construct = re.search(
            r"<GraphicWin::construct\(\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            graphic_disassembly, re.DOTALL)
        if not construct or not returns_without_popping(construct.group("body")):
            fail("GraphicWin constructor violates thiscall cleanup")
        direct_constructor_order = (
            r"Win::construct\(\).*?0x444.*?Buffer::construct\(\).*?"
            r"\$0x66fc50.*?\$0x66fc48.*?0xa10.*?0x134.*?0x138.*?"
            r"0x9cc.*?0xa08.*?0xa0c"
        )
        # Release prepares the [0x9CC,0xA0C) loop bounds before the second
        # vtable store, then emits two ordered volatile stores per iteration.
        optimized_constructor_order = (
            r"Win::construct\(\).*?0x444.*?Buffer::construct\(\).*?"
            r"\$0x66fc50.*?0x9cc.*?0xa0c.*?\$0x66fc48.*?"
            r"0xa10.*?0x134.*?0x138.*?"
            r"movl\s+\$0x0,\(%e[a-z]{2}\).*?"
            r"movl\s+\$0x0,-0x4\(%e[a-z]{2}\).*?0xa0c"
        )
        if not (re.search(
                    direct_constructor_order,
                    construct.group("body"), re.DOTALL)
                or re.search(
                    optimized_constructor_order,
                    construct.group("body"), re.DOTALL)):
            fail("GraphicWin constructor does not preserve its closure/order")
        close_adapter = re.search(
            r"<@_Z26graphic_win_close_redirectP10GraphicWinPv@8>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            graphic_disassembly, re.DOTALL)
        if not close_adapter or not returns_without_popping(
                close_adapter.group("body")):
            fail("GraphicWin close adapter violates no-argument thiscall cleanup")

    if args.graphicwin_oracle_object:
        oracle_headers = run(
            [args.objdump, "-f", args.graphicwin_oracle_object])
        if "file format pe-i386" not in oracle_headers:
            fail("GraphicWin runtime oracle object is not a 32-bit PE COFF object")
        oracle_symbols = run(
            [args.nm, "--defined-only", args.graphicwin_oracle_object])
        if "__Z28run_graphic_win_oracle_suitev" not in oracle_symbols:
            fail("missing GraphicWin runtime oracle suite entry point")
        oracle_disassembly = run(
            [args.objdump, "-d", "-C", args.graphicwin_oracle_object])
        call = re.search(
            r"\bmov\s+\$0x5d4e40,%(?P<register>e[a-z]{2})"
            r"(?:[^\n]*\n){0,8}?[^\n]*\bcall\s+\*%(?P=register)",
            oracle_disassembly)
        if not call:
            fail(
                "GraphicWin runtime oracle lacks raw original close call "
                "at 0x005D4E40")

    if args.base_button_object:
        base_headers = run([args.objdump, "-f", args.base_button_object])
        if "file format pe-i386" not in base_headers:
            fail("BaseButton object is not a 32-bit PE COFF object")
        base_symbols = run(
            [args.nm, "--defined-only", args.base_button_object])
        required_base_symbols = {
            "BaseButton constructor body": "__ZN10BaseButton9constructEv",
            "BaseButton constructor adapter":
                "@_Z30base_button_construct_redirectP10BaseButtonPv@8",
            "BaseButton close": "__ZN10BaseButton5closeEv",
            "BaseButton destructor body": "__ZN10BaseButton7destroyEv",
            "BaseButton close adapter":
                "@_Z26base_button_close_redirectP10BaseButtonPv@8",
            "BaseButton destructor adapter":
                "@_Z31base_button_destructor_redirectP10BaseButtonPv@8",
        }
        for description, symbol in required_base_symbols.items():
            if symbol not in base_symbols:
                fail(f"missing required {description} symbol")
        base_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.base_button_object])
        base_construct = re.search(
            r"<BaseButton::construct\(\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            base_disassembly, re.DOTALL)
        if not base_construct or not returns_without_popping(
                base_construct.group("body")):
            fail("BaseButton constructor violates thiscall cleanup")
        if not re.search(
                r"GraphicWin::construct\(\).*?0xa1c.*?Time::Time\(\).*?"
                r"0xa4c.*?Time::Time\(\).*?\$0x670290.*?\$0x670288.*?"
                r"0xa74.*?0xa44.*?0xa48.*?0xa78.*?0xa9c.*?"
                r"0xa7c.*?0xa80.*?0xaa8.*?0xaac.*?0xab0.*?0xab4",
                base_construct.group("body"), re.DOTALL):
            fail("BaseButton constructor does not preserve its closure/order")
        base_bodies = {}
        for description, label in (
                ("BaseButton close", "BaseButton::close()"),
                ("BaseButton destructor body", "BaseButton::destroy()"),
                ("BaseButton close adapter",
                 "@_Z26base_button_close_redirectP10BaseButtonPv@8"),
                ("BaseButton destructor adapter",
                 "@_Z31base_button_destructor_redirectP10BaseButtonPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                base_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            base_bodies[label] = match.group("body")
            if not returns_without_popping(match.group("body")):
                fail(f"{description} violates no-argument thiscall cleanup")

        base_close = base_bodies["BaseButton::close()"]
        if not re.search(
                r"GraphicWin::close\(\).*?0xa74.*?0xa9c.*?0xa78.*?"
                r"0xa44.*?0xa48.*?0xaac.*?0xab0.*?0xab4.*?"
                r"0xa94.*?0xa84.*?0xa88.*?0xa8c.*?0xa90.*?"
                r"0xa98.*?0xaa4.*?0xaa0.*?0xa7c.*?_free.*?"
                r"0xa80.*?_free.*?0xaa8",
                base_close, re.DOTALL):
            fail("BaseButton close does not preserve its legacy access order")
        if len(re.findall(r"\bdir32\s+_free\s*$", base_close,
                          re.MULTILINE)) != 2:
            fail("BaseButton close lacks exactly two executable free calls")

        base_destroy = base_bodies["BaseButton::destroy()"]
        if not re.search(
                r"\$0x670290.*?\$0x670288.*?BaseButton::close\(\).*?"
                r"0xa4c.*?Time::~Time\(\).*?0xa1c.*?Time::~Time\(\).*?"
                r"graphic_win_destructor_redirect",
                base_destroy, re.DOTALL):
            fail("BaseButton destructor does not preserve its teardown order")
        if not re.search(r"\bmov\s+%e(?:bx|cx|si|di),%eax\b", base_destroy):
            fail("BaseButton destructor does not return its instance pointer")

    if args.base_button_oracle_object:
        oracle_headers = run(
            [args.objdump, "-f", args.base_button_oracle_object])
        if "file format pe-i386" not in oracle_headers:
            fail("BaseButton runtime oracle object is not a 32-bit PE COFF object")
        oracle_symbols = run(
            [args.nm, "--defined-only", args.base_button_oracle_object])
        for description, symbol in (
                ("BaseButton phase-one runtime oracle",
                 "__Z28run_base_button_oracle_suitev"),
                ("BaseButton deferred release oracle",
                 "__Z29run_base_button_release_suitev")):
            if symbol not in oracle_symbols:
                fail(f"missing {description} entry point")
        oracle_disassembly = run(
            [args.objdump, "-d", "-C", args.base_button_oracle_object])
        for description, address in (
                ("FlatButton destructor", 0x00406880),
                ("BaseButton destructor", 0x00607040),
                ("BaseButton close", 0x006070C0),
                ("FlatButton close", 0x00607DA0)):
            call = re.search(
                rf"\bmov\s+\$0x{address:x},%(?P<register>e[a-z]{{2}})"
                rf"(?:[^\n]*\n){{0,8}}?[^\n]*\bcall\s+\*%(?P=register)",
                oracle_disassembly)
            if not call:
                fail(
                    f"BaseButton runtime oracle lacks raw original "
                    f"{description} call at 0x{address:08X}")

    if args.constructor_oracle_object:
        constructor_headers = run(
            [args.objdump, "-f", args.constructor_oracle_object])
        if "file format pe-i386" not in constructor_headers:
            fail("Constructor runtime oracle object is not 32-bit PE COFF")
        constructor_symbols = run(
            [args.nm, "--defined-only", args.constructor_oracle_object])
        if "__Z28run_constructor_oracle_suitev" not in constructor_symbols:
            fail("missing constructor runtime oracle suite entry point")
        constructor_disassembly = run(
            [args.objdump, "-d", "-C", args.constructor_oracle_object])
        for description, address in (
                ("AutoSound constructor", 0x0062BA80),
                ("Win constructor", 0x005EB3D0),
                ("Palette RGBQUAD conversion", 0x005FE560),
                ("Buffer constructor", 0x005D7210),
                ("GraphicWin constructor", 0x005D4CF0),
                ("BaseButton constructor", 0x00606F30)):
            if not re.search(rf"\$0x{address:x}\b", constructor_disassembly):
                fail(
                    f"constructor runtime oracle lacks original {description} "
                    f"address 0x{address:08X}")

    if args.flat_button_object:
        flat_headers = run([args.objdump, "-f", args.flat_button_object])
        if "file format pe-i386" not in flat_headers:
            fail("FlatButton object is not a 32-bit PE COFF object")
        flat_symbols = run(
            [args.nm, "--defined-only", args.flat_button_object])
        required_flat_symbols = {
            "FlatButton close": "__ZN10FlatButton5closeEv",
            "FlatButton destructor body": "__ZN10FlatButton7destroyEv",
            "FlatButton close adapter":
                "@_Z26flat_button_close_redirectP10FlatButtonPv@8",
            "FlatButton destructor adapter":
                "@_Z31flat_button_destructor_redirectP10FlatButtonPv@8",
        }
        for description, symbol in required_flat_symbols.items():
            if symbol not in flat_symbols:
                fail(f"missing required {description} symbol")
        flat_disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.flat_button_object])
        flat_bodies = {}
        for description, label in (
                ("FlatButton close", "FlatButton::close()"),
                ("FlatButton destructor body", "FlatButton::destroy()"),
                ("FlatButton close adapter",
                 "@_Z26flat_button_close_redirectP10FlatButtonPv@8"),
                ("FlatButton destructor adapter",
                 "@_Z31flat_button_destructor_redirectP10FlatButtonPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                flat_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            flat_bodies[label] = match.group("body")
            if not returns_without_popping(match.group("body")):
                fail(f"{description} violates no-argument thiscall cleanup")

        flat_close = flat_bodies["FlatButton::close()"]
        close_prefix = re.search(
            r"Time::close\(\).*?0xa18.*?0xa14.*?0xab8.*?"
            r"BaseButton::close\(\)",
            flat_close, re.DOTALL)
        direct_table_loop = all(
            f"0x{offset:x}" in flat_close
            for offset in (0xABC, 0xAC8, 0xAD4, 0xAE0, 0xAEC))
        optimized_table_loop = re.search(
            r"0xabc.*?0x24.*?-0xc.*?0x18.*?"
            r"\$0x4,%edi.*?0x8.*?0xaec",
            flat_close, re.DOTALL)
        if not close_prefix or not (direct_table_loop or optimized_table_loop):
            fail("FlatButton close does not preserve its legacy access order")
        flat_destroy = flat_bodies["FlatButton::destroy()"]
        if not re.search(
                r"\$0x669754.*?\$0x66974c.*?FlatButton::close\(\).*?"
                r"BaseButton::destroy\(\)",
                flat_destroy, re.DOTALL):
            fail("FlatButton destructor does not preserve its teardown order")
        if not re.search(r"\bmov\s+%e(?:bx|cx|si|di),%eax\b", flat_destroy):
            fail("FlatButton destructor does not return its instance pointer")

    if args.vector_object:
        vector_headers = run([args.objdump, "-f", args.vector_object])
        if "file format pe-i386" not in vector_headers:
            fail("Vector object is not a 32-bit PE COFF object")
        vector_symbols = run([args.nm, "--defined-only", args.vector_object])
        required_vector_symbols = {
            "Vector constructor": "__ZN6VectorC1Ev",
            "Vector close": "__ZN6Vector5closeEv",
            "Vector subtraction": "__ZN6Vector4__miERS_S0_",
            "Vector addition assignment": "__ZN6Vector5__aplERS_",
            "Vector subtraction assignment": "__ZN6Vector5__amiERS_",
            "Vector scaling": "__ZN6Vector5scaleERS_f",
            "Vector constructor adapter":
                "@_Z25vector_construct_redirectP6VectorPv@8",
            "Vector close adapter": "@_Z21vector_close_redirectP6VectorPv@8",
            "Vector subtraction adapter":
                "@_Z24vector_subtract_redirectP6VectorPvS0_S0_@16",
            "Vector addition-assignment adapter":
                "@_Z26vector_add_assign_redirectP6VectorPvS0_@12",
            "Vector subtraction-assignment adapter":
                "@_Z31vector_subtract_assign_redirectP6VectorPvS0_@12",
            "Vector scaling adapter":
                "@_Z21vector_scale_redirectP6VectorPvS0_j@16",
        }
        for description, symbol in required_vector_symbols.items():
            if symbol not in vector_symbols:
                fail(f"missing required Vector symbol: {description}")
        vector_disassembly = run(
            [args.objdump, "-d", "-C", args.vector_object])

        vector_bodies = {}
        for description, label, stack_bytes in (
                ("Vector subtraction", "Vector::__mi(Vector&, Vector&)", "8"),
                ("Vector addition assignment", "Vector::__apl(Vector&)", "4"),
                ("Vector subtraction assignment", "Vector::__ami(Vector&)", "4"),
                ("Vector scaling", "Vector::scale(Vector&, float)", "8"),
                ("Vector subtraction adapter",
                 "@_Z24vector_subtract_redirectP6VectorPvS0_S0_@16", "8"),
                ("Vector addition-assignment adapter",
                 "@_Z26vector_add_assign_redirectP6VectorPvS0_@12", "4"),
                ("Vector subtraction-assignment adapter",
                 "@_Z31vector_subtract_assign_redirectP6VectorPvS0_@12", "4"),
                ("Vector scaling adapter",
                 "@_Z21vector_scale_redirectP6VectorPvS0_j@16", "8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                vector_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            vector_bodies[label] = body
            returns = re.findall(r"\bret\s+\$0x([0-9a-f]+)\b", body)
            if not returns or any(value != stack_bytes for value in returns):
                fail(f"{description} does not pop {stack_bytes} stack bytes")

        for description, label in (
                ("Vector constructor", "Vector::Vector()"),
                ("Vector close", "Vector::close()"),
                ("Vector constructor adapter",
                 "@_Z25vector_construct_redirectP6VectorPv@8"),
                ("Vector close adapter",
                 "@_Z21vector_close_redirectP6VectorPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                vector_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            vector_bodies[label] = body
            if not re.search(r"\bret\s*$", body, re.MULTILINE) or re.search(
                    r"\bret\s+\$", body):
                fail(f"{description} unexpectedly pops stack arguments")

        constructor_body = vector_bodies["Vector::Vector()"]
        constructor_stores = re.findall(
            r"\bmovl\s+\$0x0,(?:(0x[0-9a-f]+)?)\(%ecx\)", constructor_body)
        if [offset or "" for offset in constructor_stores] != ["", "0x4", "0x8"]:
            fail("Vector constructor lacks three ordered binary32 zero stores")
        if not re.search(r"\bmov\s+%ecx,%eax\b", constructor_body):
            fail("Vector constructor does not leave the instance pointer in EAX")

        constructor_adapter_body = vector_bodies[
            "@_Z25vector_construct_redirectP6VectorPv@8"]
        for description, body in (
                ("Vector constructor", constructor_body),
                ("Vector constructor adapter", constructor_adapter_body)):
            if re.search(r"\bxmm[0-9]+\b", body):
                fail(f"{description} unexpectedly uses SSE stores")
            if re.search(r"\bj[a-z]+\s+[0-9a-f]+\b", body):
                fail(f"{description} unexpectedly branches around its stores")

        arithmetic_requirements = (
            ("Vector subtraction", "Vector::__mi(Vector&, Vector&)", "fsubs"),
            ("Vector addition assignment", "Vector::__apl(Vector&)", "fadds"),
            ("Vector subtraction assignment", "Vector::__ami(Vector&)", "fsubs"),
            ("Vector scaling", "Vector::scale(Vector&, float)", "fmuls"),
        )
        for description, label, operation in arithmetic_requirements:
            body = vector_bodies[label]
            if (len(re.findall(r"\bflds\b", body)) != 3
                    or len(re.findall(rf"\b{operation}\b", body)) != 3
                    or len(re.findall(r"\bfstps\b", body)) != 3):
                fail(f"{description} lacks three ordered binary32 x87 operations")
            if re.search(r"\bxmm[0-9]+\b", body):
                fail(f"{description} unexpectedly uses SSE floating-point operations")

    if args.scroll_object:
        scroll_headers = run([args.objdump, "-f", args.scroll_object])
        if "file format pe-i386" not in scroll_headers:
            fail("Scroll object is not a 32-bit PE COFF object")
        scroll_symbols = run([args.nm, "--defined-only", args.scroll_object])
        required_scroll_symbols = {
            "Scroll close": "__ZN6Scroll5closeEv",
            "Scroll close adapter":
                "@_Z21scroll_close_redirectP6ScrollPv@8",
            "RECT expansion helper": "__Z11expand_rectP7tagRECTii",
            "Scroll RECT initializer": "__ZN6Scroll4initEP7tagRECTP3Winii",
            "Scroll vertical initializer": "__ZN6Scroll9init_vertEiiiP3Wini",
            "Scroll horizontal initializer": "__ZN6Scroll9init_horzEiiiP3Wini",
            "Scroll vertical nonclient initializer":
                "__ZN6Scroll12init_vert_ncEiiiP3Wini",
            "Scroll horizontal nonclient initializer":
                "__ZN6Scroll12init_horz_ncEiiiP3Wini",
            "Scroll RECT initializer adapter":
                "@_Z25scroll_init_rect_redirectP6ScrollPvP7tagRECTP3Winii@24",
            "Scroll vertical initializer adapter":
                "@_Z25scroll_init_vert_redirectP6ScrollPviiiP3Wini@28",
            "Scroll horizontal initializer adapter":
                "@_Z25scroll_init_horz_redirectP6ScrollPviiiP3Wini@28",
            "Scroll vertical nonclient initializer adapter":
                "@_Z28scroll_init_vert_nc_redirectP6ScrollPviiiP3Wini@28",
            "Scroll horizontal nonclient initializer adapter":
                "@_Z28scroll_init_horz_nc_redirectP6ScrollPviiiP3Wini@28",
            "Scroll range setter": "__ZN6Scroll9set_rangeEii",
            "Scroll button-color setter": "__ZN6Scroll16set_button_colorEi",
            "Scroll bevel-thickness setter":
                "__ZN6Scroll19set_bevel_thicknessEi",
            "Scroll upper-bevel setter": "__ZN6Scroll15set_bevel_upperEi",
            "Scroll lower-bevel setter": "__ZN6Scroll15set_bevel_lowerEi",
            "Scroll bar-thickness setter":
                "__ZN6Scroll17set_bar_thicknessEi",
            "Scroll border-color setter": "__ZN6Scroll16set_border_colorEi",
            "Scroll left-sprite setter":
                "__ZN6Scroll15set_sprite_leftEP6SpriteS1_S1_",
            "Scroll right-sprite setter":
                "__ZN6Scroll16set_sprite_rightEP6SpriteS1_S1_",
            "Scroll upper-sprite setter":
                "__ZN6Scroll13set_sprite_upEP6SpriteS1_S1_",
            "Scroll lower-sprite setter":
                "__ZN6Scroll15set_sprite_downEP6SpriteS1_S1_",
            "Scroll thumb computation":
                "__ZN6Scroll18compute_thumb_rectEP7tagRECT",
            "Scroll position setter": "__ZN6Scroll7set_posEi",
            "Scroll thumb-rectangle reset": "__ZN6Scroll14set_thumb_rectEv",
            "Scroll range adapter":
                "@_Z25scroll_set_range_redirectP6ScrollPvii@16",
            "Scroll button-color adapter":
                "@_Z32scroll_set_button_color_redirectP6ScrollPvi@12",
            "Scroll bevel-thickness adapter":
                "@_Z35scroll_set_bevel_thickness_redirectP6ScrollPvi@12",
            "Scroll upper-bevel adapter":
                "@_Z31scroll_set_bevel_upper_redirectP6ScrollPvi@12",
            "Scroll lower-bevel adapter":
                "@_Z31scroll_set_bevel_lower_redirectP6ScrollPvi@12",
            "Scroll bar-thickness adapter":
                "@_Z33scroll_set_bar_thickness_redirectP6ScrollPvi@12",
            "Scroll border-color adapter":
                "@_Z32scroll_set_border_color_redirectP6ScrollPvi@12",
            "Scroll left-sprite adapter":
                "@_Z31scroll_set_sprite_left_redirectP6ScrollPvP6SpriteS3_S3_@20",
            "Scroll right-sprite adapter":
                "@_Z32scroll_set_sprite_right_redirectP6ScrollPvP6SpriteS3_S3_@20",
            "Scroll upper-sprite adapter":
                "@_Z29scroll_set_sprite_up_redirectP6ScrollPvP6SpriteS3_S3_@20",
            "Scroll lower-sprite adapter":
                "@_Z31scroll_set_sprite_down_redirectP6ScrollPvP6SpriteS3_S3_@20",
            "Scroll thumb-computation adapter":
                "@_Z34scroll_compute_thumb_rect_redirectP6ScrollPvP7tagRECT@12",
            "Scroll position adapter":
                "@_Z23scroll_set_pos_redirectP6ScrollPvi@12",
            "Scroll thumb-rectangle adapter":
                "@_Z30scroll_set_thumb_rect_redirectP6ScrollPv@8",
        }
        for description, symbol in required_scroll_symbols.items():
            if symbol not in scroll_symbols:
                fail(f"missing required Scroll symbol: {description}")
        scroll_disassembly = run(
            [args.objdump, "-d", "-C", args.scroll_object])
        scroll_relocations = run(
            [args.objdump, "-r", "-C", args.scroll_object])
        expand_rect_match = re.search(
            r"<expand_rect\(tagRECT\*, int, int\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        if not expand_rect_match:
            fail("could not locate RECT expansion helper in disassembly")
        expand_rect_body = expand_rect_match.group("body")
        if (not re.search(r"\bret\s*$", expand_rect_body, re.MULTILINE)
                or re.search(r"\bret\s+\$", expand_rect_body)):
            fail("RECT expansion helper is not cdecl")
        rect_accesses = []
        rect_operands = (
            (0, r"(?:^|[\s,])\(%(?!e?sp\))[a-z0-9]+\)"),
            (8, r"\b0x8\(%(?!e?sp\))[a-z0-9]+\)"),
            (4, r"\b0x4\(%(?!e?sp\))[a-z0-9]+\)"),
            (12, r"\b0xc\(%(?!e?sp\))[a-z0-9]+\)"),
        )
        for line in expand_rect_body.splitlines():
            for offset, operand in rect_operands:
                if re.search(operand, line):
                    rect_accesses.append(offset)
                    break
        if rect_accesses not in (
                [0, 8, 4, 12],
                [0, 0, 8, 8, 4, 4, 12, 12]):
            fail("RECT expansion helper does not preserve field access order")
        for description, label, stack_bytes in (
                ("Scroll RECT initializer",
                 "Scroll::init(tagRECT*, Win*, int, int)", "10"),
                ("Scroll vertical initializer",
                 "Scroll::init_vert(int, int, int, Win*, int)", "14"),
                ("Scroll horizontal initializer",
                 "Scroll::init_horz(int, int, int, Win*, int)", "14"),
                ("Scroll vertical nonclient initializer",
                 "Scroll::init_vert_nc(int, int, int, Win*, int)", "14"),
                ("Scroll horizontal nonclient initializer",
                 "Scroll::init_horz_nc(int, int, int, Win*, int)", "14"),
                ("Scroll RECT initializer adapter",
                 "@_Z25scroll_init_rect_redirectP6ScrollPvP7tagRECTP3Winii@24",
                 "10"),
                ("Scroll vertical initializer adapter",
                 "@_Z25scroll_init_vert_redirectP6ScrollPviiiP3Wini@28", "14"),
                ("Scroll horizontal initializer adapter",
                 "@_Z25scroll_init_horz_redirectP6ScrollPviiiP3Wini@28", "14"),
                ("Scroll vertical nonclient initializer adapter",
                 "@_Z28scroll_init_vert_nc_redirectP6ScrollPviiiP3Wini@28", "14"),
                ("Scroll horizontal nonclient initializer adapter",
                 "@_Z28scroll_init_horz_nc_redirectP6ScrollPviiiP3Wini@28", "14")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                scroll_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            returns = re.findall(
                r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            if not returns or any(value != stack_bytes for value in returns):
                fail(f"{description} does not pop {stack_bytes} stack bytes")

        original_init = re.search(
            r"<\(anonymous namespace\)::call_original_scroll_init"
            r"\(Scroll\*, int, int, int, int, Win\*, int, int\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        if (not original_init
                or "0x6054d0" not in original_init.group("body")
                or not re.search(r"\bcall\s+\*", original_init.group("body"))
                or not re.search(r"\bmov\b.*%ecx", original_init.group("body"))):
            fail("Scroll primary-init dependency lacks a raw thiscall dispatch")
        scroll_bodies = {}
        for description, label, stack_bytes in (
                ("Scroll range setter", "Scroll::set_range(int, int)", "8"),
                ("Scroll button-color setter",
                 "Scroll::set_button_color(int)", "4"),
                ("Scroll bevel-thickness setter",
                 "Scroll::set_bevel_thickness(int)", "4"),
                ("Scroll upper-bevel setter",
                 "Scroll::set_bevel_upper(int)", "4"),
                ("Scroll lower-bevel setter",
                 "Scroll::set_bevel_lower(int)", "4"),
                ("Scroll bar-thickness setter",
                 "Scroll::set_bar_thickness(int)", "4"),
                ("Scroll border-color setter", "Scroll::set_border_color(int)", "4"),
                ("Scroll left-sprite setter",
                 "Scroll::set_sprite_left(Sprite*, Sprite*, Sprite*)", "c"),
                ("Scroll right-sprite setter",
                 "Scroll::set_sprite_right(Sprite*, Sprite*, Sprite*)", "c"),
                ("Scroll upper-sprite setter",
                 "Scroll::set_sprite_up(Sprite*, Sprite*, Sprite*)", "c"),
                ("Scroll lower-sprite setter",
                 "Scroll::set_sprite_down(Sprite*, Sprite*, Sprite*)", "c"),
                ("Scroll border-color adapter",
                 "@_Z32scroll_set_border_color_redirectP6ScrollPvi@12", "4"),
                ("Scroll range adapter",
                 "@_Z25scroll_set_range_redirectP6ScrollPvii@16", "8"),
                ("Scroll button-color adapter",
                 "@_Z32scroll_set_button_color_redirectP6ScrollPvi@12", "4"),
                ("Scroll bevel-thickness adapter",
                 "@_Z35scroll_set_bevel_thickness_redirectP6ScrollPvi@12", "4"),
                ("Scroll upper-bevel adapter",
                 "@_Z31scroll_set_bevel_upper_redirectP6ScrollPvi@12", "4"),
                ("Scroll lower-bevel adapter",
                 "@_Z31scroll_set_bevel_lower_redirectP6ScrollPvi@12", "4"),
                ("Scroll bar-thickness adapter",
                 "@_Z33scroll_set_bar_thickness_redirectP6ScrollPvi@12", "4"),
                ("Scroll left-sprite adapter",
                 "@_Z31scroll_set_sprite_left_redirectP6ScrollPvP6SpriteS3_S3_@20",
                 "c"),
                ("Scroll right-sprite adapter",
                 "@_Z32scroll_set_sprite_right_redirectP6ScrollPvP6SpriteS3_S3_@20",
                 "c"),
                ("Scroll upper-sprite adapter",
                 "@_Z29scroll_set_sprite_up_redirectP6ScrollPvP6SpriteS3_S3_@20",
                 "c"),
                ("Scroll lower-sprite adapter",
                 "@_Z31scroll_set_sprite_down_redirectP6ScrollPvP6SpriteS3_S3_@20",
                 "c"),
                ("Scroll thumb computation",
                 "Scroll::compute_thumb_rect(tagRECT*)", "4"),
                ("Scroll thumb-computation adapter",
                 "@_Z34scroll_compute_thumb_rect_redirectP6ScrollPvP7tagRECT@12",
                 "4"),
                ("Scroll position setter", "Scroll::set_pos(int)", "4"),
                ("Scroll position adapter",
                 "@_Z23scroll_set_pos_redirectP6ScrollPvi@12", "4")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                scroll_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            scroll_bodies[label] = match.group("body")
            returns = re.findall(
                r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            if not returns or any(value != stack_bytes for value in returns):
                fail(f"{description} does not pop {stack_bytes} stack bytes")

        for description, label in (
                ("Scroll close", "Scroll::close()"),
                ("Scroll close adapter",
                 "@_Z21scroll_close_redirectP6ScrollPv@8"),
                ("Scroll thumb-rectangle reset", "Scroll::set_thumb_rect()"),
                ("Scroll thumb-rectangle adapter",
                 "@_Z30scroll_set_thumb_rect_redirectP6ScrollPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                scroll_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            if not returns_without_popping(body):
                fail(f"{description} unexpectedly pops stack arguments")

        redraw_dispatch = re.search(
            r"<\(anonymous namespace\)::redraw_from_vtable\(void\*, unsigned int\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        helper_has_dispatch = bool(redraw_dispatch and re.search(
            r"\bcall\s+\*", redraw_dispatch.group("body")))
        for description, label in (
                ("Scroll range setter", "Scroll::set_range(int, int)"),
                ("Scroll position setter", "Scroll::set_pos(int)")):
            body = scroll_bodies[label]
            has_inline_dispatch = bool(re.search(r"\bcall\s+\*", body))
            calls_helper = bool(re.search(
                r"\bcall\b.*<\(anonymous namespace\)::redraw_from_vtable", body))
            if not has_inline_dispatch and not (calls_helper and helper_has_dispatch):
                fail(f"{description} lacks raw thiscall indirect redraw dispatch")

        style_functions = (
            ("Scroll button-color setter", "Scroll::set_button_color(int)", None),
            ("Scroll bevel-thickness setter",
             "Scroll::set_bevel_thickness(int)", None),
            ("Scroll upper-bevel setter", "Scroll::set_bevel_upper(int)", None),
            ("Scroll lower-bevel setter", "Scroll::set_bevel_lower(int)", None),
            ("Scroll button-color adapter",
             "@_Z32scroll_set_button_color_redirectP6ScrollPvi@12",
             "Scroll::set_button_color(int)"),
            ("Scroll bevel-thickness adapter",
             "@_Z35scroll_set_bevel_thickness_redirectP6ScrollPvi@12",
             "Scroll::set_bevel_thickness(int)"),
            ("Scroll upper-bevel adapter",
             "@_Z31scroll_set_bevel_upper_redirectP6ScrollPvi@12",
             "Scroll::set_bevel_upper(int)"),
            ("Scroll lower-bevel adapter",
             "@_Z31scroll_set_bevel_lower_redirectP6ScrollPvi@12",
             "Scroll::set_bevel_lower(int)"),
        )
        for description, label, method_label in style_functions:
            body = scroll_bodies[label]
            inline_dispatches = len(re.findall(r"\bcall\s+\*", body))
            helper_dispatches = len(re.findall(
                r"\bcall\b.*<\(anonymous namespace\)::redraw_from_vtable", body))
            calls_method = bool(method_label and re.search(
                rf"\bcall\b.*<{re.escape(method_label)}>", body))
            if inline_dispatches != 2 and not (
                    helper_dispatches == 2 and helper_has_dispatch) \
                    and not calls_method:
                fail(f"{description} lacks two ordered indirect redraw calls")

        sprite_setters = (
            ("left", "Scroll::set_sprite_left(Sprite*, Sprite*, Sprite*)",
             "@_Z31scroll_set_sprite_left_redirectP6ScrollPvP6SpriteS3_S3_@20",
             (0xA7C, 0xA80, 0xA84, 0x4C8, 0x4C4,
              0x15BC, 0x15C0, 0x15C4)),
            ("right", "Scroll::set_sprite_right(Sprite*, Sprite*, Sprite*)",
             "@_Z32scroll_set_sprite_right_redirectP6ScrollPvP6SpriteS3_S3_@20",
             (0xA94, 0xA98, 0xA9C, 0x4C8, 0x4C4,
               0x2108, 0x210C, 0x2110)),
            ("upper", "Scroll::set_sprite_up(Sprite*, Sprite*, Sprite*)",
             "@_Z29scroll_set_sprite_up_redirectP6ScrollPvP6SpriteS3_S3_@20",
             (0xA88, 0xA8C, 0xA90, 0x4C8, 0x4C4,
              0x15BC, 0x15C0, 0x15C4)),
            ("lower", "Scroll::set_sprite_down(Sprite*, Sprite*, Sprite*)",
             "@_Z31scroll_set_sprite_down_redirectP6ScrollPvP6SpriteS3_S3_@20",
             (0xAA0, 0xAA4, 0xAA8, 0x4C8, 0x4C4,
              0x2108, 0x210C, 0x2110)),
        )
        for side, method_label, adapter_label, offsets in sprite_setters:
            for description, body in (
                    (f"Scroll {side}-sprite setter", scroll_bodies[method_label]),
                    (f"Scroll {side}-sprite adapter", scroll_bodies[adapter_label])):
                offset_matches = [
                    re.search(rf"\b0x{offset:x}\b", body) for offset in offsets]
                if all(offset_matches):
                    positions = [
                        match.start() for match in offset_matches if match]
                    if positions != sorted(positions):
                        fail(f"{description} does not preserve legacy access order")
                elif not re.search(
                        rf"\bcall\b.*<{re.escape(method_label)}>", body) \
                        and "adapter" in description:
                    fail(f"{description} lacks ordered sprite stores or method call")
                elif "setter" in description and not re.search(
                        r"\bcall\b.*<\(anonymous namespace\)::set_sprite_triplet",
                        body):
                    fail(f"{description} lacks ordered sprite stores or helper call")
        thumb_adapter = re.search(
            r"<@_Z34scroll_compute_thumb_rect_redirectP6ScrollPvP7tagRECT@12>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        if not thumb_adapter or not re.search(
                r"\blea\b.*0xa4c.*%eax", thumb_adapter.group("body")):
            fail("Scroll thumb-computation adapter does not return the internal RECT")
        thumb_method = re.search(
            r"<Scroll::compute_thumb_rect\(tagRECT\*\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        signed_divide = re.search(
            r"<\(anonymous namespace\)::signed_divide\(unsigned int, unsigned int\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        if (not thumb_method or not re.search(
                r"\bcall\b.*<\(anonymous namespace\)::signed_divide",
                thumb_method.group("body"))):
            fail("Scroll thumb computation does not call its signed divider")
        if not signed_divide or not re.search(
                r"\bidivl?\b", signed_divide.group("body")):
            fail("Scroll signed divider lacks an IDIV instruction")

        close_method = re.search(
            r"<Scroll::close\(\)>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        close_dispatches = re.findall(
            r"<\(anonymous namespace\)::call_noarg_virtual"
            r"\(void\*, unsigned int\)[^>]*>:"
            r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            scroll_disassembly, re.DOTALL)
        if not close_method:
            fail("could not locate Scroll close in disassembly")
        close_body = close_method.group("body")
        ordered_calls = re.search(
            r"0xaac.*?\bcall\b.*?"
            r"<\(anonymous namespace\)::call_noarg_virtual.*?"
            r"0x15f8.*?\bcall\b.*?"
            r"<\(anonymous namespace\)::call_noarg_virtual",
            close_body, re.DOTALL)
        slot_is_explicit = len(re.findall(r"\$0x168\b", close_body)) >= 2
        slot_is_specialized = any(
            re.search(r"\b0x168\b", body) for body in close_dispatches)
        if not ordered_calls or not (slot_is_explicit or slot_is_specialized):
            fail("Scroll close lacks two ordered virtual slot-0x168 calls")
        if not any(re.search(
                r"\b(?:call|jmp)\s+\*", body) for body in close_dispatches):
            fail("Scroll close helper lacks a raw thiscall indirect dispatch")
        if not re.search(
                r"\bDISP32\s+GraphicWin::close\(\)\s*$",
                scroll_relocations, re.MULTILINE):
            fail("Scroll close does not delegate to source-owned GraphicWin close")

    if args.scroll_oracle_object:
        oracle_headers = run([args.objdump, "-f", args.scroll_oracle_object])
        if "file format pe-i386" not in oracle_headers:
            fail("Scroll runtime oracle object is not a 32-bit PE COFF object")
        oracle_symbols = run(
            [args.nm, "--defined-only", args.scroll_oracle_object])
        if "__Z23run_scroll_oracle_suitev" not in oracle_symbols:
            fail("missing Scroll runtime oracle suite entry point")
        oracle_disassembly = run(
            [args.objdump, "-d", "-C", args.scroll_oracle_object])
        oracle_calls = {
            "(anonymous namespace)::verify_close()": (0x00605370,),
            "(anonymous namespace)::verify_init_wrappers()": (
                0x00605840, 0x00605890, 0x006058D0, 0x00605910, 0x00605960),
            "(anonymous namespace)::verify_range()": (0x006059B0,),
            "(anonymous namespace)::verify_styles()": (
                0x00605A10, 0x00605A50, 0x00605A90, 0x00605AD0),
            "(anonymous namespace)::verify_thumb_resetters()": (
                0x00605B80, 0x00606EA0),
            "(anonymous namespace)::verify_vertical_sprites()": (
                0x00605C80, 0x00605CD0),
            "(anonymous namespace)::verify_position()": (0x00605D20,),
        }
        for label, addresses in oracle_calls.items():
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                oracle_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Scroll runtime oracle helper {label}")
            body = match.group("body")
            for address in addresses:
                if f"0x{address:x}" not in body:
                    fail(
                        f"Scroll runtime oracle helper {label} lacks original "
                        f"address 0x{address:08X}")
            if not re.search(r"\bcall\b\s+\*%", body):
                fail(f"Scroll runtime oracle helper {label} lacks an original call")
            if not re.search(r"\b(?:lea|mov)\b.*%ecx", body):
                fail(f"Scroll runtime oracle helper {label} does not prepare ECX")

    if args.runtime_oracle_object:
        runtime_headers = run([args.objdump, "-f", args.runtime_oracle_object])
        if "file format pe-i386" not in runtime_headers:
            fail("runtime oracle object is not a 32-bit PE COFF object")
        runtime_symbols = run(
            [args.nm, "--defined-only", args.runtime_oracle_object])
        for description, symbol in (
                ("runtime oracle dispatcher", "__Z19run_runtime_oraclesv"),
                ("runtime oracle probe",
                 "@_ZN14runtime_oracle5probeEPvS0_@8"),
                ("runtime oracle fixture initializer",
                 "__ZN14runtime_oracle15initialize_pairEPhS0_RKNS_9ClassSpecEPj")):
            if symbol not in runtime_symbols:
                fail(f"missing required {description} symbol")

    if args.menu_object:
        menu_headers = run([args.objdump, "-f", args.menu_object])
        if "file format pe-i386" not in menu_headers:
            fail("Menu object is not a 32-bit PE COFF object")
        menu_symbols = run([args.nm, "--defined-only", args.menu_object])
        required_menu_symbols = {
            "Menu callback setter": "__ZN4Menu13set_menu_procEPU5cdeclFviE",
            "Menu ID lookup": "__ZN4Menu11id_to_indexEi",
            "Menu callback adapter":
                "@_Z27menu_set_menu_proc_redirectP4MenuPvPU5cdeclFviE@12",
            "Menu ID lookup adapter":
                "@_Z25menu_id_to_index_redirectP4MenuPvi@12",
        }
        for description, symbol in required_menu_symbols.items():
            if symbol not in menu_symbols:
                fail(f"missing required Menu symbol: {description}")
        menu_disassembly = run([args.objdump, "-d", "-C", args.menu_object])
        for description, label in (
                ("Menu callback setter",
                 "Menu::set_menu_proc(void ( cdecl*)(int))"),
                ("Menu ID lookup", "Menu::id_to_index(int)"),
                ("Menu callback adapter",
                 "@_Z27menu_set_menu_proc_redirectP4MenuPvPU5cdeclFviE@12"),
                ("Menu ID lookup adapter",
                 "@_Z25menu_id_to_index_redirectP4MenuPvi@12")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                menu_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            returns = re.findall(
                r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            if not returns or any(value != "4" for value in returns):
                fail(f"{description} does not pop its stack argument")

    if args.pulldown_object:
        pulldown_headers = run([args.objdump, "-f", args.pulldown_object])
        if "file format pe-i386" not in pulldown_headers:
            fail("PullDown object is not a 32-bit PE COFF object")
        pulldown_symbols = run(
            [args.nm, "--defined-only", args.pulldown_object])
        required_pulldown_symbols = {
            "PullDown hide": "__ZN8PullDown9hide_itemEi",
            "PullDown show": "__ZN8PullDown9show_itemEi",
            "PullDown disable": "__ZN8PullDown12disable_itemEi",
            "PullDown enable": "__ZN8PullDown11enable_itemEi",
            "PullDown check": "__ZN8PullDown10check_itemEi",
            "PullDown uncheck": "__ZN8PullDown12uncheck_itemEi",
            "PullDown selection": "__ZN8PullDown12get_selectedEv",
            "PullDown hide adapter":
                "@_Z28pull_down_hide_item_redirectP8PullDownPvi@12",
            "PullDown show adapter":
                "@_Z28pull_down_show_item_redirectP8PullDownPvi@12",
            "PullDown disable adapter":
                "@_Z31pull_down_disable_item_redirectP8PullDownPvi@12",
            "PullDown enable adapter":
                "@_Z30pull_down_enable_item_redirectP8PullDownPvi@12",
            "PullDown check adapter":
                "@_Z29pull_down_check_item_redirectP8PullDownPvi@12",
            "PullDown uncheck adapter":
                "@_Z31pull_down_uncheck_item_redirectP8PullDownPvi@12",
            "PullDown selection adapter":
                "@_Z31pull_down_get_selected_redirectP8PullDownPv@8",
        }
        for description, symbol in required_pulldown_symbols.items():
            if symbol not in pulldown_symbols:
                fail(f"missing required PullDown symbol: {description}")
        pulldown_disassembly = run(
            [args.objdump, "-d", "-C", args.pulldown_object])
        stack_argument_functions = (
            ("PullDown hide", "PullDown::hide_item(int)", None),
            ("PullDown show", "PullDown::show_item(int)", None),
            ("PullDown disable", "PullDown::disable_item(int)", None),
            ("PullDown enable", "PullDown::enable_item(int)", None),
            ("PullDown check", "PullDown::check_item(int)", None),
            ("PullDown uncheck", "PullDown::uncheck_item(int)", None),
            ("PullDown hide adapter",
             "@_Z28pull_down_hide_item_redirectP8PullDownPvi@12",
             "PullDown::hide_item(int)"),
            ("PullDown show adapter",
             "@_Z28pull_down_show_item_redirectP8PullDownPvi@12",
             "PullDown::show_item(int)"),
            ("PullDown disable adapter",
             "@_Z31pull_down_disable_item_redirectP8PullDownPvi@12",
             "PullDown::disable_item(int)"),
            ("PullDown enable adapter",
             "@_Z30pull_down_enable_item_redirectP8PullDownPvi@12",
             "PullDown::enable_item(int)"),
            ("PullDown check adapter",
             "@_Z29pull_down_check_item_redirectP8PullDownPvi@12",
             "PullDown::check_item(int)"),
            ("PullDown uncheck adapter",
             "@_Z31pull_down_uncheck_item_redirectP8PullDownPvi@12",
             "PullDown::uncheck_item(int)"),
        )
        for description, label, tail_target in stack_argument_functions:
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                pulldown_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            returns = re.findall(
                r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            tail_jump = tail_target and re.search(
                rf"\bjmp\s+[0-9a-f]+\s+<{re.escape(tail_target)}>",
                match.group("body"))
            if (not returns or any(value != "4" for value in returns)) \
                    and not tail_jump:
                fail(f"{description} does not pop its stack argument")
        for description, label in (
                ("PullDown selection", "PullDown::get_selected()"),
                ("PullDown selection adapter",
                 "@_Z31pull_down_get_selected_redirectP8PullDownPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                pulldown_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            if not re.search(r"\bret\b", match.group("body")) or re.search(
                    r"\bret\s+\$", match.group("body")):
                fail(f"{description} unexpectedly pops a stack argument")

    if args.basepop_font_object:
        basepop_headers = run([args.objdump, "-f", args.basepop_font_object])
        if "file format pe-i386" not in basepop_headers:
            fail("BasePop font object is not a 32-bit PE COFF object")
        basepop_symbols = run([args.nm, "--defined-only", args.basepop_font_object])
        required_basepop_symbols = {
            "BasePop string-font setter":
                "__ZN7BasePop15set_string_fontEP4FontS1_S1_S1_",
            "BasePop string-font adapter":
                "@_Z33base_pop_set_string_font_redirectP7BasePopPvP4FontS3_S3_S3_@24",
        }
        for description, symbol in required_basepop_symbols.items():
            if symbol not in basepop_symbols:
                fail(f"missing required BasePop symbol: {description}")
        basepop_disassembly = run(
            [args.objdump, "-d", "-C", args.basepop_font_object])
        for description, label in (
                ("BasePop string-font setter",
                 "BasePop::set_string_font(Font*, Font*, Font*, Font*)"),
                ("BasePop string-font adapter",
                 "@_Z33base_pop_set_string_font_redirectP7BasePopPvP4FontS3_S3_S3_@24")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                basepop_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            returns = re.findall(r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            if not returns or any(value != "10" for value in returns):
                fail(f"{description} does not pop all four stack arguments")

    if args.dialog_object:
        dialog_headers = run([args.objdump, "-f", args.dialog_object])
        if "file format pe-i386" not in dialog_headers:
            fail("Dialog object is not a 32-bit PE COFF object")
        dialog_symbols = run([args.nm, "--defined-only", args.dialog_object])
        required_dialog_symbols = {
            "Dialog ID lookup": "__ZN6Dialog9id_to_posEi",
            "Dialog ID lookup adapter":
                "@_Z25dialog_id_to_pos_redirectP6DialogPvi@12",
            "Dialog selected-ID setter": "__ZN6Dialog15set_selected_idEi",
            "Dialog selected-ID adapter":
                "@_Z31dialog_set_selected_id_redirectP6DialogPvi@12",
            "Dialog selected-ID getter": "__ZN6Dialog15get_selected_idEv",
            "Dialog selected-ID getter adapter":
                "@_Z31dialog_get_selected_id_redirectP6DialogPv@8",
            "Dialog position lookup": "__ZN6Dialog9pos_to_idEi",
            "Dialog position lookup adapter":
                "@_Z25dialog_pos_to_id_redirectP6DialogPvi@12",
        }
        for description, symbol in required_dialog_symbols.items():
            if symbol not in dialog_symbols:
                fail(f"missing required Dialog symbol: {description}")
        dialog_disassembly = run([args.objdump, "-d", "-C", args.dialog_object])
        for description, label in (
                ("Dialog ID lookup", "Dialog::id_to_pos(int)"),
                ("Dialog ID lookup adapter",
                 "@_Z25dialog_id_to_pos_redirectP6DialogPvi@12"),
                ("Dialog selected-ID setter", "Dialog::set_selected_id(int)"),
                ("Dialog selected-ID adapter",
                 "@_Z31dialog_set_selected_id_redirectP6DialogPvi@12"),
                ("Dialog position lookup", "Dialog::pos_to_id(int)"),
                ("Dialog position lookup adapter",
                 "@_Z25dialog_pos_to_id_redirectP6DialogPvi@12")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                dialog_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            returns = re.findall(r"\bret\s+\$0x([0-9a-f]+)\b", match.group("body"))
            if not returns or any(value != "4" for value in returns):
                fail(f"{description} does not pop its single stack argument")
            if description == "Dialog position lookup" and re.search(
                    r"\bcall\b", match.group("body")):
                fail("Dialog position lookup retains an external call")
        for description, label in (
                ("Dialog selected-ID getter", "Dialog::get_selected_id()"),
                ("Dialog selected-ID getter adapter",
                 "@_Z31dialog_get_selected_id_redirectP6DialogPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                dialog_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            if not returns_without_popping(body):
                fail(f"{description} does not use a plain no-argument return")
            if description == "Dialog selected-ID getter" and re.search(r"\bcall\b", body):
                fail("Dialog selected-ID getter retains an external call")

    if args.string_struct_object:
        headers = run([args.objdump, "-f", args.string_struct_object])
        if "file format pe-i386" not in headers:
            fail("StringStruct object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.string_struct_object])
        required_symbols = {
            "StringStruct current ID": "__ZN12StringStruct10current_idEv",
            "StringStruct current entry": "__ZN12StringStruct13current_entryEv",
            "StringStruct next entry": "__ZN12StringStruct10next_entryEv",
            "StringStruct ID seek": "__ZN12StringStruct7seek_idEi",
            "StringStruct ID seek bridge": "_string_struct_seek_id_source",
            "StringStruct current ID adapter":
                "@_Z33string_struct_current_id_redirectP12StringStructPv@8",
            "StringStruct current entry adapter":
                "@_Z36string_struct_current_entry_redirectP12StringStructPv@8",
            "StringStruct next entry adapter":
                "@_Z33string_struct_next_entry_redirectP12StringStructPv@8",
            "StringStruct ID seek adapter":
                "@_Z30string_struct_seek_id_redirectP12StringStructPvi@12",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required StringStruct symbol: {description}")
        disassembly = run([args.objdump, "-d", "-C", args.string_struct_object])
        for description, label in (
                ("StringStruct current ID", "StringStruct::current_id()"),
                ("StringStruct current entry", "StringStruct::current_entry()"),
                ("StringStruct next entry", "StringStruct::next_entry()"),
                ("StringStruct current ID adapter",
                 "@_Z33string_struct_current_id_redirectP12StringStructPv@8"),
                ("StringStruct current entry adapter",
                 "@_Z36string_struct_current_entry_redirectP12StringStructPv@8"),
                ("StringStruct next entry adapter",
                 "@_Z33string_struct_next_entry_redirectP12StringStructPv@8")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            if not returns_without_popping(body):
                fail(f"{description} does not use a plain no-argument return")
            if description in (
                    "StringStruct current ID", "StringStruct current entry",
                    "StringStruct next entry") and re.search(
                r"\bcall\b", body):
                fail(f"{description} unexpectedly contains a call")
        for description, label in (
                ("StringStruct ID seek", "StringStruct::seek_id(int)"),
                ("StringStruct ID seek adapter",
                 "@_Z30string_struct_seek_id_redirectP12StringStructPvi@12")):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate {description} in disassembly")
            body = match.group("body")
            returns = re.findall(r"\bret\s+\$0x([0-9a-f]+)\b", body)
            if description == "StringStruct ID seek":
                if not returns or any(value != "4" for value in returns):
                    fail("StringStruct ID seek does not pop its stack argument")
                if re.search(r"\bcall\b", body):
                    fail("StringStruct ID seek unexpectedly contains a call")
            elif (len(returns) != 1 or returns[0] != "4"
                  or len(re.findall(r"\bcall\b", body)) != 1
                  or not re.search(
                      r"\bpush\s+0x4\(%esp\)\s*\n[^\n]*"
                      r"\bpush\s+%ecx\s*\n[^\n]*"
                      r"\bcall\s+[0-9a-f]+ <string_struct_seek_id_source>\s*\n"
                      r"[^\n]*\badd\s+\$0x8,%esp\s*\n[^\n]*"
                      r"\bcmp\s+%eax,%eax\s*\n[^\n]*"
                      r"\bret\s+\$0x4\b", body)):
                fail("StringStruct ID seek adapter does not preserve flags and cleanup")

    if args.button_group_object:
        button_headers = run([args.objdump, "-f", args.button_group_object])
        if "file format pe-i386" not in button_headers:
            fail("ButtonGroup object is not a 32-bit PE COFF object")
        button_symbols = run([args.nm, "--defined-only", args.button_group_object])
        required_button_symbols = {
            "ButtonGroup constructor": "__ZN11ButtonGroupC1Ev",
            "ButtonGroup destructor": "__ZN11ButtonGroupD1Ev",
            "ButtonGroup close": "__ZN11ButtonGroup5closeEv",
            "ButtonGroup init": "__ZN11ButtonGroup4initEii",
            "ButtonGroup constructor adapter":
                "@_Z31button_group_construct_redirectP11ButtonGroupPv@8",
            "ButtonGroup close adapter":
                "@_Z27button_group_close_redirectP11ButtonGroupPv@8",
            "ButtonGroup init adapter":
                "@_Z26button_group_init_redirectP11ButtonGroupPvii@16",
        }
        for description, symbol in required_button_symbols.items():
            if symbol not in button_symbols:
                fail(f"missing required ButtonGroup symbol: {description}")

        button_disassembly = run(
            [args.objdump, "-d", "-C", args.button_group_object])

        def button_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                button_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate ButtonGroup function in disassembly: {label}")
            return match.group("body")

        init_body = button_body("ButtonGroup::init(int, int)")
        if not re.search(r"\bret\s+\$0x8\b", init_body):
            fail("ButtonGroup::init does not use two-argument thiscall cleanup")
        constructor_adapter = button_body(
            "@_Z31button_group_construct_redirectP11ButtonGroupPv@8")
        if not returns_without_popping(constructor_adapter):
            fail("ButtonGroup constructor adapter does not use plain fastcall return")
        close_adapter = button_body(
            "@_Z27button_group_close_redirectP11ButtonGroupPv@8")
        if not returns_without_popping(close_adapter):
            fail("ButtonGroup close adapter does not use plain fastcall return")
        init_adapter = button_body(
            "@_Z26button_group_init_redirectP11ButtonGroupPvii@16")
        if not re.search(r"\bret\s+\$0x8\b", init_adapter):
            fail("ButtonGroup init adapter does not pop its two stack arguments")

    if args.text_index_object:
        headers = run([args.objdump, "-f", args.text_index_object])
        if "file format pe-i386" not in headers:
            fail("TextIndex object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.text_index_object])
        required_symbols = {
            "TextIndex constructor": "__ZN9TextIndexC1Ev",
            "TextIndex destructor": "__ZN9TextIndexD1Ev",
            "TextIndex clear helper": "__Z23text_clear_index_sourceP9TextIndex",
            "TextIndex make wrapper": "__Z15text_make_indexPKc",
            "TextIndex search wrapper": "__Z17text_search_indexPKcS0_",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required TextIndex symbol: {description}")

        disassembly = run(
            [args.objdump, "-d", "-r", "-C", args.text_index_object])

        def text_index_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate TextIndex function in disassembly: {label}")
            return match.group("body")

        constructor = text_index_body("TextIndex::TextIndex()")
        if re.search(r"\bcall\b", constructor):
            fail("TextIndex constructor unexpectedly contains a call")
        for field in ("100", "108", "10c", "110", "114"):
            if not re.search(rf"\bmovl\s+\$0x0,0x{field}\(%ecx\)", constructor):
                fail(f"TextIndex constructor does not clear field 0x{field}")
        if not re.search(r"\bmovb\s+\$0x0,0x104\(%ecx\)", constructor):
            fail("TextIndex constructor does not clear the Heap error byte")
        if not re.search(r"\bmovb\s+\$0x0,\(%ecx\)", constructor):
            fail("TextIndex constructor does not terminate the filename")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("TextIndex constructor does not return this in EAX")
        if not returns_without_popping(constructor):
            fail("TextIndex constructor does not use a plain thiscall return")

        destructor = text_index_body("TextIndex::~TextIndex()")
        if not re.search(r"\bmovl\s+\$0x0,0x100\(%e[a-z][a-z]\)", destructor):
            fail("TextIndex destructor does not clear the section count")
        if not re.search(r"\bmovb\s+\$0x0,\(%e[a-z][a-z]\)", destructor):
            fail("TextIndex destructor does not terminate the filename")
        if len(re.findall(r"DISP32\s+Heap::shutdown\(\)", destructor)) != 1:
            fail("TextIndex destructor does not invoke one Heap shutdown")
        if not returns_without_popping(destructor):
            fail("TextIndex destructor does not use a plain thiscall return")

        clear_helper = text_index_body("text_clear_index_source(TextIndex*)")
        if len(re.findall(r"DISP32\s+Heap::shutdown\(\)", clear_helper)) != 1:
            fail("TextIndex clear helper does not contain one Heap shutdown call site")
        if not returns_without_popping(clear_helper):
            fail("TextIndex clear helper does not use a plain cdecl return")

        make_wrapper = text_index_body("text_make_index(char const*)")
        if "TxtIndex" not in make_wrapper or "TextIndex::make_index(char const*)" not in make_wrapper:
            fail("TextIndex make wrapper does not use the global array and member method")
        if not returns_without_popping(make_wrapper):
            fail("TextIndex make wrapper does not use a plain cdecl return")

        search_wrapper = text_index_body(
            "text_search_index(char const*, char const*)")
        if "TxtIndex" not in search_wrapper or "TextIndex::search_index(char const*, char const*)" not in search_wrapper:
            fail("TextIndex search wrapper does not use the global array and member method")
        if not returns_without_popping(search_wrapper):
            fail("TextIndex search wrapper does not use a plain cdecl return")

    if args.spot_object:
        headers = run([args.objdump, "-f", args.spot_object])
        if "file format pe-i386" not in headers:
            fail("Spot object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.spot_object])
        required_symbols = {
            "Spot constructor": "__ZN4SpotC1Ev",
            "Spot destructor": "__ZN4SpotD1Ev",
            "Spot clear": "__ZN4Spot5clearEv",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Spot symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.spot_object])

        def spot_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Spot function in disassembly: {label}")
            return match.group("body")

        for description, label in (
                ("constructor", "Spot::Spot()"),
                ("clear", "Spot::clear()")):
            body = spot_body(label)
            for field in ("", "0x4", "0x8"):
                target = rf"{field}\(%ecx\)" if field else r"\(%ecx\)"
                if not re.search(rf"\bmovl\s+\$0x0,{target}", body):
                    fail(f"Spot {description} does not clear field {field or '0x0'}")
            if description == "constructor" and not re.search(
                    r"\bmov\s+%e(?:cx|bx|si|di),%eax", body):
                fail("Spot constructor does not return this in EAX")
            if not returns_without_popping(body):
                fail(f"Spot {description} does not use a plain thiscall return")

        destructor = spot_body("Spot::~Spot()")
        clears_fields = all(re.search(pattern, destructor) for pattern in (
            r"\bmovl\s+\$0x0,\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x4\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x8\(%e?\w+\)",
        ))
        calls_shutdown = "Spot::shutdown()" in destructor
        if not clears_fields and not calls_shutdown:
            fail("Spot destructor neither clears all fields nor calls Spot::shutdown")
        if not returns_without_popping(destructor):
            fail("Spot destructor does not use a plain thiscall return")

    if args.font_object:
        headers = run([args.objdump, "-f", args.font_object])
        if "file format pe-i386" not in headers:
            fail("Font object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.font_object])
        required_symbols = {
            "Font default constructor": "__ZN4FontC1Ev",
            "Font initializing constructor": "__ZN4FontC1EPcii",
            "Font destructor": "__ZN4FontD1Ev",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Font symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.font_object])

        def font_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Font function in disassembly: {label}")
            return match.group("body")

        constructor = font_body("Font::Font()")
        if not re.search(r"\bmovl\s+\$0xffffffff,\(%ecx\)", constructor):
            fail("Font default constructor does not initialize field 0x0 to -1")
        for field in ("4", "8", "c", "10", "18", "1c", "24"):
            if not re.search(rf"\bmovl\s+\$0x0,0x{field}\(%ecx\)", constructor):
                fail(f"Font default constructor does not clear field 0x{field}")
        for preserved in ("14", "20"):
            if re.search(rf"\bmov[^\n]*0x{preserved}\(%ecx\)", constructor):
                fail(f"Font default constructor overwrites preserved field 0x{preserved}")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Font default constructor does not return this in EAX")
        if not returns_without_popping(constructor):
            fail("Font default constructor does not use a plain thiscall return")

        initializing_constructor = font_body("Font::Font(char*, int, int)")
        if not re.search(r"\bret\s+\$0xc\b", initializing_constructor):
            fail("Font initializing constructor does not pop its three arguments")
        if not re.search(
                r"\bmov\s+%e(?:cx|bx|si|di),%eax", initializing_constructor):
            fail("Font initializing constructor does not return this in EAX")

        destructor = font_body("Font::~Font()")
        calls_close = "Font::close()" in destructor
        clears_fields = all(re.search(pattern, destructor) for pattern in (
            r"\bmovl\s+\$0xffffffff,\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x8\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0xc\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x10\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x18\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x1c\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x24\(%e?\w+\)",
        ))
        if not calls_close and not clears_fields:
            fail("Font destructor neither performs full cleanup nor calls Font::close")
        if not returns_without_popping(destructor):
            fail("Font destructor does not use a plain thiscall return")

    if args.time_object:
        headers = run([args.objdump, "-f", args.time_object])
        if "file format pe-i386" not in headers:
            fail("Time object is not a 32-bit PE COFF object")
        section_headers = run([args.objdump, "-h", args.time_object])
        if any(symbol in section_headers for symbol in (
                "COMDAT __ZN4Time10init_classEv",
                "COMDAT __ZN4Time11close_classEv")):
            fail("Time class lifecycle wrappers remain inline COMDAT implementations")
        symbols = run([args.nm, "--defined-only", args.time_object])
        required_symbols = {
            "Time constructor": "__ZN4TimeC1Ev",
            "Time destructor": "__ZN4TimeD1Ev",
            "Time set-modal method": "__ZN4Time9set_modalEv",
            "Time release-modal method": "__ZN4Time13release_modalEv",
            "Time class initializer": "__ZN4Time10init_classEv",
            "Time class cleanup": "__ZN4Time11close_classEv",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Time symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.time_object])

        def time_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Time function in disassembly: {label}")
            return match.group("body")

        constructor = time_body("Time::Time()")
        for field in ("", "0x4", "0x8", "0xc", "0x10", "0x14", "0x18", "0x1c", "0x24"):
            target = rf"{field}\(%ecx\)" if field else r"\(%ecx\)"
            if not re.search(rf"\bmovl\s+\$0x0,{target}", constructor):
                fail(f"Time constructor does not clear field {field or '0x0'}")
        if not re.search(r"\bmovl\s+\$0x5,0x20\(%ecx\)", constructor):
            fail("Time constructor does not initialize resolution to five")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Time constructor does not return this in EAX")

        destructor = time_body("Time::~Time()")
        calls_close = "Time::close()" in destructor
        clears_fields = all(re.search(pattern, destructor) for pattern in (
            r"\bmovl\s+\$0x0,\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x4\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x8\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0xc\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x10\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x14\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x18\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x1c\(%e?\w+\)",
            r"\bmovl\s+\$0x5,0x20\(%e?\w+\)",
            r"\bmovl\s+\$0x0,0x24\(%e?\w+\)",
        ))
        if not calls_close and not clears_fields:
            fail("Time destructor neither performs full cleanup nor calls Time::close")

        set_modal = time_body("Time::set_modal()")
        if "Time::TimeModal" not in set_modal or not re.search(
                r"\bmov\s+%ecx,", set_modal):
            fail("Time set-modal method does not publish this")
        release_modal = time_body("Time::release_modal()")
        if "Time::TimeModal" not in release_modal or not re.search(
                r"\bmovl\s+\$0x0,", release_modal):
            fail("Time release-modal method does not clear modal state")

        initializer = time_body("Time::init_class()")
        if "Time::TimeInitCount" not in initializer or not re.search(
                r"\b(?:inc|add)\w*\b", initializer):
            fail("Time class initializer does not increment the global count")
        if not re.search(r"\b(?:xor\s+%eax,%eax|mov\s+\$0x0,%eax)", initializer):
            fail("Time class initializer does not return zero")
        if not returns_without_popping(initializer):
            fail("Time class initializer does not use a plain cdecl return")

        cleanup = time_body("Time::close_class()")
        if "Time::TimeInitCount" not in cleanup or not re.search(
                r"\b(?:dec|sub)\w*\b", cleanup):
            fail("Time class cleanup does not decrement the global count")
        if not returns_without_popping(cleanup):
            fail("Time class cleanup does not use a plain cdecl return")

    if args.filemap_object:
        headers = run([args.objdump, "-f", args.filemap_object])
        if "file format pe-i386" not in headers:
            fail("Filemap object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.filemap_object])
        required_symbols = {
            "Filemap constructor": "__ZN7FilemapC1Ev",
            "Filemap destructor": "__ZN7FilemapD1Ev",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Filemap symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.filemap_object])

        def filemap_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Filemap function in disassembly: {label}")
            return match.group("body")

        constructor = filemap_body("Filemap::Filemap()")
        for pattern in (
                r"\bmovl\s+\$0x0,\(%ecx\)",
                r"\bmovl\s+\$0xffffffff,0x4\(%ecx\)",
                r"\bmovl\s+\$0x0,0x8\(%ecx\)"):
            if not re.search(pattern, constructor):
                fail("Filemap constructor does not initialize its three legacy fields")
        if re.search(r"\bmov[^\n]*0xc\(%ecx\)", constructor):
            fail("Filemap constructor overwrites preserved file size")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Filemap constructor does not return this in EAX")

        destructor = filemap_body("Filemap::~Filemap()")
        if re.search(r"\bmov[^\n]*0xc\(%e?\w+\)", destructor):
            fail("Filemap destructor overwrites preserved file size")
        if "UnmapViewOfFile" not in destructor:
            fail("Filemap destructor does not contain mapped-view cleanup")
        if len(re.findall(r"CloseHandle", destructor)) != 2:
            fail("Filemap destructor does not contain both handle cleanup paths")
        if not re.search(r"\bmovl\s+\$0x0,0x4\(%e?\w+\)", destructor):
            fail("Filemap destructor does not clear a closed file handle")

    if args.heap_object:
        headers = run([args.objdump, "-f", args.heap_object])
        if "file format pe-i386" not in headers:
            fail("Heap object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.heap_object])
        required_symbols = {
            "Heap constructor": "__ZN4HeapC1Ev",
            "Heap destructor": "__ZN4HeapD1Ev",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Heap symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.heap_object])

        def heap_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Heap function in disassembly: {label}")
            return match.group("body")

        constructor = heap_body("Heap::Heap()")
        if not re.search(r"\bmovb\s+\$0x0,\(%ecx\)", constructor):
            fail("Heap constructor does not clear only the byte-sized error flags")
        for field in ("4", "8", "c", "10"):
            if not re.search(rf"\bmovl\s+\$0x0,0x{field}\(%ecx\)", constructor):
                fail(f"Heap constructor does not clear field 0x{field}")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Heap constructor does not return this in EAX")
        if (re.search(r"\bmov(?:w|l)\s+[^,\n]+,\(%e(?:ax|bx|cx|dx|si|di)\)",
                      constructor)
                or re.search(
                    r"\bmov\w*\s+[^,\n]+,0x[123]\(%e(?:ax|bx|cx|dx|si|di)\)",
                    constructor)):
            fail("Heap constructor overwrites legacy padding")

        destructor = heap_body("Heap::~Heap()")
        if "Heap::shutdown()" in destructor:
            fail("Heap destructor delegates instead of performing legacy direct cleanup")
        if not re.search(r"\bfree\b", destructor):
            fail("Heap destructor does not contain direct allocation cleanup")
        if not re.search(r"\bmovb\s+\$0x0,\(%e?\w+\)", destructor):
            fail("Heap destructor does not clear only the byte-sized error flags")
        for field in ("4", "8", "c", "10"):
            if not re.search(rf"\bmovl\s+\$0x0,0x{field}\(%e?\w+\)", destructor):
                fail(f"Heap destructor does not clear field 0x{field}")
        if (re.search(r"\bmov(?:w|l)\s+[^,\n]+,\(%e(?:ax|bx|cx|dx|si|di)\)",
                      destructor)
                or re.search(
                    r"\bmov\w*\s+[^,\n]+,0x[123]\(%e(?:ax|bx|cx|dx|si|di)\)",
                    destructor)):
            fail("Heap destructor overwrites legacy padding")
        if not returns_without_popping(destructor):
            fail("Heap destructor does not use a plain thiscall return")

    if args.strings_object:
        headers = run([args.objdump, "-f", args.strings_object])
        if "file format pe-i386" not in headers:
            fail("Strings object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.strings_object])
        required_symbols = {
            "Strings constructor": "__ZN7StringsC1Ev",
            "Strings destructor": "__ZN7StringsD1Ev",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Strings symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.strings_object])

        def strings_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Strings function in disassembly: {label}")
            return match.group("body")

        constructor = strings_body("Strings::Strings()")
        if not re.search(r"\bmovb\s+\$0x0,\(%ecx\)", constructor):
            fail("Strings constructor does not preserve Heap padding")
        for field in ("4", "8", "c", "10", "14"):
            if not re.search(rf"\bmovl\s+\$0x0,0x{field}\(%ecx\)", constructor):
                fail(f"Strings constructor does not clear field 0x{field}")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Strings constructor does not return this in EAX")
        if (re.search(r"\bmov(?:w|l)\s+[^,\n]+,\(%e(?:ax|bx|cx|dx|si|di)\)",
                      constructor)
                or re.search(
                    r"\bmov\w*\s+[^,\n]+,0x[123]\(%e(?:ax|bx|cx|dx|si|di)\)",
                    constructor)):
            fail("Strings constructor overwrites Heap padding")
        if not returns_without_popping(constructor):
            fail("Strings constructor does not use a plain thiscall return")

        destructor = strings_body("Strings::~Strings()")
        if len(re.findall(r"DISP32\s+Heap::shutdown\(\)", destructor)) != 1:
            fail("Strings destructor does not invoke one Heap shutdown")
        if re.search(r"\bmov[^\n]*0x14\(%e?\w+\)", destructor):
            fail("Strings destructor overwrites preserved populated state")
        if not returns_without_popping(destructor):
            fail("Strings destructor does not use a plain thiscall return")

    if args.log_object:
        headers = run([args.objdump, "-f", args.log_object])
        if "file format pe-i386" not in headers:
            fail("Log object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.log_object])
        required_symbols = {
            "Log default constructor": "__ZN3LogC1Ev",
            "Log filename constructor": "__ZN3LogC1EPKc",
            "Log destructor": "__ZN3LogD1Ev",
            "Log init method": "__ZN3Log4initEPKc",
            "Log initializer": "__Z11log_loggingv",
            "Log exit cleanup": "__Z16log_logging_exitv",
            "Log reset wrapper": "__Z9log_resetv",
            "Log decimal two-string wrapper": "__Z7log_sayPKcS0_iii",
            "Log decimal one-string wrapper": "__Z7log_sayPKciii",
            "Log hexadecimal two-string wrapper": "__Z11log_say_hexPKcS0_iii",
            "Log hexadecimal one-string wrapper": "__Z11log_say_hexPKciii",
            "Log state wrapper": "__Z13log_set_statei",
            "Log global object": "_Logging",
            "Log global disable flag": "_IsLoggingDisabled",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Log symbol: {description}")

        raw_disassembly = run([args.objdump, "-d", "-r", args.log_object])
        disassembly = run([args.objdump, "-d", "-r", "-C", args.log_object])

        def log_exact_body(symbol):
            match = re.search(
                rf"<{re.escape(symbol)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                raw_disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate exact Log symbol in disassembly: {symbol}")
            return match.group("body")

        def log_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Log function in disassembly: {label}")
            return match.group("body")

        def uses_log_data_offset(body, offset):
            return re.search(
                rf"\bmov\s+0x{offset:x},%e(?:ax|cx|dx|bx|si|di)\s*\n"
                rf"\s*[0-9a-f]+:\s+dir32\s+\.data\b", body)

        constructor = log_exact_body("__ZN3LogC1Ev")
        for pattern in (
                r"\bmovl\s+\$0x0,\(%ecx\)",
                r"\bmovl\s+\$0x0,0x4\(%ecx\)"):
            if not re.search(pattern, constructor):
                fail("Log default constructor does not clear both fields")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Log default constructor does not return this in EAX")
        if not returns_without_popping(constructor):
            fail("Log default constructor does not use a plain thiscall return")

        filename_constructor = log_exact_body("__ZN3LogC1EPKc")
        if not re.search(r"\bmovl\s+\$0x0,\(%e(?:cx|bx|si|di)\)",
                         filename_constructor):
            fail("Log filename constructor does not clear the filename field")
        if re.search(r"\bmov[^\n]*,0x4\(%e(?:cx|bx|si|di)\)",
                     filename_constructor):
            fail("Log filename constructor overwrites preserved disabled state")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", filename_constructor):
            fail("Log filename constructor does not return this in EAX")
        if not re.search(r"\bret\s+\$0x4\b", filename_constructor):
            fail("Log filename constructor does not pop its stack argument")

        destructor = log_exact_body("__ZN3LogD1Ev")
        if "free" not in destructor or not re.search(
                r"\bmovl\s+\$0x0,\(%e(?:cx|bx|si|di)\)", destructor):
            fail("Log destructor does not free and clear the filename")
        if re.search(r"\bmov[^\n]*,0x4\(%e(?:cx|bx|si|di)\)", destructor):
            fail("Log destructor overwrites preserved disabled state")
        if not returns_without_popping(destructor):
            fail("Log destructor does not use a plain thiscall return")

        init_method = log_exact_body("__ZN3Log4initEPKc")
        init_returns = re.findall(
            r"\bret(?:\s+\$0x([0-9a-f]+))?\b", init_method)
        if not init_returns or any(value != "4" for value in init_returns):
            fail("Log::init does not pop its one stack argument on every return path")

        initializer = log_body("log_logging()")
        if "operator new" in initializer:
            fail("Log initializer retains the non-legacy allocation leak")
        if "atexit" not in initializer:
            fail("Log initializer does not register exit cleanup")
        calls_constructor = "Log::Log(char const*)" in initializer
        inlines_constructor = "mem_get" in initializer and "env_open" in initializer
        if not calls_constructor and not inlines_constructor:
            fail("Log initializer neither constructs nor initializes the global log")

        exit_cleanup = log_body("log_logging_exit()")
        calls_destructor = "Log::~Log()" in exit_cleanup
        inlines_destructor = "free" in exit_cleanup and re.search(
            r"\bmovl\s+\$0x0,\(%e(?:ax|cx|bx|si|di)\)", exit_cleanup)
        if not calls_destructor and not inlines_destructor:
            fail("Log exit cleanup neither destroys nor clears the global log")

        reset_wrapper = log_body("log_reset()")
        if "Log::reset()" not in reset_wrapper and "env_open" not in reset_wrapper:
            fail("Log reset wrapper neither delegates nor resets the global log")

        for description, label, member in (
                ("decimal two-string", "log_say(char const*, char const*, int, int, int)", "Log::say("),
                ("decimal one-string", "log_say(char const*, int, int, int)", "Log::say("),
                ("hexadecimal two-string", "log_say_hex(char const*, char const*, int, int, int)", "Log::say_hex("),
                ("hexadecimal one-string", "log_say_hex(char const*, int, int, int)", "Log::say_hex(")):
            wrapper = log_body(label)
            if not uses_log_data_offset(wrapper, 4):
                fail(f"Log {description} wrapper does not use the global object")
            delegates = member in wrapper
            inlines_output = "env_open" in wrapper
            if not delegates and not inlines_output:
                fail(f"Log {description} wrapper neither delegates nor writes output")
            if inlines_output and (not uses_log_data_offset(wrapper, 0)
                                   or len(re.findall(
                                       r"\bdir32\s+\.data\b", wrapper)) < 2
                                   or not re.search(
                                       r"\bmov\s+0x4\(%e(?:ax|cx|dx|bx|si|di)\),"
                                       r"%e(?:ax|cx|dx|bx|si|di)", wrapper)):
                fail(f"Log {description} inlined wrapper omits a disable guard")
            if not returns_without_popping(wrapper):
                fail(f"Log {description} wrapper does not use a plain cdecl return")

        for description, label in (
                ("decimal", "Log::say(char const*, char const*, int, int, int)"),
                ("hexadecimal", "Log::say_hex(char const*, char const*, int, int, int)")):
            member_body = log_body(label)
            if not uses_log_data_offset(member_body, 0):
                fail(f"Log {description} member omits the global disable guard")

        state_wrapper = log_body("log_set_state(int)")
        if not re.search(r"\bsete\b", state_wrapper) or not re.search(
                r"\bmov[^\n]*,0x4\(%e(?:ax|cx|bx|si|di)\)", state_wrapper):
            fail("Log state wrapper does not invert and store the requested state")

        for description, body in (
                ("Log initializer", initializer),
                ("Log exit cleanup", exit_cleanup),
                ("Log reset wrapper", reset_wrapper),
                ("Log state wrapper", state_wrapper)):
            if not returns_without_popping(body):
                fail(f"{description} does not use a plain cdecl return")

    if args.random_object:
        headers = run([args.objdump, "-f", args.random_object])
        if "file format pe-i386" not in headers:
            fail("Random object is not a 32-bit PE COFF object")
        symbols = run([args.nm, "--defined-only", args.random_object])
        required_symbols = {
            "Random constructor": "__ZN6RandomC1Ev",
            "Random destructor": "__ZN6RandomD1Ev",
            "Random integer generator": "__ZN6Random3getEjj",
            "Random floating generator": "__ZN6Random3getEv",
            "random initializer": "__Z11random_randv",
            "random exit cleanup": "__Z16random_rand_exitv",
            "random reseed wrapper": "__Z13random_reseedj",
            "random_get wrapper": "__Z10random_getv",
            "random integer wrapper": "__Z6randomjj",
            "random floating wrapper": "__Z6randomv",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required Random symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", "-C", args.random_object])

        def random_body(label):
            match = re.search(
                rf"<{re.escape(label)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate Random function in disassembly: {label}")
            return match.group("body")

        constructor = random_body("Random::Random()")
        if not re.search(r"\bmovl\s+\$0x0,\(%ecx\)", constructor):
            fail("Random constructor does not clear the seed")
        if not re.search(r"\bmov\s+%e(?:cx|bx|si|di),%eax", constructor):
            fail("Random constructor does not return this in EAX")

        destructor = random_body("Random::~Random()")
        if not re.search(r"\bmovl\s+\$0x0,\(%ecx\)", destructor):
            fail("Random destructor does not clear the seed")
        if not returns_without_popping(destructor):
            fail("Random destructor does not use a plain thiscall return")

        integer_generator = random_body("Random::get(unsigned int, unsigned int)")
        if not re.search(r"\b(?:jg|jle|cmovg|cmovle)\b", integer_generator):
            fail("Random integer generator does not order bounds as signed integers")
        for constant in ("0x19660d", "0x3c6ef35f"):
            if constant not in integer_generator:
                fail("Random integer generator does not use the legacy LCG")
        if not re.search(r"\bret\s+\$0x8\b", integer_generator):
            fail("Random integer generator does not pop both thiscall arguments")

        floating_generator = random_body("Random::get()")
        subtract = re.search(r"\bfsub[^\n]*", floating_generator)
        if not subtract or not re.search(r"\bflds\b", floating_generator[:subtract.start()]):
            fail("Random floating generator does not load the synthesized single")
        if re.search(r"\bfldl\b", floating_generator[:subtract.start()]):
            fail("Random floating generator reads beyond its four-byte temporary")
        for constant in ("0x19660d", "0x3c6ef35f", "0x7fffff", "0x3f800000"):
            if constant not in floating_generator:
                fail("Random floating generator does not use the legacy bit construction")

        initializer = random_body("random_rand()")
        if "operator new" in initializer:
            fail("random initializer retains the non-legacy allocation leak")
        if "atexit" not in initializer:
            fail("random initializer does not register exit cleanup")
        clears_seed = re.search(r"\bmovl\s+\$0x0,\(%e?\w+\)", initializer)
        calls_reseed = "Random::reseed(unsigned int)" in initializer
        if not clears_seed and not calls_reseed:
            fail("random initializer does not reset the global seed")

        exit_cleanup = random_body("random_rand_exit()")
        clears_seed = re.search(r"\bmovl\s+\$0x0,\(%e?\w+\)", exit_cleanup)
        calls_destructor = "Random::~Random()" in exit_cleanup
        if not clears_seed and not calls_destructor:
            fail("random exit cleanup does not clear the global seed")

        reseed_wrapper = random_body("random_reseed(unsigned int)")
        writes_seed = re.search(r"\bmov[^\n]*\(%e?\w+\)", reseed_wrapper)
        calls_reseed = "Random::reseed(unsigned int)" in reseed_wrapper
        if not writes_seed and not calls_reseed:
            fail("random reseed wrapper does not update the global seed")
        if not returns_without_popping(reseed_wrapper):
            fail("random reseed wrapper does not use a plain cdecl return")

        get_wrapper = random_body("random_get()")
        if re.search(r"\bcall\b", get_wrapper):
            fail("random_get wrapper unexpectedly calls another function")
        if len(re.findall(r"\bmov", get_wrapper)) < 2:
            fail("random_get wrapper does not read the global generator seed")

        integer_wrapper = random_body("random(unsigned int, unsigned int)")
        delegates_integer = "Random::get(unsigned int, unsigned int)" in integer_wrapper
        if not delegates_integer and not re.search(
                r"\b(?:jg|jle|cmovg|cmovle)\b", integer_wrapper):
            fail("random integer wrapper neither delegates nor orders signed bounds")
        if not returns_without_popping(integer_wrapper):
            fail("random integer wrapper does not use a plain cdecl return")

        floating_wrapper = random_body("random()")
        delegates_floating = "Random::get()" in floating_wrapper
        if not delegates_floating:
            subtract = re.search(r"\bfsub[^\n]*", floating_wrapper)
            if not subtract or not re.search(r"\bflds\b", floating_wrapper[:subtract.start()]):
                fail("random floating wrapper neither delegates nor synthesizes a single")
            if re.search(r"\bfldl\b", floating_wrapper[:subtract.start()]):
                fail("random floating wrapper reads beyond its four-byte temporary")
        if not returns_without_popping(floating_wrapper):
            fail("random floating wrapper does not use a plain cdecl return")

    if args.text_object:
        headers = run([args.objdump, "-f", args.text_object])
        if "file format pe-i386" not in headers:
            fail("Text object is not a 32-bit PE COFF object")
        section_headers = run([args.objdump, "-h", args.text_object])
        if any(symbol in section_headers for symbol in (
                "COMDAT __ZN4TextC1Ev",
                "COMDAT __ZN4TextC1Ej",
                "COMDAT __ZN4TextD1Ev")):
            fail("Text lifecycle functions remain inline COMDAT implementations")
        symbols = run([args.nm, "--defined-only", args.text_object])
        required_symbols = {
            "Text default constructor": "__ZN4TextC1Ev",
            "Text sized constructor": "__ZN4TextC1Ej",
            "Text destructor": "__ZN4TextD1Ev",
            "Text global initializer": "__Z8text_txtv",
            "Text global exit cleanup": "__Z13text_txt_exitv",
            "text_open wrapper": "__Z9text_openPKcS0_",
        }
        for description, symbol in required_symbols.items():
            if symbol not in symbols:
                fail(f"missing required symbol: {description}")

        disassembly = run([args.objdump, "-d", "-r", args.text_object])

        def text_body(symbol):
            match = re.search(
                rf"<{re.escape(symbol)}>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
                disassembly, re.DOTALL)
            if not match:
                fail(f"could not locate exact Text function in disassembly: {symbol}")
            return match.group("body")

        default_constructor = text_body("__ZN4TextC1Ev")
        for field in ("150", "154", "158", "15c"):
            if not re.search(
                    rf"\bmovl\s+\$0x0,0x{field}\(%e(?:ax|cx|bx|si|di)\)",
                    default_constructor):
                fail(f"Text default constructor does not clear field 0x{field}")
        if not re.search(r"\bmovb\s+\$0x0,\(%e(?:ax|cx|bx|si|di)\)",
                         default_constructor):
            fail("Text default constructor does not terminate the filename")
        if re.search(r"\bmov[^\n]*,0x50\(%e(?:ax|cx|bx|si|di)\)",
                     default_constructor):
            fail("Text default constructor overwrites the preserved path")
        if not re.search(r"\bmov\s+%e(?:ax|cx|bx|si|di),%eax",
                         default_constructor):
            fail("Text default constructor does not return this in EAX")
        if not returns_without_popping(default_constructor):
            fail("Text default constructor does not use a plain thiscall return")

        sized_constructor = text_body("__ZN4TextC1Ej")
        for field in ("150", "154", "158", "15c"):
            if not re.search(
                    rf"\bmovl\s+\$0x0,0x{field}\(%e(?:ax|cx|bx|si|di)\)",
                    sized_constructor):
                fail(f"Text sized constructor does not clear field 0x{field}")
        if not re.search(r"\bmovb\s+\$0x0,\(%e(?:ax|cx|bx|si|di)\)",
                         sized_constructor):
            fail("Text sized constructor does not terminate the filename")
        if re.search(r"\bmov[^\n]*,0x50\(%e(?:ax|cx|bx|si|di)\)",
                     sized_constructor):
            fail("Text sized constructor overwrites the preserved path")
        if len(re.findall(r"DISP32\s+__Z7mem_getj\b", sized_constructor)) != 2:
            fail("Text sized constructor does not contain two allocation call sites")
        if not re.search(r"\bmov\s+%e(?:ax|cx|bx|si|di),%eax",
                         sized_constructor):
            fail("Text sized constructor does not return this in EAX")
        if not re.search(r"\bret\s+\$0x4\b", sized_constructor):
            fail("Text sized constructor does not pop its stack argument")

        initializer = text_body("__Z8text_txtv")
        if "operator new" in initializer or "__Znwj" in initializer:
            fail("Text global initializer retains the temporary allocation leak")
        calls_constructor = "__ZN4TextC1Ej" in initializer
        inlines_constructor = (
            len(re.findall(r"DISP32\s+__Z7mem_getj\b", initializer)) == 2
            and all(f"0x{field}" in initializer
                    for field in ("150", "154", "158", "15c")))
        if "_Txt" not in initializer or not (
                calls_constructor or inlines_constructor):
            fail("Text global initializer does not construct the process-owned object")
        if "atexit" not in initializer:
            fail("Text global initializer does not register exit cleanup")
        if not returns_without_popping(initializer):
            fail("Text global initializer does not use a plain cdecl return")

        exit_cleanup = text_body("__Z13text_txt_exitv")
        if "_Txt" not in exit_cleanup or "__ZN4TextD1Ev" not in exit_cleanup:
            fail("Text global exit cleanup does not destroy the process-owned object")
        plain_return = re.search(r"\bret\b", exit_cleanup) and not re.search(
            r"\bret\s+\$", exit_cleanup)
        tail_jump = re.search(r"\bjmp[^\n]*__ZN4TextD1Ev", exit_cleanup)
        if not plain_return and not tail_jump:
            fail("Text global exit cleanup does not use a plain cdecl return")

        match = re.search(
            r"<__ZN4TextD1Ev>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            disassembly, re.DOTALL)
        if not match:
            fail("could not locate exact Text destructor in disassembly")
        destructor = match.group("body")
        if not re.match(
                r"\s*[0-9a-f]+:\s+e9\s+00\s+00\s+00\s+00\s+"
                r"jmp[^\n]*\n\s*[0-9a-f]+:\s+DISP32\s+"
                r"__ZN4Text8shutdownEv\b", destructor):
            fail("Text destructor is not a direct tail jump to shutdown")

        match = re.search(
            r"<__Z9text_openPKcS0_>:(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
            disassembly, re.DOTALL)
        if not match:
            fail("could not locate text_open wrapper in disassembly")
        text_open = match.group("body")
        if "_Txt" not in text_open or "__ZN4Text4openEPKcS1_" not in text_open:
            fail("text_open wrapper does not delegate through the global Text instance")
        if not returns_without_popping(text_open):
            fail("text_open wrapper does not use a plain cdecl return")

    if not args.scenario_object:
        return
    scenario_headers = run([args.objdump, "-f", args.scenario_object])
    if "file format pe-i386" not in scenario_headers:
        fail("scenario object is not a 32-bit PE COFF object")

    scenario_symbols = run([args.nm, "--defined-only", args.scenario_object])
    required_scenario_symbols = {
        "scenario human-turn callback": "_scenario_human_turn_ready",
        "scenario human-turn trampoline": "__Z30scenario_human_turn_trampolinev",
        "scenario trampoline action": "_ScenarioTrampolineAction",
        "scenario turn-advance callback": "_scenario_turn_advanced",
        "scenario turn-advance trampoline": "__Z32scenario_turn_advance_trampolinev",
        "scenario turn-advance action": "_ScenarioTurnAdvanceAction",
    }
    for description, symbol in required_scenario_symbols.items():
        if symbol not in scenario_symbols:
            fail(f"missing required scenario symbol: {description}")
    action_offsets = {}
    for symbol in ("_ScenarioTrampolineAction", "_ScenarioTurnAdvanceAction"):
        match = re.search(
            rf"^([0-9a-f]{{8}}) B {re.escape(symbol)}$",
            scenario_symbols, re.MULTILINE)
        if not match:
            fail(f"scenario action lacks a .bss offset: {symbol}")
        action_offsets[symbol] = int(match.group(1), 16)
    if set(action_offsets.values()) != {0, 4}:
        fail("scenario actions do not occupy distinct verified .bss slots")

    scenario_disassembly = run([args.objdump, "-d", "-r", "-C", args.scenario_object])
    trampoline = re.search(
        r"<scenario_human_turn_trampoline\(\)>:"
        r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
        scenario_disassembly, re.DOTALL)
    if not trampoline:
        fail("could not locate scenario human-turn trampoline in disassembly")
    trampoline_body = trampoline.group("body")
    instructions = re.findall(
        r"^\s*[0-9a-f]+:\s+(?:[0-9a-f]{2}\s+)+(?P<asm>[a-z].*)$",
        trampoline_body, re.MULTILINE | re.IGNORECASE)
    required_instructions = (
        r"mov\s+%edi,0x23d24\(%esi\)",
        r"pushf",
        r"pusha",
        r"push\s+%esi",
        r"call\s+[0-9a-f]+\s+<scenario_human_turn_ready>",
        r"add\s+\$0x4,%esp",
        rf"cmpl\s+\$0x0,0x{action_offsets['_ScenarioTrampolineAction']:x}",
        r"jne\s+[0-9a-f]+\s+<scenario_human_turn_trampoline\(\)\+0x[0-9a-f]+>",
        r"popa",
        r"popf",
        r"push\s+\$0x51418f",
        r"ret",
        r"popa",
        r"popf",
        r"push\s+\$0x5147b9",
        r"ret",
    )
    if len(instructions) < len(required_instructions):
        fail("scenario trampoline has too few instructions")
    for actual, pattern in zip(instructions, required_instructions):
        if not re.fullmatch(pattern, actual.strip()):
            fail(f"scenario trampoline instruction {actual!r} does not match {pattern!r}")
    if not re.search(r"dir32\s+\.bss", trampoline_body):
        fail("scenario trampoline action comparison lacks its .bss relocation")

    turn_trampoline = re.search(
        r"<scenario_turn_advance_trampoline\(\)>:"
        r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
        scenario_disassembly, re.DOTALL)
    if not turn_trampoline:
        fail("could not locate scenario turn-advance trampoline in disassembly")
    turn_body = turn_trampoline.group("body")
    turn_instructions = re.findall(
        r"^\s*[0-9a-f]+:\s+(?:[0-9a-f]{2}\s+)+(?P<asm>[a-z].*)$",
        turn_body, re.MULTILINE | re.IGNORECASE)
    required_turn_instructions = (
        r"pushf",
        r"pusha",
        r"push\s+0x4\(%ebp\)",
        r"call\s+[0-9a-f]+\s+<scenario_turn_advanced>",
        r"add\s+\$0x4,%esp",
        rf"cmpl\s+\$0x0,0x{action_offsets['_ScenarioTurnAdvanceAction']:x}",
        r"jne\s+[0-9a-f]+\s+<scenario_turn_advance_trampoline\(\)\+0x[0-9a-f]+>",
        r"popa",
        r"popf",
        r"push\s+\$0x525af9",
        r"push\s+\$0x46fb10",
        r"ret",
        r"popa",
        r"popf",
        r"movl\s+\$0x5282ce,0x4\(%ebp\)",
        r"push\s+\$0x526026",
        r"ret",
    )
    if len(turn_instructions) < len(required_turn_instructions):
        fail("scenario turn-advance trampoline has too few instructions")
    for actual, pattern in zip(turn_instructions, required_turn_instructions):
        if not re.fullmatch(pattern, actual.strip()):
            fail(
                f"scenario turn-advance instruction {actual!r} does not match {pattern!r}")
    if not re.search(r"dir32\s+\.bss", turn_body):
        fail("scenario turn-advance comparison lacks its .bss relocation")

    # Printed so that a prefetch plan which has drifted out of step with the
    # checks shows up as a number rather than as an unexplained slowdown. A
    # large on-demand count is a performance bug, never a correctness one.
    print(f"recovery-abi: {len(_CACHE)} tool invocations, "
          f"{_EXECUTED_ON_DEMAND} executed on demand at {args.jobs} job(s)")


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f"ABI verification failed: {error}", file=sys.stderr)
        raise SystemExit(1)

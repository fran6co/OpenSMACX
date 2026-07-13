#!/usr/bin/env python3

import argparse
import re
import subprocess
import sys


def run(command):
    return subprocess.run(command, check=True, text=True, capture_output=True).stdout


def fail(message):
    raise RuntimeError(message)


def main():
    parser = argparse.ArgumentParser(
        description="Verify the ABI of source-recovered redirect functions")
    parser.add_argument("--nm", required=True)
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--object", required=True)
    parser.add_argument("--scenario-object")
    args = parser.parse_args()

    headers = run([args.objdump, "-f", args.object])
    if "file format pe-i386" not in headers:
        fail("AlphaNet object is not a 32-bit PE COFF object")

    symbols = run([args.nm, "--defined-only", args.object])
    required_symbols = {
        "AlphaNet::pid_2_idx(unsigned int)": r"_ZN8AlphaNet9pid_2_idxEj",
        "alpha_net_pid_to_idx_redirect": r"@_Z29alpha_net_pid_to_idx_redirectP8AlphaNetPvj@12",
    }
    for description, symbol in required_symbols.items():
        if symbol not in symbols:
            fail(f"missing required AlphaNet symbol: {description}")

    disassembly = run([args.objdump, "-d", "-C", args.object])
    method = re.search(
        r"<AlphaNet::pid_2_idx\(unsigned int\)>:"
        r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
        disassembly, re.DOTALL)
    if not method:
        fail("could not locate AlphaNet::pid_2_idx in disassembly")
    if not re.search(r"\bret\s+\$0x4\b", method.group("body")):
        fail("AlphaNet::pid_2_idx does not use one-argument thiscall cleanup")

    adapter = re.search(
        r"<@_Z29alpha_net_pid_to_idx_redirectP8AlphaNetPvj@12>:"
        r"(?P<body>.*?)(?=\n[0-9a-f]+ <|\Z)",
        disassembly, re.DOTALL)
    if not adapter:
        fail("could not locate the AlphaNet fastcall adapter in disassembly")
    if not re.search(r"\bret\s+\$0x4\b", adapter.group("body")):
        fail("AlphaNet fastcall adapter does not pop its one stack argument")

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
    }
    for description, symbol in required_scenario_symbols.items():
        if symbol not in scenario_symbols:
            fail(f"missing required scenario symbol: {description}")
    if not re.search(r"^00000000 B _ScenarioTrampolineAction$",
                     scenario_symbols, re.MULTILINE):
        fail("scenario trampoline action is not the verified .bss relocation target")

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
        r"popa",
        r"popf",
        r"cmpl\s+\$0x0,0x0",
        r"je\s+[0-9a-f]+\s+<scenario_human_turn_trampoline\(\)\+0x[0-9a-f]+>",
        r"push\s+\$0x5147b9",
        r"ret",
        r"push\s+\$0x51418f",
        r"ret",
    )
    if len(instructions) < len(required_instructions):
        fail("scenario trampoline has too few instructions")
    for actual, pattern in zip(instructions, required_instructions):
        if not re.fullmatch(pattern, actual.strip()):
            fail(f"scenario trampoline instruction {actual!r} does not match {pattern!r}")
    if not re.search(r"dir32\s+\.bss", trampoline_body):
        fail("scenario trampoline action comparison lacks its .bss relocation")


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f"ABI verification failed: {error}", file=sys.stderr)
        raise SystemExit(1)

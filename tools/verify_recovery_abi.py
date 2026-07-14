#!/usr/bin/env python3

import argparse
import re
import subprocess
import sys
from typing import NoReturn


def run(command):
    return subprocess.run(command, check=True, text=True, capture_output=True).stdout


def fail(message) -> NoReturn:
    raise RuntimeError(message)


def main():
    parser = argparse.ArgumentParser(
        description="Verify the ABI of source-recovered redirect functions")
    parser.add_argument("--nm", required=True)
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--object", required=True)
    parser.add_argument("--scenario-object")
    parser.add_argument("--button-group-object")
    parser.add_argument("--basepop-font-object")
    parser.add_argument("--dialog-object")
    parser.add_argument("--string-struct-object")
    parser.add_argument("--spot-object")
    parser.add_argument("--text-index-object")
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
            if not re.search(r"\bret\b", body) or re.search(r"\bret\s+\$", body):
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
            if not re.search(r"\bret\b", body) or re.search(r"\bret\s+\$", body):
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
        if not re.search(r"\bret\b", constructor_adapter) or re.search(
                r"\bret\s+\$", constructor_adapter):
            fail("ButtonGroup constructor adapter does not use plain fastcall return")
        close_adapter = button_body(
            "@_Z27button_group_close_redirectP11ButtonGroupPv@8")
        if not re.search(r"\bret\b", close_adapter) or re.search(
                r"\bret\s+\$", close_adapter):
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
        if not re.search(r"\bret\b", constructor) or re.search(
                r"\bret\s+\$", constructor):
            fail("TextIndex constructor does not use a plain thiscall return")

        destructor = text_index_body("TextIndex::~TextIndex()")
        if not re.search(r"\bmovl\s+\$0x0,0x100\(%e?bx\)", destructor):
            fail("TextIndex destructor does not clear the section count")
        if not re.search(r"\bmovb\s+\$0x0,\(%e?bx\)", destructor):
            fail("TextIndex destructor does not terminate the filename")
        if len(re.findall(r"DISP32\s+Heap::shutdown\(\)", destructor)) != 1:
            fail("TextIndex destructor does not invoke one Heap shutdown")
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
            fail("TextIndex destructor does not use a plain thiscall return")

        clear_helper = text_index_body("text_clear_index_source(TextIndex*)")
        if len(re.findall(r"DISP32\s+Heap::shutdown\(\)", clear_helper)) != 1:
            fail("TextIndex clear helper does not contain one Heap shutdown call site")
        if not re.search(r"\bret\b", clear_helper) or re.search(
                r"\bret\s+\$", clear_helper):
            fail("TextIndex clear helper does not use a plain cdecl return")

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
            if not re.search(r"\bret\b", body) or re.search(r"\bret\s+\$", body):
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
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
            fail("Spot destructor does not use a plain thiscall return")

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


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f"ABI verification failed: {error}", file=sys.stderr)
        raise SystemExit(1)

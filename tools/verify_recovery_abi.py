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
    parser.add_argument("--font-object")
    parser.add_argument("--filemap-object")
    parser.add_argument("--heap-object")
    parser.add_argument("--log-object")
    parser.add_argument("--random-object")
    parser.add_argument("--string-struct-object")
    parser.add_argument("--spot-object")
    parser.add_argument("--strings-object")
    parser.add_argument("--text-object")
    parser.add_argument("--text-index-object")
    parser.add_argument("--time-object")
    args = parser.parse_args()

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

        make_wrapper = text_index_body("text_make_index(char const*)")
        if "TxtIndex" not in make_wrapper or "TextIndex::make_index(char const*)" not in make_wrapper:
            fail("TextIndex make wrapper does not use the global array and member method")
        if not re.search(r"\bret\b", make_wrapper) or re.search(
                r"\bret\s+\$", make_wrapper):
            fail("TextIndex make wrapper does not use a plain cdecl return")

        search_wrapper = text_index_body(
            "text_search_index(char const*, char const*)")
        if "TxtIndex" not in search_wrapper or "TextIndex::search_index(char const*, char const*)" not in search_wrapper:
            fail("TextIndex search wrapper does not use the global array and member method")
        if not re.search(r"\bret\b", search_wrapper) or re.search(
                r"\bret\s+\$", search_wrapper):
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
        if not re.search(r"\bret\b", constructor) or re.search(
                r"\bret\s+\$", constructor):
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
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
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
        if not re.search(r"\bret\b", initializer) or re.search(
                r"\bret\s+\$", initializer):
            fail("Time class initializer does not use a plain cdecl return")

        cleanup = time_body("Time::close_class()")
        if "Time::TimeInitCount" not in cleanup or not re.search(
                r"\b(?:dec|sub)\w*\b", cleanup):
            fail("Time class cleanup does not decrement the global count")
        if not re.search(r"\bret\b", cleanup) or re.search(
                r"\bret\s+\$", cleanup):
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
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
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
        if not re.search(r"\bret\b", constructor) or re.search(
                r"\bret\s+\$", constructor):
            fail("Strings constructor does not use a plain thiscall return")

        destructor = strings_body("Strings::~Strings()")
        if len(re.findall(r"DISP32\s+Heap::shutdown\(\)", destructor)) != 1:
            fail("Strings destructor does not invoke one Heap shutdown")
        if re.search(r"\bmov[^\n]*0x14\(%e?\w+\)", destructor):
            fail("Strings destructor overwrites preserved populated state")
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
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
        if not re.search(r"\bret\b", constructor) or re.search(
                r"\bret\s+\$", constructor):
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
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
            fail("Log destructor does not use a plain thiscall return")

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
            if not re.search(r"\bret\b", wrapper) or re.search(
                    r"\bret\s+\$", wrapper):
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
            if not re.search(r"\bret\b", body) or re.search(r"\bret\s+\$", body):
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
        if not re.search(r"\bret\b", destructor) or re.search(
                r"\bret\s+\$", destructor):
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
        if not re.search(r"\bret\b", reseed_wrapper) or re.search(
                r"\bret\s+\$", reseed_wrapper):
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
        if not re.search(r"\bret\b", integer_wrapper) or re.search(
                r"\bret\s+\$", integer_wrapper):
            fail("random integer wrapper does not use a plain cdecl return")

        floating_wrapper = random_body("random()")
        delegates_floating = "Random::get()" in floating_wrapper
        if not delegates_floating:
            subtract = re.search(r"\bfsub[^\n]*", floating_wrapper)
            if not subtract or not re.search(r"\bflds\b", floating_wrapper[:subtract.start()]):
                fail("random floating wrapper neither delegates nor synthesizes a single")
            if re.search(r"\bfldl\b", floating_wrapper[:subtract.start()]):
                fail("random floating wrapper reads beyond its four-byte temporary")
        if not re.search(r"\bret\b", floating_wrapper) or re.search(
                r"\bret\s+\$", floating_wrapper):
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
        if not re.search(r"\bret\b", default_constructor) or re.search(
                r"\bret\s+\$", default_constructor):
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
        if not re.search(r"\bret\b", initializer) or re.search(
                r"\bret\s+\$", initializer):
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
        if not re.search(r"\bret\b", text_open) or re.search(
                r"\bret\s+\$", text_open):
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


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f"ABI verification failed: {error}", file=sys.stderr)
        raise SystemExit(1)

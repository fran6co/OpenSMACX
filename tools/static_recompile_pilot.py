#!/usr/bin/env python3

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tempfile

import capstone
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
from capstone.x86_const import (
    X86_INS_CMP,
    X86_INS_JBE,
    X86_INS_LEA,
    X86_INS_MOV,
    X86_INS_POP,
    X86_INS_PUSH,
    X86_INS_RET,
    X86_INS_SETE,
    X86_INS_TEST,
    X86_REG_AL,
    X86_REG_DL,
    X86_REG_EAX,
    X86_REG_EBP,
    X86_REG_ECX,
    X86_REG_EDX,
    X86_REG_ESP,
    X86_REG_INVALID,
)
import pefile

from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
TARGET_ADDRESS = 0x004C5460
TARGET_END = 0x004C5481
TARGET_SIZE = TARGET_END - TARGET_ADDRESS
TARGET_BODY_SHA256 = "e112330c5b6d9e863cfc2a23736fdbf97b2027d0f49cc88befcf6cde2cdefa3e"
TARGET_NAME = "?is_group_disabled@Wave_Device@@QAEHI@Z"
SUPPORTED_EXE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ARTIFACT_KIND = "local_proprietary_static_recompilation"
GENERATED_SOURCE = "static-recompiled-004C5460.cpp"


def sha256_bytes(content):
    return hashlib.sha256(content).hexdigest()


def sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def hexadecimal(value):
    return f"0x{value:08X}"


def decode(content, address=TARGET_ADDRESS):
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True
    instructions = list(disassembler.disasm(content, address))
    if (not instructions
            or instructions[0].address != address
            or instructions[-1].address + instructions[-1].size
                != address + len(content)
            or sum(instruction.size for instruction in instructions) != len(content)):
        raise RuntimeError("function body does not decode completely")
    return instructions


def state_register(register):
    names = {
        X86_REG_EAX: "state->eax",
        X86_REG_EBP: "state->ebp",
        X86_REG_ECX: "state->ecx",
        X86_REG_EDX: "state->edx",
        X86_REG_ESP: "state->esp",
    }
    try:
        return names[register]
    except KeyError as error:
        raise RuntimeError(f"unsupported register id {register}") from error


def low_register(register):
    names = {
        X86_REG_AL: "state->eax",
        X86_REG_DL: "state->edx",
    }
    try:
        return names[register]
    except KeyError as error:
        raise RuntimeError(f"unsupported byte register id {register}") from error


def effective_address(instruction, memory):
    if memory.segment != X86_REG_INVALID:
        raise RuntimeError("segment-relative memory is unsupported")
    terms = []
    if memory.base != X86_REG_INVALID:
        terms.append(state_register(memory.base))
    if memory.index != X86_REG_INVALID:
        index = state_register(memory.index)
        terms.append(index if memory.scale == 1 else f"({index} * {memory.scale}U)")
    if memory.disp:
        terms.append(f"UINT32_C(0x{memory.disp & 0xFFFFFFFF:08X})")
    return " + ".join(terms) if terms else "UINT32_C(0)"


def lower_instruction(instruction):
    operands = instruction.operands
    if (instruction.id == X86_INS_PUSH and len(operands) == 1
            and operands[0].type == X86_OP_REG
            and operands[0].reg == X86_REG_EBP):
        return [
            "saved_ebp = state->ebp;",
            "state->esp -= UINT32_C(4);",
        ]
    if (instruction.id == X86_INS_POP and len(operands) == 1
            and operands[0].type == X86_OP_REG
            and operands[0].reg == X86_REG_EBP):
        return [
            "state->ebp = saved_ebp;",
            "state->esp += UINT32_C(4);",
        ]
    if instruction.id == X86_INS_MOV and len(operands) == 2:
        destination, source = operands
        if (destination.type == X86_OP_REG and source.type == X86_OP_REG
                and destination.size == 4 and source.size == 4):
            return [
                f"{state_register(destination.reg)} = {state_register(source.reg)};"
            ]
        if (destination.type == X86_OP_REG and destination.reg == X86_REG_EAX
                and source.type == X86_OP_MEM and source.size == 4
                and source.mem.base == X86_REG_EBP
                and source.mem.index == X86_REG_INVALID
                and source.mem.disp == 8):
            return ["state->eax = argument;"]
        if (destination.type == X86_OP_REG and destination.size == 1
                and source.type == X86_OP_IMM):
            target = low_register(destination.reg)
            return [
                f"opensmacx_set_low8({target}, UINT8_C(0x{source.imm & 0xFF:02X}));"
            ]
        if (destination.type == X86_OP_REG and destination.size == 1
                and source.type == X86_OP_MEM and source.size == 1
                and source.mem.base == X86_REG_ECX):
            target = low_register(destination.reg)
            address = effective_address(instruction, source.mem)
            address = address.replace("state->ecx + ", "", 1)
            return [
                f"opensmacx_set_low8({target}, object[{address}]);"
            ]
    if (instruction.id == X86_INS_CMP and len(operands) == 2
            and operands[0].type == X86_OP_REG and operands[0].size == 4
            and operands[1].type == X86_OP_IMM):
        left = state_register(operands[0].reg)
        return [
            f"opensmacx_cmp32(*state, {left}, "
            f"UINT32_C(0x{operands[1].imm & 0xFFFFFFFF:08X}));"
        ]
    if (instruction.id == X86_INS_JBE and len(operands) == 1
            and operands[0].type == X86_OP_IMM):
        return [
            "if (opensmacx_below_or_equal(state->eflags)) {",
            f"    goto block_{operands[0].imm:08X};",
            "}",
        ]
    if (instruction.id == X86_INS_LEA and len(operands) == 2
            and operands[0].type == X86_OP_REG and operands[0].size == 4
            and operands[1].type == X86_OP_MEM):
        destination = state_register(operands[0].reg)
        return [f"{destination} = {effective_address(instruction, operands[1].mem)};"]
    if (instruction.id == X86_INS_TEST and len(operands) == 2
            and all(operand.type == X86_OP_REG and operand.size == 1
                    for operand in operands)):
        left = low_register(operands[0].reg)
        right = low_register(operands[1].reg)
        return [
            f"opensmacx_test8(*state, opensmacx_low8({left}), "
            f"opensmacx_low8({right}));"
        ]
    if (instruction.id == X86_INS_SETE and len(operands) == 1
            and operands[0].type == X86_OP_REG and operands[0].size == 1):
        target = low_register(operands[0].reg)
        return [
            f"opensmacx_set_low8({target}, "
            "(state->eflags & OpensmacxZeroFlag) != 0 ? UINT8_C(1) : UINT8_C(0));"
        ]
    if instruction.id == X86_INS_RET and len(operands) <= 1:
        immediate = operands[0].imm if operands else 0
        return [
            f"state->esp += UINT32_C({4 + immediate});",
            "return;",
        ]
    raise RuntimeError(
        f"unsupported instruction at {hexadecimal(instruction.address)}: "
        f"{instruction.mnemonic} {instruction.op_str}".rstrip())


def format_source(instructions):
    boundaries = {instruction.address for instruction in instructions}
    branch_targets = {
        instruction.operands[0].imm
        for instruction in instructions
        if instruction.id == X86_INS_JBE
    }
    if not branch_targets.issubset(boundaries):
        targets = ", ".join(hexadecimal(target) for target in sorted(branch_targets))
        raise RuntimeError(f"branch target is outside instruction boundaries: {targets}")
    lines = [
        '#include "static_recompile_runtime.h"',
        "",
        "#include <cstdint>",
        "",
        "extern \"C\" void opensmacx_static_recompiled_core_004C5460(",
        "        OpensmacxStaticRecompileState *state, const uint8_t *object,",
        "        uint32_t argument) {",
        "    uint32_t saved_ebp;",
    ]
    for instruction in instructions:
        if instruction.address in branch_targets:
            lines.append(f"block_{instruction.address:08X}:")
        for statement in lower_instruction(instruction):
            lines.append(f"    {statement}")
    lines.extend([
        "}",
        "",
        "extern \"C\" uint32_t __attribute__((thiscall, noinline, used))",
        "opensmacx_static_recompiled_result_004C5460(",
        "        const uint8_t *, uint32_t)",
        '        asm("_opensmacx_static_recompiled_result_004C5460");',
        "",
        "extern \"C\" uint32_t __attribute__((thiscall, noinline, used))",
        "opensmacx_static_recompiled_result_004C5460(",
        "        const uint8_t *object, uint32_t argument) {",
        "    OpensmacxStaticRecompileState state = {};",
        "    state.ecx = reinterpret_cast<uintptr_t>(object);",
        "    state.esp = UINT32_C(0x00001000);",
        "    opensmacx_static_recompiled_core_004C5460(",
        "        &state, object, argument);",
        "    return state.eax;",
        "}",
        "",
    ])
    return "\n".join(lines)


def validate_oracle_manifest(path):
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if (manifest.get("artifact_kind") != "local_proprietary_legacy_leaves"
            or manifest.get("distribution") != "local_only"
            or manifest.get("candidate_count") != 1
            or manifest.get("open_source_ready") is not False):
        raise RuntimeError("invalid original-oracle manifest")
    functions = manifest.get("functions", [])
    if len(functions) != 1:
        raise RuntimeError("original-oracle manifest must contain one function")
    function = functions[0]
    expected = {
        "address": hexadecimal(TARGET_ADDRESS),
        "end_address": hexadecimal(TARGET_END),
        "size": TARGET_SIZE,
        "sha256": TARGET_BODY_SHA256,
        "canonical_name": TARGET_NAME,
    }
    for key, value in expected.items():
        if function.get(key) != value:
            raise RuntimeError(f"original-oracle manifest has unexpected {key}")
    source = manifest.get("source", {})
    if (source.get("file_name") != "terranx_original.exe"
            or source.get("sha256") != SUPPORTED_EXE_SHA256
            or source.get("size") != 3084288):
        raise RuntimeError("original-oracle manifest has unexpected source identity")
    return manifest, function


def replace_output(temporary, output):
    backup = None
    if output.exists():
        if output.is_symlink() or not output.is_dir():
            raise RuntimeError(f"output is not a regular directory: {output}")
        entries = list(output.iterdir())
        if entries:
            manifest_path = output / "manifest.json"
            try:
                manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as error:
                raise RuntimeError(
                    f"refusing to replace unrecognized output directory: {output}") from error
            expected = {"manifest.json", manifest.get("generated_source", {}).get("path")}
            actual = {
                path.name for path in entries if path.is_file() and not path.is_symlink()
            }
            if (manifest.get("artifact_kind") != ARTIFACT_KIND
                    or None in expected or actual != expected or len(entries) != 2):
                raise RuntimeError(
                    f"refusing to replace unrecognized output directory: {output}")
            source = manifest["generated_source"]
            if sha256_file(output / source["path"]) != source["sha256"]:
                raise RuntimeError(f"refusing to replace modified output directory: {output}")
        backup = Path(tempfile.mkdtemp(
            prefix=f".{output.name}-previous-", dir=output.parent))
        backup.rmdir()
        output.replace(backup)
    try:
        temporary.replace(output)
    except Exception:
        if backup is not None:
            backup.replace(output)
        raise
    if backup is not None:
        shutil.rmtree(backup, ignore_errors=True)


def generate(exe_path, oracle_manifest_path, output):
    output = require_local_artifact_path(output, "static recompilation output")
    source = exe_path.read_bytes()
    if sha256_bytes(source) != SUPPORTED_EXE_SHA256:
        raise RuntimeError("unsupported executable SHA-256")
    oracle_manifest, function = validate_oracle_manifest(oracle_manifest_path)
    try:
        pe = pefile.PE(data=source, fast_load=True)
    except pefile.PEFormatError as error:
        raise RuntimeError(f"invalid PE executable: {error}") from error
    try:
        if pe.FILE_HEADER.Machine != pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_I386"]:
            raise RuntimeError("static recompilation pilot requires an i386 executable")
        offset = int(function["file_offset"], 0)
        expected_offset = pe.get_offset_from_rva(
            TARGET_ADDRESS - pe.OPTIONAL_HEADER.ImageBase)
        if offset != expected_offset:
            raise RuntimeError("original-oracle file offset does not match the PE image")
        content = source[offset:offset + TARGET_SIZE]
    finally:
        pe.close()
    if len(content) != TARGET_SIZE or sha256_bytes(content) != TARGET_BODY_SHA256:
        raise RuntimeError("target function body hash does not match")
    generated = format_source(decode(content))

    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        generated_path = temporary / GENERATED_SOURCE
        generated_path.write_text(generated, encoding="ascii")
        manifest = {
            "artifact_kind": ARTIFACT_KIND,
            "backend": "cxx11_static_basic_blocks_v1",
            "distribution": "local_only",
            "format_version": 1,
            "function": {
                "address": hexadecimal(TARGET_ADDRESS),
                "canonical_name": TARGET_NAME,
                "end_address": hexadecimal(TARGET_END),
                "sha256": TARGET_BODY_SHA256,
                "size": TARGET_SIZE,
            },
            "generated_source": {
                "path": generated_path.name,
                "sha256": sha256_file(generated_path),
            },
            "open_source_ready": False,
            "original_oracle_manifest_sha256": sha256_file(oracle_manifest_path),
            "provenance": "mechanically_derived_from_original_executable",
            "source": oracle_manifest["source"],
            "tool_versions": {
                "capstone": capstone.__version__,
                "pefile": pefile.__version__,
            },
        }
        (temporary / "manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        replace_output(temporary, output)
    except Exception:
        shutil.rmtree(temporary, ignore_errors=True)
        raise


def main():
    parser = argparse.ArgumentParser(
        description="Run the local-only OpenSMACX static recompilation micro-pilot")
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--oracle-manifest", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        generate(
            args.exe.expanduser().resolve(),
            args.oracle_manifest.expanduser().resolve(),
            args.output.expanduser().absolute(),
        )
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()

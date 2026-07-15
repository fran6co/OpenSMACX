#!/usr/bin/env python3

import argparse
from collections import Counter
from collections import defaultdict
import csv
import hashlib
import json
from pathlib import Path
import shutil
import tempfile

from capstone import (
    CS_ARCH_X86,
    CS_GRP_CALL,
    CS_GRP_INT,
    CS_GRP_IRET,
    CS_GRP_JUMP,
    CS_GRP_PRIVILEGE,
    CS_GRP_RET,
    CS_MODE_32,
    Cs,
)
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_INVALID
import pefile


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_CORRELATION = (
    REPO_ROOT / "docs" / "recovery" / "analysis-correlation.csv")
DEFAULT_ANALYSIS_SUMMARY = (
    REPO_ROOT / "docs" / "recovery" / "analysis-summary.json")
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_GHIDRA = REPO_ROOT / "docs" / "recovery" / "ghidra-functions.csv"
DEFAULT_GHIDRA_REFERENCES = (
    REPO_ROOT / "docs" / "recovery" / "ghidra-interior-references.csv")
DEFAULT_OUTPUT = REPO_ROOT / ".opensmacx" / "legacy-leaves"
ARTIFACT_KIND = "local_proprietary_legacy_leaves"
SUPPORTED_EXE_HASHES = {
    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
}


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def hexadecimal(value):
    return f"0x{value:08X}"


def parse_ranges(value):
    ranges = []
    for item in value.split(";"):
        if not item:
            continue
        start, end = item.split("-", 1)
        ranges.append((int(start, 0), int(end, 0)))
    result = []
    for start, end in sorted(ranges):
        if result and start <= result[-1][1]:
            result[-1] = (result[-1][0], max(result[-1][1], end))
        else:
            result.append((start, end))
    return result


def load_rows(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        return list(csv.DictReader(file))


def select_rows(correlations, requested_addresses):
    requested = set(requested_addresses)
    if not requested:
        return [row for row in correlations
                if row["binary_kind"] == "game"
                and row["recovery_state"] == "unrecovered"]
    available = {int(row["address"], 0) for row in correlations}
    missing = requested - available
    if missing:
        addresses = ", ".join(hexadecimal(address) for address in sorted(missing))
        raise RuntimeError(f"requested addresses are absent from correlation: {addresses}")
    return [row for row in correlations
            if int(row["address"], 0) in requested]


def validate_output_path(output):
    output = output.expanduser().resolve()
    allowed_roots = (
        (REPO_ROOT / ".opensmacx").resolve(),
        (REPO_ROOT / "build").resolve(),
    )
    if not any(root in output.parents for root in allowed_roots):
        raise RuntimeError(
            "output must be inside the ignored .opensmacx/ or build/ directory")


def is_relative_branch(instruction):
    return (instruction.group(CS_GRP_JUMP) or
            instruction.mnemonic in {
                "jcxz", "jecxz", "loop", "loope", "loopne", "loopnz", "loopz",
            })


def get_relocation_info(pe, source):
    spans = []
    references = defaultdict(set)
    widths = {1: 2, 2: 2, 3: 4, 4: 4, 10: 8}
    for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []):
        for entry in block.entries:
            if not entry.type:
                continue
            address = pe.OPTIONAL_HEADER.ImageBase + entry.rva
            width = widths.get(entry.type, 8)
            spans.append((address, address + width))
            if entry.type == 3:
                offset = pe.get_offset_from_rva(entry.rva)
                target = int.from_bytes(source[offset:offset + 4], "little")
                references[target].add(address)
    return spans, references


def read_virtual_range(pe, source, start, end):
    image_base = pe.OPTIONAL_HEADER.ImageBase
    start_rva = start - image_base
    end_rva = end - image_base
    for section in pe.sections:
        section_start = section.VirtualAddress
        section_end = section_start + section.SizeOfRawData
        if section_start <= start_rva and end_rva <= section_end:
            offset = section.PointerToRawData + start_rva - section_start
            content = source[offset:offset + end - start]
            if len(content) == end - start:
                name = section.Name.rstrip(b"\0").decode("ascii", errors="replace")
                return offset, name, content
    raise RuntimeError(
        f"function range {hexadecimal(start)}-{hexadecimal(end)} is not file-backed")


def collect_code_references(disassembler, pe, source, ghidra_rows,
                            image_start, image_end):
    references = defaultdict(set)
    for row in ghidra_rows:
        for start, end in parse_ranges(row["body_ranges"]):
            try:
                _offset, _section, content = read_virtual_range(
                    pe, source, start, end)
            except RuntimeError:
                continue
            for instruction in disassembler.disasm(content, start):
                for operand in instruction.operands:
                    if operand.type == X86_OP_MEM:
                        value = operand.mem.disp & 0xFFFFFFFF
                        if image_start <= value < image_end:
                            references[value].add(instruction.address)
                    elif operand.type == X86_OP_IMM:
                        value = operand.imm & 0xFFFFFFFF
                        if (instruction.group(CS_GRP_CALL) or
                                is_relative_branch(instruction) or
                                image_start <= value < image_end):
                            references[value].add(instruction.address)
    return references


def classify_function(disassembler, content, start, end, image_start, image_end,
                      relocation_spans, all_entry_points, inbound_references,
                      max_size):
    if len(content) > max_size:
        return "too_large", []
    if any(relocation_start < end and start < relocation_end
           for relocation_start, relocation_end in relocation_spans):
        return "contains_relocation", []
    if any(start < address < end for address in all_entry_points):
        return "contains_nested_entry", []
    if any(start < target < end and
           any(not start <= source_address < end for source_address in sources)
           for target, sources in inbound_references.items()):
        return "contains_external_interior_reference", []

    instructions = list(disassembler.disasm(content, start))
    if not instructions or sum(item.size for item in instructions) != len(content):
        return "incomplete_disassembly", instructions
    instruction_by_address = {instruction.address: instruction
                              for instruction in instructions}
    successors = {}
    for instruction in instructions:
        if instruction.group(CS_GRP_CALL):
            return "contains_call", instructions
        if (instruction.group(CS_GRP_INT) or instruction.group(CS_GRP_IRET) or
                instruction.group(CS_GRP_PRIVILEGE)):
            return "contains_system_instruction", instructions
        if instruction.mnemonic in {
                "fnsave", "fnstenv", "fsave", "fstenv", "fxsave", "fxsave64",
                "xsave", "xsave64", "xsavec", "xsaveopt", "xsaves"}:
            return "observes_instruction_pointer", instructions
        is_branch = is_relative_branch(instruction)
        branch_target = None
        if is_branch:
            if not instruction.operands or instruction.operands[0].type != X86_OP_IMM:
                return "contains_indirect_branch", instructions
            branch_target = instruction.operands[0].imm & 0xFFFFFFFF
            if not start <= branch_target < end:
                return "contains_external_branch", instructions
            if branch_target not in instruction_by_address:
                return "branch_targets_mid_instruction", instructions
        for operand in instruction.operands:
            if operand.type == X86_OP_MEM:
                memory = operand.mem
                if memory.segment != X86_REG_INVALID:
                    return "contains_segment_memory", instructions
                displacement = memory.disp & 0xFFFFFFFF
                if ((memory.base == X86_REG_INVALID and
                     memory.index == X86_REG_INVALID) or
                        image_start <= displacement < image_end):
                    return "contains_absolute_memory", instructions
            elif operand.type == X86_OP_IMM and not is_branch:
                value = operand.imm & 0xFFFFFFFF
                if image_start <= value < image_end:
                    return "contains_image_immediate", instructions

        next_address = instruction.address + instruction.size
        if instruction.group(CS_GRP_RET):
            successors[instruction.address] = []
        elif is_branch:
            branch_successors = [branch_target]
            if instruction.mnemonic not in {"jmp", "ljmp"}:
                branch_successors.append(next_address)
            successors[instruction.address] = branch_successors
        else:
            successors[instruction.address] = [next_address]

    reachable = set()
    pending = [start]
    reached_return = False
    while pending:
        address = pending.pop()
        if address in reachable:
            continue
        instruction = instruction_by_address.get(address)
        if instruction is None:
            return "reachable_fallthrough", instructions
        reachable.add(address)
        if instruction.group(CS_GRP_RET):
            reached_return = True
        for successor in successors[address]:
            if successor == end:
                return "reachable_fallthrough", instructions
            if successor not in instruction_by_address:
                return "branch_targets_mid_instruction", instructions
            pending.append(successor)
    if not reached_return:
        return "has_no_reachable_return", instructions
    if len(reachable) != len(instructions):
        return "contains_unreachable_bytes", instructions
    return "candidate", instructions


def format_assembly(functions):
    lines = [
        "/* Generated locally from a user-owned executable. Do not distribute. */",
        "/* Each symbol is recovery debt and is forbidden in open-source builds. */",
        "",
    ]
    for function in functions:
        lines.extend([
            f"    .section .text$legacy${function['address'][2:]},\"xr\"",
            f"    .globl _{function['symbol']}",
            f"_{function['symbol']}:",
        ])
        content = bytes.fromhex(function["bytes_hex"])
        for offset in range(0, len(content), 12):
            values = ", ".join(f"0x{value:02X}" for value in content[offset:offset + 12])
            lines.append(f"    .byte {values}")
        lines.append("")
    return "\n".join(lines)


def write_csv(path, rows):
    fields = [
        "address", "end_address", "size", "symbol", "canonical_name",
        "file_offset", "section", "sha256", "caller_count",
    ]
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fields, lineterminator="\n")
        writer.writeheader()
        writer.writerows({field: row[field] for field in fields} for row in rows)


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
            if manifest.get("artifact_kind") != ARTIFACT_KIND:
                raise RuntimeError(
                    f"refusing to replace unrecognized output directory: {output}")
            expected = {
                "manifest.json",
                manifest.get("assembly", {}).get("path"),
                manifest.get("symbols", {}).get("path"),
            }
            actual = {
                path.relative_to(output).as_posix()
                for path in entries
                if path.is_file() and not path.is_symlink()
            }
            if None in expected or actual != expected or len(entries) != 3:
                raise RuntimeError(f"refusing to replace modified output directory: {output}")
            for key in ("assembly", "symbols"):
                artifact = manifest[key]
                if sha256_file(output / artifact["path"]) != artifact["sha256"]:
                    raise RuntimeError(
                        f"refusing to replace modified output directory: {output}")
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


def extract(exe_path, correlation_path, analysis_summary_path, functions_path,
            ghidra_path, ghidra_references_path, output, max_size,
            requested_addresses=()):
    validate_output_path(output)
    source = exe_path.read_bytes()
    source_hash = sha256_bytes(source)
    if source_hash not in SUPPORTED_EXE_HASHES:
        raise RuntimeError(f"unsupported executable SHA-256: {source_hash}")
    try:
        pe = pefile.PE(data=source, fast_load=False)
        pe.parse_data_directories()
    except pefile.PEFormatError as error:
        raise RuntimeError(f"invalid PE executable: {error}") from error
    if pe.FILE_HEADER.Machine != pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_I386"]:
        raise RuntimeError("legacy leaf extraction requires an i386 executable")

    correlations = load_rows(correlation_path)
    if len({row["address"] for row in correlations}) != len(correlations):
        raise RuntimeError("analysis correlation contains duplicate addresses")
    canonical_rows = load_rows(functions_path)
    canonical_functions = {row["address"]: row for row in canonical_rows}
    ghidra_rows = load_rows(ghidra_path)
    ghidra_reference_rows = load_rows(ghidra_references_path)
    ghidra_functions = {row["address"]: row for row in ghidra_rows}
    if len(canonical_functions) != len(canonical_rows):
        raise RuntimeError("canonical inventory contains duplicate addresses")
    if len(ghidra_functions) != len(ghidra_rows):
        raise RuntimeError("Ghidra inventory contains duplicate addresses")
    if (not ghidra_rows or
            {row["program_sha256"] for row in ghidra_rows} != {source_hash}):
        raise RuntimeError("Ghidra metadata does not describe the source executable")
    if (not ghidra_reference_rows or
            {row["program_sha256"] for row in ghidra_reference_rows} != {source_hash}):
        raise RuntimeError("Ghidra references do not describe the source executable")
    analysis_summary = json.loads(
        analysis_summary_path.read_text(encoding="utf-8"))
    if sha256_file(correlation_path) != analysis_summary["outputs"][
            "analysis_correlation_sha256"]:
        raise RuntimeError("analysis correlation does not match its summary")
    if sha256_file(ghidra_path) != analysis_summary["analyses"]["ghidra"][
            "input_sha256"]:
        raise RuntimeError("Ghidra inventory does not match its analysis summary")
    if sha256_file(ghidra_references_path) != analysis_summary["analyses"][
            "ghidra"]["interior_references_input_sha256"]:
        raise RuntimeError("Ghidra references do not match the analysis summary")
    if len(correlations) != len(canonical_rows):
        raise RuntimeError("analysis correlation and canonical inventory counts differ")
    for row in correlations:
        canonical = canonical_functions.get(row["address"])
        if canonical is None:
            raise RuntimeError(
                f"correlation address is absent from canonical inventory: {row['address']}")
        for field in ("binary_kind", "name", "recovery_state"):
            if row[field] != canonical[field]:
                raise RuntimeError(
                    f"correlation {field} differs at {row['address']}")
        if (int(row["end_address"], 0) != int(canonical["end_address"], 0) or
                int(row["size"]) != int(canonical["size"]) or
                parse_ranges(row["body_ranges"]) !=
                parse_ranges(canonical["body_ranges"])):
            raise RuntimeError(
                f"correlation canonical body differs at {row['address']}")
        if row["ghidra_address"]:
            ghidra = ghidra_functions.get(row["ghidra_address"])
            if ghidra is None or row["ghidra_body_ranges"] != ghidra["body_ranges"]:
                raise RuntimeError(
                    f"correlation Ghidra body differs at {row['address']}")
    all_entry_points = {int(row["address"], 0) for row in ghidra_rows}
    image_start = pe.OPTIONAL_HEADER.ImageBase
    image_end = image_start + pe.OPTIONAL_HEADER.SizeOfImage
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True
    relocation_spans, inbound_references = get_relocation_info(pe, source)
    for row in ghidra_reference_rows:
        inbound_references[int(row["target"], 0)].add(int(row["source"], 0))
    code_references = collect_code_references(
        disassembler, pe, source, ghidra_rows, image_start, image_end)
    for target, owners in code_references.items():
        inbound_references[target].update(owners)

    reasons = Counter()
    candidates = []
    reviewed_rows = select_rows(correlations, requested_addresses)
    reviewed_unrecovered = sum(
        row["recovery_state"] == "unrecovered" for row in reviewed_rows)
    for row in reviewed_rows:
        if row["binary_kind"] != "game":
            reasons["non_game_function"] += 1
            continue
        if row["ghidra_relation"] != "exact":
            reasons["non_exact_analysis"] += 1
            continue
        ranges = parse_ranges(row["ghidra_body_ranges"])
        if len(ranges) != 1:
            reasons["discontiguous_body"] += 1
            continue
        start, end = ranges[0]
        offset, section, content = read_virtual_range(pe, source, start, end)
        reason, instructions = classify_function(
            disassembler, content, start, end, image_start, image_end,
            relocation_spans, all_entry_points, inbound_references, max_size)
        reasons[reason] += 1
        if reason != "candidate":
            continue
        symbol = f"opensmacx_legacy_{start:08X}"
        candidates.append({
            "address": hexadecimal(start),
            "bytes_hex": content.hex(),
            "caller_count": int(canonical_functions[row["address"]]["caller_count"]),
            "canonical_name": row["name"],
            "end_address": hexadecimal(end),
            "file_offset": hexadecimal(offset),
            "instruction_count": len(instructions),
            "section": section,
            "sha256": sha256_bytes(content),
            "size": len(content),
            "symbol": symbol,
        })
    candidates.sort(key=lambda item: int(item["address"], 0))
    if requested_addresses:
        extracted = {int(item["address"], 0) for item in candidates}
        rejected = set(requested_addresses) - extracted
        if rejected:
            addresses = ", ".join(
                hexadecimal(address) for address in sorted(rejected))
            raise RuntimeError(
                f"requested addresses are not eligible legacy leaves: {addresses}")

    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        assembly_path = temporary / "legacy-leaves.S"
        assembly_path.write_text(format_assembly(candidates), encoding="ascii")
        symbols_path = temporary / "symbols.csv"
        write_csv(symbols_path, candidates)
        manifest = {
            "artifact_kind": ARTIFACT_KIND,
            "assembly": {
                "path": assembly_path.name,
                "sha256": sha256_file(assembly_path),
            },
            "candidate_count": len(candidates),
            "criteria": {
                "exact_cross_build_boundary": True,
                "maximum_size": max_size,
                "no_absolute_or_image_references": True,
                "no_calls_or_external_branches": True,
                "no_nested_entry_points": True,
                "no_relocations": True,
                "one_contiguous_body": True,
                "requires_return": True,
            },
            "distribution": "local_only",
            "format_version": 1,
            "functions": [{key: value for key, value in item.items() if key != "bytes_hex"}
                          for item in candidates],
            "open_source_ready": False,
            "proprietary_blob_count": len(candidates),
            "requested_addresses": [
                hexadecimal(address) for address in sorted(set(requested_addresses))],
            "rejected_by_reason": dict(sorted(
                (reason, count) for reason, count in reasons.items()
                if reason != "candidate")),
            "reviewed_functions": len(reviewed_rows),
            "reviewed_unrecovered_functions": reviewed_unrecovered,
            "source": {
                "file_name": exe_path.name,
                "sha256": source_hash,
                "size": len(source),
            },
            "ghidra_interior_references_sha256": sha256_file(ghidra_references_path),
            "symbols": {
                "path": symbols_path.name,
                "sha256": sha256_file(symbols_path),
            },
        }
        (temporary / "manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        replace_output(temporary, output)
    except Exception:
        shutil.rmtree(temporary, ignore_errors=True)
        raise
    finally:
        pe.close()

    print(f"Extracted {len(candidates)} conservative legacy leaves to {output}")
    if requested_addresses:
        print(f"Reviewed explicitly requested functions: {len(reviewed_rows)}")
    else:
        print(f"Reviewed unrecovered game functions: {reviewed_unrecovered}")


def main():
    parser = argparse.ArgumentParser(
        description="Extract conservative local-only x86 legacy leaf islands")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--correlation", type=Path, default=DEFAULT_CORRELATION)
    parser.add_argument("--analysis-summary", type=Path,
                        default=DEFAULT_ANALYSIS_SUMMARY)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--ghidra", type=Path, default=DEFAULT_GHIDRA)
    parser.add_argument("--ghidra-references", type=Path,
                        default=DEFAULT_GHIDRA_REFERENCES)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--max-size", type=int, default=512)
    parser.add_argument(
        "--address", action="append", default=[], type=lambda value: int(value, 0),
        help="extract only this canonical function start, even when source-complete")
    args = parser.parse_args()
    if args.max_size <= 0:
        parser.error("--max-size must be positive")
    args.exe = args.exe.expanduser().resolve()
    args.correlation = args.correlation.expanduser().resolve()
    args.analysis_summary = args.analysis_summary.expanduser().resolve()
    args.functions = args.functions.expanduser().resolve()
    args.ghidra = args.ghidra.expanduser().resolve()
    args.ghidra_references = args.ghidra_references.expanduser().resolve()
    for path in (args.exe, args.correlation, args.analysis_summary,
                 args.functions, args.ghidra, args.ghidra_references):
        if not path.is_file():
            parser.error(f"input not found: {path}")
    try:
        extract(
            args.exe,
            args.correlation,
            args.analysis_summary,
            args.functions,
            args.ghidra,
            args.ghidra_references,
            args.output.expanduser().resolve(),
            args.max_size,
            args.address,
        )
    except (OSError, RuntimeError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()

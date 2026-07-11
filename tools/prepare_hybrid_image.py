#!/usr/bin/env python3

import argparse
from collections import Counter
import csv
import hashlib
import json
from pathlib import Path
import re
import shutil
import tempfile

import pefile


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_PRIORITIES = REPO_ROOT / "docs" / "recovery" / "priorities.csv"
DEFAULT_SUMMARY = REPO_ROOT / "docs" / "recovery" / "summary.json"
DEFAULT_ANALYSIS_SUMMARY = (
    REPO_ROOT / "docs" / "recovery" / "analysis-summary.json")
DEFAULT_CORRELATION = (
    REPO_ROOT / "docs" / "recovery" / "analysis-correlation.csv")
DEFAULT_OUTPUT = REPO_ROOT / ".opensmacx" / "hybrid-image"


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def hexadecimal(value, width=8):
    return f"0x{value:0{width}X}"


def decode_bytes(value):
    if value is None:
        return None
    return value.decode("utf-8", errors="replace")


def parse_ranges(value):
    ranges = []
    for item in value.split(";"):
        if not item:
            continue
        start, end = item.split("-", 1)
        start_value = int(start, 0)
        end_value = int(end, 0)
        if end_value <= start_value:
            raise RuntimeError(f"invalid function body range: {item}")
        ranges.append((start_value, end_value))
    result = []
    for start, end in sorted(ranges):
        if result and start <= result[-1][1]:
            result[-1] = (result[-1][0], max(result[-1][1], end))
        else:
            result.append((start, end))
    return result


def write_json(path, value):
    path.write_text(
        json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_compact_json(path, value):
    path.write_text(
        json.dumps(value, separators=(",", ":"), sort_keys=True) + "\n",
        encoding="utf-8")


def write_blob(root, relative_path, data):
    path = root / relative_path
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    return {
        "path": relative_path.as_posix(),
        "sha256": sha256_bytes(data),
        "size": len(data),
    }


def section_name(section):
    return decode_bytes(section.Name.rstrip(b"\0")) or "unnamed"


def safe_name(value):
    result = re.sub(r"[^A-Za-z0-9_.-]+", "_", value)
    return result.strip("._") or "unnamed"


def get_data_directories(pe):
    names = {
        value: name
        for name, value in pefile.DIRECTORY_ENTRY.items()
        if isinstance(name, str)
    }
    result = []
    for index, directory in enumerate(pe.OPTIONAL_HEADER.DATA_DIRECTORY):
        item = {
            "address": hexadecimal(directory.VirtualAddress),
            "address_kind": "file_offset" if index == 4 else "rva",
            "name": names.get(index, f"IMAGE_DIRECTORY_ENTRY_{index}"),
            "size": directory.Size,
        }
        result.append(item)
    return result


def get_imports(pe):
    result = []
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
        symbols = []
        for item in entry.imports:
            symbols.append({
                "hint": item.hint,
                "iat_rva": hexadecimal(item.address - pe.OPTIONAL_HEADER.ImageBase),
                "iat_va": hexadecimal(item.address),
                "name": decode_bytes(item.name),
                "ordinal": item.ordinal,
            })
        result.append({
            "module": decode_bytes(entry.dll),
            "symbols": symbols,
        })
    return result


def get_relocations(pe):
    result = []
    for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []):
        entries = [
            [item.type, item.rva - block.struct.VirtualAddress]
            for item in block.entries
        ]
        result.append({
            "entries": entries,
            "page_rva": hexadecimal(block.struct.VirtualAddress),
            "size": block.struct.SizeOfBlock,
        })
    return result


def resource_component(entry):
    if entry.name is not None:
        return str(entry.name)
    return entry.struct.Id


def get_resources(pe):
    result = []

    def visit(directory, path):
        for entry in directory.entries:
            item_path = [*path, resource_component(entry)]
            if hasattr(entry, "directory"):
                visit(entry.directory, item_path)
                continue
            data = entry.data.struct
            content = pe.get_data(data.OffsetToData, data.Size)
            if len(content) != data.Size:
                raise RuntimeError(
                    f"resource at {hexadecimal(data.OffsetToData)} is truncated")
            result.append({
                "code_page": data.CodePage,
                "path": item_path,
                "rva": hexadecimal(data.OffsetToData),
                "sha256": sha256_bytes(content),
                "size": data.Size,
            })

    root = getattr(pe, "DIRECTORY_ENTRY_RESOURCE", None)
    if root is not None:
        visit(root, [])
    return result


def read_virtual_range(pe, source, start_va, end_va):
    image_base = pe.OPTIONAL_HEADER.ImageBase
    start_rva = start_va - image_base
    end_rva = end_va - image_base
    for section in pe.sections:
        section_start = section.VirtualAddress
        section_end = section_start + section.SizeOfRawData
        if section_start <= start_rva and end_rva <= section_end:
            offset = section.PointerToRawData + start_rva - section_start
            size = end_rva - start_rva
            content = source[offset:offset + size]
            if len(content) != size:
                break
            return offset, content
    raise RuntimeError(
        f"canonical range {hexadecimal(start_va)}-{hexadecimal(end_va)} "
        "does not map to raw bytes in the source executable")


def build_function_map(pe, source, functions_path, priorities_path, summary_path,
                       analysis_summary_path, correlation_path):
    with functions_path.open(newline="", encoding="utf-8-sig") as file:
        function_rows = list(csv.DictReader(file))
    with priorities_path.open(newline="", encoding="utf-8-sig") as file:
        priorities = list(csv.DictReader(file))
    with correlation_path.open(newline="", encoding="utf-8-sig") as file:
        correlation_rows = list(csv.DictReader(file))
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    analysis_summary = json.loads(
        analysis_summary_path.read_text(encoding="utf-8"))

    functions = {row["address"]: row for row in function_rows}
    correlations = {row["address"]: row for row in correlation_rows}
    priority_addresses = [row["address"] for row in priorities]
    if len(functions) != len(function_rows):
        raise RuntimeError("canonical inventory contains duplicate addresses")
    if len(correlations) != len(correlation_rows):
        raise RuntimeError("analysis correlation contains duplicate addresses")
    if len(set(priority_addresses)) != len(priority_addresses):
        raise RuntimeError("recovery priorities contain duplicate addresses")
    if len(functions) != summary["functions"]["total"]:
        raise RuntimeError("canonical inventory and summary function counts differ")
    if len(correlations) != analysis_summary["canonical_functions"]:
        raise RuntimeError("analysis correlation and summary function counts differ")
    if len(priorities) != analysis_summary["priorities"]["total"]:
        raise RuntimeError("recovery priority and analysis summary counts differ")
    if sha256_file(correlation_path) != analysis_summary["outputs"][
            "analysis_correlation_sha256"]:
        raise RuntimeError("analysis correlation does not match its summary")
    if sha256_file(priorities_path) != analysis_summary["outputs"][
            "priorities_sha256"]:
        raise RuntimeError("recovery priorities do not match their analysis summary")
    relation_counts = Counter(row["ghidra_relation"] for row in correlation_rows)
    if dict(sorted(relation_counts.items())) != analysis_summary["analyses"]["ghidra"]["relations"]:
        raise RuntimeError("analysis correlation and summary relation counts differ")
    for address, correlation in correlations.items():
        function = functions.get(address)
        if function is None:
            raise RuntimeError(
                f"correlation address is absent from canonical inventory: {address}")
        if sorted(parse_ranges(correlation["body_ranges"])) != sorted(
                parse_ranges(function["body_ranges"])):
            raise RuntimeError(
                f"correlation body ranges differ at canonical address {address}")
        for field in ("binary_kind", "name", "recovery_state"):
            if correlation[field] != function[field]:
                raise RuntimeError(
                    f"correlation {field} differs at canonical address {address}")

    canonical_hash = summary["inputs"]["original_input_sha256"]
    if canonical_hash != analysis_summary["binary_identity"]["canonical_program_sha256"]:
        raise RuntimeError("recovery summaries identify different canonical executables")
    ghidra_hash = analysis_summary["binary_identity"]["ghidra_program_sha256"]
    source_hash = sha256_bytes(source)
    if source_hash == canonical_hash:
        source_relation = "same_binary"
    elif source_hash == ghidra_hash:
        source_relation = "independently_analyzed_cross_build"
    else:
        raise RuntimeError(
            f"unsupported executable SHA-256: {source_hash}; input must match "
            "the canonical or independently analyzed recovery binary")

    result = []
    mapped_count = 0
    for priority in priorities:
        function = functions.get(priority["address"])
        if function is None:
            raise RuntimeError(
                f"priority address is absent from canonical inventory: {priority['address']}")
        correlation = correlations.get(priority["address"])
        if correlation is None:
            raise RuntimeError(
                f"correlation is absent for canonical address: {priority['address']}")
        if source_relation == "same_binary":
            analysis_relation = "canonical"
            source_address = function["address"]
            source_ranges = function["body_ranges"]
            mapping_state = "mapped"
        else:
            analysis_relation = correlation["ghidra_relation"]
            source_address = correlation["ghidra_address"] or None
            source_ranges = correlation["ghidra_body_ranges"]
            if analysis_relation in {"exact", "entry_range", "start_only"}:
                mapping_state = "mapped"
            elif analysis_relation in {"containing", "split"}:
                mapping_state = "ambiguous"
            else:
                mapping_state = "unmapped"

        ranges = []
        body = bytearray()
        if mapping_state == "mapped":
            for start, end in parse_ranges(source_ranges):
                offset, content = read_virtual_range(pe, source, start, end)
                body.extend(content)
                ranges.append({
                    "end_va": hexadecimal(end),
                    "file_offset": hexadecimal(offset),
                    "sha256": sha256_bytes(content),
                    "size": len(content),
                    "start_va": hexadecimal(start),
                })
            mapped_count += 1
        result.append({
            "address": function["address"],
            "analysis_relation": analysis_relation,
            "binary_kind": function["binary_kind"],
            "binding_category": priority["binding_category"],
            "binding_strategy": priority["binding_strategy"],
            "body_ranges": ranges,
            "body_sha256": sha256_bytes(body) if ranges else None,
            "body_size": len(body),
            "mapping_state": mapping_state,
            "name": function["name"],
            "priority": priority["priority"],
            "rank": int(priority["rank"]),
            "recovery_state": function["recovery_state"],
            "source_address": source_address if mapping_state == "mapped" else None,
        })

    return {
        "analysis_summary_sha256": sha256_file(analysis_summary_path),
        "canonical_program_sha256": canonical_hash,
        "format_version": 1,
        "function_count": len(result),
        "functions": result,
        "inventory_sha256": sha256_file(functions_path),
        "mapped_function_count": mapped_count,
        "priorities_sha256": sha256_file(priorities_path),
        "correlation_sha256": sha256_file(correlation_path),
        "source_binary_relation": source_relation,
        "source_program_sha256": source_hash,
        "unmapped_function_count": len(result) - mapped_count,
    }


def build_file_layout(pe, source, output):
    source_size = len(source)
    header_size = pe.OPTIONAL_HEADER.SizeOfHeaders
    if not 0 < header_size <= source_size:
        raise RuntimeError(f"invalid PE header size: {header_size}")

    header = write_blob(output, Path("blobs/headers.bin"), source[:header_size])
    header.update({"file_offset": hexadecimal(0), "kind": "headers"})
    layout = [header]
    sections = []
    occupied = [(0, header_size, "headers")]
    for index, section in enumerate(pe.sections):
        name = section_name(section)
        offset = section.PointerToRawData
        size = section.SizeOfRawData
        section_end = offset + size
        if size and (offset < header_size or section_end > source_size):
            raise RuntimeError(f"section {name} has an invalid raw file range")
        blob = None
        if size:
            relative_path = Path(
                f"blobs/sections/{index:02d}-{safe_name(name)}.bin")
            blob = write_blob(output, relative_path, source[offset:section_end])
            layout_item = {
                **blob,
                "file_offset": hexadecimal(offset),
                "kind": "section",
                "section_index": index,
            }
            layout.append(layout_item)
            occupied.append((offset, section_end, f"section {name}"))
        sections.append({
            "blob": blob,
            "characteristics": hexadecimal(section.Characteristics),
            "index": index,
            "name": name,
            "raw_offset": hexadecimal(offset),
            "raw_size": size,
            "virtual_address": hexadecimal(section.VirtualAddress),
            "virtual_size": section.Misc_VirtualSize,
        })

    occupied.sort()
    gaps = []
    cursor = 0
    for start, end, description in occupied:
        if start < cursor:
            raise RuntimeError(f"overlapping PE file range at {description}")
        if start > cursor:
            index = len(gaps)
            blob = write_blob(
                output, Path(f"blobs/gaps/{index:02d}.bin"), source[cursor:start])
            item = {
                **blob,
                "file_offset": hexadecimal(cursor),
                "kind": "gap",
            }
            gaps.append(item)
            layout.append(item)
        cursor = end

    overlay = None
    if cursor < source_size:
        blob = write_blob(output, Path("blobs/overlay.bin"), source[cursor:])
        overlay = {
            **blob,
            "file_offset": hexadecimal(cursor),
            "kind": "overlay",
        }
        layout.append(overlay)

    layout.sort(key=lambda item: int(item["file_offset"], 0))
    rebuilt = bytearray(source_size)
    rebuilt_size = 0
    for item in layout:
        content = (output / item["path"]).read_bytes()
        offset = int(item["file_offset"], 0)
        rebuilt[offset:offset + len(content)] = content
        rebuilt_size += len(content)
    if rebuilt_size != source_size or bytes(rebuilt) != source:
        raise RuntimeError("generated blobs do not reconstruct the source executable exactly")

    return header, sections, gaps, overlay, layout


def validate_existing_output(output):
    manifest_path = output / "manifest.json"
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise RuntimeError(
            f"refusing to replace unrecognized output directory: {output}") from error
    if (manifest.get("artifact_kind") != "local_legacy_pe_image" or
            manifest.get("format_version") != 1 or
            manifest.get("distribution") != "local_only"):
        raise RuntimeError(
            f"refusing to replace unrecognized output directory: {output}")

    artifacts = [*manifest.get("file_layout", [])]
    for key in ("legacy_functions", "relocations"):
        artifacts.append(manifest.get(key, {}))
    expected = {"manifest.json"}
    for artifact in artifacts:
        value = artifact.get("path")
        relative = Path(value) if isinstance(value, str) else None
        if (relative is None or relative.is_absolute() or
                any(part in {"", ".", ".."} for part in relative.parts)):
            raise RuntimeError(
                f"refusing to replace malformed hybrid image: {output}")
        normalized = relative.as_posix()
        if normalized in expected:
            raise RuntimeError(
                f"refusing to replace malformed hybrid image: {output}")
        expected.add(normalized)

    actual_files = set()
    actual_directories = set()
    for path in output.rglob("*"):
        relative = path.relative_to(output).as_posix()
        if path.is_symlink():
            raise RuntimeError(
                f"refusing to replace modified hybrid image: {output}")
        if path.is_dir():
            actual_directories.add(relative)
        elif path.is_file():
            actual_files.add(relative)
        else:
            raise RuntimeError(
                f"refusing to replace modified hybrid image: {output}")
    expected_directories = {
        parent.as_posix()
        for value in expected
        for parent in Path(value).parents
        if parent != Path(".")
    }
    if actual_files != expected or actual_directories != expected_directories:
        raise RuntimeError(
            f"refusing to replace modified hybrid image: {output}")
    for artifact in artifacts:
        path = output / artifact["path"]
        if sha256_file(path) != artifact.get("sha256"):
            raise RuntimeError(
                f"refusing to replace modified hybrid image: {output}")
        if "size" in artifact and path.stat().st_size != artifact["size"]:
            raise RuntimeError(
                f"refusing to replace modified hybrid image: {output}")


def replace_output(temporary, output):
    backup = None
    if output.exists():
        if output.is_symlink() or not output.is_dir():
            raise RuntimeError(f"output exists and is not a regular directory: {output}")
        entries = list(output.iterdir())
        if entries:
            validate_existing_output(output)
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


def generate(exe_path, functions_path, priorities_path, summary_path,
             analysis_summary_path, correlation_path, output):
    source = exe_path.read_bytes()
    try:
        pe = pefile.PE(data=source, fast_load=False)
        pe.parse_data_directories()
    except pefile.PEFormatError as error:
        raise RuntimeError(f"invalid PE executable: {error}") from error
    if pe.FILE_HEADER.Machine != pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_I386"]:
        raise RuntimeError("hybrid image input must target i386")
    if pe.OPTIONAL_HEADER.Magic != pefile.OPTIONAL_HEADER_MAGIC_PE:
        raise RuntimeError("hybrid image input must be PE32")

    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        header, sections, gaps, overlay, layout = build_file_layout(pe, source, temporary)
        function_map = build_function_map(
            pe, source, functions_path, priorities_path, summary_path,
            analysis_summary_path, correlation_path)
        function_map_path = temporary / "legacy-functions.json"
        write_json(function_map_path, function_map)
        relocations = get_relocations(pe)
        relocations_path = temporary / "relocations.json"
        write_compact_json(relocations_path, {
            "blocks": relocations,
            "entry_count": sum(len(block["entries"]) for block in relocations),
            "format_version": 1,
        })

        image_base = pe.OPTIONAL_HEADER.ImageBase
        manifest = {
            "artifact_kind": "local_legacy_pe_image",
            "data_directories": get_data_directories(pe),
            "distribution": "local_only",
            "file_layout": layout,
            "format_version": 1,
            "gaps": gaps,
            "headers": header,
            "imports": get_imports(pe),
            "legacy_functions": {
                "count": function_map["function_count"],
                "mapped_count": function_map["mapped_function_count"],
                "path": function_map_path.name,
                "sha256": sha256_file(function_map_path),
                "unmapped_count": function_map["unmapped_function_count"],
            },
            "open_source_ready": False,
            "open_source_ready_reason": (
                "legacy executable sections and unresolved function bodies remain"),
            "overlay": overlay,
            "pe": {
                "characteristics": hexadecimal(pe.FILE_HEADER.Characteristics, 4),
                "dll_characteristics": hexadecimal(
                    pe.OPTIONAL_HEADER.DllCharacteristics, 4),
                "entry_point_rva": hexadecimal(
                    pe.OPTIONAL_HEADER.AddressOfEntryPoint),
                "entry_point_va": hexadecimal(
                    image_base + pe.OPTIONAL_HEADER.AddressOfEntryPoint),
                "file_alignment": pe.OPTIONAL_HEADER.FileAlignment,
                "image_base": hexadecimal(image_base),
                "machine": hexadecimal(pe.FILE_HEADER.Machine, 4),
                "section_alignment": pe.OPTIONAL_HEADER.SectionAlignment,
                "size_of_headers": pe.OPTIONAL_HEADER.SizeOfHeaders,
                "size_of_image": pe.OPTIONAL_HEADER.SizeOfImage,
                "subsystem": pe.OPTIONAL_HEADER.Subsystem,
            },
            "relocations": {
                "block_count": len(relocations),
                "entry_count": sum(len(block["entries"]) for block in relocations),
                "path": relocations_path.name,
                "sha256": sha256_file(relocations_path),
            },
            "resources": get_resources(pe),
            "sections": sections,
            "source": {
                "file_name": exe_path.name,
                "sha256": sha256_bytes(source),
                "size": len(source),
            },
        }
        write_json(temporary / "manifest.json", manifest)

        replace_output(temporary, output)
    except Exception:
        shutil.rmtree(temporary, ignore_errors=True)
        raise

    print(f"Prepared local hybrid image at {output}")
    print(f"Source SHA-256: {sha256_bytes(source)}")
    print(
        f"Legacy recovery candidates: {function_map['function_count']} "
        f"({function_map['mapped_function_count']} mapped)")


def main():
    parser = argparse.ArgumentParser(
        description="Prepare a deterministic local-only hybrid PE image pack")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE,
                        help="User-owned legacy PE32 executable")
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--priorities", type=Path, default=DEFAULT_PRIORITIES)
    parser.add_argument("--summary", type=Path, default=DEFAULT_SUMMARY)
    parser.add_argument("--analysis-summary", type=Path,
                        default=DEFAULT_ANALYSIS_SUMMARY)
    parser.add_argument("--correlation", type=Path, default=DEFAULT_CORRELATION)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    args.exe = args.exe.expanduser().resolve()
    args.functions = args.functions.expanduser().resolve()
    args.priorities = args.priorities.expanduser().resolve()
    args.summary = args.summary.expanduser().resolve()
    args.analysis_summary = args.analysis_summary.expanduser().resolve()
    args.correlation = args.correlation.expanduser().resolve()
    inputs = (args.exe, args.functions, args.priorities, args.summary,
              args.analysis_summary, args.correlation)
    for path in inputs:
        if not path.is_file():
            parser.error(f"input not found: {path}")
    output = args.output.expanduser().resolve()
    if output == Path(output.anchor):
        parser.error("refusing to use a filesystem root as output")

    try:
        generate(
            args.exe,
            args.functions,
            args.priorities,
            args.summary,
            args.analysis_summary,
            args.correlation,
            output,
        )
    except (OSError, RuntimeError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile

import pefile

from prepare_hybrid_image import sha256_file, validate_hybrid_image
from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_IMAGE = REPO_ROOT / ".opensmacx" / "hybrid-image"
DEFAULT_OUTPUT = REPO_ROOT / ".opensmacx" / "hybrid" / "terranx_legacy.exe"
IMAGE_FILE_MACHINE_I386 = 0x014C
PE32_MAGIC = 0x010B


def load_json(path, description):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise RuntimeError(f"invalid {description}: {path}") from error


def parse_offset(value, description):
    try:
        result = int(value, 0) if isinstance(value, str) else int(value)
    except (TypeError, ValueError) as error:
        raise RuntimeError(f"invalid {description}: {value!r}") from error
    if result < 0:
        raise RuntimeError(f"invalid {description}: {value!r}")
    return result


def validate_sidecars(image_dir, manifest):
    function_ref = manifest["legacy_functions"]
    functions = load_json(
        image_dir / function_ref["path"], "legacy function map")
    expected_function_values = {
        "function_count": function_ref["count"],
        "mapped_function_count": function_ref["mapped_count"],
        "unmapped_function_count": function_ref["unmapped_count"],
    }
    for field, expected in expected_function_values.items():
        if functions.get(field) != expected:
            raise RuntimeError(
                f"legacy function map {field} does not match the manifest")

    relocation_ref = manifest["relocations"]
    relocations = load_json(
        image_dir / relocation_ref["path"], "relocation map")
    blocks = relocations.get("blocks")
    if not isinstance(blocks, list):
        raise RuntimeError("relocation map has no block list")
    if len(blocks) != relocation_ref["block_count"]:
        raise RuntimeError("relocation block count does not match the manifest")
    entry_count = sum(len(block.get("entries", [])) for block in blocks)
    if (entry_count != relocation_ref["entry_count"] or
            entry_count != relocations.get("entry_count")):
        raise RuntimeError("relocation entry count does not match the manifest")
    return functions


def reconstruct(image_dir, manifest):
    source = manifest.get("source", {})
    source_size = source.get("size")
    if not isinstance(source_size, int) or source_size <= 0:
        raise RuntimeError("manifest has an invalid source size")
    layout = manifest.get("file_layout")
    if not isinstance(layout, list) or not layout:
        raise RuntimeError("manifest has no file layout")

    result = bytearray(source_size)
    cursor = 0
    for item in sorted(layout, key=lambda value: parse_offset(
            value.get("file_offset"), "file offset")):
        offset = parse_offset(item.get("file_offset"), "file offset")
        if offset != cursor:
            raise RuntimeError(
                f"file layout is not contiguous at offset 0x{cursor:08X}")
        blob = image_dir / item["path"]
        content = blob.read_bytes()
        if len(content) != item.get("size"):
            raise RuntimeError(f"blob size does not match the manifest: {blob}")
        end = offset + len(content)
        if end > source_size:
            raise RuntimeError(f"blob exceeds the source size: {blob}")
        result[offset:end] = content
        cursor = end
    if cursor != source_size:
        raise RuntimeError("file layout does not cover the complete source executable")

    digest = hashlib.sha256(result).hexdigest()
    if digest != source.get("sha256"):
        raise RuntimeError(
            f"assembled SHA-256 {digest} does not match the manifest")
    return bytes(result), digest


def validate_pe(image, manifest):
    try:
        pe = pefile.PE(data=image, fast_load=True)
    except pefile.PEFormatError as error:
        raise RuntimeError(f"assembled image is not a valid PE: {error}") from error
    try:
        expected = manifest["pe"]
        values = {
            "entry_point_rva": pe.OPTIONAL_HEADER.AddressOfEntryPoint,
            "characteristics": pe.FILE_HEADER.Characteristics,
            "dll_characteristics": pe.OPTIONAL_HEADER.DllCharacteristics,
            "file_alignment": pe.OPTIONAL_HEADER.FileAlignment,
            "image_base": pe.OPTIONAL_HEADER.ImageBase,
            "machine": pe.FILE_HEADER.Machine,
            "section_alignment": pe.OPTIONAL_HEADER.SectionAlignment,
            "size_of_headers": pe.OPTIONAL_HEADER.SizeOfHeaders,
            "size_of_image": pe.OPTIONAL_HEADER.SizeOfImage,
            "subsystem": pe.OPTIONAL_HEADER.Subsystem,
        }
        for field, actual in values.items():
            expected_value = expected.get(field)
            if isinstance(expected_value, str):
                expected_value = int(expected_value, 0)
            if actual != expected_value:
                raise RuntimeError(f"assembled PE {field} does not match the manifest")
        if pe.FILE_HEADER.Machine != IMAGE_FILE_MACHINE_I386:
            raise RuntimeError("assembled image is not i386")
        if pe.OPTIONAL_HEADER.Magic != PE32_MAGIC:
            raise RuntimeError("assembled image is not PE32")
        if pe.FILE_HEADER.Characteristics & 0x2000:
            raise RuntimeError("assembled image is a DLL, not an executable")
    finally:
        pe.close()


def write_output(output, image, digest):
    if output.exists():
        if output.is_symlink() or not output.is_file():
            raise RuntimeError(f"output is not a regular file: {output}")
        if sha256_file(output) != digest:
            raise RuntimeError(
                f"refusing to replace a different executable: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{output.name}-", dir=output.parent)
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as file:
            file.write(image)
            file.flush()
            os.fsync(file.fileno())
        temporary.replace(output)
    except Exception:
        temporary.unlink(missing_ok=True)
        raise


def assemble(image_dir, output):
    output = require_local_artifact_path(output, "assembled executable output")
    if image_dir.is_symlink() or not image_dir.is_dir():
        raise RuntimeError(f"hybrid image directory not found: {image_dir}")
    try:
        output.relative_to(image_dir)
    except ValueError:
        pass
    else:
        raise RuntimeError("assembled executable must be outside the hybrid image pack")

    manifest = validate_hybrid_image(image_dir)
    if manifest.get("open_source_ready") is not False:
        raise RuntimeError("local hybrid manifest has an invalid source readiness state")
    functions = validate_sidecars(image_dir, manifest)
    image, digest = reconstruct(image_dir, manifest)
    if functions.get("source_program_sha256") != digest:
        raise RuntimeError("legacy function map identifies a different source executable")
    validate_pe(image, manifest)
    write_output(output, image, digest)
    print(f"Assembled local legacy PE at {output}")
    print(f"SHA-256: {digest}")


def main():
    parser = argparse.ArgumentParser(
        description="Assemble and verify a local hybrid PE image pack")
    parser.add_argument("--image", type=Path, default=DEFAULT_IMAGE,
                        help="Prepared local hybrid image directory")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT,
                        help="Assembled local executable path")
    args = parser.parse_args()
    image_dir = args.image.expanduser().resolve()
    try:
        output = require_local_artifact_path(
            args.output, "assembled executable output")
        assemble(image_dir, output)
    except (KeyError, OSError, RuntimeError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()

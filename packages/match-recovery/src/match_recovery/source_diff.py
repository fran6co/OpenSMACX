"""Fresh, source-linked diagnostic views; never grants recovery claims."""

from contextlib import nullcontext
from dataclasses import asdict
from hashlib import sha256
from pathlib import Path
import json
import re
import subprocess
import tempfile

from .compiler import Compiler
from .coff_preservation import compare_outside_functions
from .measurement import Measurement
from .model import Span
from .pe_coff import Coff, PeImage
from .records import read_json
from .source import Mode, State
from .source.asm import compare_unit, shared_spans
from .source.eh_regions import bind_eh_regions
from .source_inventory import image_pin, source_snapshot
from .source_selection import (compilation_context, select_records,
                               selected_source_records, snapshot_mappings)


def listing_lines(text: str, symbol: str, code: bytes) -> dict[int, int]:
    """Read MSVC /FAcs source anchors, checking each listed byte against COFF.

    Wrapped byte lines continue the preceding instruction. Unsupported listings
    refuse attribution rather than guessing source positions from instructions.
    """
    active = False
    line = None
    result = {}
    seen = bytearray(len(code))
    cursor = None
    found = 0
    for row in text.splitlines():
        if re.match(re.escape(symbol) + r"\s+PROC\b", row):
            active = True
            found += 1
            continue
        if active and re.match(re.escape(symbol) + r"\s+ENDP\b", row):
            active = False
        if not active:
            continue
        anchor = re.match(r";\s*(\d+)\s*:", row)
        if anchor:
            line = int(anchor[1])
            continue
        start = re.match(r"\s+([0-9a-fA-F]{5,8})\t", row)
        if start:
            cursor = int(start[1], 16)
            rest = row[start.end():]
            if line is not None:
                result[cursor] = line
        elif row.startswith("\t") and cursor is not None:
            rest = row[1:]
        else:
            continue
        raw = rest.split("\t", 1)[0].strip()
        if not re.fullmatch(r"[0-9a-fA-F]{2}(?: +[0-9a-fA-F]{2})*", raw):
            continue
        data = bytes.fromhex(raw)
        if cursor is None or code[cursor:cursor + len(data)] != data:
            raise ValueError("compiler listing bytes do not match the fresh object")
        seen[cursor:cursor + len(data)] = b"\1" * len(data)
        cursor += len(data)
    if found != 1 or not result or not all(seen):
        raise ValueError("listing does not completely cover one selected function")
    return result


def _instructions(code, base, relocations=(), coff=None, lines=None):
    import capstone
    disassembler = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    rows = []
    consumed = 0
    for ins in disassembler.disasm(code, base):
        offset = ins.address - base
        names = []
        for relocation in relocations:
            if offset <= relocation.offset < offset + ins.size:
                symbol = coff.symbols.get(int(relocation.target))
                names.append(symbol.name if symbol else f"symbol-index:{relocation.target}")
        anchors = [at for at in (lines or {}) if at <= offset]
        rows.append({"address": ins.address, "offset": offset,
                     "bytes": ins.bytes.hex(" "), "mnemonic": ins.mnemonic,
                     "operands": ins.op_str, "relocations": names,
                     "source_line": lines[max(anchors)] if anchors else None})
        consumed += ins.size
    if consumed != len(code):
        raise ValueError("selected code contains undecoded bytes; diff cannot silently omit them")
    return rows


def generate_diff(root: Path, target: str, *, image: Path, in_file=None,
                  output=None, flag_set=0, resources=None):
    root = root.resolve()
    if resources is not None:
        from .resources import resource_environment
        lease = resource_environment(read_json(root / resources), [root])
    else:
        lease = nullcontext()
    with lease:
        return _generate(root, target, image=image, in_file=in_file,
                         output=output, flag_set=flag_set)


def _generate(root, target, *, image, in_file, output, flag_set):
    from .diff_html import render_diff
    settings = Measurement.read(root)
    config_path = root / "recovery/diff.json"
    config_bytes = config_path.read_bytes()
    config = json.loads(config_bytes)
    if (set(config) != {"schema_version", "format", "flags"}
            or config["schema_version"] != 1 or config["format"] != "msvc-assembly"
            or not isinstance(config["flags"], list)
            or not config["flags"] or not all(isinstance(x, str) and x for x in config["flags"])
            or not any("{listing}" in x for x in config["flags"])):
        raise ValueError("diff requires configured compiler listing flags in recovery/diff.json")
    image = (root / image).resolve()
    original_bytes = image.read_bytes()
    pin = image_pin(root, settings.source)
    original = PeImage(original_bytes, pin)
    before = settings.snapshot(root, {"operation": "diff", "target": target})
    snapshot = source_snapshot(root, allow_empty=True)
    mappings = snapshot_mappings(root, snapshot)
    selected = select_records(mappings, [target],
                              settings.source.path(root, in_file) if in_file else None,
                              settings.source.symbol_format)
    if len(selected) != 1:
        raise ValueError("diff requires exactly one unambiguous source mapping; use --in")
    record, = selected_source_records(root, snapshot, selected)
    if record.mode is Mode.UNBOUND or record.state is State.PLACEHOLDER or record.function_fragment:
        raise ValueError("diff requires a complete implemented source-owned function")
    unit = compilation_context(root, record, settings)
    flags = settings.flags_for(unit.relative_to(root).as_posix())
    if type(flag_set) is not int or not 0 <= flag_set < len(flags):
        raise ValueError("flag-set index is outside the configured TU trials")
    flags = flags[flag_set]
    output = (root / output).absolute() if output else root / "build/recovery/diffs" / record.address_hex / "diff.html"
    if not output.is_relative_to(root) or output.is_symlink():
        raise ValueError("diff output must be a regular ignored path inside the project")
    ignored = subprocess.run(["git", "check-ignore", "--no-index", str(output)], cwd=root,
                             capture_output=True, check=False)
    if ignored.returncode != 0:
        raise ValueError("diff output contains executable-derived data and must be Git-ignored")
    if output.parent.resolve() != output.parent:
        raise ValueError("diff output cannot traverse symlinks")
    output.parent.mkdir(parents=True, exist_ok=True)
    source_text = record.path.read_text()
    compiler = Compiler(root, profile=settings.compiler_profile)
    with tempfile.TemporaryDirectory(prefix="diff-", dir=output.parent) as directory:
        directory = Path(directory)
        listing_path = directory / "compiler.cod"
        raw = compiler(unit, (), flags)
        listed = compiler(unit, (), flags + tuple(x.replace("{listing}", str(listing_path)) for x in config["flags"]))
        if compare_outside_functions(Coff(raw), Coff(listed), {}, {})["result"] != "pass":
            raise ValueError("listing options changed the physical compiler output")
        listing = listing_path.read_text(encoding="latin1")
        obj = Coff(raw)
        symbol = record.symbol or record.name
        function = obj.function(symbol)
        code, relocations, _ = obj.subject(function)
        lines = listing_lines(listing, symbol, code)
        if any(n < 1 or n > len(source_text.splitlines()) for n in lines.values()):
            raise ValueError("compiler source anchors lie outside the selected source file")
        spans = tuple(record.image_spans)
        regions = []
        if len(spans) == 2:
            pairs = bind_eh_regions(raw, symbol, original, spans)
            for pair in pairs:
                regions.append({"role": pair.role.value, "original_start": pair.original.start,
                                "original": _instructions(original.read(pair.original), pair.original.start),
                                "compiled": _instructions(pair.code, 0, pair.relocations, obj,
                                                          lines if pair.role.value == "root" else None)})
        elif len(spans) == 1:
            span = Span(*spans[0])
            regions.append({"role": "root", "original_start": span.start,
                            "original": _instructions(original.read(span), span.start),
                            "compiled": _instructions(code, 0, relocations, obj, lines)})
        else:
            raise ValueError("diff does not support this mapping's span layout")
        def observed(path, arguments, trial):
            if path != unit or arguments or trial != flags:
                raise ValueError("unobserved comparison recipe")
            return raw
        measured = compare_unit([record], image, unit, (), (flags,), shared_spans(mappings),
                                compiler=observed, root=root, strict_command_for=lambda _: ())
        comparison = measured.results[0][1]
        if isinstance(comparison, str):
            raise ValueError(comparison)
        first, last = min(lines.values()), max(lines.values())
        payload = {"title": symbol, "source_path": record.path.relative_to(root).as_posix(),
                   "source_lines": [{"number": n, "text": text} for n, text in enumerate(source_text.splitlines(), 1)
                                    if first <= n <= last], "regions": regions}
        document = render_diff(payload)
        if (settings.snapshot(root, {"operation": "diff", "target": target}) != before
                or config_path.read_bytes() != config_bytes or image.read_bytes() != original_bytes
                or record.path.read_text() != source_text):
            raise ValueError("diff inputs changed during compilation; no viewer published")
        temporary = directory / "diff.html"
        temporary.write_text(document)
        temporary.replace(output)
    return {"operation": "diff", "result": "pass", "output": str(output),
            "source_sha256": sha256(source_text.encode()).hexdigest(), "image_sha256": pin,
            "compiler_invocations": 2, "flags": list(flags), "comparison": asdict(comparison),
            "limitations": ["Diagnostic view, not reference, structure, linked preservation or game acceptance.",
                            "Source attribution is compiler-reported and may move under optimization.",
                            "Refresh this command after editing; original source lines are unknown."]}

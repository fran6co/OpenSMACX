"""Fresh, source-linked diagnostic views; never grants recovery claims."""

from dataclasses import asdict
import base64
from hashlib import sha256
from pathlib import Path
import json
import subprocess
import tempfile

from .compiler import Compiler
from .coff_preservation import compare_outside_functions
from .measurement import Measurement
from .model import Span
from .pe_coff import Coff, PeImage
from .source import Mode, State
from .source.asm import compare_unit, shared_spans
from .source.eh_regions import bind_eh_regions
from .source_inventory import image_pin, source_snapshot
from .source_selection import (compilation_context, select_records,
                               selected_source_records, snapshot_mappings)
from .source_listing import listing_lines

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
                  output=None, flag_set=0):
    root = root.resolve()
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
        from .source_listing import listing_source_texts
        texts = {name: base64.b64decode(info['content_base64']).decode('utf-8', errors='replace')
                 for name, info in snapshot['files'].items()}
        relative, source_text = listing_source_texts(listing, symbol, texts, set(lines.values()) - {None})
        source_path = root / relative
        if any(n < 1 or n > len(source_text.splitlines()) for n in lines.values() if n is not None):
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
        source_numbers = [n for n in lines.values() if n is not None]
        first, last = min(source_numbers), max(source_numbers)
        payload = {"title": symbol, "source_path": source_path.relative_to(root).as_posix(),
                   "source_lines": [{"number": n, "text": text} for n, text in enumerate(source_text.splitlines(), 1)
                                    if first <= n <= last], "regions": regions}
        document = render_diff(payload)
        if (settings.snapshot(root, {"operation": "diff", "target": target}) != before
                or config_path.read_bytes() != config_bytes or image.read_bytes() != original_bytes
                or source_path.read_text() != source_text):
            raise ValueError("diff inputs changed during compilation; no viewer published")
        temporary = directory / "diff.html"
        temporary.write_text(document)
        temporary.replace(output)
    return {"operation": "diff", "result": "pass", "output": str(output),
            "source_sha256": sha256(source_text.encode()).hexdigest(), "image_sha256": pin,
            "compiler_invocations": 2, "flags": list(flags), "comparison": asdict(comparison),
            "source_path": source_path.relative_to(root).as_posix(),
            "declared_body_path": record.body or None,
            "body_attribution": "not_checked_diagnostic_only",
            "limitations": ["Diagnostic view, not reference, structure, linked preservation or game acceptance.",
                            "Source attribution is compiler-reported and may move under optimization.",
                            "Captured source quotes can describe inlined code; this viewer does not qualify the declared body binding.",
                            "Refresh this command after editing; original source lines are unknown."]}

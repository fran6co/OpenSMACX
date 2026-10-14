"""Advisory per-mapping source quality observations, separate from matching."""

from collections import Counter, defaultdict
import base64
import hashlib
import re
from pathlib import Path

from .source_inventory import source_snapshot, assert_snapshot_current
from .source import read_text


_CAST = re.compile(r"\b(?:reinterpret_cast|static_cast|dynamic_cast|const_cast)\s*<|\(\s*(?:(?:unsigned|signed|long|short)\s+)*(?:void|char|wchar_t|bool|int|float|double|size_t|intptr_t|uintptr_t|u?int\d+_t|[A-Z][A-Za-z_0-9]*(?:::[A-Z][A-Za-z_0-9]*)*)\s*[*&]*\s*\)\s*(?=[A-Za-z_*&0-9])")
_AMBIGUOUS_C_CAST = re.compile(r"\(\s*(?:const\s+)?[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*(?:\s*[*&]+)?\s*\)\s*(?=[A-Za-z_*&0-9])")
_RAW = re.compile(r"\bthis\s*\+\s*(?:0x[0-9a-f]+|\d+)|\bfield_[0-9a-f]+_?\b|\*\s*\(\s*(?:char|BYTE|uint8_t)\s*\*\s*\)\s*this", re.I)
_VTABLE = re.compile(r"\b(?:vtable|vtbl|virtual_table)\b|\b(?:slots|functions)\s*\[\s*\d+\s*\]")
_ADDRESS = re.compile(r"\b0x(?:00[0-9a-f]{6}|004[0-9a-f]{4})\b", re.I)
_DEBT = re.compile(r"return\s+reinterpret_cast<[^>]+>\s*\(\s*0x", re.I)


def _mask_non_code(text):
    """Blank C++ comments and ordinary/raw literals, retaining offsets."""
    from .source.reader import _RAW_STRING_OPEN
    out = list(text)
    i, raw_delimiter, block = 0, None, False
    while i < len(text):
        if raw_delimiter is not None:
            end = text.find(")" + raw_delimiter + '"', i)
            end = len(text) if end < 0 else end + len(raw_delimiter) + 2
            for j in range(i, end):
                if text[j] != "\n": out[j] = " "
            i, raw_delimiter = end, None
        elif block:
            end = text.find("*/", i)
            end = len(text) if end < 0 else end + 2
            for j in range(i, end):
                if text[j] != "\n": out[j] = " "
            i, block = end, False
        elif opener := _RAW_STRING_OPEN.match(text, i):
            raw_delimiter = opener.group(1)
            for j in range(i, opener.end()):
                if text[j] != "\n": out[j] = " "
            i = opener.end()
        elif text.startswith("//", i):
            end = text.find("\n", i)
            end = len(text) if end < 0 else end
            for j in range(i, end): out[j] = " "
            i = end
        elif text.startswith("/*", i):
            for j in range(i, i + 2): out[j] = " "
            i, block = i + 2, True
        elif text[i] in "\"'":
            quote = text[i]; j = i + 1
            while j < len(text):
                if text[j] == "\\": j += 2; continue
                if text[j] == quote: j += 1; break
                j += 1
            for k in range(i, min(j, len(text))):
                if text[k] != "\n": out[k] = " "
            i = j
        else:
            i += 1
    return "".join(out)


def _findings(body, *, path, first_line):
    code = _mask_non_code(body)
    findings = []
    lines = code.splitlines()
    for number, line in enumerate(lines, 1):
        explicit_casts = [match.span() for match in _CAST.finditer(line)]
        for category, pattern, label in (
            ("needs_investigation", _CAST, "cast or reinterpretation boundary"),
            ("needs_investigation", _AMBIGUOUS_C_CAST, "ambiguous C-style cast or parenthesized expression"),
            ("needs_investigation", _RAW, "raw offset or low-level access"),
            ("needs_investigation", _VTABLE, "manual virtual dispatch representation"),
            ("needs_investigation", _ADDRESS, "hardcoded address-like literal"),
            ("reconstruction_debt", _DEBT, "unfinished body or fixed-address forwarder"),
        ):
            for match in pattern.finditer(line):
                if pattern is _AMBIGUOUS_C_CAST and any(a <= match.start() < b for a, b in explicit_casts):
                    continue
                findings.append({"category": category, "kind": label, "path": path,
                                 "line": first_line + number - 1,
                                 "column": match.start() + 1, "text": line.strip()})
    return findings


def _generated_body_reason(record, source_config):
    """Don't attribute generated ABI glue to the source method it calls."""
    symbol = record.symbol or record.name
    if source_config.get("symbol_format") == "msvc":
        if re.search(r"@@\$[0-9A-Z]", symbol):
            return "compiler-generated MSVC adjustor has no authored function body"
        if record.name.startswith(("??_G", "??_E", "??__E", "??__F")):
            return "compiler-generated MSVC deleting/lifetime entry has no authored function body"
        if "thunk" in record.name.lower():
            return "catalogued thunk identity has no independent authored function body"
    if record.kind == "thunk":
        return "mapping is classified as a compiler thunk without an authored function body"
    return None


def quality_report(root: Path) -> dict:
    snapshot = source_snapshot(root, allow_empty=True)
    grouped = defaultdict(list)
    for row in snapshot["records"]:
        grouped[row["entity_id"]].append(row)
    obligations = {row["entity_id"] for row in snapshot["historical_obligations"]
                   if row["dimension"] == "bytes"}
    output, totals = [], Counter()
    identity_by_location = {(row["path"], row["line"]): row["entity_id"] for row in snapshot["records"]}
    records_by_location = {}
    for path, info in snapshot["files"].items():
        text = base64.b64decode(info["content_base64"]).decode("utf-8", errors="replace")
        for record in read_text(text, root / path):
            eid = identity_by_location.get((path, record.line))
            if eid is None: continue
            records_by_location[(eid, path, record.line)] = record
    source_config = snapshot["source_configuration"]
    texts = {path: base64.b64decode(info["content_base64"]).decode("utf-8", errors="replace")
             for path, info in snapshot["files"].items()}

    def resolved_body(row, record):
        """Resolve only the annotation region or one uniquely named definition."""
        from .source.reader import _is_placeholder_region, _region_end, function_line
        if reason := _generated_body_reason(record, source_config):
            return None, record.body or row["path"], row["line"], False, reason
        if record.mode.value != "file" and not record.body:
            return record.region, row["path"], record.line, record.state.value == "placeholder", "annotation body region"
        body_path = record.body or row["path"]
        body_text = texts.get(body_path)
        if body_text is None:
            return None, body_path, row["line"], False, "body source is outside configured source snapshot"
        symbol = record.symbol or record.name
        if source_config.get("symbol_format") == "msvc":
            from .source.mangled import qualified_name
            name = qualified_name(symbol)
        else:
            from .source.mangled import identifier
            name = identifier(symbol) or symbol
        if not name:
            return None, body_path, row["line"], False, "mapping symbol has no supported source name"
        try:
            first = function_line(root / body_path, name)
        except ValueError as error:
            return None, body_path, row["line"], False, f"unique source definition unresolved: {error}"
        lines = body_text.splitlines()
        end = _region_end(lines, first - 1, name="")
        if end is None:
            return None, body_path, first, False, "resolved definition has no bounded body region"
        body = "\n".join(lines[first - 1:end + 1])
        return body, body_path, first, _is_placeholder_region(body), "unique symbol-resolved definition"

    for entity_id, rows in sorted(grouped.items()):
        locations = {(row["path"], row["line"]) for row in rows}
        if len(locations) != 1:
            status, body, path, line, reason, placeholder = "unreviewed", None, rows[0]["path"], rows[0]["line"], "identity has multiple source owners", False
        else:
            path, line = next(iter(locations))
            record = records_by_location.get((entity_id, path, line))
            body, path, line, placeholder, reason = resolved_body(rows[0], record) if record else (None, path, line, False, "source body record unresolved")
            findings = _findings(body, path=path, first_line=line) if body else []
            if placeholder:
                marker = "BODY GOES HERE"
                relative = body.find(marker)
                sentinel_line = line + (body[:relative].count("\n") if relative >= 0 else 0)
                findings.append({"category": "reconstruction_debt", "kind": "native placeholder classification",
                                 "path": path, "line": sentinel_line, "column": 1,
                                 "text": "The source reader classifies this mapped body as a placeholder."})
            status = ("unreviewed" if body is None else
                      "reconstruction_debt" if placeholder or any(f["category"] == "reconstruction_debt" for f in findings) else
                      "needs_investigation" if findings else "unreviewed")
        if len(locations) != 1:
            findings = []
        scan_observation = ("unresolved" if body is None else
                            "patterns_found" if findings else "no_patterns_found")
        totals[status] += 1
        output.append({"entity_id": entity_id, "path": path, "line": line,
                       "catalogued_symbols": sorted({r["catalogued_symbol"] for r in rows}),
                       "status": status, "findings": findings,
                       "body_resolution": reason,
                       "scan_observation": scan_observation,
                       "historical_byte_claim": entity_id in obligations,
                       "current_exactness": "not_measured"})
    claim_cross_tab = {status: {"claimed": 0, "unclaimed": 0}
                       for status in ("needs_investigation", "reconstruction_debt", "unreviewed")}
    for item in output:
        claim_cross_tab[item["status"]]["claimed" if item["historical_byte_claim"] else "unclaimed"] += 1
    assert_snapshot_current(root, snapshot)
    return {"schema_version": 1, "result": "advisory", "identity_count": len(output),
            "source_revision": snapshot["revision"],
            "image_sha256": snapshot["image_sha256"],
            "source_fingerprint": hashlib.sha256(
                "".join(f"{path}:{snapshot['files'][path]['sha256']}\n" for path in sorted(snapshot["files"])).encode()).hexdigest(),
            "counts": {key: totals[key] for key in ("needs_investigation", "reconstruction_debt", "unreviewed")},
            "historical_byte_claims_by_status": claim_cross_tab,
            "limitations": ["Historical byte claims are not fresh exactness measurements.",
                            "No absence of pattern findings establishes structural review or quality.",
                            "FILE mappings are scanned only when a unique symbol-resolved definition is found; unresolved mappings remain unreviewed.",
                            "C-style cast-shaped expressions are conservative investigation signals and can include parenthesized expressions."],
            "functions": output}

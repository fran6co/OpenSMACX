"""Source identity/build-host consistency, not compiled ownership proof.

Seeds are evidence alternatives, never production definitions. A compilation
database describes intended build inputs; only separate compiler/link checks
can establish what was actually emitted and used.
"""

import base64
from collections import Counter, defaultdict
from hashlib import sha256
from pathlib import Path
import re

from .model import digest
from .records import parse_json
from .compilation_database import entries
from .source_config import SourceConfiguration
from .source_inventory import source_snapshot, assert_snapshot_current
from .component_original_routing import (component_record_identity_candidates,
                                         exact_component_record_bindings)


RULES = ("ambiguous_source", "unbuilt_host", "seed_build_input", "claimed_forwarder", "body_reference",
         "context_reference", "unbound_source")
_LITERAL = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|(?:u8|u|U|L)?R"(?P<delimiter>[^\s()\\]{0,16})'
    r'\([\s\S]*?\)(?P=delimiter)"|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\'')


def forwarder_sites(text: str, macros: tuple[str, ...]) -> tuple[list, list]:
    """Find literal-address macro calls, excluding comments and literals.

    Backslash-newline splicing precedes lexical masking; preserve original
    offsets for navigation. This is not preprocessing: conditional branches
    remain visible, and nonliteral address arguments remain unknown.
    """
    if not macros:
        return [], []
    positions, chunks, last = [], [], 0
    for match in re.finditer(r"\\\r?\n", text):
        chunks.append(text[last:match.start()])
        positions.extend(range(last, match.start()))
        last = match.end()
    chunks.append(text[last:])
    positions.extend(range(last, len(text)))
    logical = "".join(chunks)
    code = _LITERAL.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), logical)
    calls = re.compile(r"\b(" + "|".join(re.escape(name) for name in macros) + r")\s*\(")
    number = re.compile(r"\s*(0[xX][0-9A-Fa-f]+|0[0-7]*|[1-9][0-9]*)(?:[uUlL]*)\s*,")
    sites, errors = [], []
    for match in calls.finditer(code):
        prefix = code[code.rfind("\n", 0, match.start()) + 1:match.start()]
        if re.fullmatch(r"\s*#\s*define\s+", prefix):
            continue  # The configured macro's own parameter declaration.
        row = {"macro": match[1], "line": text.count("\n", 0, positions[match.start()]) + 1}
        argument = number.match(code, match.end())
        if argument is None:
            errors.append({**row, "reason": "forwarder address is not one literal integer argument"})
        else:
            value = argument[1]
            radix = 16 if value.lower().startswith("0x") else 8 if value.startswith("0") else 10
            sites.append({**row, "address": int(value, radix)})
    return sites, errors


def build_units(root: Path, data: bytes) -> tuple[set[str], list[str]]:
    """Resolve compilation database file identities without executing commands."""
    units, external = set(), set()
    for entry in entries(data, root):
        if not entry.directory.is_dir():
            raise ValueError("compilation database working directory is missing")
        path = entry.file
        if not path.is_file():
            raise ValueError(f"compilation database source is missing: {path}")
        if path.is_relative_to(root):
            units.add(path.relative_to(root).as_posix())
        else:
            external.add(str(path))
    if not units:
        raise ValueError("compilation database has no project-local build inputs")
    return units, sorted(external)


def _is_unbound(record: dict) -> bool:
    return record.get("legacy_mode") == "unbound" or record.get("legacy_state") == "unbound"


def _unbound_problem(record: dict, source: SourceConfiguration, units: set[str]) -> str | None:
    """Return why an explicit UNBOUND identity is not a permitted interface note.

    UNBOUND preserves an original identity in a configured, non-built interface
    host. It is never a body candidate, seed substitute, or symbol/body lookup.
    """
    path = record.get("path")
    if not isinstance(path, str) or not path:
        return "unbound identity has no source path"
    if record.get("candidate_kind") != "source" or source.is_seed(path):
        return "unbound identity cannot be supplied by a seed"
    try:
        relative = Path(path)
        if relative.is_absolute() or relative.as_posix() != path or not any(
                relative.is_relative_to(Path(root)) for root in source.roots):
            return "unbound identity is outside configured source roots"
    except (TypeError, ValueError):
        return "unbound identity has an invalid source path"
    if relative.suffix not in source.interface_extensions:
        return "unbound identity must be in a configured interface file"
    if path in units:
        return "unbound interface host is an accidental build input"
    if record.get("legacy_mode") != "unbound" or record.get("legacy_state") != "unbound":
        return "unbound mode and state are inconsistent"
    origin = record.get("origin")
    address = origin.get("address") if isinstance(origin, dict) else None
    if (not isinstance(record.get("entity_id"), str) or not record["entity_id"]
            or type(address) is not int or not 0 <= address <= 0xFFFFFFFF):
        return "unbound identity is missing a valid original identity"
    spans = record.get("spans")
    if not isinstance(spans, list) or not spans or record.get("invalid_spans"):
        return "unbound identity has missing or malformed original spans"
    if any(not isinstance(span, list) or len(span) != 2
           or type(span[0]) is not int or type(span[1]) is not int
           or not 0 <= span[0] < span[1] <= 0x100000000 for span in spans):
        return "unbound identity has malformed original spans"
    if (record.get("source_body_region_present") is not False
            or record.get("body_path") or record.get("context_path")
            or record.get("emitted_symbol_hint")):
        return "unbound identity cannot have a body, context, or emitted-symbol fallback"
    return None


def check_mappings(root: Path) -> dict:
    root = root.resolve()
    path = root / "recovery/project.json"
    if not path.resolve().is_relative_to(root):
        raise ValueError("project configuration resolves outside the worktree")
    project_bytes = path.read_bytes()
    project = parse_json(project_bytes)
    if (not isinstance(project, dict) or type(project.get("schema_version")) is not int
            or project["schema_version"] != 1):
        raise ValueError("unsupported project configuration version")
    source = SourceConfiguration.from_dict(project.get("source"))
    settings = project.get("mappings")
    if not isinstance(settings, dict) or set(settings) != {"compile_commands", "forwarder_macros"}:
        raise ValueError("mapping check requires compile_commands and forwarder_macros configuration")
    macros = settings["forwarder_macros"]
    if (not isinstance(macros, list) or any(not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_]\w*", name)
                                          for name in macros) or len(set(macros)) != len(macros)):
        raise ValueError("forwarder_macros must be unique C/C++ identifiers")
    database = source.path(root, settings["compile_commands"])
    database_bytes = database.read_bytes()
    package = Path(__file__).parent
    def implementation():
        return {item.relative_to(package).as_posix(): sha256(item.read_bytes()).hexdigest()
                for item in sorted(package.rglob("*.py"))}
    code = implementation()

    def refuse_aliases():
        for relative in source.roots:
            directory = source.path(root, relative)
            if any(item.is_symlink() for item in (directory, *directory.rglob("*"))):
                raise ValueError("mapping check cannot enumerate source symlinks")

    refuse_aliases()
    snapshot = source_snapshot(root)
    if SourceConfiguration.from_dict(snapshot["source_configuration"]) != source:
        raise ValueError("source configuration changed during mapping check")
    units, external = build_units(root, database_bytes)
    measurement = project.get("measurement", {})
    unit_extensions = measurement.get("unit_extensions", []) if isinstance(measurement, dict) else []
    records = snapshot["records"]
    component_owners = exact_component_record_bindings(snapshot)
    component_candidates = component_record_identity_candidates(snapshot)
    by_entity, by_file = defaultdict(list), defaultdict(list)
    for record in records:
        by_entity[record.get("entity_id")].append(record)
        by_file[record.get("path", "")].append(record)
    findings, alternatives, errors, forwarders = [], [], [], []
    for record in records:
        if _is_unbound(record):
            problem = _unbound_problem(record, source, units)
            if (record.get("path"), record.get("entity_id")) in component_candidates:
                problem = "an unbound identity collides with a component manifest interpretation"
            if problem:
                findings.append({"rule": "unbound_source", "path": record.get("path", ""),
                                 "line": record.get("line"),
                                 "entity_id": record.get("entity_id"), "reason": problem})

    # A source-owned body link is an exact project-relative identity, not a
    # request to search ancestors until some same-named file happens to exist.
    # Keep the old lexical name witness, without calling it a definition proof.
    body_references = []
    for record in records:
        name = record.get("context_path", "")
        if not name:
            continue
        row = {"path": record["path"], "line": record["line"], "context_path": name}
        try:
            target = source.path(root, name)
        except ValueError as error:
            findings.append({"rule": "context_reference", **row, "reason": str(error)})
            continue
        if (not target.is_file() or target.is_symlink()
                or target.resolve().relative_to(root).as_posix() != name):
            findings.append({"rule": "context_reference", **row,
                             "reason": "context must name an existing canonical non-symlink source file"})
        elif (not any(Path(name).is_relative_to(directory) for directory in source.roots)
              or Path(name).suffix not in unit_extensions
              or Path(name).suffix in source.interface_extensions
              or source.is_seed(name) or name not in units):
            findings.append({"rule": "context_reference", **row,
                             "reason": "context must name a configured product translation unit with a compilation recipe"})
    for record in records:
        name = record["body_path"]
        if not name:
            continue
        row = {"path": record["path"], "line": record["line"], "body_path": name}
        try:
            target = source.path(root, name)
        except ValueError as error:
            findings.append({"rule": "body_reference", **row, "reason": str(error)})
            continue
        if not target.is_file():
            findings.append({"rule": "body_reference", **row, "reason": "body reference does not exist"})
        elif name not in snapshot["files"]:
            errors.append({**row, "reason": "body reference is outside configured source coverage"})
        else:
            text = base64.b64decode(snapshot["files"][name]["content_base64"]).decode("utf-8")
            symbol = record["emitted_symbol_hint"]
            if source.symbol_format == "msvc":
                from .source.mangled import identifier
                symbol = identifier(symbol)
            if not symbol:
                errors.append({**row, "reason": "body reference has no interpretable identifier"})
            elif symbol not in text:
                findings.append({"rule": "body_reference", **row, "reason": "body reference does not mention mapped identifier",
                                 "identifier": symbol})
            else:
                body_references.append({**row, "identifier": symbol})

    def location(record):
        return {key: record.get(key) for key in ("path", "line", "candidate_kind", "legacy_byte_exact", "legacy_semantic")}

    for identity, candidates in sorted(by_entity.items()):
        product = [row for row in candidates if not source.is_seed(row.get("path", ""))]
        unbound = [row for row in candidates if _is_unbound(row)]
        if unbound and len(candidates) > 1:
            details = {"entity_id": identity, "origin": candidates[0].get("origin"),
                       "candidates": [location(row) for row in candidates],
                       "reason": "an unbound identity cannot be adopted as or compete with a body candidate"}
            findings.append({"rule": "ambiguous_source", **details})
            continue
        if len(candidates) > 1:
            details = {"entity_id": identity, "origin": candidates[0]["origin"],
                       "candidates": [location(row) for row in candidates]}
            # The native binder deliberately retains compatible seed evidence.
            # An explicit product interpretation can own the definition without
            # deleting those hints. This is not permission to borrow a seed's
            # claim or silently choose between product owners/boundaries.
            # A retained claimed seed is compatible only when the product
            # carries every historical obligation too. Matching checks still
            # independently remeasure those claims; this is ownership, not proof.
            component_owner = (len(product) == 1 and
                               (product[0]["path"], product[0]["entity_id"]) in component_owners)
            adopted = (len(product) == 1 and (bool(product[0]["binding_evidence"]) or component_owner)
                       and bool(product[0]["spans"]) and not product[0]["invalid_spans"]
                       and all(row["spans"] == product[0]["spans"]
                               and not row["invalid_spans"]
                               and (not row["legacy_byte_exact"] or product[0]["legacy_byte_exact"])
                               and (not row["legacy_semantic"] or product[0]["legacy_semantic"]
                                    or product[0]["legacy_byte_exact"])
                               for row in candidates if source.is_seed(row["path"])))
            if (product and not adopted) or len({row["path"] for row in candidates}) != len(candidates):
                findings.append({"rule": "ambiguous_source", **details})
            else:
                if adopted:
                    details["product_owner"] = location(product[0])
                alternatives.append(details)
    for name, candidates in sorted(by_file.items()):
        component_owned = bool(candidates) and all(
            (name, row["entity_id"]) in component_owners for row in candidates)
        unbound_only = bool(candidates) and all(_is_unbound(row) for row in candidates)
        if not source.is_seed(name) and name not in units and not component_owned and not unbound_only:
            findings.append({"rule": "unbuilt_host", "path": name,
                             "entities": sorted({row["entity_id"] for row in candidates})})
    for name in sorted(units):
        if source.is_seed(name):
            findings.append({"rule": "seed_build_input", "path": name})

    by_address = {
        candidates[0]["origin"]["address"]: candidates
        for candidates in by_entity.values()
        if (not any(_is_unbound(row) for row in candidates)
            and isinstance(candidates[0].get("origin"), dict)
            and type(candidates[0]["origin"].get("address")) is int)
    }
    for name, content in sorted(snapshot["files"].items()):
        if source.is_seed(name):
            continue
        text = base64.b64decode(content["content_base64"]).decode("utf-8")
        sites, unknown = forwarder_sites(text, tuple(macros))
        errors.extend({"path": name, **row} for row in unknown)
        for row in sites:
            candidates = by_address.get(row["address"], [])
            claims = [item for item in candidates if item["legacy_byte_exact"] or item["legacy_semantic"]]
            claimed = bool(claims)
            category = ("compiled_claim" if any(item["path"] in units for item in claims) else
                        "unbuilt_product_claim" if any(not source.is_seed(item["path"]) for item in claims) else
                        "seed_claim")
            forwarders.append({"path": name, **row, "claimed": claimed})
            if claimed:
                findings.append({"rule": "claimed_forwarder", "path": name, **row,
                                 "classification": category, "candidates": [location(item) for item in candidates]})
    assert_snapshot_current(root, snapshot)
    refuse_aliases()
    if (path.read_bytes() != project_bytes or database.read_bytes() != database_bytes
            or implementation() != code):
        raise ValueError("mapping policy, build inputs or checker changed during verification")
    counts = Counter(row["rule"] for row in findings)
    return {
        "schema_version": 1, "check": "source_build_mappings",
        "result": "fail" if findings else "unknown" if errors else "pass",
        "annotations": len(records), "entities": len(by_entity), "build_units": len(units),
        "rules": [{"name": name, "failures": counts[name]} for name in RULES],
        "findings": findings, "errors": errors, "seed_alternatives": alternatives,
        "component_owned_records": [
            {"path": path, "entity_id": entity, "component": owner[0],
             "symbol": owner[2].symbol, "unit": owner[2].unit}
            for (path, entity), owner in sorted(component_owners.items())],
        "forwarders": forwarders, "external_build_units": external,
        "body_references": body_references,
        "input_fingerprint": digest({"source": {name: row["sha256"] for name, row in snapshot["files"].items()},
                                     "image_sha256": snapshot["image_sha256"],
                                     "project": sha256(project_bytes).hexdigest(),
                                     "database": sha256(database_bytes).hexdigest(),
                                     "implementation": code}),
        "persisted": False, "ready_for_collection": False,
        "limitations": ["source mappings and intended compilation inputs, not compiled symbols or link ownership",
                        "compilation databases do not establish included/generated source coverage",
                        "seed alternatives remain hints; all historical claims still require separate remeasurement",
                        "forwarder macros are a lexical lens, not preprocessing, reachability or complete fixed-address detection",
                        "body paths require an in-scope text witness, not proof of a compiled definition",
                        "component, global, import and generated-table bindings require their separate checks"],
    }

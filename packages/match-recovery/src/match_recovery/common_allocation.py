"""Bounded MSVC common storage in a candidate PE, never original bindings.

COFF common values are size requests. A complete conservative input population
with uniform requests and no overriding definitions bounds the selected size
without guessing which ambiguous archive member won. A final common placement
and actual zero-filled writable PE storage must independently agree. Conflicting
sizes remain unsupported: the largest *unselected* request is not a link fact.
"""

from collections import Counter
import hashlib
from pathlib import PurePosixPath, PureWindowsPath
import re

from .link_file_evidence import parse_link_files
from .link_input_coverage import _input_inventory
from .model import Result, Span
from .msvc_link_map import parse_link_map
from .pe_coff import PeImage


def _tokens(text):
    # COFF directive strings use quoted Windows words, not POSIX backslash
    # escaping. Reject incomplete quoting instead of dropping unknown suffixes.
    pattern = re.compile(r'(?:[^\s"\x00]+|"[^"\x00]*")+')
    words, end = [], 0
    for match in pattern.finditer(text):
        if text[end:match.start()].strip():
            raise ValueError("unsupported linker directive quoting")
        words.append(match.group().replace('"', ''))
        end = match.end()
    if text[end:].strip():
        raise ValueError("unsupported linker directive quoting")
    return words


def _option(word):
    """Small fail-closed policy for allocation-neutral options.

    Input selection options are safe only with the separately complete input
    population. Definition files, aliases, forced links and unknown extensions
    are intentionally not silently treated as neutral.
    """
    if not word or word[0] not in "/-" or any(ord(c) < 32 or ord(c) >= 127 for c in word):
        return False
    option = word[1:]
    if option.lower() in {"nologo", "nodefaultlib", "incremental:no", "opt:noref", "opt:ref",
                          "opt:icf", "opt:noicf", "fixed:no", "machine:x86", "verbose", "mapinfo:lines",
                          "debug", "debugtype:coff", "debugtype:both", "debugtype:cv,fixup", "pdbtype:sept"}:
        return True
    key, separator, value = option.partition(":")
    if not separator or not value:
        return False
    key = key.lower()
    if key in {"out", "implib", "pdb", "map", "libpath", "defaultlib", "nodefaultlib", "disallowlib"}:
        return '"' not in value
    if key == "version":
        return re.fullmatch(r"[0-9]+(?:\.[0-9]+)?", value) is not None
    if key == "subsystem":
        return value.lower() in {"console", "windows"}
    if key == "export" and value.lower().endswith(",data"):
        value = value[:-5]
    if key in {"entry", "export", "include"}:
        return re.fullmatch(r'[^\s=,"\x00]+', value) is not None
    if key == "merge":
        return re.fullmatch(r"[.A-Za-z_$][.A-Za-z_0-9$]*=[.A-Za-z_$][.A-Za-z_0-9$]*", value) is not None
    return False


def _policy(evidence, records):
    gaps = []
    for word in evidence["arguments"]:
        # The compiler boundary already resolved explicit files and library
        # searches. A bare file is not an option; response files are unsupported.
        if not word.startswith(("/", "-", "@")) or word in evidence["explicit_inputs"]:
            if PureWindowsPath(word).suffix.lower() in {".obj", ".lib", ".exp"}:
                continue
        if not _option(word):
            gaps.append({"origin": "arguments", "option": word})
    for row in records:
        for directive in row["directives"]:
            try:
                words = _tokens(directive["text"])
                gaps.extend({"origin": row["path"], "member": row.get("member"), "option": word}
                            for word in words if not _option(word))
            except ValueError as error:
                gaps.append({"origin": row["path"], "member": row.get("member"), "reason": str(error)})
    return gaps


def _symbols(symbols):
    if not symbols or len(set(symbols)) != len(symbols) or any(
            not isinstance(name, str) or not name or any(ord(c) < 33 or ord(c) >= 127 for c in name)
            for name in symbols):
        raise ValueError("common allocation requires unique nonempty symbol names")


def verify_common_allocations(data: bytes, log_data: bytes, image_data: bytes, map_data: bytes,
                              symbols: tuple[str, ...]) -> dict:
    """Re-read physical inputs; supplied output bytes alone prove no freshness.

    The component command owns the fresh build and pins/rechecks its artifacts.
    This API never accepts a replayed inventory report as absence evidence.
    """
    _symbols(symbols)
    coverage, records = _input_inventory(data, log_data)
    evidence = parse_link_files(data)
    image_pin = hashlib.sha256(image_data).hexdigest()
    image = PeImage(image_data, image_pin)
    mapping = parse_link_map(map_data)
    return {**_verify_common_inputs(evidence, coverage, records, image, mapping, symbols),
            "map_sha256": hashlib.sha256(map_data).hexdigest()}


def _verify_common_inputs(evidence, coverage, records, image, mapping, symbols):
    """Observe already parsed native inputs without granting residual coverage.

    The enclosing native caller must establish complete physical population,
    immutable parsed evidence and live artifact/input guards before and after
    use. This helper does not authenticate caller-supplied dictionaries, saved
    reports or permissions. Its result is an observation, never a capability.
    Only the bytes wrapper can include the original map-file digest.
    """
    _symbols(symbols)
    policy_gaps = _policy(evidence, records)
    image_pin = hashlib.sha256(image.data).hexdigest()
    mapping.validate(image)
    publics = {row.symbol: row for row in mapping.publics}
    results = []
    for name in symbols:
        declarations, blockers = [], []
        for record in records:
            identity = {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}
            for symbol in record["symbols"]:
                kind = symbol["declaration"]
                if symbol["name"] == name and kind not in {"undefined", "local_or_metadata"}:
                    item = {**identity, "declaration": kind, "size": symbol["value"]}
                    if (kind == "common" and symbol["type"] == 0 and symbol["auxiliary_size"] == 0
                            and record["machine"] == 0x14C and record["optional_header_size"] == 0):
                        declarations.append(item)
                    else:
                        blockers.append(item)
                elif kind == "weak" and symbol.get("fallback") == name:
                    blockers.append({**identity, "declaration": "weak_fallback", "symbol": symbol["name"]})
            for imported in record["imports"]:
                if name in {imported["symbol"], "__imp_" + imported["symbol"]}:
                    blockers.append({**identity, "declaration": "short_import", "symbol": imported["symbol"]})
        row = {"symbol": name, "requests": declarations, "blockers": blockers, "result": Result.UNKNOWN}
        try:
            if coverage["result"] != Result.PASS or policy_gaps:
                raise ValueError("incomplete input population or unsupported linker policy")
            if blockers or not declarations:
                raise ValueError("missing common requests or potentially overriding definitions/aliases")
            sizes = {item["size"] for item in declarations}
            if len(sizes) != 1:
                raise ValueError("conflicting requests require exact selected-input size analysis")
            size, = sizes
            public = publics.get(name)
            if public is None or not public.segment or public.function or public.owner != "<common>":
                raise ValueError("missing data-only common placement")
            row.update(address=public.address, size=size)
            # At least one request must be from an explicit object or a unique
            # final archive candidate. Do not promote an unselected-only hint.
            def selected(item):
                if "member" not in item:
                    return "generated" not in item and item["path"] in evidence["explicit_inputs"]
                matches = [entry for entry in coverage["inputs"] if entry["selection"] == Result.PASS
                           and entry["library"] is not None and entry["paths"] == [item["path"]]
                           and PureWindowsPath(entry["member"]).name.casefold()
                           == PurePosixPath(item["member"]).name.casefold()]
                return len(matches) == 1
            if not any(selected(item) for item in declarations):
                raise ValueError("no common request has a unique participating input")
            span = Span(public.address, public.address + size)
            flags = image.section_flags[public.segment - 1]
            if flags & 0x20000000 or flags & 0xC0000000 != 0xC0000000:
                raise ValueError("common allocation is not in readable writable non-executable storage")
            # Bound before allocating zero bytes, including adversarial huge
            # requests; read_initial also refuses code sections and cross-seams.
            region = image.sections[public.segment - 1][0]
            if span.end > region.end or size > 16 * 1024 * 1024:
                raise ValueError("common allocation exceeds its section or verification resource bound")
            # Preserve independently selected, bounded storage even when a
            # later public/contents check refuses. Allocation consumers must
            # not lose an overlapping competitor merely because it failed.
            row["extent_verification"] = Result.PASS
            if any(other.symbol != name and span.start <= other.address < span.end for other in mapping.publics):
                raise ValueError("common allocation overlaps another public placement")
            initial = image.read_initial(span)
            if image.absolute_sites(span):
                raise ValueError("common allocation contains loader relocations")
            row.update(result=Result.PASS if not any(initial) else Result.FAIL,
                       initialization=Result.PASS if not any(initial) else Result.FAIL,
                       basis="uniform_complete_requests_and_selected_common_placement")
        except ValueError as error:
            row["reason"] = str(error)
        results.append(row)
    counts = Counter(row["result"] for row in results)
    return {"check": "linked_common_allocations", "result": Result.FAIL if counts[Result.FAIL] else
            Result.UNKNOWN if counts[Result.UNKNOWN] else Result.PASS, "allocations": results, "counts": dict(counts),
            "image_sha256": image_pin,
            "input_coverage": coverage, "policy_gaps": policy_gaps,
            "original_verification": Result.UNKNOWN, "runtime_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["candidate common size and pre-CRT zero initialization, not original ownership or runtime state",
                            "uniform conservative requests only; conflicting sizes, strong overrides and aliases are refused",
                            "no original application target bindings are published"]}

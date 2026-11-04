"""Bound selected code regions without guessing ordinary function boundaries.

An ordinary section may contain assembler entry labels and jump vectors. Its
external entries jointly place the entire section, not inferred functions.
Leading bytes before the first entry remain covered. Other code/data sections
and runtime behavior remain unproved.
"""

from hashlib import sha256
from pathlib import Path

from .binary import RelocationKind
from .linked_function import _verify_region, verify_function
from .model import Result, Span, digest
from .pe_coff import Coff, unaddressed_end_label, weak_external


_LIVE_WEAK_TOKEN = object()


class _WeakTargetBinding:
    """One unpersistable weak-edge qualification, consumed by one code region."""

    __slots__ = ("address", "details", "_scope", "_object", "_object_state", "_owner", "_section", "_subject",
                 "_weak_index", "_offset", "_kind", "_source_unit", "_receipt")

    def __init__(self, token, scope, obj, owner, section, subject, weak_index, offset, kind, address, details):
        if token is not _LIVE_WEAK_TOKEN:
            raise TypeError("weak target bindings require a live linked-code observation")
        self._scope, self._object = scope, obj
        self._object_state = (obj.__dict__.copy(), sha256(obj.raw_data).hexdigest())
        self._owner, self._section, self._subject = owner, section, subject
        self._weak_index, self._offset, self._kind = weak_index, offset, kind
        self._source_unit, self._receipt = scope.source_receipt(owner, obj)
        self.address, self.details = address, details

    def __reduce__(self):
        raise TypeError("live weak target bindings cannot be serialized")

    def consume(self, obj, subject, owner, relocation):
        scope = self._scope
        scope.require_live()
        if (obj.__dict__ != self._object_state[0] or sha256(obj.raw_data).hexdigest() != self._object_state[1]
                or subject != self._subject or owner != self._owner
                or subject.section != self._section or relocation.target != str(self._weak_index)
                or relocation.offset != self._offset or relocation.kind != self._kind):
            raise ValueError("weak target qualification belongs to another caller reference")
        return self.address, dict(self.details)

    def original_target(self, binding, obj, subject, relocation, fallback_binding, fallback_row):
        scope = self._scope
        scope.require_live()
        if (self.consume(obj, subject, self._owner, relocation)[0] != self.address
                or binding.unit != self._source_unit or binding.symbol != self._subject.name
                or binding.origin.entity_id == fallback_binding.origin.entity_id
                or fallback_binding.unit != self._source_unit
                or fallback_binding.symbol != self.details["fallback_symbol"]
                or len(fallback_binding.spans) != 1 or fallback_binding.source_spans
                or fallback_binding.local_only or fallback_row.get("entity_id") != fallback_binding.origin.entity_id
                or fallback_row.get("result") != "pass"):
            raise ValueError("weak original target lacks a strict same-unit fallback source comparison")
        return _OriginalWeakTarget(_LIVE_WEAK_TOKEN, scope, self, binding, fallback_binding,
                                   relocation, fallback_row)


class _OriginalWeakTarget:
    """One in-memory original target, scoped to one strict weak relocation."""

    __slots__ = ("address", "details", "_scope", "_current", "_binding", "_fallback", "_offset", "_index",
                 "_used")

    def __init__(self, token, scope, current, binding, fallback, relocation, fallback_row):
        if token is not _LIVE_WEAK_TOKEN:
            raise TypeError("original weak targets require a live linked source comparison")
        self._scope, self._current = scope, current
        self._binding, self._fallback = binding, fallback
        self._offset, self._index = relocation.offset, int(relocation.target)
        self.address = fallback.origin.address
        self.details = {"current_edge": dict(current.details),
                        "fallback_entity_id": fallback.origin.entity_id,
                        "fallback_original_address": self.address,
                        "fallback_result": fallback_row["result"],
                        "source_receipt_sha256": current._receipt}
        self._used = False

    def __reduce__(self):
        raise TypeError("live original weak targets cannot be serialized")

    def consume(self, binding, obj, subject, relocation):
        self._scope.require_live()
        if self._used:
            raise ValueError("original weak target capability was already consumed")
        if (binding != self._binding or obj.__dict__ != self._current._object_state[0]
                or sha256(obj.raw_data).hexdigest() != self._current._object_state[1]
                or subject != self._current._subject or subject.section != self._current._section
                or relocation.offset != self._offset or relocation.target != str(self._index)
                or relocation.kind != self._current._kind):
            raise ValueError("original weak target belongs to another caller row or relocation")
        self._used = True
        return self.address, dict(self.details)


class _LiveWeakReferenceScope:
    """Private capability minted only inside one expiring LinkedBuild.code call."""

    __slots__ = ("_objects", "_mapping", "_image", "_context", "_fresh", "_verified_fallbacks",
                 "_weak_bindings", "_current_results", "_original_consumed", "_refused_current_edges",
                 "_refused_source_roots", "_live")

    def __init__(self, token, objects, mapping, image, context=None, fresh=None):
        if token is not _LIVE_WEAK_TOKEN:
            raise TypeError("live weak qualification is available only from LinkedBuild.code")
        self._objects, self._mapping, self._image, self._context, self._fresh = objects, mapping, image, context, fresh
        self._verified_fallbacks = {}
        self._weak_bindings = {}
        self._current_results = {}
        self._original_consumed = set()
        self._refused_current_edges = {}
        self._refused_source_roots = {}
        self._live = True

    @staticmethod
    def _edge_key(obj, subject, relocation, owner):
        return (owner.casefold(), sha256(obj.raw_data).hexdigest(), subject.name, subject.section,
                int(relocation.target), relocation.offset, relocation.kind)

    @staticmethod
    def _binding_edge_key(binding):
        return (binding._owner.casefold(), binding._object_state[1], binding._subject.name,
                binding._section, binding._weak_index, binding._offset, binding._kind)

    def refuse_current_edge(self, obj, subject, relocation, owner, reason):
        self.require_live()
        key = self._edge_key(obj, subject, relocation, owner)
        self._refused_current_edges[key] = reason
        self.refuse_source_root(owner, subject.name, reason)
        for binding in self._weak_bindings.values():
            if self._binding_edge_key(binding) == key:
                current = self._current_results.get(id(binding))
                self._current_results[id(binding)] = Result.FAIL if current == Result.FAIL else Result.UNKNOWN

    def refuse_source_root(self, owner, symbol, reason):
        self.require_live()
        key = (owner.casefold(), symbol)
        self._refused_source_roots[key] = reason
        for binding in self._weak_bindings.values():
            if (binding._owner.casefold(), binding._subject.name) == key:
                current = self._current_results.get(id(binding))
                self._current_results[id(binding)] = Result.FAIL if current == Result.FAIL else Result.UNKNOWN

    def __reduce__(self):
        raise TypeError("live weak qualification scopes cannot be serialized")

    def require_live(self):
        if not self._live:
            raise ValueError("live weak reference scope has expired")

    def expire(self):
        self._live = False

    def source_receipt(self, owner, obj):
        self.require_live()
        if self._context is None:
            return None, None
        report = self._context.report
        compile_link = report.get("compile_link") if isinstance(report, dict) else None
        if not isinstance(compile_link, dict) or compile_link.get("result") != Result.PASS:
            return None, None
        selected = self._objects.resolve(owner)
        identity = selected["identity"]
        if set(identity) != {"path"}:
            return None, None
        matches = [row for row in compile_link.get("compilations", ())
                   if row.get("result") == Result.PASS and row.get("link_correspondence") == Result.PASS
                   and row.get("output") == identity["path"]
                   and row.get("compilation", {}).get("output", {}).get("after") == {
                       "sha256": selected["sha256"], "size": len(selected["raw"])}]
        commands = compile_link.get("command_bindings", {})
        command_matches = [row for row in commands.get("compilations", ())
                           if row.get("result") == Result.PASS
                           and row.get("output") == identity["path"]]
        if (len(matches) != 1 or len(command_matches) != 1
                or command_matches[0].get("source") != matches[0].get("source")
                or not command_matches[0].get("arguments")):
            return None, None
        source = matches[0]["source"]
        try:
            unit = Path(source).relative_to(self._context.root).as_posix()
        except (ValueError, OSError):
            return None, None
        receipt = sha256(repr((matches[0], command_matches[0])).encode()).hexdigest()
        if Coff(selected["raw"]).__dict__ != obj.__dict__:
            raise ValueError("weak caller COFF differs from its receipt-bound physical source object")
        return unit, receipt

    def bind(self, obj, subject, owner, relocation, *, objects, mapping, image, fixups, index,
             constants, codeview):
        self.require_live()
        if objects is not self._objects or mapping is not self._mapping or image is not self._image:
            raise ValueError("weak target qualification belongs to another live linked build")
        target = obj.symbols.get(int(relocation.target))
        if (target is None or target.storage != 105 or target.type != 0x20
                or obj.symbols.get(target.index) != target):
            raise ValueError("weak target qualification does not name one exact COFF weak external")
        edge_key = self._edge_key(obj, subject, relocation, owner)
        root_reason = self._refused_source_roots.get((owner.casefold(), subject.name))
        if root_reason is not None:
            raise ValueError("weak source root was refused: " + root_reason)
        if edge_key in self._refused_current_edges:
            raise ValueError("weak current edge was refused: " + self._refused_current_edges[edge_key])
        if not obj.sections[subject.section - 1].flags & 0x1000:
            raise ValueError("weak fallback qualification requires one independently bounded COMDAT caller")
        code = obj.sections[subject.section - 1].data
        if (relocation.kind != RelocationKind.RELATIVE32 or relocation.offset + 4 > len(code)
                or int.from_bytes(code[relocation.offset:relocation.offset + 4], "little", signed=True) != 0):
            raise ValueError("weak fallback qualification supports only zero-addend relative code references")
        fallback, mode = weak_external(obj, target)
        if (mode != 2 or fallback.storage != 2 or fallback.section != 0 or fallback.value != 0
                or fallback.type != 0x20 or obj.auxiliary.get(fallback.index)):
            raise ValueError("weak fallback is not one supported mode-2 external function reference")
        weak_declarations = [item for item in obj.symbols.values()
                             if item.name == target.name and item.storage == 105]
        same_name = [item for item in obj.symbols.values() if item.name == fallback.name]
        definitions = [item for item in same_name if item.storage == 2 and item.section > 0
                       and item.type == 0x20 and not obj.auxiliary.get(item.index)]
        if (len(weak_declarations) != 1 or weak_declarations[0] != target
                or len(same_name) != 2 or fallback not in same_name or len(definitions) != 1):
            raise ValueError("weak declaration or same-object fallback definition is ambiguous")
        definition, = definitions
        if (not 1 <= definition.section <= len(obj.sections)
                or definition.value >= len(obj.sections[definition.section - 1].data)
                or not obj.sections[definition.section - 1].flags & 0x20):
            raise ValueError("weak fallback definition is not a bounded code entry")

        selected = objects.resolve(owner)
        if (selected["owner"].casefold() != owner.casefold()
                or selected["sha256"] != sha256(selected["raw"]).hexdigest()
                or Coff(selected["raw"]).__dict__ != obj.__dict__):
            raise ValueError("weak caller COFF differs from its fresh selected physical owner")
        from .linked_map_index import index_for
        lookup = index_for(mapping, index)
        weak_rows = lookup.publics.get(target.name, ())
        fallback_rows = lookup.publics.get(fallback.name, ())
        if (len(weak_rows) != 1 or len(fallback_rows) != 1
                or any(not row.function or not row.segment for row in (*weak_rows, *fallback_rows))
                or weak_rows[0].address != fallback_rows[0].address
                or weak_rows[0].owner.casefold() != owner.casefold()
                or fallback_rows[0].owner.casefold() != owner.casefold()):
            raise ValueError("weak and fallback symbols lack unique same-owner equal-VA placements")

        key = (owner.casefold(), selected["sha256"], definition.index)
        proof = self._verified_fallbacks.get(key)
        if proof is None:
            from .linked_code_region import verify_code_region
            proof = verify_code_region(obj, definition, owner, mapping, image, objects=objects,
                                       fixups=fixups, _index=index, _constants=constants,
                                       codeview=codeview)
            if proof["result"] != Result.PASS:
                raise ValueError("weak fallback body or references are not independently linked-code verified")
            self._verified_fallbacks[key] = proof
        details = {
            "kind": "coff_mode2_effective_fallback",
            "weak_symbol": target.name,
            "weak_symbol_index": target.index,
            "fallback_symbol": fallback.name,
            "fallback_undefined_index": fallback.index,
            "fallback_definition_index": definition.index,
            "search_mode": mode,
            "relocation_offset": relocation.offset,
            "relocation_kind": relocation.kind,
            "caller": {"owner": owner, "symbol": subject.name, "section": subject.section,
                       "object_identity": selected["identity"], "object_sha256": selected["sha256"]},
            "fallback": {"owner": owner, "symbol": definition.name, "section": definition.section,
                         "object_identity": selected["identity"], "object_sha256": selected["sha256"],
                         "address": proof["address"], "size": proof["size"],
                         "body_fingerprint": proof["body_fingerprint"], "result": proof["result"]},
            "map": {"weak_address": weak_rows[0].address, "fallback_address": fallback_rows[0].address},
            "selection_authority": "bounded same-owner body/reference proof only; retained-section and whole-link selection remain unknown"
        }
        binding = _WeakTargetBinding(_LIVE_WEAK_TOKEN, self, obj, owner, subject.section, subject,
                                     target.index, relocation.offset, relocation.kind,
                                     fallback_rows[0].address, details)
        key = (owner.casefold(), selected["sha256"], subject.name, subject.section,
               target.index, relocation.offset, relocation.kind)
        self._weak_bindings[key] = binding
        return binding

    def qualify_original(self, obj, subject, relocation, binding, fallback_binding, fallback_row):
        self.require_live()
        if self._fresh is not None:
            self._fresh()
        edge_tail = (sha256(obj.raw_data).hexdigest(), subject.name, subject.section,
                     int(relocation.target), relocation.offset, relocation.kind)
        if any(key[1:] == edge_tail for key in self._refused_current_edges):
            raise ValueError("weak current edge was refused by source-region resource proof")
        object_sha = sha256(obj.raw_data).hexdigest()
        candidates = [current for key, current in self._weak_bindings.items()
                      if key[1] == object_sha and key[2:] == (subject.name, subject.section,
                          int(relocation.target), relocation.offset, relocation.kind)]
        if len(candidates) != 1:
            raise ValueError("weak original comparison has no matching live current-edge proof")
        current, = candidates
        if self._current_results.get(id(current)) != Result.PASS:
            raise ValueError("current linked caller relocation did not compare successfully")
        unit, receipt = current._source_unit, current._receipt
        if unit is None or receipt is None:
            raise ValueError("weak original comparison lacks a source-bound compile and linker receipt")
        identity = (id(current), binding.origin.entity_id, fallback_binding.origin.entity_id)
        if identity in self._original_consumed:
            raise ValueError("weak original target capability was already consumed")
        target = current.original_target(binding, obj, subject, relocation, fallback_binding, fallback_row)
        if fallback_binding.unit != unit:
            raise ValueError("weak fallback source unit differs from the selected caller receipt")
        self._original_consumed.add(identity)
        return target

    def confirm_current(self, binding, result):
        self.require_live()
        if binding not in self._weak_bindings.values() or result not in (Result.PASS, Result.FAIL, Result.UNKNOWN):
            raise ValueError("current weak proof does not belong to this live scope")
        if self._binding_edge_key(binding) in self._refused_current_edges:
            result = Result.FAIL if result == Result.FAIL else Result.UNKNOWN
        self._current_results[id(binding)] = result


def _live_weak_reference_scope(objects, mapping, image, context=None, fresh=None):
    return _LiveWeakReferenceScope(_LIVE_WEAK_TOKEN, objects, mapping, image, context, fresh)


def _ordinary_symbols(obj, subject):
    section = obj.sections[subject.section - 1]
    if (subject.storage != 2 or not 0 <= subject.value < len(section.data) or not subject.type & 0x20
            or not section.flags & 0x20 or section.flags & 0x1000
            or not section.data or len(section.data) != obj.section_sizes[subject.section - 1]
            or obj.code_sections(subject) != (subject.section,)):
        raise ValueError("ordinary linked code requires an exact section and bounded external entry")
    members = [item for item in obj.symbols.values() if item.section == subject.section]
    rows = []
    for item in members:
        if (item.storage not in (2, 3, 6)
                or not (0 <= item.value < len(section.data) or unaddressed_end_label(obj, item))
                or item.type not in (0, 0x20)
                or item.storage == 2 and (item.type != 0x20 or obj.auxiliary.get(item.index))):
            raise ValueError("ordinary linked section has unsupported or unbounded internal symbols")
        if obj.auxiliary.get(item.index) and not (item.name == section.name and item.storage == 3
                                                  and item.value == 0 and item.type == 0):
            raise ValueError("ordinary section has unsupported auxiliary symbol metadata")
        # Preserve distinct names, kinds, offsets and multiplicity. Static
        # function-typed labels do not imply independently bounded functions.
        rows.append({"name": item.name, "offset": item.value, "type": item.type, "storage": item.storage})
    return sorted(rows, key=lambda row: (row["offset"], row["name"], row["type"], row["storage"]))


def _ordinary_placement(obj, subject, owner, mapping, image, objects, *, _index=None):
    from .linked_map_index import index_for
    lookup = index_for(mapping, _index)
    entries = [item for item in obj.symbols.values() if item.section == subject.section and item.storage == 2]
    if len(entries) > 1 or subject.value:
        from .linked_fixups import check_selected
        check_selected(obj, owner, objects)
    bases, names = set(), set()
    for entry in entries:
        if entry.name in names:
            raise ValueError("ordinary section has ambiguous external entry names")
        names.add(entry.name)
        matches = lookup.publics.get(entry.name, ())
        if (len(matches) != 1 or not matches[0].function or not matches[0].segment
                or matches[0].owner.casefold() != owner.casefold()):
            raise ValueError("ordinary entry lacks a unique same-owner code placement")
        bases.add(matches[0].address - entry.value)
    if len(bases) != 1:
        raise ValueError("ordinary entries disagree on their shared section base")
    base, = bases
    for item in obj.symbols.values():
        if item.section != subject.section or item.storage not in (3, 6) or obj.auxiliary.get(item.index):
            continue
        matches = lookup.statics.get((owner.casefold(), item.name), ())
        if matches and (len(matches) != 1 or not matches[0].segment
                        or matches[0].address != base + item.value):
            raise ValueError("ordinary internal symbol has contradictory map placement")
    span = Span(base, base + len(obj.sections[subject.section - 1].data))
    containing = [flags for (region, _, _), flags in zip(image.sections, image.section_flags)
                  if region.start <= span.start and span.end <= region.end]
    if len(containing) != 1 or not containing[0] & 0x20 or not containing[0] & 0x20000000:
        raise ValueError("ordinary section is not wholly placed in executable code")
    image.read(span)
    return base


def select_code_region(obj, name):
    matches = [item for item in obj.symbols.values() if item.name == name
               and item.section > 0 and item.type & 0x20 and item.storage in (2, 3)]
    if len(matches) != 1:
        raise ValueError("linked code requires one defined named entry")
    subject, = matches
    if obj.sections[subject.section - 1].flags & 0x1000:
        return obj.function(name)
    _ordinary_symbols(obj, subject)
    return subject


def verify_code_region(obj, symbol, owner, mapping, image, *, objects=None, fixups=None,
                       folded_frames=None, _index=None, _constants=None, codeview=None,
                       _weak_scope=None):
    subject = select_code_region(obj, symbol.name)
    if subject != symbol:
        raise ValueError("linked code entry differs from its selected object symbol")
    if obj.sections[subject.section - 1].flags & 0x1000:
        if subject.section in obj.associations.values() and fixups is not None:
            from .linked_bundle import verify_bundle
            return verify_bundle(obj, subject, owner, mapping, image, objects=objects, fixups=fixups,
                                 folded_frames=folded_frames, codeview=codeview,
                                 _index=_index, _constants=_constants)
        return verify_function(obj, subject, owner, mapping, image, objects=objects, fixups=fixups,
                               _index=_index, _constants=_constants, _weak_scope=_weak_scope)
    symbols = _ordinary_symbols(obj, subject)
    from .linked_map_index import index_for
    lookup = index_for(mapping, _index)
    base = _ordinary_placement(obj, subject, owner, mapping, image, objects, _index=lookup)
    proof = _verify_region(obj, subject, owner, mapping, image, objects=objects, fixups=fixups,
                           _index=lookup, _constants=_constants, _weak_scope=_weak_scope)
    if proof["address"] != base:
        raise ValueError("ordinary code proof differs from its shared section placement")
    return {**proof, "extent_kind": "selected_code_section", "section_symbols": symbols,
            "entry_address": base + subject.value, "entry_offset": subject.value,
            "body_fingerprint": digest({"section": proof["body_fingerprint"], "symbols": symbols,
                                        "entry": subject.name, "entry_offset": subject.value}),
            "other_sections": Result.UNKNOWN}

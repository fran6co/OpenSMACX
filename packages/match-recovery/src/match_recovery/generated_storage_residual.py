"""Live ordinary/common storage closure for generated-change preservation.

Inputs are the native code observations still owned by the enclosing two-build
producer. No persisted report grants scope. Authorized root removal does not
remove its data dependencies, nor callbacks found through those dependencies.
"""

from copy import deepcopy
from hashlib import sha256
import json
from pathlib import Path

from .linked_code_ownership import _compilations, _portable
from .linked_imports import import_slots
from .model import Result
from .pe_coff import Coff


def _key(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False)


class StorageClosure:
    """One bounded paired native epoch, retaining the union across code rounds."""

    def __init__(self, live, data_producers, *, limit=4096):
        if type(limit) is not int or not 1 <= limit <= 4096 or set(live) != {"baseline", "current"}:
            raise ValueError("storage closure requires two native builds and a bounded section limit")
        self.live, self.limit = dict(live), limit
        if set(data_producers) != set(live):
            raise ValueError("storage closure requires a native producer epoch for each build")
        self.data_producers = dict(data_producers)
        self.required, self.code_required = {}, {}
        self.code_uses = {version: [] for version in live}
        self.data_uses = {version: set() for version in live}
        self.observed = {version: {} for version in live}
        self.compiled = {}
        self.slots = {}
        self._fresh()
        for version, native in live.items():
            self.compiled[version] = _compilations(native.context)
            self.slots[version] = import_slots(native.inputs.image)
        self._fresh()

    def _fresh(self):
        try:
            try:
                self.live["baseline"].check_current()
            finally:
                self.data_producers["baseline"].check_current()
        finally:
            try:
                self.live["current"].check_current()
            finally:
                self.data_producers["current"].check_current()

    def _require(self, descriptor):
        key = _key(descriptor)
        if key not in self.required and len(self.required) >= self.limit:
            raise ValueError("storage dependency closure exceeds its section bound")
        self.required[key] = deepcopy(descriptor)
        return descriptor

    def _code(self, version, owner, name):
        native = self.live[version]
        selected = native.inputs.objects.resolve(owner)
        root, directory, compiled = self.compiled[version]
        matches = [unit for unit, row in compiled.items()
                   if selected["identity"] == {"path": row["path"]}]
        if matches:
            if len(matches) != 1 or compiled[matches[0]]["sha256"] != selected["sha256"]:
                raise ValueError("data code target differs from its exact compilation receipt")
            identity = {"kind": "compiled", "unit": matches[0], "symbol": name}
        else:
            source = selected["identity"]
            identity = {"kind": "dependency", "input": _portable(Path(source["path"]), root, directory),
                        "object_sha256": selected["sha256"], "symbol": name,
                        **{key: source[key] for key in ("member", "occurrence", "generated") if key in source}}
        key = _key(identity)
        if key not in self.code_required and len(self.code_required) >= self.limit:
            raise ValueError("data-to-code dependency closure exceeds its node bound")
        self.code_required[key] = identity
        return {"code": identity}

    def _track(self, version, target, addend):
        if type(addend) is not int or not -(1 << 31) <= addend < (1 << 31):
            raise ValueError("storage reference needs its exact signed COFF addend")
        if "storage" in target:
            self.data_uses[version].add((_key(target["storage"]), target["offset"] + addend))

    def _reference(self, version, identity, address):
        """Resolve symbolic targets independently; an operand never supplies identity."""
        native = self.live[version]
        mapping, objects = native.inputs.mapping, native.inputs.objects
        if set(identity) == {"external"}:
            name, owner, storage = identity["external"], None, 2
            rows = [row for row in mapping.publics if row.symbol == name and row.segment]
        elif set(identity) in ({"local_owner", "symbol"},
                              {"local_owner", "symbol", "object_section", "object_offset"}):
            name, owner, storage = identity["symbol"], identity["local_owner"], 3
            rows = [row for row in mapping.statics or () if row.symbol == name and row.segment
                    and row.owner.casefold() == owner.casefold()]
        else:
            raise ValueError("unsupported native data-reference identity")
        if rows:
            if len(rows) != 1 or rows[0].address != address:
                raise ValueError("storage dependency contradicts its independent map placement")
            placed, = rows
            owner = placed.owner
            if placed.function:
                return self._code(version, owner, name)
            if storage == 2 and address in self.slots[version]:
                slot = self.slots[version][address]
                if placed.segment != slot["segment"]:
                    raise ValueError("storage dependency import slot has a contradictory segment")
                descriptor = {"kind": "import", "symbol": name,
                              **{key: slot[key] for key in ("module", "name", "ordinal")}}
                return {"storage": self._require(descriptor), "offset": 0}
            if storage == 2 and owner == "<common>":
                return {"storage": self._require({"kind": "common", "symbol": name}), "offset": 0}
        elif storage != 3 or any(row.symbol == name for row in
                                (*mapping.publics, *(mapping.statics or ()))):
            raise ValueError("storage dependency lacks an independent named placement")
        selected = objects.resolve(owner)
        obj = Coff(selected["raw"])
        symbols = [symbol for symbol in obj.symbols.values() if symbol.name == name
                   and symbol.storage == storage and symbol.section > 0]
        if len(symbols) != 1 or symbols[0].type != 0 or obj.auxiliary.get(symbols[0].index):
            raise ValueError("storage dependency lacks one exact selected data definition")
        symbol, = symbols
        if "object_section" in identity and (identity["object_section"] != symbol.section
                                               or identity["object_offset"] != symbol.value):
            raise ValueError("private storage dependency changed its exact object section or offset")
        descriptor = self.data_producers[version].identify(owner, symbol.section)
        envelope = native.data_section(owner, symbol.section)
        if envelope["result"] != Result.PASS or envelope["address"] + symbol.value != address:
            raise ValueError("storage dependency differs from independently anchored whole-section placement")
        return {"storage": self._require(descriptor), "offset": symbol.value,
                "symbol": name, "storage_class": storage}

    def _observe(self, version, descriptor):
        native = self.live[version]
        if descriptor["kind"] == "common":
            proof = native.common((descriptor["symbol"],))
            allocation, = proof["allocations"]
            if proof["result"] != Result.PASS:
                return {"result": proof["result"], "input_consistency": proof["result"], "proof": proof}
            regions = [flags for (span, _, _), flags in zip(native.inputs.image.sections,
                       native.inputs.image.section_flags)
                       if span.start <= allocation["address"]
                       and allocation["address"] + allocation["size"] <= span.end]
            if len(regions) != 1:
                raise ValueError("common allocation lacks one complete native image section")
            return {"result": Result.PASS, "input_consistency": Result.PASS, "proof": proof,
                    "address": allocation["address"], "size": allocation["size"],
                    "canonical": {"kind": "common", "size": allocation["size"], "initialization": "zero",
                        "flags": regions[0]}}
        if descriptor["kind"] == "import":
            matches = [slot for slot in self.slots[version].values() if all(
                slot[key] == descriptor[key] for key in ("module", "name", "ordinal"))]
            placed = [row for row in native.inputs.mapping.publics if row.symbol == descriptor["symbol"]]
            if len(placed) != 1 or placed[0].function or not placed[0].segment:
                raise ValueError("retained import dependency lost its exact map identity")
            matches = [slot for slot in matches if slot["address"] == placed[0].address
                       and slot["segment"] == placed[0].segment]
            if len(matches) != 1:
                raise ValueError("retained import dependency lost its native loader slot")
            return {"result": Result.PASS, "input_consistency": Result.PASS,
                    "address": matches[0]["address"], "size": 4,
                    "canonical": {**descriptor, "flags": native.inputs.image.section_flags[matches[0]["segment"] - 1]},
                    "implementation": Result.UNKNOWN,
                    "selection": Result.UNKNOWN}
        located = self.data_producers[version].resolve(descriptor)
        proof = native.data_contents(located["owner"], located["section"])
        if proof["result"] != Result.PASS:
            return {"result": proof["result"], "input_consistency": proof["input_consistency"], "proof": proof}
        references = []
        for row in proof["references"]:
            identity = row["identity"]
            if "absolute_constant" in identity:
                target = identity
            elif set(identity) in ({"self_offset"}, {"self_offset", "external"}):
                target = {"storage": descriptor, "offset": identity["self_offset"]}
                if "external" in identity:
                    target["external"] = identity["external"]
            else:
                target = self._reference(version, identity, row["address"])
            if "code" in target:
                self.code_uses[version].append((_key(target["code"]), row["addend"]))
            self._track(version, target, row["addend"])
            if target.get("storage", {}).get("kind") == "import":
                symbol = row["object_target"]
                if (row["kind"] != "absolute32" or row["addend"] != 0
                        or symbol["storage"] != 2 or symbol["section"] != 0 or symbol["value"] != 0):
                    raise ValueError("data-to-import edge lacks an ordinary zero-addend pointer declaration")
            references.append({"offset": row["offset"], "kind": row["kind"],
                               "addend": row["addend"], "target": target})
        envelope = proof["envelope"]
        obj = Coff(native.inputs.objects.resolve(located["owner"])["raw"])
        section = obj.sections[located["section"] - 1]
        canonical = {"kind": "ordinary", "raw_sha256": sha256(section.data).hexdigest(),
                     "size": envelope["size"], "flags": section.flags, "alignment": section.alignment,
                     "symbols": [{key: row[key] for key in ("name", "offset", "storage")}
                                 for row in envelope["symbols"]], "references": references}
        canonical["symbols"].sort(key=_key)
        return {"result": Result.PASS, "input_consistency": Result.PASS, "proof": proof,
                "address": envelope["address"], "size": envelope["size"], "canonical": canonical}

    def observe(self, graphs, producers):
        """Expand all data, including orphaned old roots; return required code too."""
        report = {"check": "application_generated_storage_residual", "result": Result.UNKNOWN,
                  "input_consistency": Result.UNKNOWN, "remaining_linked_effects": Result.UNKNOWN,
                  "allocation_isolation": Result.UNKNOWN, "ready_for_collection": False}
        try:
            self._fresh()
            for version, graph in graphs.items():
                if graph["result"] != Result.PASS:
                    raise ValueError("storage closure requires complete native code observations")
                for row in graph["data_references"]:
                    target = self._reference(version, row["target"]["data"], row["address"])
                    self._track(version, target, row["addend"])
                for row in graph["import_references"]:
                    identity = row["target"]["loader_import"]
                    self._reference(version, {"external": identity["symbol"]}, row["address"])
            while any(set(self.required) - set(self.observed[version]) for version in self.live):
                for version in self.live:
                    for key in sorted(set(self.required) - set(self.observed[version])):
                        self.observed[version][key] = self._observe(version, self.required[key])
            outcomes = [row["result"] for rows in self.observed.values() for row in rows.values()]
            consistency = [row["input_consistency"] for rows in self.observed.values() for row in rows.values()]
            report["input_consistency"] = (Result.FAIL if Result.FAIL in consistency else Result.UNKNOWN
                                            if Result.UNKNOWN in consistency else Result.PASS)
            if any(outcome != Result.PASS for outcome in outcomes):
                report.update(result=Result.FAIL if Result.FAIL in outcomes else Result.UNKNOWN,
                              reason="native storage contents or references are incomplete")
            else:
                for version, uses in self.data_uses.items():
                    if any(not 0 <= offset < self.observed[version][key]["size"] for key, offset in uses):
                        raise ValueError("effective data reference escapes its independently observed storage extent")
                canonical = {version: {key: row["canonical"] for key, row in sorted(rows.items())}
                             for version, rows in self.observed.items()}
                # Preserve observed overlapping/aliased storage relationships;
                # an empty list is not isolation from unobserved whole-image data.
                sharing = {}
                for version, rows in self.observed.items():
                    keys = sorted(rows)
                    sharing[version] = [(left, right, rows[right]["address"] - rows[left]["address"])
                        for index, left in enumerate(keys) for right in keys[index + 1:]
                        if rows[left]["address"] < rows[right]["address"] + rows[right]["size"]
                        and rows[right]["address"] < rows[left]["address"] + rows[left]["size"]]
                same = canonical["baseline"] == canonical["current"] and sharing["baseline"] == sharing["current"]
                code_covered = True
                for version, uses in self.code_uses.items():
                    nodes = {_key(producers[version][node["id"]]): node for node in graphs[version]["nodes"]}
                    for key, addend in uses:
                        if key not in nodes:
                            code_covered = False
                        elif not 0 <= nodes[key]["proof"].get("entry_offset", 0) + addend < nodes[key]["proof"]["size"]:
                            raise ValueError("data-to-code reference exceeds its independently verified code extent")
                report.update(result=(Result.PASS if same else Result.FAIL) if code_covered else Result.UNKNOWN,
                              equal=same, code_coverage=Result.PASS if code_covered else Result.UNKNOWN,
                              canonical=canonical, sharing=sharing)
            report.update(dependencies=deepcopy(self.observed), sections=len(self.required),
                          required_code=deepcopy(self.code_required))
            return report
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            report.update(reason=str(error), dependencies=deepcopy(self.observed),
                          required_code=deepcopy(self.code_required))
            # Preserve an already established physical counterexample.
            if any(row.get("input_consistency") == Result.FAIL
                   for rows in self.observed.values() for row in rows.values()):
                report.update(result=Result.FAIL, input_consistency=Result.FAIL)
            return report
        finally:
            self._fresh()

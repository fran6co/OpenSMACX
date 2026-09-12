"""Bind a preservation trial to a live application object, without a waiver.

Replica agreement is necessary before linked evidence can explain a compiled
residual. It is not itself proof that an initializer, datum or other residual
may be removed from preservation coverage. Reports cannot be replayed as inputs.
"""

from contextlib import contextmanager
from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path

from .build_graph_commands import _context
from .coff_replica import object_signature
from .link_file_evidence import _stamp
from .model import Result, digest
from .pe_coff import Coff
from .source_replica import snapshot_digest


@dataclass(frozen=True)
class ObjectTrial:
    raw: bytes
    root: Path
    task_name: str
    baseline_revision: str
    authorization: str
    source_digest: str
    unit: str
    version: str
    flags: tuple[str, ...]
    input_fingerprint: str


@contextmanager
def application_objects(graph):
    """Index native membership once inside a single live preservation epoch.

    Individual comparisons remain provisional until normal context exit. Each
    reads its actual output against the original receipt; final graph freshness
    covers all source, policy, membership and input/output changes. Only pure
    whole-object signatures are memoized by exact raw SHA256, never verdicts.
    """
    live = False
    indexes, signatures = {}, {}
    try:
        graph.check_current()
        root, task_name, baseline_revision = graph.root, graph.task_name, graph.baseline_revision
        authorization, sources = digest(graph.authorization), graph.source_versions
        source_digests = {version: snapshot_digest(sources[version]) for version in ("baseline", "current")}
        for version, context in (("baseline", graph.baseline), ("current", graph.current)):
            _, _, rows, _ = _context(context)
            receipts = {row["source"]: row for row in context.report["compile_link"]["compilations"]}
            indexes[version] = {}
            for unit, row in rows.items():
                receipt = receipts[str(context.root / unit)]["compilation"]["output"].get("after")
                _stamp(receipt)
                if receipt is None:
                    raise ValueError("application object lacks a fresh compiler receipt")
                indexes[version][unit] = (row["output"], dict(receipt))
        graph.check_current()
        live = True

        def signature(raw, identity):
            if identity not in signatures:
                signatures[identity] = object_signature(Coff(raw))
            return signatures[identity]

        def compare(trial, obj, *, unit, version, flags, input_fingerprint):
            if not live:
                raise ValueError("application object comparison context has expired")
            try:
                if (not isinstance(trial, ObjectTrial) or type(trial.raw) is not bytes
                        or trial.root != root or trial.task_name != task_name
                        or trial.baseline_revision != baseline_revision or trial.authorization != authorization
                        or version not in ("baseline", "current") or trial.version != version
                        or trial.unit != unit or trial.flags != tuple(flags)
                        or trial.input_fingerprint != input_fingerprint):
                    raise ValueError("preservation object trial identity or authorization differs from live graph")
                if unit not in sources[version] or trial.source_digest != source_digests[version]:
                    raise ValueError("preservation object source version differs from live graph")
                trial_sha256 = sha256(trial.raw).hexdigest()
                observed = signature(trial.raw, trial_sha256)
                if object_signature(obj) != observed:
                    raise ValueError("preservation object changed from captured compiler output")
                if unit not in indexes[version]:
                    raise ValueError("preservation object has no unique application compilation")
                output, expected = indexes[version][unit]
                if output.resolve() != output or not output.is_file() or output.stat().st_nlink != 1:
                    raise ValueError("application object must remain an unaliased physical file")
                raw = output.read_bytes()
                stamp = {"sha256": sha256(raw).hexdigest(), "size": len(raw)}
                if stamp != expected:
                    raise ValueError("application object differs from its fresh compiler receipt")
                linked = signature(raw, stamp["sha256"])
                same = observed == linked
                return {"check": "preservation_application_object", "unit": unit, "version": version,
                        "flags": list(flags), "source_digest": trial.source_digest,
                        "input_fingerprint": input_fingerprint,
                        "trial_sha256": trial_sha256, "application_object": str(output),
                        "application_sha256": stamp["sha256"], "trial_signature": observed,
                        "application_signature": linked, "result": Result.PASS if same else Result.UNKNOWN,
                        "linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                        "reason": ("same-source whole-object replica agreement; residual effects remain unproved" if same else
                                   "application output differs from this preservation trial; linked evidence cannot cover it")}
            except (KeyError, TypeError, AttributeError) as error:
                raise ValueError("incomplete preservation/application object binding") from error

        yield compare
    except (KeyError, TypeError, AttributeError) as error:
        if live:
            raise  # Do not translate unrelated exceptions from the caller body.
        raise ValueError("incomplete preservation/application object binding") from error
    finally:
        live = False
        graph.check_current()


def compare_application_object(graph, trial, obj, *, unit, version, flags, input_fingerprint):
    """Standalone observation with the same entry and mandatory exit guards."""
    with application_objects(graph) as compare:
        return compare(trial, obj, unit=unit, version=version, flags=flags, input_fingerprint=input_fingerprint)

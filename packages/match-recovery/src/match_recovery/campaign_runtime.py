"""Append-only installed runtime decisions for a preserved bounded campaign.

The original v1 grant stays untouched. Later v2 transitions bind the immediately
preceding decision, not an old report or a replacement assignment. The caller
freezes destination/workers before publication; no source or policy is upgraded.
"""

from pathlib import Path
import re

from .campaign_choices import ancestor, batches, coordinator_authority, revision
from .dispatch import _configuration
from .model import digest, require_digest, require_text
from .records import publish_json
from .source_history import git


FILENAME = "runtime-authorization.json"


def _records(directory):
    """The unsuffixed original is grant 1; numbered successors start at 2."""
    entries = []
    for path in directory.glob("runtime-authorization*.json"):
        if path.name == FILENAME:
            number = 1
        else:
            match = re.fullmatch(r"runtime-authorization-([1-9][0-9]*)\.json", path.name)
            if match is None or int(match[1]) < 2:
                raise ValueError("invalid campaign runtime authorization filename")
            number = int(match[1])
        entries.append((number, path.name))
    entries.sort()
    if [number for number, _ in entries] != list(range(1, len(entries) + 1)):
        raise ValueError("campaign runtime authorization chain must be contiguous")
    return [_configuration(directory, name) for _, name in entries]


def authority(root, directory, configuration, profiles, authorization=None, *, publish=False):
    """Return the persisted effective grant, or None for an unbound legacy run.

    Only a new grant needs current HEAD/latest assignment/current profile pins and
    explicit frozen publication. Exact retries of the current tail are idempotent.
    Historical grants remain validated decisions, but their old bindings/interpreters
    are never opened or executed. Only the effective runtime must remain installed.
    """
    from .framework_runtime import export_binding, fingerprint

    root, directory = Path(root), Path(directory)
    if (not root.is_absolute() or root.resolve() != root or not root.is_dir()
            or directory.resolve() != directory or not directory.is_dir()
            or directory.parent != root / "build/recovery/campaigns"):
        raise ValueError("runtime authorization requires this coordinator's canonical preserved run")
    if type(publish) is not bool:
        raise ValueError("runtime authorization publication must be explicit")
    if coordinator_authority(root, directory, configuration, profiles) != digest(profiles):
        raise ValueError("runtime authorization cannot change campaign profiles")
    manifest = _configuration(directory, "run.json")
    records = _records(directory)
    selected, binding = fingerprint(), export_binding()
    if not records and authorization is None:
        if selected is not None or binding is not None:
            raise ValueError("preserved campaign requires explicit runtime authorization before execution")
        return None
    if selected is None or binding is None:
        raise ValueError("campaign runtime authorization requires its explicit installed runtime binding")
    assigned = batches(root, directory, configuration)
    by_digest = {digest(batch): batch for batch in assigned}
    historical_profiles = {manifest["profiles_sha256"], digest(profiles)}
    # coordinator_authority already validates this separate scope-only chain.
    for entry in directory.glob("coordinator-authorization-*.json"):
        decision = _configuration(directory, entry.name)
        historical_profiles.add(decision["previous_profiles_sha256"])
        historical_profiles.add(digest({**profiles, "coordinator": decision["coordinator"]}))

    def validate(value, previous, *, fresh):
        fields = {"schema_version", "author", "run_sha256", "batch_sha256", "profiles_sha256",
                  "destination_revision", "runtime_binding", "runtime_sha256", "evidence"}
        if previous is not None:
            fields.add("previous_authorization_sha256")
        if (not isinstance(value, dict) or set(value) != fields
                or type(value["schema_version"]) is not int
                or value["schema_version"] != (1 if previous is None else 2)):
            raise ValueError("invalid campaign runtime authorization")
        for key in ("run_sha256", "batch_sha256", "profiles_sha256", "runtime_sha256"):
            require_digest(value[key])
        require_text(value["author"], "runtime authorization author")
        require_text(value["runtime_binding"], "runtime binding path")
        old_path = Path(value["runtime_binding"])
        if not old_path.is_absolute() or str(old_path) != value["runtime_binding"] or ".." in old_path.parts:
            raise ValueError("runtime authorization requires an absolute normalized binding path")
        evidence = value["evidence"]
        if (not isinstance(evidence, list) or not evidence
                or any(not isinstance(item, str) for item in evidence) or len(set(evidence)) != len(evidence)):
            raise ValueError("runtime authorization requires unique evidence")
        for item in evidence:
            require_text(item, "runtime authorization evidence")
        if value["author"] != configuration["author"] or value["run_sha256"] != digest(manifest):
            raise ValueError("runtime authorization does not bind this campaign")
        if value["batch_sha256"] not in by_digest:
            raise ValueError("runtime authorization requires a retained campaign assignment")
        if value["profiles_sha256"] not in historical_profiles:
            raise ValueError("runtime authorization names an unknown campaign profile epoch")
        destination = revision(value["destination_revision"])
        bound = by_digest[value["batch_sha256"]]
        if not ancestor(root, bound["revision"], destination) or not ancestor(root, destination):
            raise ValueError("runtime authorization destination is outside retained campaign ancestry")
        if previous is not None:
            require_digest(value["previous_authorization_sha256"])
            if value["previous_authorization_sha256"] != digest(previous):
                raise ValueError("runtime transition does not bind its immediate predecessor")
            if (value["runtime_binding"], value["runtime_sha256"]) == (previous["runtime_binding"], previous["runtime_sha256"]):
                raise ValueError("runtime transition must select a changed runtime binding")
            if (bound["index"] < by_digest[previous["batch_sha256"]]["index"]
                    or not ancestor(root, previous["destination_revision"], destination)):
                raise ValueError("runtime transition cannot rewind campaign progress")
        if fresh and (value["batch_sha256"] != digest(assigned[-1]) or value["profiles_sha256"] != digest(profiles)
                      or destination != git(root, "rev-parse", "HEAD").decode().strip()):
            raise ValueError("new runtime authorization requires the latest assignment, profiles and destination")

    previous = None
    for record in records:
        validate(record, previous, fresh=False)
        previous = record
    new = authorization is not None and authorization != previous
    if new:
        validate(authorization, previous, fresh=True)
    effective = authorization if new else previous
    if effective is None:
        raise ValueError("campaign requires an explicit installed runtime authorization")
    if effective["runtime_binding"] != binding or effective["runtime_sha256"] != selected:
        raise ValueError("runtime authorization does not bind the selected installation")
    bound_path = Path(binding)
    if (not bound_path.is_absolute() or bound_path.resolve() != bound_path or not bound_path.is_file()
            or bound_path.stat().st_nlink != 1):
        raise ValueError("campaign runtime binding must remain a canonical unaliased file")
    if new:
        if not publish:
            raise ValueError("new runtime authorization requires frozen publication before execution")
        path = directory / (FILENAME if not records else f"runtime-authorization-{len(records) + 1}.json")
        publish_json(path, effective)
        records = [*records, effective]
    if fingerprint() != selected or export_binding() != binding:
        raise ValueError("selected runtime changed during campaign authorization")
    if _records(directory) != records:
        raise ValueError("campaign runtime authorization chain changed during execution")
    return effective

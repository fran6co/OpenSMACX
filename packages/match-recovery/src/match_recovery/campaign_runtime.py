"""One explicit installed runtime transition for a preserved bounded campaign.

This publishes a coordinator decision, not a worker upgrade, a new assignment,
or acceptance. The caller holds destination/worker leases and freezes source
before publication. Original manifests, profiles and baselines stay untouched.
"""

from pathlib import Path

from .campaign_choices import ancestor, batches, coordinator_authority, revision
from .dispatch import _configuration
from .model import digest, require_digest, require_text
from .records import publish_json
from .source_history import git


FILENAME = "runtime-authorization.json"


def authority(root, directory, configuration, profiles, authorization=None, *, publish=False):
    """Return the persisted runtime decision, or None for an unbound legacy run.

    New decisions require explicit publication by a caller holding all applicable
    leases. Existing decisions are immutable; an exact retry is idempotent. Their
    original batch and destination may become historical through normal campaign
    progress, but their selected runtime cannot silently change or disappear.
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
    path = directory / FILENAME
    saved = _configuration(directory, FILENAME) if path.exists() or path.is_symlink() else None
    selected = fingerprint()
    binding = export_binding()
    if saved is None and authorization is None:
        if selected is not None or binding is not None:
            raise ValueError("preserved campaign requires explicit runtime authorization before execution")
        return None
    if selected is None or binding is None:
        raise ValueError("campaign runtime authorization requires its explicit installed runtime binding")
    if saved is not None and authorization is not None and authorization != saved:
        raise ValueError("a preserved campaign's installed runtime authorization cannot be replaced")
    value = saved if saved is not None else authorization
    fields = {"schema_version", "author", "run_sha256", "batch_sha256", "profiles_sha256",
              "destination_revision", "runtime_binding", "runtime_sha256", "evidence"}
    if (not isinstance(value, dict) or set(value) != fields
            or type(value["schema_version"]) is not int or value["schema_version"] != 1):
        raise ValueError("invalid campaign runtime authorization")
    for key in ("run_sha256", "batch_sha256", "profiles_sha256", "runtime_sha256"):
        require_digest(value[key])
    require_text(value["author"], "runtime authorization author")
    require_text(value["runtime_binding"], "runtime binding path")
    evidence = value["evidence"]
    if (not isinstance(evidence, list) or not evidence
            or any(not isinstance(item, str) for item in evidence) or len(set(evidence)) != len(evidence)):
        raise ValueError("runtime authorization requires unique evidence")
    for item in evidence:
        require_text(item, "runtime authorization evidence")
    if (value["author"] != configuration["author"] or value["run_sha256"] != digest(manifest)
            or value["runtime_binding"] != binding or value["runtime_sha256"] != selected):
        raise ValueError("runtime authorization does not bind this campaign and selected installation")
    bound_path = Path(binding)
    if (not bound_path.is_absolute() or bound_path.resolve() != bound_path or not bound_path.is_file()
            or bound_path.stat().st_nlink != 1):
        raise ValueError("campaign runtime binding must remain a canonical unaliased file")
    assigned = batches(root, directory, configuration)
    by_digest = {digest(batch): batch for batch in assigned}
    if value["batch_sha256"] not in by_digest:
        raise ValueError("runtime authorization requires a retained campaign assignment")
    historical_profiles = {manifest["profiles_sha256"], digest(profiles)}
    # coordinator_authority above validates the entire existing scope-only
    # transition chain. A runtime grant survives later legitimate delegation;
    # its historical profile digest is not permission to change other profiles.
    for entry in directory.glob("coordinator-authorization-*.json"):
        decision = _configuration(directory, entry.name)
        historical_profiles.add(decision["previous_profiles_sha256"])
        historical_profiles.add(digest({**profiles, "coordinator": decision["coordinator"]}))
    if value["profiles_sha256"] not in historical_profiles:
        raise ValueError("runtime authorization names an unknown campaign profile epoch")
    destination = revision(value["destination_revision"])
    if (not ancestor(root, by_digest[value["batch_sha256"]]["revision"], destination)
            or not ancestor(root, destination)):
        raise ValueError("runtime authorization destination is outside retained campaign ancestry")
    if saved is None:
        if (value["batch_sha256"] != digest(assigned[-1]) or value["profiles_sha256"] != digest(profiles)
                or destination != git(root, "rev-parse", "HEAD").decode().strip()):
            raise ValueError("new runtime authorization requires the latest assignment, profiles and destination")
        if not publish:
            raise ValueError("new runtime authorization requires frozen publication before execution")
        publish_json(path, value)
        saved = _configuration(directory, FILENAME)
        if saved != value:
            raise ValueError("campaign runtime authorization changed during publication")
    # Keep the selected package/interpreter/binding live through publication.
    if fingerprint() != selected or export_binding() != binding:
        raise ValueError("selected runtime changed during campaign authorization")
    return saved

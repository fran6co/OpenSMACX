"""Source anchors for explicitly intended generated changes, not emission proof.

The original identity stays in C++; the coordinator decision names a particular
generated occurrence. Neither proximity nor a compiler symbol prefix establishes
that relationship. Actual table/root ownership requires fresh native link evidence.
"""

from copy import deepcopy
from pathlib import Path

from .migration_protocol import permissions
from .model import Origin
from .object_source import parse_objects
from .source.reader import read_text


def generated_sources(permission, task, scope, editable, versions, settings, pin, root):
    """Validate decisions against unique original definitions in each source version.

    Initially supported anchors are original functions and original objects in
    the selected TU. A header, shared interpretation or seed alone does not
    establish a generated registration's source owner.
    """
    permissions(permission, task, scope)
    rows = permission.get("generated_changes", [])
    anchors = {}
    source = settings.source
    for version in {row["version"] for row in rows}:
        if not isinstance(versions, dict) or not isinstance(versions.get(version), dict):
            raise ValueError("generated change requires its source version contents")
        owners = {}
        for name, raw in versions[version].items():
            if (source.is_seed(name) or Path(name).suffix not in source.extensions
                    or not any(Path(name).is_relative_to(directory) for directory in source.roots)):
                continue
            text = raw.decode("utf-8", errors="strict")
            identities = [Origin(pin, source.address_space, record.address).entity_id
                          for record in read_text(text, root / name)]
            identities += [item.origin.entity_id for item in
                           parse_objects(text, name, pin, address_space=source.address_space)]
            for identity in identities:
                owners.setdefault(identity, []).append(name)
        anchors[version] = owners
    for row in rows:
        unit = row["unit"]
        if (unit not in editable or Path(unit).suffix not in settings.unit_extensions
                or source.is_seed(unit)
                or not any(Path(unit).is_relative_to(directory) for directory in source.roots)):
            raise ValueError("generated change requires an editable product translation unit")
        if anchors[row["version"]].get(row["entity_id"]) != [unit]:
            raise ValueError("generated change requires a unique original source anchor in its exact version and TU")
    return deepcopy(rows)

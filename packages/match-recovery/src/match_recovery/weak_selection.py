"""Linker selection of VC6 `??_E` weak externals through their `??_G` default.

VC6 emits a class's vftable deleting slot as a relocation to a weak external
`??_E<class>...` whose COFF Auxiliary Format 3 record names the scalar deleting
destructor `??_G<class>...` and requests a library search (mode 2). The linker
selects a strong `??_E` definition when any link input supplies one (a vector
deleting destructor) and the `??_G` default otherwise. Neither COFF object
alone decides that, so a slot stays `linker_selection_required` until a link
observation shows which definition the linker took.

`qualify_weak_fallbacks` admits `W -> F` only when all of the following hold:

* every freshly compiled object that references `W` records it as a mode-2
  weak external whose default is exactly `F = "??_G" + W[4:]`;
* no freshly compiled object (the current spelling of every unit in the
  measured closure) defines `W` strongly;
* the observed link's own input files still have the content it recorded
  (`verify_link_files` PASS), its MAP agrees with its executable, and that MAP
  places `W` and `F` in exactly one public function row each, at the same VA
  and from the same owner object. A strong `W` from any explicit or library
  input would have its own placement there.

Anything else refuses that name with a reason. The observation is the build
directory's last link; units outside the measured closure are taken as that
link observed them. It never resolves a weak name without a MAP placement,
never treats a mode 1/3 record, a non-`??_G` default or an adjustor `??_E`
entry as a fallback, and does not choose source ownership.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
from pathlib import Path

from .model import Result
from .pe_coff import weak_external


WEAK_PREFIX, FALLBACK_PREFIX = "??_E", "??_G"


@dataclass(frozen=True)
class LinkObservation:
    """One parsed, content-verified build-directory link (MAP + inputs)."""
    publics: dict  # symbol -> tuple[Public, ...]
    evidence: dict = field(default_factory=dict)
    # Casefolded basename -> number of explicit link inputs with that name;
    # a MAP owner names an object only by basename.
    explicit_owner_counts: dict = field(default_factory=dict)


@dataclass(frozen=True)
class WeakFallbackSelections:
    selected: dict  # weak name -> fallback name
    refused: dict  # weak name -> reason
    evidence: dict = field(default_factory=dict)

    def fallback(self, name):
        return self.selected.get(name)


def observe_link(map_data: bytes, image_data: bytes, inputs_data: bytes, log_data: bytes) -> LinkObservation:
    """Parse and verify one completed link; raise when it is not usable evidence."""
    from .link_file_evidence import verify_link_files
    from .msvc_link_map import parse_link_map
    from .pe_coff import PeImage

    from .link_file_evidence import parse_link_files
    files = verify_link_files(inputs_data, log_data)
    owners = {}
    for path in parse_link_files(inputs_data)["explicit_inputs"]:
        name = Path(path).name.casefold()
        owners[name] = owners.get(name, 0) + 1
    if files["result"] != Result.PASS:
        changed = [row["path"] for row in files["files"] if row["result"] != Result.PASS][:4]
        raise ValueError("observed link inputs are not the files it recorded: " + ", ".join(changed))
    image = PeImage(image_data, hashlib.sha256(image_data).hexdigest())
    mapping = parse_link_map(map_data)
    mapping.validate(image)
    publics = {}
    for row in mapping.publics:
        publics.setdefault(row.symbol, []).append(row)
    return LinkObservation({name: tuple(rows) for name, rows in publics.items()},
                           {"image_sha256": hashlib.sha256(image_data).hexdigest(),
                            "map_sha256": hashlib.sha256(map_data).hexdigest(),
                            "link_inputs_sha256": files["sha256"],
                            "link_log_sha256": files["link_log_sha256"]},
                           owners)


def read_link_observation(root: Path, paths) -> LinkObservation:
    """Read a configured observation (`map`, `image`, `inputs`, `log`) under root."""
    root = Path(root).resolve()
    raw = {}
    for key in ("map", "image", "inputs", "log"):
        path = (root / paths[key]).resolve()
        if not path.is_relative_to(root) or not path.is_file():
            raise ValueError(f"observed link {key} is missing: {paths[key]}")
        raw[key] = path.read_bytes()
    return observe_link(raw["map"], raw["image"], raw["inputs"], raw["log"])


def weak_edges(obj):
    """Weak name -> set of (fallback name, mode) recorded by one object."""
    edges = {}
    for symbol in obj.symbols.values():
        if symbol.storage != 105:
            continue
        fallback, mode = weak_external(obj, symbol)
        edges.setdefault(symbol.name, set()).add((fallback.name, mode))
    return edges


def qualify_weak_fallbacks(names, objects, observation: LinkObservation | None) -> WeakFallbackSelections:
    selected, refused = {}, {}
    if observation is None:
        return WeakFallbackSelections({}, {name: "no observed link supplies a selection" for name in names})
    observed_edges, strong, errors = {}, set(), {}
    for unit, obj in sorted(objects.items()):
        try:
            for name, edges in weak_edges(obj).items():
                observed_edges.setdefault(name, set()).update(edges)
        except ValueError as error:
            errors[unit] = str(error)
        strong.update(symbol.name for symbol in obj.symbols.values()
                      if symbol.storage == 2 and symbol.section > 0)
    for name in sorted(set(names)):
        fallback = FALLBACK_PREFIX + name[len(WEAK_PREFIX):]
        edges = observed_edges.get(name, set())
        if errors:
            refused[name] = "compiled weak metadata is invalid: " + "; ".join(
                f"{unit}: {reason}" for unit, reason in sorted(errors.items()))
        elif not name.startswith(WEAK_PREFIX):
            refused[name] = "only ??_E weak externals with a ??_G default are qualified"
        elif edges != {(fallback, 2)}:
            refused[name] = (f"compiled objects do not uniformly default {name} to {fallback} "
                             f"with a library search: {sorted(edges)}")
        elif name in strong:
            refused[name] = "a compiled object in the measured closure defines the weak name strongly"
        else:
            weak_rows = observation.publics.get(name, ())
            fallback_rows = observation.publics.get(fallback, ())
            if len(weak_rows) != 1 or len(fallback_rows) != 1:
                refused[name] = "observed link map lacks one placement each for the weak name and its default"
            elif not (weak_rows[0].function and fallback_rows[0].function
                      and weak_rows[0].segment and fallback_rows[0].segment):
                refused[name] = "observed link map placements are not function code"
            elif (weak_rows[0].address != fallback_rows[0].address
                  or weak_rows[0].owner.casefold() != fallback_rows[0].owner.casefold()):
                refused[name] = "observed link selected a separate strong definition for the weak name"
            else:
                selected[name] = fallback
    return WeakFallbackSelections(selected, refused, dict(observation.evidence))

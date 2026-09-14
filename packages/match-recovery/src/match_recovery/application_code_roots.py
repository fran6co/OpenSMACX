"""Normalize root spellings only after complete physical ICF group binding.

No address or byte equality joins definitions. Repeated requests remain repeated,
and the downstream native graph must still verify every peer and its frames.
"""

from .linked_context import LinkedBuild


def combine_roots(native, roots):
    """Canonicalize original/MAP aliases without weakening graph enrollment.

    This is a private live-application helper, not code or scope evidence. The
    full group is registered even when only one of its members was requested,
    so conflicting physical identities cannot be hidden by request order.
    """
    if type(native) is not LinkedBuild:
        raise ValueError("application roots require a native linked build")
    try:
        native.check_current()
        folding = native.inputs.folding
        if folding is not None:
            if (folding._objects is not native.inputs.objects or folding._mapping is not native.inputs.mapping
                    or folding._image is not native.inputs.image or folding._fixups is not native.inputs.fixups):
                raise ValueError("application roots have another image/input folding binding")
            candidates = folding.candidate_keys()
        else:
            candidates = frozenset()
        aliases, result = {}, []
        for index, root in enumerate(roots):
            if index >= 100000:
                raise ValueError("application root requests exceed their resource bound")
            if (not isinstance(root, dict) or set(root) != {"owner", "symbol"}
                    or any(not isinstance(root[key], str) or not root[key].strip() for key in root)):
                raise ValueError("application roots require exact owner and symbol identities")
            key = root["owner"].casefold(), root["symbol"]
            if key not in aliases and key in candidates:
                group = folding.for_map(root["owner"], root["symbol"], optional=True)
                if group is None:
                    group = folding.group(root["owner"], root["symbol"], optional=True)
                if group is not None:
                    pending = {}
                    for member in group.members:
                        canonical = {"owner": group.selected.owner, "symbol": member.symbol}
                        identity = (member.input, member.object_sha256, member.root)
                        for alias in {(member.owner.casefold(), member.symbol),
                                      (group.selected.owner.casefold(), member.symbol)}:
                            binding = (group, identity, canonical)
                            if alias in pending and pending[alias] != binding:
                                raise ValueError("folded root aliases name distinct physical members")
                            if alias in aliases and aliases[alias] != binding:
                                raise ValueError("folded root conflicts with an earlier group or ordinary interpretation")
                            pending[alias] = binding
                    aliases.update(pending)
            if key not in aliases:
                aliases[key] = (None, None, dict(root))
            result.append(dict(aliases[key][2]))
        return result
    finally:
        native.check_current()

"""Compare the indexed section-key computation with its previous algorithm."""

from copy import deepcopy
from dataclasses import replace
from types import SimpleNamespace

import pytest

from match_recovery import coff_replica
from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.model import digest
from match_recovery.pe_coff import Coff, Symbol
from test_multispan import object_fixture
from test_replica_identity import pair as replica_pair
from test_root_label_identity import pair as root_pair


def previous_section_keys(obj, overrides=None, names=None):
    # Deliberately retain the old independent full scan as an equivalence oracle.
    keys = {}
    for number, section in enumerate(obj.sections, 1):
        symbols = sorted(((names or {}).get(symbol.index, symbol.name), symbol.value, symbol.type, symbol.storage)
                         for symbol in obj.symbols.values() if symbol.section == number and symbol.storage != 103
                         and not (symbol.name == section.name and symbol.storage == 3 and symbol.type == 0))
        if symbols and number not in obj.associations:
            keys[number] = "named:" + digest(symbols)
    keys.update(overrides or {})
    def key(number):
        if number not in keys:
            section = obj.sections[number - 1]
            keys[number] = ("associated:" + key(obj.associations[number]) + ":" + section.name
                            if number in obj.associations else "section:" + section.name)
        return keys[number]
    for number in range(1, len(obj.sections) + 1):
        key(number)
    if len(set(keys.values())) != len(keys):
        raise ValueError("replica control requires unambiguous section names or associative owners")
    return keys


def outcome(operation):
    try:
        return "value", operation()
    except ValueError as error:
        return "refused", str(error)


@pytest.mark.parametrize("family", ["ordinary", "replica", "root_labels"])
@pytest.mark.parametrize("damage", [None, "renumber", "reverse", "duplicate", "external", "section_symbol", "association"])
def test_complete_signature_and_preservation_match_previous_algorithm(monkeypatch, family, damage):
    if family == "ordinary":
        old = Coff(object_fixture(extra=(b".data", bytes(8), (), 0, 2, 0xC0501040)))
        new = deepcopy(old)
    else:
        old, new = replica_pair() if family == "replica" else root_pair()
    if damage == "renumber":
        # Renumber an unreferenced local; all existing relocation edges stay intact.
        old.symbols[400] = Symbol(400, "ordinary_local", 0, 1, 0, 6)
        new.symbols[500] = replace(old.symbols[400], index=500)
    elif damage == "reverse":
        new.symbols = dict(reversed(list(new.symbols.items())))
    elif damage == "duplicate":
        new.symbols[500] = replace(new.symbols[2], index=500)
    elif damage == "external":
        new.symbols[500] = Symbol(500, "_undefined", 0, 0, 0, 2)
    elif damage == "section_symbol":
        new.symbols[500] = Symbol(500, new.sections[0].name, 0, 1, 0, 3)
    elif damage == "association":
        new.associations[2] = 3
    def evidence():
        return (outcome(lambda: coff_replica.object_signature(old)),
                outcome(lambda: coff_replica.object_signature(new)),
                outcome(lambda: compare_outside_functions(old, new, {}, {})))
    indexed = evidence()
    with monkeypatch.context() as context:
        context.setattr(coff_replica, "_section_keys", previous_section_keys)
        previous = evidence()
    assert indexed == previous


@pytest.mark.parametrize("names", [None, {}, {1: "renamed", 2: "renamed", 4: "not-a-section-record"}])
@pytest.mark.parametrize("overrides", [None, {}, {1: "selected:body"}, {1: "collision", 2: "collision"}])
def test_index_preserves_order_multiplicity_names_and_associative_ownership(names, overrides):
    obj = SimpleNamespace(sections=[SimpleNamespace(name=name) for name in (".text", ".child", ".bss")],
                          associations={2: 1}, symbols={
        1: Symbol(1, "_same", 0, 1, 0x20, 2),
        2: Symbol(2, "_same", 0, 1, 0x20, 2),
        3: Symbol(3, "child_label", 4, 2, 0, 6),
        4: Symbol(4, ".text", 0, 1, 0, 3),
        5: Symbol(5, ".file", 0, 1, 0, 103),
        6: Symbol(6, "_external", 0, 0, 0, 2),
        7: Symbol(7, "_absolute", 0, -1, 0, 2),
        8: Symbol(8, "beyond", 0, 4, 0, 2),
    })
    assert outcome(lambda: coff_replica._section_keys(obj, overrides, names)) == outcome(
        lambda: previous_section_keys(obj, overrides, names))
    if not overrides:
        keys = coff_replica._section_keys(obj, names=names)
        assert keys[2] == "associated:" + keys[1] + ":.child"
        assert keys[3] == "section:.bss"
        del obj.symbols[2]
        assert coff_replica._section_keys(obj, names=names)[1] != keys[1]


def test_ambiguous_unnamed_sections_still_refuse():
    obj = SimpleNamespace(sections=[SimpleNamespace(name=".text"), SimpleNamespace(name=".text")],
                          associations={}, symbols={})
    assert outcome(lambda: coff_replica._section_keys(obj)) == outcome(lambda: previous_section_keys(obj))
    with pytest.raises(ValueError, match="unambiguous"):
        coff_replica._section_keys(obj)


def test_symbols_are_visited_once_per_call_without_reusing_stale_index():
    class CountedSymbols(dict):
        visits = 0
        def values(self):
            for symbol in super().values():
                self.visits += 1
                yield symbol
    symbols = CountedSymbols({number: Symbol(number, "_f" + str(number), 0, number, 0x20, 2)
                              for number in range(1, 41)})
    obj = SimpleNamespace(sections=[SimpleNamespace(name=".text") for _ in range(40)],
                          associations={}, symbols=symbols)
    first = coff_replica._section_keys(obj)
    assert symbols.visits == len(symbols)
    symbols[1] = replace(symbols[1], name="_changed")
    second = coff_replica._section_keys(obj)
    assert symbols.visits == 2 * len(symbols)
    assert first[1] != second[1] and all(first[number] == second[number] for number in range(2, 41))

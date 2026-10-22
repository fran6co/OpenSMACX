"""Fail-closed COFF target typing controls for array EH helper edges."""

import struct

import pytest

from match_recovery.pe_coff import Coff
from match_recovery.source.eh_regions import bind_eh_regions
from test_eh_vector_regions import ROOT_NAME, _build


def test_array_eh_helper_must_be_a_function_symbol():
    object_bytes, image, spans, record, *_ = _build([("array", {})])
    parsed = Coff(object_bytes)
    helper = next(symbol for symbol in parsed.symbols.values()
                  if symbol.name == "??_M@YGXPAXIHP6EX0@Z@Z")
    symbol_table, = struct.unpack_from("<I", object_bytes, 8)
    damaged = bytearray(object_bytes)
    struct.pack_into("<H", damaged, symbol_table + helper.index * 18 + 14, 0)

    with pytest.raises(ValueError, match="runtime helper"):
        bind_eh_regions(bytes(damaged), ROOT_NAME, image, spans)

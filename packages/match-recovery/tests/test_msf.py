"""Physical SmallMSF2 encodings, including stale stream-zero transaction data."""

import struct

import pytest

from match_recovery.msf import MAGIC, SmallMsf


def small_msf(streams, *, page_size=1024, active_fpm=None):
    """Build physical bytes from a full stream-index list; slot zero is stale.

    The supplied slot-zero bytes are serialized as the previous directory;
    callers receive the current directory at zero, regardless of this value.
    """
    fpm_pages = {1024: 8, 2048: 4, 4096: 1}[page_size]
    first = 1 + 2 * fpm_pages
    descriptions, page_lists, chunks = [], [], {}
    next_page = first
    for stream in streams:
        if stream is None:
            descriptions.append((0xFFFFFFFF, 0))
            page_lists.append(())
            continue
        descriptions.append((len(stream), 0))
        pages = []
        for at in range(0, len(stream), page_size):
            pages.append(next_page)
            chunks[next_page] = stream[at:at + page_size]
            next_page += 1
        page_lists.append(tuple(pages))
    directory = struct.pack("<HH", len(streams), 0)
    directory += b"".join(struct.pack("<II", *item) for item in descriptions)
    directory += b"".join(struct.pack("<" + "H" * len(pages), *pages) for pages in page_lists)
    directory_pages = []
    for at in range(0, len(directory), page_size):
        directory_pages.append(next_page)
        chunks[next_page] = directory[at:at + page_size]
        next_page += 1
    raw = bytearray(next_page * page_size)
    raw[:44] = MAGIC
    fpm = 1 if active_fpm is None else active_fpm
    struct.pack_into("<IHHII", raw, 44, page_size, fpm, next_page, len(directory), 0)
    struct.pack_into("<" + "H" * len(directory_pages), raw, 60, *directory_pages)
    for start in (1, 1 + fpm_pages):
        raw[start * page_size:(start + fpm_pages) * page_size] = b"\xff" * (fpm_pages * page_size)
        for page in range(next_page):
            raw[start * page_size + page // 8] &= ~(1 << (page % 8))
    for page, chunk in chunks.items():
        raw[page * page_size:page * page_size + len(chunk)] = chunk
    return bytes(raw)


def directory_start(raw):
    page, = struct.unpack_from("<I", raw, 44)
    index, = struct.unpack_from("<H", raw, 60)
    return index * page


def changed(raw, at, fmt, *values):
    result = bytearray(raw)
    struct.pack_into(fmt, result, at, *values)
    return bytes(result)


@pytest.mark.parametrize("page_size,fpm", [(1024, 1), (1024, 9), (2048, 1), (2048, 5), (4096, 1), (4096, 2)])
def test_all_small_page_layouts_and_active_fpm_banks(page_size, fpm):
    streams = [None, b"information", None, b"", b"x" * (page_size + 37)]
    raw = small_msf(streams, page_size=page_size, active_fpm=fpm)
    obj = SmallMsf(raw)
    assert obj.stream_count == len(streams) and obj.page_size == page_size
    assert [obj.stream(i) for i in range(1, len(streams))] == streams[1:]
    begin = directory_start(raw)
    length, = struct.unpack_from("<I", raw, 52)
    assert obj.stream(0) == raw[begin:begin + length]


def test_multpage_directory_and_noncontiguous_stream_preserve_logical_order():
    streams = [None] + [bytes([i % 256]) for i in range(170)] + [b"a" * 1024 + b"b" * 33]
    raw = bytearray(small_msf(streams))
    # Swap physical data pages AND their encoded order; directory has two pages.
    begin = directory_start(raw)
    at = begin + 4 + len(streams) * 8 + 170 * 2
    first, second = struct.unpack_from("<HH", raw, at)
    a, b = raw[first * 1024:(first + 1) * 1024], raw[second * 1024:(second + 1) * 1024]
    raw[first * 1024:(first + 1) * 1024], raw[second * 1024:(second + 1) * 1024] = b, a
    struct.pack_into("<HH", raw, at, second, first)
    obj = SmallMsf(bytes(raw))
    assert [obj.stream(i) for i in range(1, len(streams))] == streams[1:]
    assert len(obj.stream(0)) > 1024


def test_stream_pages_can_be_interleaved_with_another_stream():
    raw = bytearray(small_msf([None, b"a" * 1024 + b"b" * 33, b"other"]))
    at = directory_start(raw) + 4 + 3 * 8
    first, second, other = struct.unpack_from("<HHH", raw, at)
    second_bytes = raw[second * 1024:(second + 1) * 1024]
    other_bytes = raw[other * 1024:(other + 1) * 1024]
    raw[second * 1024:(second + 1) * 1024] = other_bytes
    raw[other * 1024:(other + 1) * 1024] = second_bytes
    struct.pack_into("<HHH", raw, at, first, other, second)
    obj = SmallMsf(bytes(raw))
    assert obj.stream(1) == b"a" * 1024 + b"b" * 33
    assert obj.stream(2) == b"other"


def test_directory_page_list_not_physical_order_controls_decoding():
    raw = bytearray(small_msf([None] + [b""] * 150))
    first, second = struct.unpack_from("<HH", raw, 60)
    a, b = raw[first * 1024:(first + 1) * 1024], raw[second * 1024:(second + 1) * 1024]
    raw[first * 1024:(first + 1) * 1024], raw[second * 1024:(second + 1) * 1024] = b, a
    struct.pack_into("<HH", raw, 60, second, first)
    obj = SmallMsf(bytes(raw))
    assert obj.stream_count == 151
    assert all(obj.stream(i) == b"" for i in range(1, 151))


def test_largest_supported_stream_count_and_directory_only_container():
    obj = SmallMsf(small_msf([None] + [b""] * 4094))
    assert obj.stream_count == 4095 and obj.stream(4094) == b""
    only_directory = SmallMsf(small_msf([None]))
    assert only_directory.stream_count == 1 and len(only_directory.stream(0)) == 12


@pytest.mark.parametrize("prior", [None, b"", b"old", b"a" * 1050])
def test_stream_zero_is_current_directory_not_previous_transaction(prior):
    raw = small_msf([prior, b"live"])
    obj = SmallMsf(raw)
    assert obj.stream(0)[:4] == struct.pack("<HH", 2, 0)
    assert obj.stream(0) != prior
    assert obj.stream(1) == b"live"


def test_obsolete_stream_zero_may_alias_an_active_stream():
    raw = small_msf([b"old", b"live"])
    begin = directory_start(raw)
    page, = struct.unpack_from("<H", raw, begin + 4 + 2 * 8 + 2)
    raw = changed(raw, begin + 4 + 2 * 8, "<H", page)
    obj = SmallMsf(raw)
    assert obj.stream(1) == b"live" and obj.stream(0) != b"live"


def test_old_persisted_pointers_and_directory_padding_are_not_offsets():
    raw = small_msf([None, b"data"])
    begin = directory_start(raw)
    raw = changed(raw, 56, "<I", 0xFEDCBA98)
    raw = changed(raw, begin + 2, "<H", 0xFFFF)
    raw = changed(raw, begin + 8, "<I", 0x12345678)
    raw = changed(raw, begin + 16, "<I", 0xFFFFFFFF)
    assert SmallMsf(raw).stream(1) == b"data"


@pytest.mark.parametrize("index", [-1, 2, 0xFFFF, True, False, 1.0, "1", None])
def test_invalid_indices_do_not_alias_or_imply_absence(index):
    with pytest.raises(ValueError, match="index"):
        SmallMsf(small_msf([None, b"data"])).stream(index)


@pytest.mark.parametrize("data", [None, "text", bytearray(100), memoryview(bytes(100)), b"", bytes(59), bytes(1024)])
def test_invalid_or_mutable_input_refuses(data):
    with pytest.raises(ValueError):
        SmallMsf(data)


@pytest.mark.parametrize("at,fmt,value", [(44, "<I", 512), (44, "<I", 8192), (48, "<H", 0),
    (48, "<H", 2), (50, "<H", 0), (50, "<H", 17), (50, "<H", 65535),
    (52, "<I", 0), (52, "<I", 11), (52, "<I", 0xFFFFFFFF)])
def test_invalid_header_fields_refuse(at, fmt, value):
    with pytest.raises(ValueError):
        SmallMsf(changed(small_msf([None, b"data"]), at, fmt, value))


@pytest.mark.parametrize("change", [lambda raw: raw[:-1], lambda raw: raw + b"\0", lambda raw: raw + bytes(1024),
                                  lambda raw: b"Microsoft C/C++ MSF 7.00" + raw[23:]])
def test_backing_and_format_cannot_be_substituted(change):
    with pytest.raises(ValueError):
        SmallMsf(change(small_msf([None, b"data"])))


@pytest.mark.parametrize("page", [0, 1, 8, 9, 16, 19, 65535])
@pytest.mark.parametrize("where", ["directory", "stream"])
def test_reserved_or_out_of_bounds_active_pages_refuse(page, where):
    raw = small_msf([None, b"data"])
    at = 60 if where == "directory" else directory_start(raw) + 4 + 2 * 8
    with pytest.raises(ValueError):
        SmallMsf(changed(raw, at, "<H", page))


def test_stream_cannot_overlap_current_directory():
    raw = small_msf([None, b"data"])
    page, = struct.unpack_from("<H", raw, 60)
    with pytest.raises(ValueError, match="overlap"):
        SmallMsf(changed(raw, directory_start(raw) + 20, "<H", page))


@pytest.mark.parametrize("same_stream", [False, True])
def test_active_stream_page_aliases_refuse(same_stream):
    raw = small_msf([None, b"x" * 1025] if same_stream else [None, b"x", b"y"])
    count = 2 if same_stream else 3
    at = directory_start(raw) + 4 + count * 8
    page, = struct.unpack_from("<H", raw, at)
    with pytest.raises(ValueError, match="overlap"):
        SmallMsf(changed(raw, at + 2, "<H", page))


def test_duplicate_directory_pages_refuse():
    raw = small_msf([None] + [b""] * 150)
    page, = struct.unpack_from("<H", raw, 60)
    with pytest.raises(ValueError, match="overlap"):
        SmallMsf(changed(raw, 62, "<H", page))


@pytest.mark.parametrize("where", ["header", "first_fpm", "second_fpm", "stream", "directory"])
def test_reserved_or_active_pages_marked_free_refuse(where):
    raw = bytearray(small_msf([None, b"data"]))
    page = {"header": 0, "first_fpm": 1, "second_fpm": 9, "stream": 17, "directory": 18}[where]
    raw[1024 + page // 8] |= 1 << (page % 8)
    with pytest.raises(ValueError, match="marked free"):
        SmallMsf(bytes(raw))


def test_inactive_fpm_and_free_slack_do_not_override_live_map():
    raw = bytearray(small_msf([None, b"data"]))
    raw[9 * 1024:17 * 1024] = b"\xff" * (8 * 1024)
    raw.extend(b"ignored free page".ljust(1024, b"x"))
    struct.pack_into("<H", raw, 50, 20)
    assert SmallMsf(bytes(raw)).stream(1) == b"data"


@pytest.mark.parametrize("count", [0, 4096, 65535, 3])
def test_invalid_or_truncated_stream_descriptions(count):
    raw = small_msf([None, b"data"])
    with pytest.raises(ValueError):
        SmallMsf(changed(raw, directory_start(raw), "<H", count))


@pytest.mark.parametrize("size", [0x80000000, 0xFFFFFFFE, 10000, 0, 0xFFFFFFFF])
def test_stream_size_cannot_hide_or_read_beyond_page_list(size):
    raw = small_msf([None, b"data"])
    with pytest.raises(ValueError):
        SmallMsf(changed(raw, directory_start(raw) + 12, "<I", size))


@pytest.mark.parametrize("delta", [-1, 1, 2])
def test_directory_must_be_exactly_consumed(delta):
    raw = small_msf([None, b"data"])
    length, = struct.unpack_from("<I", raw, 52)
    with pytest.raises(ValueError):
        SmallMsf(changed(raw, 52, "<I", length + delta))

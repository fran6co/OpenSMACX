"""Trace the real Fractal::init body against its pinned memory accesses.

The larger instrumented backing is test-only. It makes the original logical
reads observable without asserting that production owns those bytes or that
the original out-of-object effects are defined.
"""
from pathlib import Path
import os
import re
import shutil
import struct
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
INIT_SPAN = (0x005C1E20, 0x005C1F12)
WEIGHT_STEPS = (1, 2, 1, 0, -1, -2, -1, 0)
WEIGHTED_READS = 17 * 17 * 10  # center, eight neighbors, then center again


def _actual_init_body(source):
    from match_recovery.source import function_line
    from match_recovery.source.reader import _extract_forward_text

    line = function_line(source, "Fractal::init")
    return _extract_forward_text(source.splitlines(), line, "Fractal::init").strip()


def _pinned_image_path():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    if explicit is not None:
        return Path(explicit)
    candidates = [ROOT / ".opensmacx/game/terranx_original.exe"]
    candidates.extend(parent / ".opensmacx/game/terranx_original.exe"
                      for parent in ROOT.parents)
    return next((path for path in candidates if path.is_file()), candidates[-1])


def _load_pinned_image(path):
    from match_recovery.pe_coff import PeImage

    return PeImage(Path(path).read_bytes(), PIN)


def _pinned_init_listing():
    from match_recovery.model import Span
    from match_recovery.source.asm import Listing

    image = _load_pinned_image(_pinned_image_path())
    span = Span(*INIT_SPAN)
    return image, Listing(image.read(span), span.start)


def _instruction_at(listing, address):
    return next(item for item in listing.instructions if item.address == address)


def _assert_pinned_read_sequence():
    from match_recovery.model import Span

    image, listing = _pinned_init_listing()
    assert len(listing.instructions) == 86
    address_forms = {
        0x005C1E81: ("lea", "esi, [ebx + 0x169]"),
        0x005C1E87: ("xor", "edi, edi"),
        0x005C1E8C: ("xor", "edx, edx"),
        0x005C1E8E: ("movsx", "esi, byte ptr [esi + edx]"),
        0x005C1E95: ("mov", "eax, 0x66efc0"),
        0x005C1E9A: ("mov", "ecx, dword ptr [eax]"),
        0x005C1E9F: ("lea", "ecx, [ecx + ecx*4]"),
        0x005C1EA2: ("lea", "ecx, [edi + ecx*4]"),
        0x005C1EA5: ("add", "ecx, edx"),
        0x005C1EB1: ("cmp", "eax, 0x66efdc"),
        0x005C1EB8: ("mov", "esi, dword ptr [ebp - 4]"),
        0x005C1EBE: ("mov", "al, byte ptr [esi + edx]"),
        0x005C1EC5: ("mov", "byte ptr [ecx + ebx], al"),
    }
    for address, expected in address_forms.items():
        item = _instruction_at(listing, address)
        assert (item.mnemonic, item.op_str) == expected

    # In the original loop EDI is the row byte offset and advances by 19;
    # the inner column counter EDX advances by one. RadiusOffsetX entries
    # 1..8 are multiplied by 20, then the load adds object offset 0x17d
    # (381) and the column counter.
    assert (_instruction_at(listing, 0x005C1EA7).mnemonic,
            _instruction_at(listing, 0x005C1EA7).op_str) == (
        "movsx", "ecx, byte ptr [ecx + ebx + 0x17d]")
    assert (_instruction_at(listing, 0x005C1ECA).mnemonic,
            _instruction_at(listing, 0x005C1ECA).op_str) == ("add", "edi, 0x13")
    assert (_instruction_at(listing, 0x005C1ECD).mnemonic,
            _instruction_at(listing, 0x005C1ECD).op_str) == ("add", "esi, 0x13")
    assert (_instruction_at(listing, 0x005C1ED0).mnemonic,
            _instruction_at(listing, 0x005C1ED0).op_str) == ("cmp", "edi, 0x130")
    assert (_instruction_at(listing, 0x005C1EB6).mnemonic,
            _instruction_at(listing, 0x005C1EB6).op_str) == ("jle", "0x5c1e9a")
    assert (_instruction_at(listing, 0x005C1EC8).mnemonic,
            _instruction_at(listing, 0x005C1EC8).op_str) == ("jle", "0x5c1e8e")

    # The table starts at 0x66efbc; the original begins at +4 and consumes
    # eight signed dwords through +0x20. This is independent of the source
    # loop used to construct the host trace oracle.
    values = struct.unpack("<8i", image.read(Span(0x0066EFC0, 0x0066EFE0)))
    assert values == WEIGHT_STEPS
    return values


HARNESS_PREFIX = r'''
#include <cassert>
#include <cstdio>

enum { BACKING_SIZE = 800, EVENT_LIMIT = 5000 };
struct Event {
    char operation;
    int offset;
    unsigned char value;
};
static Event events[EVENT_LIMIT];
static int event_count;

static void record(char operation, int offset, unsigned char value) {
    assert(offset >= 0 && offset < BACKING_SIZE);
    assert(event_count < EVENT_LIMIT);
    events[event_count].operation = operation;
    events[event_count].offset = offset;
    events[event_count].value = value;
    ++event_count;
}

struct Cell {
    unsigned char *memory;
    int offset;
    Cell() : memory(0), offset(-1) {}
    void bind(unsigned char *bytes, int logical_offset) {
        memory = bytes;
        offset = logical_offset;
    }
    operator char() const {
        assert(memory != 0);
        record('R', offset, memory[offset]);
        return static_cast<char>(memory[offset]);
    }
    Cell &operator=(char value) {
        assert(memory != 0);
        const unsigned char byte = static_cast<unsigned char>(value);
        record('W', offset, byte);
        memory[offset] = byte;
        return *this;
    }
    Cell &operator=(const Cell &other) {
        const char value = other;
        return *this = value;
    }
};

struct Row {
    Cell *first;
    explicit Row(Cell *cells) : first(cells) {}
    Cell &operator[](int column) const { return first[column]; }
};

struct Plane {
    Cell *first;
    explicit Plane(Cell *cells) : first(cells) {}
    Row operator[](int row) const { return Row(first + row * 19); }
};

struct Grid {
    Cell *first;
    explicit Grid(Cell *cells) : first(cells) {}
    Plane operator[](int plane) const { return Plane(first + plane * 361); }
};

static int random_calls;
int rand() {
    ++random_calls;
    return (random_calls * 37 + 5) & 0x7fff;
}

static const int RadiusOffsetX[9] = { 0, 1, 2, 1, 0, -1, -2, -1, 0 };

struct Fractal {
    unsigned char backing[BACKING_SIZE];
    Cell cells[BACKING_SIZE];
    Grid grid_;
    Fractal() : grid_(cells) {
        for (int i = 0; i < BACKING_SIZE; ++i) {
            backing[i] = 0xa5;
            cells[i].bind(backing, i);
        }
    }
    void init();
};
'''


HARNESS_SUFFIX = r'''
int main() {
    Fractal fractal;
    fractal.init();
    std::printf("RANDS %d\n", random_calls);
    for (int i = 0; i < event_count; ++i) {
        std::printf("%c %d %u\n", events[i].operation, events[i].offset,
                    static_cast<unsigned int>(events[i].value));
    }
    return 0;
}
'''


def _oracle_trace():
    memory = [0xA5] * 800
    trace = []

    def read(offset):
        value = memory[offset]
        trace.append(("R", offset, value))
        return value

    def write(offset, value):
        value &= 0xFF
        trace.append(("W", offset, value))
        memory[offset] = value

    random_calls = 0
    for row in range(18):
        for column in range(18):
            random_calls += 1
            value = ((random_calls * 37 + 5) & 0x7FFF) & 0xF
            write(361 + 19 * row + column, value)
        write(361 + 19 * row + 18, read(361 + 19 * row))

    for column in range(18):
        write(361 + 19 * 18 + column, read(361 + column))

    for row in range(17):
        for column in range(17):
            center = 361 + 19 * row + column
            read(center)
            for step in WEIGHT_STEPS:
                read(381 + 19 * row + 20 * step + column)
            write(19 * row + column, read(center))

    for row in range(17):
        write(19 * row + 16, read(19 * row))
    for column in range(17):
        write(19 * 16 + column, read(column))
    return random_calls, trace


def _run_actual_body(body, compiler, tmp_path, optimization):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    cpp = tmp_path / "actual_fractal_init.cpp"
    binary = tmp_path / "actual_fractal_init"
    cpp.write_text(HARNESS_PREFIX + "\n" + body + "\n" + HARNESS_SUFFIX)
    built = subprocess.run(
        [executable, "-std=c++98", optimization, "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
    rows = ran.stdout.splitlines()
    assert rows and rows[0].startswith("RANDS "), ran.stdout
    random_calls = int(rows[0].split()[1])
    trace = []
    for row in rows[1:]:
        operation, offset, value = row.split()
        trace.append((operation, int(offset), int(value)))
    return random_calls, trace


def _first_trace_difference(actual, expected):
    for index, pair in enumerate(zip(actual, expected)):
        if pair[0] != pair[1]:
            return index, pair[0], pair[1]
    if len(actual) != len(expected):
        return min(len(actual), len(expected)), actual[len(expected):], expected[len(actual):]
    return None


def _without_weighted_row_stride(body):
    damaged, count = re.subn(r"band_row\s*\*\s*19", "band_row", body, count=1)
    if count == 0:
        damaged, count = re.subn(r"19\s*\*\s*band_row", "band_row", body, count=1)
    assert count == 1, "actual body lacks the pinned weighted row-stride expression"
    return damaged


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_fractal_init_matches_pinned_ordered_memory_trace(compiler, optimization, tmp_path):
    source = (ROOT / "src/world.cpp").read_text()
    body = _actual_init_body(source)
    pinned_steps = _assert_pinned_read_sequence()
    assert pinned_steps == WEIGHT_STEPS

    expected_random_calls, expected_trace = _oracle_trace()
    random_calls, trace = _run_actual_body(body, compiler, tmp_path, optimization)
    assert random_calls == expected_random_calls == 324
    difference = _first_trace_difference(trace, expected_trace)
    assert difference is None, (
        f"actual Fractal::init read/write trace diverges at {difference}; "
        f"actual events={len(trace)}, pinned-oracle events={len(expected_trace)}")
    assert sum(1 for operation, _, _ in trace if operation == "R") == (
        WEIGHTED_READS + 18 + 18 + 17 + 17)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_trace_control_rejects_missing_weighted_row_stride(compiler, optimization, tmp_path):
    source = (ROOT / "src/world.cpp").read_text()
    body = _without_weighted_row_stride(_actual_init_body(source))
    _, expected = _oracle_trace()
    _, trace = _run_actual_body(body, compiler, tmp_path, optimization)
    difference = _first_trace_difference(trace, expected)
    assert difference is not None, "row-stride damage escaped ordered trace control"
    assert [event for event in trace if event[0] == "W"] == [
        event for event in expected if event[0] == "W"], (
            "this defect changes dead reads, not the final grid writes")


def test_pinned_image_loader_refuses_missing_corrupt_and_bad_override(tmp_path, monkeypatch):
    missing = tmp_path / "missing.exe"
    with pytest.raises(FileNotFoundError):
        _load_pinned_image(missing)

    corrupt = tmp_path / "corrupt.exe"
    corrupt.write_bytes(b"not the pinned executable")
    with pytest.raises(ValueError, match="digest mismatch|PE"):
        _load_pinned_image(corrupt)

    monkeypatch.setenv("OPENSMACX_IMAGE", str(missing))
    assert _pinned_image_path() == missing
    with pytest.raises(FileNotFoundError):
        _load_pinned_image(_pinned_image_path())

    monkeypatch.setenv("OPENSMACX_IMAGE", "")
    assert _pinned_image_path() == Path("")
    with pytest.raises(IsADirectoryError):
        _load_pinned_image(_pinned_image_path())

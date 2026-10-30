"""Behavioral host control and source-map continuity for AlphaNet/Net transfer."""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from match_recovery.source import Mode, State, read


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/alphanet.cpp"
NET_SOURCE = ROOT / "src/net_class.cpp"
FIXTURE = Path(__file__).with_name("alphanet-pick-service-control.cpp").read_text()
MARKER = "// @@PICK_SERVICE_BODY@@"
SIGNATURE = "int AlphaNet::pick_service(int *allow_pbem)"

PICK_ADDRESS = 0x004E26E0
PICK_NAME = "?pick_service@AlphaNet@@QAEHPAH@Z"
PICK_SPANS = (
    (0x004E26E0, 0x004E29DF),
    (0x0065C630, 0x0065C664),
)
PICK_OLD_SPANS = PICK_SPANS[:1] + ((0x004E38E0, 0x004E38FB),) + PICK_SPANS[1:]
SERVICE_LIST_HELPER_SPANS = ((0x004E38E0, 0x004E38FB),)
NET_CHOOSE_ADDRESS = 0x00630D70
NET_CHOOSE_SPANS = (
    (0x00630D70, 0x00631044),
    (0x006634FF, 0x00663528),
)
NET_CHOOSE_OLD_SPANS = (
    (0x00630D70, 0x00631044),
    (0x004E38E0, 0x004E38FB),
    (0x006634FF, 0x00663528),
)
SERVICE_LIST_HELPER_NAME = "sub_4e38e0"
SERVICE_LIST_HELPER_SYMBOL = "??_DServiceList@@QAEXXZ"
JOIN_ADDRESS = 0x0062E540
JOIN_NAME = "?join_session@Net@@QAEHPAUSessionStruct@@PADPAD@Z"
JOIN_SYMBOL = "?join_session@Net@@QAEHPAVNetAllocationNode@@PAD1@Z"
JOIN_SPANS = (
    (0x0062E540, 0x0062EA97),
    (0x006634DF, 0x006634FF),
)


def mapped_owner(address, path):
    candidates = [
        record for record in read(ROOT / "src")
        if record.address == address and record.path == path.resolve()
    ]
    assert len(candidates) == 1, (
        "expected exactly one mapped owner at "
        + hex(address) + " in " + str(path)
    )
    return candidates[0]


def test_transferred_original_identities_have_real_source_owners():
    pick = mapped_owner(PICK_ADDRESS, SOURCE)
    assert pick.name == PICK_NAME
    assert pick.mode is Mode.BODY
    assert pick.state is State.IMPLEMENTED
    assert pick.image_spans == PICK_SPANS
    assert pick.kind == "game"
    assert pick.symbol in ("", PICK_NAME)  # Same current and original symbol.

    net_choose = mapped_owner(NET_CHOOSE_ADDRESS, NET_SOURCE)
    assert net_choose.name == "sub_630d70"
    assert net_choose.image_spans == NET_CHOOSE_SPANS
    assert net_choose.byte_exact is False and net_choose.semantic is False

    helper = mapped_owner(0x004E38E0, NET_SOURCE)
    assert helper.name == SERVICE_LIST_HELPER_NAME
    assert helper.symbol == SERVICE_LIST_HELPER_SYMBOL
    assert helper.mode is Mode.FILE
    assert helper.image_spans == SERVICE_LIST_HELPER_SPANS
    assert helper.byte_exact is False and helper.semantic is False

    # The split moves the shared callable span out of both callers exactly once.
    assert set(pick.image_spans) | set(net_choose.image_spans) | set(helper.image_spans) == (
        set(PICK_OLD_SPANS) | set(NET_CHOOSE_OLD_SPANS)
    )

    join = mapped_owner(JOIN_ADDRESS, NET_SOURCE)
    assert join.name == JOIN_NAME
    assert join.mode is Mode.BODY
    assert join.state is State.IMPLEMENTED
    assert join.image_spans == JOIN_SPANS
    assert join.kind == "game"
    assert join.symbol == JOIN_SYMBOL


def extract_definition(text):
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError("unclosed AlphaNet::pick_service definition")


def build_and_run(compiler, body, directory, name):
    program = FIXTURE.replace(MARKER, body, 1)
    assert program.count(MARKER) == 0
    source = directory / (name + ".cpp")
    binary = directory / name
    source.write_text(program)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_pick_service_body_and_mutants(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " host C++ compiler unavailable")
    body = extract_definition(SOURCE.read_text())
    assert body.count("providers.seek_pos(-1)") == 1
    assert body.count("providers.seek_pos(selected)") == 1
    with tempfile.TemporaryDirectory(prefix="alphanet-pick-service-") as directory:
        directory = Path(directory)
        result = build_and_run(compiler, body, directory, "current")
        assert result.returncode == 0, result.stdout + result.stderr

        wrong_provider = body.replace(
            "providers.seek_pos(selected)",
            "providers.seek_pos(selected - 1)", 1,
        )
        assert wrong_provider != body
        mutant = build_and_run(compiler, wrong_provider, directory, "wrong-provider")
        assert mutant.returncode != 0

        lost_pbem_sentinel = body.replace(
            "selected == 9999", "selected == 9998", 1,
        )
        assert lost_pbem_sentinel != body
        mutant = build_and_run(compiler, lost_pbem_sentinel, directory, "wrong-pbem")
        assert mutant.returncode != 0

        old_guard = (
            "    struct AllocationHandoffScope {\n"
            "        Heap *saved;\n"
            "        AllocationHandoffScope() : saved(StringAllocationHeap) {\n"
            "            StringAllocationHeap = 0;\n"
            "        }\n"
            "        ~AllocationHandoffScope() { StringAllocationHeap = saved; }\n"
            "    } allocation_handoff;\n"
        )
        guarded_body = body.replace("    ServiceList providers;", old_guard + "    ServiceList providers;", 1)
        assert guarded_body != body
        mutant = build_and_run(compiler, guarded_body, directory, "old-guard-owner-loss")
        assert mutant.returncode != 0  # The old guard makes the list use the wrong allocator origin.

        stale_cursor = body.replace(
            "if (providers.seek_pos(selected) == 0) providers.seek_pos(0);",
            "providers.seek_pos(selected);", 1,
        )
        assert stale_cursor != body
        mutant = build_and_run(compiler, stale_cursor, directory, "stale-cursor")
        assert mutant.returncode != 0

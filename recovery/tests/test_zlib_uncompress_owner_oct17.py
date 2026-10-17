"""Narrow ownership continuity and host behavior controls for zlib 1.0.2 uncompress."""

from dataclasses import fields, replace
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.component_source import BEGIN, END, parse_component, render_component
from match_recovery.binding_source import parse as parse_binding_evidence
from match_recovery.model import Origin
from match_recovery.source import read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
TASK_BASE = "bd1cca03b368e6266155f5470524b31f90521559"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ADDRESS = 0x00634330
END_ADDRESS = 0x006343BD
COMPONENT = "src/vendor/zlib.recovery.h"
NETDAEMON = "src/netdaemon.cpp"


def git_text(revision: str, relative: str) -> str:
    return subprocess.check_output(
        ["git", "show", f"{revision}:{relative}"], cwd=ROOT, text=True,
    )


def _record(text: str, relative: str, address: int):
    return next(row for row in read_text(text, ROOT / relative) if row.address == address)


def _binding_rationale(text: str, relative: str, address: int):
    rows = read_text(text, ROOT / relative)
    row = next(row for row in rows if row.address == address)
    return parse_binding_evidence(text, rows)[row.line]


def historical_component_view(candidate: str) -> str:
    """Invert only the authorized append/move; refuse any additional header diff."""
    baseline = git_text(TASK_BASE, COMPONENT)
    marker = f"// ORIGINAL: 0x{ADDRESS:08X} "
    assert candidate.count(marker) == 1
    row = _record(candidate, COMPONENT, ADDRESS)
    assert row.image_spans == ((ADDRESS, END_ADDRESS),)
    assert row.byte_exact is True and row.semantic is False
    manifest = parse_component(candidate)
    assert manifest.bindings[-1].origin == Origin(PIN, "pe:va", ADDRESS)
    assert manifest.bindings[-1].symbol == "_uncompress"
    without_binding = replace(manifest, bindings=manifest.bindings[:-1])
    start = candidate.index(marker)
    block_start = candidate.rfind("/*", 0, start)
    block_end = candidate.index("*/", start) + 2
    assert block_start >= 0 and candidate[block_start:block_end].count(marker) == 1
    prefix = candidate[:block_start].rstrip("\n") + "\n\n" + candidate[block_end:].lstrip("\n")
    begin = prefix.index(BEGIN)
    end = prefix.index(END, begin) + len(END)
    rendered = render_component(without_binding)
    rendered_block = rendered[rendered.index(BEGIN):]
    projected = prefix[:begin] + rendered_block + prefix[end:].lstrip("\n")
    assert projected == baseline, "new migration inverse must leave every historical byte intact"
    return projected


def historical_netdaemon_view(candidate: str) -> str:
    """Invert precisely the frozen source migration relative to its task base."""
    baseline = git_text(TASK_BASE, NETDAEMON)
    old_decl = (
        "int __cdecl netdaemon_inflate_buffer(unsigned char *, unsigned long *,\n"
        "                                      unsigned char *, unsigned int);\n"
    )
    assert baseline.count(old_decl) == 1
    projected = baseline.replace(old_decl, "", 1)
    old_heading = "// Original mapped file-transfer wrappers, now implemented in this owner TU.\n"
    new_heading = "// Original mapped file-transfer deflate helper, implemented in this owner TU.\n"
    assert projected.count(old_heading) == 1
    projected = projected.replace(old_heading, new_heading, 1)
    old_call = (
        "netdaemon_inflate_buffer(mapped, &decompressed_size,\n"
        "                                    NetDaemonFileTransferReceiveBuffer,\n"
        "                                    static_cast<unsigned int>(compressed_size));"
    )
    new_call = (
        "uncompress(mapped, &decompressed_size,\n"
        "                       NetDaemonFileTransferReceiveBuffer, compressed_size);"
    )
    assert projected.count(old_call) == 1
    projected = projected.replace(old_call, new_call, 1)
    marker = f"// ORIGINAL: 0x{ADDRESS:08X} "
    assert projected.count(marker) == 1
    row = _record(projected, NETDAEMON, ADDRESS)
    assert row.image_spans == ((ADDRESS, END_ADDRESS),)
    block_start = projected.rfind("/*", 0, projected.index(marker))
    assert block_start >= 0
    projected = projected[:block_start].rstrip() + "\n"
    assert projected == candidate, "new migration inverse must leave every other source byte intact"
    return git_text(TASK_BASE, NETDAEMON)


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated source body: {signature}")


def upstream_body() -> str:
    source = (ROOT / "src/vendor/zlib-1.0.2/uncompr.c").read_text(encoding="latin1")
    body = _body(source, "int uncompress (dest, destLen, source, sourceLen)")
    assert "stream.opaque" not in body
    assert "memset(&stream" not in body
    return body


def test_uncompress_owner_preserves_original_record_and_old_manifest():
    baseline_component = git_text(TASK_BASE, COMPONENT)
    current_text = (ROOT / COMPONENT).read_text(encoding="utf-8")
    old_manifest = parse_component(baseline_component)
    manifest = parse_component(current_text)
    assert len(old_manifest.bindings) == 58
    assert manifest.bindings[:-1] == old_manifest.bindings
    assert replace(manifest, bindings=manifest.bindings[:-1]) == old_manifest
    assert historical_component_view(current_text) == baseline_component

    old_text = git_text(TASK_BASE, NETDAEMON)
    old = _record(old_text, NETDAEMON, ADDRESS)
    new = _record(current_text, COMPONENT, ADDRESS)
    excluded = {"path", "line", "region", "origin", "symbol"}
    for field in fields(old):
        if field.name not in excluded:
            assert getattr(new, field.name) == getattr(old, field.name), field.name
    assert old.name == new.name == "sub_634330"
    assert old.symbol == "?netdaemon_inflate_buffer@@YAHPAEPAK0I@Z"
    assert new.symbol == "_uncompress"
    assert new.image_spans == ((ADDRESS, END_ADDRESS),)
    assert new.byte_exact is True and new.semantic is False
    old_lessons = tuple((item.kind, item.key, item.prose, tuple(line for _, line in item.lines))
                        for item in (old.origin or ()))
    new_lessons = tuple((item.kind, item.key, item.prose, tuple(line for _, line in item.lines))
                        for item in (new.origin or ()))
    assert new_lessons == old_lessons

    old_rationale = _binding_rationale(old_text, NETDAEMON, ADDRESS)
    new_rationale = _binding_rationale(current_text, COMPONENT, ADDRESS)
    assert new_rationale["evidence"][:len(old_rationale["evidence"])] == old_rationale["evidence"]
    assert ("Historical binding reason retained: " + old_rationale["reason"]
            in new_rationale["evidence"])
    assert "pinned original span" in new_rationale["reason"]
    assert "selection remains unproved" in new_rationale["reason"]

    binding = manifest.bindings[-1]
    assert binding.origin == Origin(PIN, "pe:va", ADDRESS)
    assert [(span.start, span.end) for span in binding.spans] == [(ADDRESS, END_ADDRESS)]
    assert (binding.symbol, binding.unit) == ("_uncompress", "uncompr.c")
    evidence = binding.evidence
    assert "second-argument-unused transcription is false" in " ".join(new_rationale["evidence"])
    assert "application archive-member selection" in evidence
    assert "shared version literal is not separately bound" in evidence
    assert "selection and complete data-target ownership remain unproved" in evidence
    assert sum(row.address == ADDRESS for row in read_file(ROOT / COMPONENT)) == 1
    assert not any(row.address == ADDRESS for row in read_file(ROOT / NETDAEMON))

    netdaemon = (ROOT / NETDAEMON).read_text(encoding="utf-8")
    assert "netdaemon_inflate_buffer" not in netdaemon
    assert "uncompress(mapped, &decompressed_size," in netdaemon
    assert historical_netdaemon_view(netdaemon) == old_text


def test_historical_inverses_reject_unrelated_source_and_claim_damage():
    current_netdaemon = (ROOT / NETDAEMON).read_text(encoding="utf-8")
    with pytest.raises(AssertionError):
        historical_netdaemon_view(current_netdaemon + "\n// unrelated body damage\n")

    current_component = (ROOT / COMPONENT).read_text(encoding="utf-8")
    changed_old_binding = current_component.replace(
        "COFF bytes/boundary/references pass",
        "COFF bytes/boundary/references changed", 1,
    )
    assert changed_old_binding != current_component
    with pytest.raises(AssertionError):
        historical_component_view(changed_old_binding)

    claim_line = next(line for line in current_component.splitlines()
                      if line.startswith("// ORIGINAL: 0x00634330 "))
    assert " BYTE_EXACT" in claim_line
    changed_claim_line = claim_line.replace(" BYTE_EXACT", "", 1)
    changed_claim = current_component.replace(claim_line, changed_claim_line, 1)
    with pytest.raises(AssertionError):
        historical_component_view(changed_claim)


def _fixture(body: str) -> str:
    return r'''#include <assert.h>
#include <limits.h>
typedef unsigned char Bytef;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef unsigned long uLongf;
typedef struct z_stream_s {
    Bytef *next_in; uInt avail_in; uLong total_in;
    Bytef *next_out; uInt avail_out; uLong total_out;
    void *zalloc; void *zfree; void *opaque;
} z_stream;
typedef void *(*alloc_func)(void *, uInt, uInt);
typedef void (*free_func)(void *, void *);
#define ZEXPORT
#define Z_NULL 0
enum { Z_OK=0, Z_STREAM_END=1, Z_BUF_ERROR=-5, Z_MEM_ERROR=-4,
       Z_NEED_DICT=2, Z_FINISH=4 };
static int init_result, inflate_result, end_result;
static int init_calls, inflate_calls, end_calls;
static Bytef *expected_dest, *expected_source;
static uLong expected_source_length, expected_capacity, produced;
int inflateInit(z_stream *stream) {
    ++init_calls;
    assert(stream->next_out == expected_dest);
    assert(stream->avail_out == (uInt)expected_capacity);
    assert(stream->next_in == expected_source);
    assert(stream->avail_in == (uInt)expected_source_length);
    assert(stream->zalloc == 0 && stream->zfree == 0);
    return init_result;
}
int inflate(z_stream *stream, int flush) {
    ++inflate_calls;
    assert(flush == Z_FINISH);
    assert(stream->next_out == expected_dest && stream->next_in == expected_source);
    stream->total_out = produced;
    return inflate_result;
}
int inflateEnd(z_stream *stream) { (void)stream; ++end_calls; return end_result; }
ACTUAL_UNCOMPRESS
static void reset(void) {
    init_calls=inflate_calls=end_calls=0;
    init_result=Z_OK; inflate_result=Z_STREAM_END; end_result=0;
    expected_source_length=11; expected_capacity=32; produced=17;
}
int main(void) {
    Bytef dest[32], source[11]; uLongf length;
    expected_dest=dest; expected_source=source;

    reset(); length=expected_capacity; init_result=Z_MEM_ERROR;
    assert(uncompress(dest,&length,source,expected_source_length)==Z_MEM_ERROR);
    assert(init_calls==1 && inflate_calls==0 && end_calls==0 && length==expected_capacity);

    reset(); length=expected_capacity; inflate_result=Z_OK; end_result=91;
    assert(uncompress(dest,&length,source,expected_source_length)==Z_OK);
    assert(init_calls==1 && inflate_calls==1 && end_calls==1 && length==expected_capacity);

    reset(); length=expected_capacity; inflate_result=Z_NEED_DICT; end_result=92;
    assert(uncompress(dest,&length,source,expected_source_length)==Z_NEED_DICT);
    assert(init_calls==1 && inflate_calls==1 && end_calls==1 && length==expected_capacity);

    reset(); length=expected_capacity; end_result=7;
    assert(uncompress(dest,&length,source,expected_source_length)==7);
    assert(init_calls==1 && inflate_calls==1 && end_calls==1 && length==produced);

    reset(); length=expected_capacity; end_result=0;
    assert(uncompress(dest,&length,source,expected_source_length)==Z_OK);
    assert(end_calls==1 && length==produced);

#if ULONG_MAX > UINT_MAX
    reset(); length=(uLongf)UINT_MAX+1UL;
    assert(uncompress(dest,&length,source,expected_source_length)==Z_BUF_ERROR);
    assert(init_calls==0 && inflate_calls==0 && end_calls==0);
#endif
    return 0;
}
'''.replace("ACTUAL_UNCOMPRESS", body)


def _compile_and_run(tmp_path: Path, compiler: str, body: str, *, expect_success: bool) -> None:
    source = tmp_path / "uncompress.c"
    executable = tmp_path / "uncompress"
    source.write_text(_fixture(body), encoding="latin1")
    built = subprocess.run(
        [compiler, "-std=gnu89", "-O0", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    if expect_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "behavior mutation escaped actual-source control"


@pytest.mark.parametrize("compiler_name", ("cc", "gcc", "clang"))
def test_actual_unmodified_upstream_kr_behavior(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    _compile_and_run(tmp_path, compiler, upstream_body(), expect_success=True)


@pytest.mark.parametrize("damage", ("changed_zero_status", "omit_cleanup", "early_size_write",
                                     "return_cleanup_status"))
def test_upstream_behavior_mutations_are_observed(damage, tmp_path):
    body = upstream_body()
    if damage == "changed_zero_status":
        before = "        inflateEnd(&stream);\n        return err;"
        after = "        inflateEnd(&stream);\n        return err == Z_OK ? -3 : err;"
    elif damage == "omit_cleanup":
        before = "        inflateEnd(&stream);\n        return err;"
        after = "        return err;"
    elif damage == "early_size_write":
        before = "    if (err != Z_STREAM_END) {"
        after = "    *destLen = stream.total_out;\n    if (err != Z_STREAM_END) {"
    else:
        before = "        inflateEnd(&stream);\n        return err;"
        after = "        inflateEnd(&stream);\n        return inflateEnd(&stream);"
    assert body.count(before) == 1, (damage, before, body.count(before))
    mutated = body.replace(before, after, 1)
    compiler = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not compiler:
        pytest.skip("host C compiler required")
    _compile_and_run(tmp_path, compiler, mutated, expect_success=False)

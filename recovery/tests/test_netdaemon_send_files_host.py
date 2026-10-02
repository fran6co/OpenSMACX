"""Strict source extractor plus host behavioral controls for send_files."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).parents[2]
HARNESS = Path(__file__).with_name("netdaemon-send-files-harness.cpp")


def extract_method(text: str) -> str:
    signature = "int NetDaemon::send_files()"
    start = text.find(signature)
    if start < 0:
        raise AssertionError(f"required actual definition missing: {signature}")
    brace = text.find("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated send_files definition")


def owner_source() -> str:
    text = (ROOT / "src/netdaemon.cpp").read_text()
    # Keep source declarations/accessors verbatim with the function. The one
    # 32-bit-owner assertion is removed only for the 64-bit host fixture; the
    # fixture stores an 8-byte pointer representation across adjacent words.
    begin = text.find("struct NetDaemonFileIndexRecord {")
    end = text.find("int NetDaemon::send_files()")
    if begin < 0 or end < 0:
        raise AssertionError("complete file-transfer owner declarations/body required")
    globals_start = text.find("// RECOVERY_OBJECT: v1 0x0093F79C", begin, end)
    if globals_start >= 0:
        end = globals_start
    prefix = text[begin:end]
    prefix = prefix.replace(
        'static_assert(sizeof(void *) == sizeof(uint32_t),\n'
        '              "the native transfer pointer word is 32-bit");',
        "/* host-only: source target is 32-bit */")
    return prefix + extract_method(text)


def compile_and_run(owner: str) -> subprocess.CompletedProcess:
    template = HARNESS.read_text()
    with tempfile.TemporaryDirectory(prefix="net-send-files-ctest-") as scratch:
        cpp = Path(scratch) / "harness.cpp"
        exe = Path(scratch) / "harness"
        cpp.write_text(template.replace("@@OWNER_SOURCE@@", owner))
        compilers = []
        for compiler in ("g++", "clang++"):
            found = subprocess.run(["sh", "-c", f"command -v {compiler}"],
                                   capture_output=True, text=True)
            if found.returncode == 0:
                compilers.append(compiler)
        assert compilers
        for compiler in compilers:
            build = subprocess.run(
                [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                 "-Wno-unused-function", "-Wno-unused-variable",
                 "-Wno-write-strings",
                 "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                 str(cpp), "-o", str(exe)], capture_output=True, text=True)
            assert build.returncode == 0, build.stdout + build.stderr
            env = dict(os.environ)
            env["ASAN_OPTIONS"] = "detect_leaks=0"
            run = subprocess.run([str(exe)], capture_output=True, text=True, env=env)
            if run.returncode:
                return run
            assert "send_files host controls passed" in run.stdout
        return run


def test_actual_send_files_owner():
    result = compile_and_run(owner_source())
    assert result.returncode == 0, result.stdout + result.stderr


def test_native_transfer_damage_controls():
    owner = owner_source()
    mutants = (
        ("while (NetDaemonFileTransferAckMask != send_mask)",
         "while (NetDaemonFileTransferAckMask == send_mask)"),
        ("while (remaining > 0x7FF)", "while (remaining > 0x800)"),
        ("NetDaemonNet.process_slots_[slot].status_bytes_[0] > 1",
         "NetDaemonNet.process_slots_[slot].status_bytes_[0] > 0"),
        ("NetDaemonNet.send_group(send_buffer, remaining + 0x20,",
         "NetDaemonNet.send_group(send_buffer, 0x820,"),
    )
    for old, new in mutants:
        assert old in owner, f"missing mutation anchor: {old}"
        result = compile_and_run(owner.replace(old, new, 1))
        assert result.returncode != 0, f"mutation survived: {old}"

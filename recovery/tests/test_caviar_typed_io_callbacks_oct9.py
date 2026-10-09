"""Actual-source host controls for Caviar's six typed voxel callbacks."""
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
SOURCE = ROOT / "src/caviar.cpp"
HEADER = ROOT / "src/caviar.h"


def extract(text, signature):
    start = text.find(signature)
    while start >= 0:
        opening = text.find("{", start)
        semi = text.find(";", start)
        if opening >= 0 and (semi < 0 or opening < semi):
            break
        start = text.find(signature, start + len(signature))
    assert start >= 0, "missing current source definition: " + signature
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unclosed source definition: " + signature)


PREAMBLE = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
struct Caviar {
  static void *vx_malloc(unsigned long);
  static void vx_free(void *);
  static int vx_read(int, char *, long);
  static int vx_write(int, void *, long);
  static long vx_seek(int, long, int);
  static long vx_tell(int);
};
static int memory_result;
static int read_result;
static int write_result;
static long seek_result;
static long tell_result;
static int call_count;
static int free_count;
static int expected_fd;
static long expected_count;
static long expected_offset;
static int expected_origin;
static char *expected_buffer;
static int last_error;
static int cpu_detections;
static int CaviarVoxelAlgoFlag = 2;
static int vox_msg_no_setup = 1;
static int vox_msg_cpu_unsupported = 2;
static int vox_msg_setup_empty = 3;
static int vox_msg_reading_file = 4;
static uint32_t vox_unk_9c0b78;
static char (*vox_user_chunk)(int, void *, void *);
static void vox_stage_error(int error) { last_error = error; }
static void vox_detect_cpu(void) { ++cpu_detections; }
static void *mem_get(unsigned long size) {
  assert(size == 37ul);
  return &memory_result;
}
static void free(void *value) { assert(value != 0); ++free_count; }
static int _read(int fd, char *buffer, long count) {
  ++call_count;
  assert(fd == expected_fd && buffer == expected_buffer && count == expected_count);
  return read_result;
}
static int _write(int fd, void *buffer, long count) {
  assert(fd == expected_fd && buffer == expected_buffer && count == expected_count);
  return write_result;
}
static long _lseek(int fd, long offset, int origin) {
  assert(fd == expected_fd && offset == expected_offset && origin == expected_origin);
  return seek_result;
}
static long _tell(int fd) { assert(fd == expected_fd); return tell_result; }
static int read_script[4];
static int read_script_length;
static int read_script_index;
static int read_script_bytes;
static int scripted_read(int fd, char *buffer, long count) {
  ++call_count;
  assert(fd == expected_fd && buffer == expected_buffer + read_script_bytes);
  assert(count == expected_count - read_script_bytes);
  int amount = read_script[read_script_index++];
  for (int index = 0; index < amount; ++index) buffer[index] = 'R';
  read_script_bytes += amount;
  return amount;
}
'''


def compile_actual_source(mutant=None):
    source = SOURCE.read_text()
    header = HEADER.read_text()
    callback_type = extract(header, "struct VoxIoCallbacks {") + ";"
    shared_type = extract(header, "struct CaviarOutputMessageWindowCandidate {") + ";"
    assert "offsetof(VoxIoCallbacks, allocate) == 0" in source
    assert "offsetof(VoxIoCallbacks, release) == 4" in source
    assert "offsetof(VoxIoCallbacks, read) == 8" in source
    assert "offsetof(VoxIoCallbacks, write) == 12" in source
    assert "offsetof(VoxIoCallbacks, seek) == 16" in source
    assert "offsetof(VoxIoCallbacks, tell) == 20" in source
    shared_storage = (shared_type + "\n" +
        "CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {};\n")
    actual = [
        callback_type,
        shared_storage,
        extract(source, "void *Caviar::vx_malloc("),
        extract(source, "void Caviar::vx_free("),
        extract(source, "int Caviar::vx_read("),
        extract(source, "int Caviar::vx_write("),
        extract(source, "long Caviar::vx_seek("),
        extract(source, "long Caviar::vx_tell("),
        extract(source, "char __cdecl vox_init_callbacks("),
        extract(source, "extern \"C\" int __cdecl vox_read_exact("),
    ]
    assert "callbacks->allocate" in actual[8]
    assert "callbacks->tell" in actual[8]
    assert "memcpy(&CaviarOutputMessageWindow, callbacks, sizeof(*callbacks))" in actual[8]
    assert "while (total < count)" in actual[9]
    assert "if (read < 0)" in actual[9]
    assert "return _read(fd, buffer, count);" in actual[4]
    assert "return _write(fd, buffer, count);" in actual[5]
    assert "return _lseek(fd, offset, origin);" in actual[6]
    assert "return _tell(fd);" in actual[7]
    initializer_start = source.index("    VoxIoCallbacks vx_callbacks = {")
    initializer_end = source.index(";", initializer_start) + 1
    initializer = source[initializer_start:initializer_end].replace("vx_callbacks", "source")
    slot_macros = "\n".join(
        line for line in source.splitlines()
        if line.startswith(("#define vox_alloc_slot ", "#define vox_free_slot ",
                            "#define vox_read_slot ", "#define vox_write_slot ",
                            "#define vox_seek_slot ", "#define vox_tell_slot "))
    )
    callback_types = "\n".join(
        line for line in source.splitlines()
        if line.startswith(("typedef void *(__cdecl *VoxAllocateCallback)",
                            "typedef void (__cdecl *VoxReleaseCallback)",
                            "typedef int (__cdecl *VoxReadCallback)",
                            "typedef int (__cdecl *VoxWriteCallback)",
                            "typedef long (__cdecl *VoxSeekCallback)",
                            "typedef long (__cdecl *VoxTellCallback)"))
    )
    load_helper = ("template <typename Callback>\n" +
                   extract(source, "static Callback vox_load_callback("))

    # Compile the real callback type and producer initializer; only unrelated
    # engine globals and the CRT functions are host stubs.
    assert "int (__cdecl *read)(int, char *, long);" in callback_type
    type_end = PREAMBLE.index("struct Caviar {")
    type_start = PREAMBLE.index("#define __cdecl") + len("#define __cdecl")
    preamble = (PREAMBLE[:type_start] + "\n" + callback_type + "\n" + shared_storage +
                "\n" + callback_types + "\n" + load_helper + "\n" +
                slot_macros + "\n" + PREAMBLE[type_end:])
    if mutant == "wrong_read_slot":
        slot_macros = slot_macros.replace(
            "offsetof(VoxIoCallbacks, read)",
            "offsetof(VoxIoCallbacks, write)")
        preamble = preamble.replace(
            next(line for line in preamble.splitlines()
                 if line.startswith("#define vox_read_slot ")),
            next(line for line in slot_macros.splitlines()
                 if line.startswith("#define vox_read_slot ")),
        )
    elif mutant == "drop_read_return":
        actual[4] = actual[4].replace("return _read(fd, buffer, count);", "return 0;")
    elif mutant == "skip_seek_tell_validation":
        actual[8] = actual[8].replace(
            "callbacks->seek != 0 && callbacks->tell != 0", "1")
    elif mutant == "copy_before_validation":
        actual[8] = actual[8].replace(
            "    if (callbacks->allocate != 0",
            "    memcpy(&CaviarOutputMessageWindow, callbacks, sizeof(*callbacks));\n"
            "    if (callbacks->allocate != 0",
            1,
        )
    elif mutant == "zero_read_is_error":
        actual[9] = actual[9].replace("if (read < 0)", "if (read <= 0)")
    main = r'''
static void ck(bool value) { assert(value); }
int main() {
  /* CALLBACK TABLE INITIALIZER */
  char buffer[8] = {};
  expected_fd = 7; expected_buffer = buffer; expected_count = 5;
  expected_offset = -3; expected_origin = 1;
  memory_result = 91; read_result = -2; write_result = 4;
  seek_result = -1; tell_result = 1234;

  ck(vox_init_callbacks(&source, 1) == 0);
  ck(cpu_detections == 1 && last_error == 0);
  ck(vox_alloc_slot(37) == &memory_result);
  expected_buffer = buffer;
  ck(vox_read_slot(expected_fd, buffer, expected_count) == -2);
  ck(call_count == 1);
  ck(vox_write_slot(expected_fd, buffer, expected_count) == 4);
  ck(vox_seek_slot(expected_fd, expected_offset, expected_origin) == -1);
  ck(vox_tell_slot(expected_fd) == 1234);
  vox_free_slot(0);
  ck(free_count == 0);
  vox_free_slot(buffer);
  ck(free_count == 1);

  // Null-table and incomplete-table rejection happen before replacing slots.
  VoxIoCallbacks installed;
  memcpy(&installed, &CaviarOutputMessageWindow, sizeof(installed));
  ck(vox_init_callbacks(0, 1) == 1 && last_error == vox_msg_no_setup);
  VoxIoCallbacks incomplete = source; incomplete.seek = 0;
  ck(vox_init_callbacks(&incomplete, 1) == 1 && last_error == vox_msg_setup_empty);
  VoxIoCallbacks after_rejection;
  memcpy(&after_rejection, &CaviarOutputMessageWindow, sizeof(after_rejection));
  ck(memcmp(&installed, &after_rejection, sizeof(installed)) == 0);

  // A zero-byte read is retried; only negative read reports the error.
  VoxIoCallbacks scripted = source; scripted.read = &scripted_read;
  ck(vox_init_callbacks(&scripted, 1) == 0);
  expected_buffer = buffer; expected_count = 3; call_count = 0;
  read_script[0] = 0; read_script[1] = 1; read_script[2] = 2;
  read_script_length = 3; read_script_index = 0; read_script_bytes = 0;
  ck(vox_read_exact(expected_fd, buffer, 3) == 0);
  ck(read_script_index == read_script_length && call_count == 3);
  ck(buffer[1] == 'R' && buffer[2] == 'R');
  scripted.read = &Caviar::vx_read;
  ck(vox_init_callbacks(&scripted, 1) == 0);
  read_result = -1;
  ck(vox_read_exact(expected_fd, buffer, 3) == -1);
  ck(last_error == vox_msg_reading_file);

  // A bad algorithm flag is reported only after the valid table is copied.
  CaviarVoxelAlgoFlag = 6;
  ck(vox_init_callbacks(&source, 1) == 1 && last_error == vox_msg_cpu_unsupported);
  VoxIoCallbacks after_bad_flag;
  memcpy(&after_bad_flag, &CaviarOutputMessageWindow, sizeof(after_bad_flag));
  ck(memcmp(&source, &after_bad_flag, sizeof(source)) == 0);
  return 0;
}
'''
    # The installer references current engine globals but this control needs
    # only their observable update/validation semantics.
    main = main.replace("  /* CALLBACK TABLE INITIALIZER */", initializer)
    preamble += "\n" + "\n".join(actual[2:]) + "\n" + main
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    assert available, "GCC or Clang required for actual-source callback controls"
    with tempfile.TemporaryDirectory(prefix="caviar-typed-io-") as directory:
        directory = Path(directory)
        cpp = directory / "actual_callbacks.cpp"
        cpp.write_text(preamble)
        for compiler in available:
            executable = directory / Path(compiler).name.replace("+", "p")
            build = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(executable)],
                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            )
            assert build.returncode == 0, build.stdout
            run = subprocess.run([str(executable)], text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if mutant is None:
                assert run.returncode == 0, run.stdout
            else:
                assert run.returncode != 0, "mutation survived: " + mutant


def test_actual_source_contracts_installation_dispatch_and_read_quirk():
    compile_actual_source()


def test_damage_mutants_are_compiled_and_rejected():
    for mutant in ("drop_read_return", "skip_seek_tell_validation",
                   "wrong_read_slot", "copy_before_validation",
                   "zero_read_is_error"):
        compile_actual_source(mutant)

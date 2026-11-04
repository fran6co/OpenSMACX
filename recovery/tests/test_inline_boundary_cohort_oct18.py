"""Live guards for the remaining owner-specific inline-boundary profiles."""
from __future__ import annotations

import copy
import json
import pathlib
import re
import shlex
import shutil
import subprocess

import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
BASE_REVISION = "28ce5634da32d7f3223dca623ca1fe76940749d8"
PROFILE_OWNERS = (
    "src/pulldown.cpp",
    "src/dipedit.cpp",
)
OPTIMIZED_INLINE_OPTION = "$<$<NOT:$<CONFIG:Debug>>:/Ob1>"
OWNER_FLAGS = {
    "src/pulldown.cpp": (OPTIMIZED_INLINE_OPTION,),
    "src/dipedit.cpp": (OPTIMIZED_INLINE_OPTION,),
}
MEASURED_PULLDOWN_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob1"]
MEASURED_DIPEDIT_PROFILE = ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX", "/Ob1"]


def _git_text(path: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{BASE_REVISION}:{path}"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def _cmake_commands(text: str):
    """Yield (command, body) pairs, honoring quoted strings and nested parens."""
    index = 0
    size = len(text)
    while index < size:
        if text[index] == "#":
            newline = text.find("\n", index)
            index = size if newline < 0 else newline + 1
            continue
        match = re.match(r"[A-Za-z_][A-Za-z0-9_]*", text[index:])
        if match is None:
            index += 1
            continue
        name = match.group(0)
        end_name = index + len(name)
        cursor = end_name
        while cursor < size and text[cursor].isspace():
            cursor += 1
        if cursor >= size or text[cursor] != "(":
            index = end_name
            continue

        start = cursor + 1
        depth = 1
        cursor += 1
        quoted = False
        escaped = False
        while cursor < size and depth:
            char = text[cursor]
            if quoted:
                if escaped:
                    escaped = False
                elif char == "\\":
                    escaped = True
                elif char == '"':
                    quoted = False
            elif char == '"':
                quoted = True
            elif char == "#":
                newline = text.find("\n", cursor)
                cursor = size if newline < 0 else newline
            elif char == "(":
                depth += 1
            elif char == ")":
                depth -= 1
            cursor += 1
        if depth:
            raise AssertionError(f"unterminated CMake command {name}")
        yield name.casefold(), text[start:cursor - 1]
        index = cursor


def _source_profiles(cmake: str):
    blocks = []
    for command, body in _cmake_commands(cmake):
        if command == "set_property":
            append = re.search(r"\bSOURCE\b(.*?)\bAPPEND(?:_STRING)?\s+PROPERTY\b", body, re.DOTALL)
            if append is None:
                continue
            files_text = append.group(1)
            option_text = body[append.end():]
        elif command == "set_source_files_properties":
            append = re.search(r"\bAPPEND(?:_STRING)?\s+PROPERTY\b", body)
            properties = re.search(r"\bPROPERTIES\b", body)
            split = append or properties
            if split is None:
                continue
            files_text = body[:split.start()]
            option_text = body[split.end():]
        else:
            continue
        files = tuple(re.findall(r"src/[A-Za-z0-9_./-]+\.cpp", files_text))
        option_match = re.search(r'\bCOMPILE_OPTIONS\s+"([^"]*)"', option_text)
        options = tuple(option_match.group(1).split()) if option_match else ()
        blocks.append((files, options, append is not None))
    return blocks


def _cohort_append_match(cmake: str):
    pattern = re.compile(
        r"set_property\s*\(\s*SOURCE\s+(.*?)\bAPPEND\s+PROPERTY\s+"
        r"COMPILE_OPTIONS\s+\"([^\"]*)\"\s*\)",
        re.DOTALL,
    )
    for match in pattern.finditer(cmake):
        files = set(re.findall(r"src/[A-Za-z0-9_./-]+\.cpp", match.group(1)))
        if files & set(PROFILE_OWNERS):
            return match
    return None


@pytest.mark.parametrize("configuration", ["Debug", "Release", "RelWithDebInfo"])
def test_generated_inline_options_respect_debug(configuration, tmp_path):
    """Let CMake evaluate the real source option, without invoking Wine."""
    if not all(shutil.which(tool) for tool in ("cmake", "ninja", "c++")):
        pytest.skip("CMake, Ninja and a host C++ compiler are required")
    cmake = (ROOT / "CMakeLists.txt").read_text()
    profile = _cohort_append_match(cmake)
    assert profile is not None
    toolchain = (ROOT / "cmake/toolchains/vc6.cmake").read_text()
    flag_sets = re.findall(
        r'^set\(CMAKE_CXX_FLAGS_(?:DEBUG|RELEASE|RELWITHDEBINFO) .*$',
        toolchain, re.MULTILINE,
    )
    assert len(flag_sets) == 3
    (tmp_path / "src").mkdir()
    for name in ("pulldown", "dipedit", "ordinary"):
        (tmp_path / "src" / (name + ".cpp")).write_text("int " + name + "() { return 1; }\n")
    (tmp_path / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\n"
        "project(InlineOptions LANGUAGES CXX)\n"
        "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n"
        + "\n".join(flag_sets) + "\n"
        'set_source_files_properties(src/dipedit.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")\n'
        + profile.group(0) + "\n"
        "add_library(sample OBJECT src/pulldown.cpp src/dipedit.cpp src/ordinary.cpp)\n"
    )
    build = tmp_path / "build"
    subprocess.run(
        ["cmake", "-S", str(tmp_path), "-B", str(build), "-G", "Ninja",
         "-DCMAKE_BUILD_TYPE=" + configuration, "-DCMAKE_CXX_COMPILER=" + shutil.which("c++")],
        check=True, capture_output=True, text=True,
    )
    rows = json.loads((build / "compile_commands.json").read_text())
    assert len(rows) == 3
    for row in rows:
        arguments = shlex.split(row["command"])
        inline = [arg for arg in arguments if re.fullmatch(r"/Ob[012]", arg)]
        name = pathlib.Path(row["file"]).stem
        expected = ["/Ob0"] if configuration == "Debug" else (
            ["/Ob2"] if name == "ordinary" else ["/Ob2", "/Ob1"]
        )
        assert inline == expected, (configuration, name, inline)
        assert ("/Oy-" in arguments) == (configuration == "Debug" or name == "dipedit")


def _effective_source_profiles(cmake: str):
    result: dict[str, list[str]] = {}
    for files, options, _append in _source_profiles(cmake):
        for owner in files:
            result.setdefault(owner, []).extend(options)
    return {owner: tuple(options) for owner, options in result.items()}


_GLOBAL_CMAKE_COMMANDS = {
    "add_compile_options",
    "add_definitions",
    "add_link_options",
    "include_directories",
    "link_directories",
    "link_libraries",
    "target_compile_definitions",
    "target_compile_options",
    "target_include_directories",
    "target_link_directories",
    "target_link_libraries",
    "target_link_options",
}


def _global_compiler_link_policy(cmake: str):
    policy = []
    for command, body in _cmake_commands(cmake):
        normalized = " ".join(body.split())
        if command in _GLOBAL_CMAKE_COMMANDS:
            policy.append((command, normalized))
        elif command == "set":
            variable = body.strip().split(None, 1)[0] if body.strip() else ""
            if re.fullmatch(
                r"(?:CMAKE_(?:CXX|EXE|SHARED|MODULE)_.*(?:FLAGS|LINKER|COMPILER).*"
                r"|CMAKE_(?:BUILD_TYPE|CONFIGURATION_TYPES|TOOLCHAIN_FILE)"
                r"|ENV\{(?:CL|_CL_|CXX)\})",
                variable,
                re.IGNORECASE,
            ):
                policy.append((command, normalized))
        elif command in {"set_property", "set_target_properties"} and not re.match(
            r"\s*SOURCE\b", body, re.IGNORECASE
        ) and re.search(
            r"\b(?:COMPILE|LINK|INCLUDE|DEFINITIONS|FLAGS)", body, re.IGNORECASE
        ):
            policy.append((command, normalized))
    return tuple(policy)


def _load_json(text: str):
    def unique_object(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError(f"duplicate JSON key: {key}")
            result[key] = value
        return result

    return json.loads(text, object_pairs_hook=unique_object)


def _measurement_contract(candidate_project: dict, baseline_project: dict) -> bool:
    candidate = candidate_project.get("measurement", {})
    baseline = baseline_project.get("measurement", {})
    candidate_global = {key: value for key, value in candidate.items() if key != "unit_flag_sets"}
    baseline_global = copy.deepcopy({key: value for key, value in baseline.items()
                                     if key != "unit_flag_sets"})
    # Carry the separately verified Release-profile calibration into this
    # historical owner guard. Keep the frozen historical snapshot intact and
    # continue refusing every other global or owner-policy change.
    for flags in baseline_global.get("flag_sets", []):
        if "/Ob2" not in flags:
            flags.insert(flags.index("/O2") + 1, "/Ob2")
    if candidate_global != baseline_global:
        return False

    candidate_units = candidate.get("unit_flag_sets", {})
    baseline_units = baseline.get("unit_flag_sets", {})
    if not isinstance(candidate_units, dict) or not isinstance(baseline_units, dict):
        return False

    # Other owners' recipes belong to their own live guards. This test only
    # pins the two owners whose explicit inline profile remains necessary.
    if candidate_units.get("src/pulldown.cpp") != {"flags": MEASURED_PULLDOWN_PROFILE}:
        return False
    if candidate_units.get("src/dipedit.cpp") != {"flags": MEASURED_DIPEDIT_PROFILE}:
        return False
    return True


def _profile_contract(cmake: str, baseline_cmake: str) -> bool:
    baseline_profiles = _effective_source_profiles(baseline_cmake)
    current_profiles = _effective_source_profiles(cmake)
    for owner, options in OWNER_FLAGS.items():
        if current_profiles.get(owner, ()) != baseline_profiles.get(owner, ()) + options:
            return False

    additions = [
        (files, options)
        for files, options, append in _source_profiles(cmake)
        if append and set(files) & set(PROFILE_OWNERS)
    ]
    return (
        len(additions) == 1
        and set(additions[0][0]) == set(PROFILE_OWNERS)
        and additions[0][1] == (OPTIMIZED_INLINE_OPTION,)
    )


def _policy_contract(cmake: str, baseline_cmake: str) -> bool:
    return (
        _global_compiler_link_policy(cmake) == _global_compiler_link_policy(baseline_cmake)
        and _profile_contract(cmake, baseline_cmake)
    )


def _current_configuration():
    return (
        (ROOT / "CMakeLists.txt").read_text(encoding="utf-8"),
        _load_json((ROOT / "recovery/project.json").read_text(encoding="utf-8")),
        _load_json((ROOT / "recovery/build.json").read_text(encoding="utf-8")),
    )


def _baseline_configuration():
    return (
        _git_text("CMakeLists.txt"),
        _load_json(_git_text("recovery/project.json")),
        _load_json(_git_text("recovery/build.json")),
    )


def _configuration_contract(cmake: str, project: dict, build: dict, baseline):
    baseline_cmake, baseline_project, _baseline_build = baseline
    if not _policy_contract(cmake, baseline_cmake):
        return False
    # Only global measurement policy and these two owners' flag recipes are
    # durable here; newly owned sources and profiles are guarded separately.
    return _measurement_contract(project, baseline_project)


def test_remaining_owner_profiles_keep_global_compiler_and_link_policy_live():
    cmake, project, build = _current_configuration()
    baseline = _baseline_configuration()
    assert _configuration_contract(cmake, project, build, baseline)
    assert all(owner in cmake for owner in PROFILE_OWNERS)


def test_remaining_owner_profile_policy_rejects_bounded_damage():
    cmake, project, build = _current_configuration()
    baseline = _baseline_configuration()
    assert _configuration_contract(cmake, project, build, baseline)

    append_blocks = [
        (files, options)
        for files, options, append in _source_profiles(cmake)
        if append and set(files) & set(PROFILE_OWNERS)
    ]
    assert len(append_blocks) == 1
    assert set(append_blocks[0][0]) == set(PROFILE_OWNERS) and append_blocks[0][1] == (OPTIMIZED_INLINE_OPTION,)
    profile_match = _cohort_append_match(cmake)
    assert profile_match is not None
    profile_text = profile_match.group(0)

    wrong_owner_text = re.sub(r"src/pulldown\.cpp", "src/video.cpp", profile_text, count=1)
    wrong_owner = cmake.replace(profile_text, wrong_owner_text, 1)
    default_recipe = cmake.replace(profile_text, profile_text.replace("/Ob1", "/Ob2"), 1)
    frame_recipe = cmake.replace(
        profile_text,
        profile_text.replace(OPTIMIZED_INLINE_OPTION, OPTIMIZED_INLINE_OPTION + ' /Oy-'),
        1,
    )
    duplicate_profile = cmake + "\n" + profile_text + "\n"
    global_compile = cmake.replace(
        'target_compile_options(OpenSMACX PRIVATE "--repro-root=${CMAKE_CURRENT_SOURCE_DIR}")',
        'target_compile_options(OpenSMACX PRIVATE "--repro-root=${CMAKE_CURRENT_SOURCE_DIR}" "/Ob1")',
        1,
    )
    global_link = cmake + '\ntarget_link_options(OpenSMACX PRIVATE "/OPT:NOICF")\n'
    for damaged in (wrong_owner, default_recipe, frame_recipe, duplicate_profile,
                    global_compile, global_link):
        assert not _policy_contract(damaged, baseline[0])

    wrong_measure_owner = copy.deepcopy(project)
    unit_flags = wrong_measure_owner["measurement"]["unit_flag_sets"]
    del unit_flags["src/pulldown.cpp"]
    unit_flags["src/video.cpp"] = {"flags": MEASURED_PULLDOWN_PROFILE}
    assert not _measurement_contract(wrong_measure_owner, baseline[1])

    wrong_default = copy.deepcopy(project)
    wrong_default["measurement"]["unit_flag_sets"]["src/pulldown.cpp"]["flags"][-1] = "/Ob2"
    assert not _measurement_contract(wrong_default, baseline[1])
    wrong_frame = copy.deepcopy(project)
    wrong_frame["measurement"]["unit_flag_sets"]["src/pulldown.cpp"]["flags"].insert(4, "/Oy-")
    assert not _measurement_contract(wrong_frame, baseline[1])
    global_measurement = copy.deepcopy(project)
    global_measurement["measurement"]["flag_sets"][0].append("/Ob1")
    assert not _measurement_contract(global_measurement, baseline[1])
    with pytest.raises(ValueError, match="duplicate JSON key"):
        _load_json('{"unit_flag_sets":{"src/video.cpp":1,"src/video.cpp":2}}')


def _body(source: str, signature: str) -> str:
    match = re.search(re.escape(signature).replace(r"\ ", r"\s+"), source)
    assert match is not None, f"missing actual source body: {signature}"
    opening = source.index("{", match.end())
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated body: {signature}")


def _definition_prefix(source: str, signature: str) -> str:
    match = re.search(re.escape(signature).replace(r"\ ", r"\s+"), source)
    assert match is not None, f"missing actual source definition: {signature}"
    opening = source.index("{", match.end())
    return source[match.start():opening].rstrip()


QUEUE_SIGNATURES = (
    "VideoFrameQueue::VideoFrameQueue()",
    "void VideoFrameQueue::set_limit(int limit)",
    "int VideoFrameQueue::allocate(",
    "int VideoFrameQueue::append(",
    "unsigned char *VideoFrameQueue::peek(",
    "void VideoFrameQueue::discard_head()",
    "void VideoFrameQueue::clear()",
)


def _queue_harness(queue_source: str, video_header: str, mutation: str | None = None) -> str:
    bodies = {signature: _body(queue_source, signature) for signature in QUEUE_SIGNATURES}
    constructor_prefix = _definition_prefix(queue_source, "VideoFrameQueue::VideoFrameQueue()")
    destructor_body = _body(video_header, "~VideoFrameQueue()")
    if mutation == "equality_gate":
        old = "if (count_ == limit_) return 0x14;"
        assert old in bodies["int VideoFrameQueue::allocate("]
        bodies["int VideoFrameQueue::allocate("] = bodies["int VideoFrameQueue::allocate("].replace(
            old, "if (count_ > limit_) return 0x14;", 1)
    elif mutation == "optional_output":
        old = "if (next_frame_id != 0) *next_frame_id = head_->frame_id;"
        assert old in bodies["unsigned char *VideoFrameQueue::peek("]
        bodies["unsigned char *VideoFrameQueue::peek("] = bodies[
            "unsigned char *VideoFrameQueue::peek("].replace(
                old, "*next_frame_id = head_->frame_id;", 1)
    elif mutation == "limit_without_clear":
        assert "head_" in bodies["void VideoFrameQueue::set_limit(int limit)"]
        assert "limit_ = limit;" in bodies["void VideoFrameQueue::set_limit(int limit)"]
        bodies["void VideoFrameQueue::set_limit(int limit)"] = " limit_ = limit; "
    elif mutation == "zero_byte_initialization":
        body = bodies["int VideoFrameQueue::allocate("]
        needle = "    if (byte_count != 0) {"
        assert needle in body
        bodies["int VideoFrameQueue::allocate("] = body.replace(
            needle,
            "    tail_->bytes = 0;\n    tail_->byte_count = 0;\n" + needle,
            1,
        )
    elif mutation == "append_allocation_count":
        body = bodies["int VideoFrameQueue::append("]
        allocation = "        tail_->next = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));\n"
        assert body.count(allocation) == 1
        bodies["int VideoFrameQueue::append("] = body.replace(allocation, "", 1)
    elif mutation == "constructor_initialization":
        old = "head_(0)"
        assert old in constructor_prefix
        constructor_prefix = constructor_prefix.replace(old, "head_(&node_pool[31])", 1)
    elif mutation == "empty_destructor":
        destructor_body = ""
    elif mutation is not None:
        raise AssertionError(f"unknown queue mutation: {mutation}")

    definitions = []
    for signature in QUEUE_SIGNATURES:
        if signature == "VideoFrameQueue::VideoFrameQueue()":
            declaration = constructor_prefix
        elif signature == "int VideoFrameQueue::allocate(":
            declaration = (
                "int VideoFrameQueue::allocate(unsigned char **payload_out, int byte_count, "
                "int frame_id, unsigned int frame_flags)"
            )
        elif signature == "int VideoFrameQueue::append(":
            declaration = (
                "int VideoFrameQueue::append(const unsigned char *source, int byte_count, "
                "int frame_id, unsigned int frame_flags)"
            )
        elif signature == "unsigned char *VideoFrameQueue::peek(":
            declaration = (
                "unsigned char *VideoFrameQueue::peek(int *next_frame_id, "
                "unsigned int *frame_flags) const"
            )
        else:
            declaration = signature
        definitions.append(f"{declaration} {{{bodies[signature]}}}\n")

    harness = r'''#include <cassert>
#include <cstddef>
#include <cstring>

struct VideoFrameNode {
  int frame_id;
  unsigned int frame_flags;
  unsigned char *bytes;
  unsigned int byte_count;
  VideoFrameNode *next;
};

// Host-only shadow of the header's member order. This exercises current
// method bodies, not target pointer size, ABI, or native compiled selection.
class VideoFrameQueue {
public:
  VideoFrameQueue(); ~VideoFrameQueue() { /*ACTUAL_QUEUE_DESTRUCTOR*/ }
  int count() const { return count_; }
  int allocate(unsigned char **, int, int, unsigned int);
  int append(const unsigned char *, int, int, unsigned int);
  unsigned char *peek(int *, unsigned int *) const;
  void discard_head(); void clear(); void set_limit(int);
  VideoFrameNode *head_;
  VideoFrameNode *tail_;
  int count_;
  int limit_;
};

static unsigned char untouched_payload;
static VideoFrameNode node_pool[32];
static unsigned char payload_pool[32][64];
static int node_used, payload_used, allocation_calls, fail_call, free_calls;

static void reset_allocator(int fail_on_call = 0) {
  node_used = payload_used = allocation_calls = free_calls = 0;
  fail_call = fail_on_call;
}
static void *mem_get(int size) {
  ++allocation_calls;
  if (fail_call != 0 && allocation_calls == fail_call) return 0;
  if (size == static_cast<int>(sizeof(VideoFrameNode))) {
    if (node_used == 32) return 0;
    VideoFrameNode *node = &node_pool[node_used++];
    node->frame_id = -1;
    node->frame_flags = 0xA5A5A5A5U;
    node->bytes = &untouched_payload;
    node->byte_count = 0xA5A5A5A5U;
    node->next = 0;
    return node;
  }
  if (size < 0 || size > 64 || payload_used == 32) return 0;
  unsigned char *payload = payload_pool[payload_used++];
  memset(payload, 0xCC, sizeof payload_pool[0]);
  return payload;
}
static void queue_free(void *pointer) { if (pointer != 0) ++free_calls; }
#define free queue_free
/*ACTUAL_QUEUE_METHODS*/
#undef free

int main() {
  reset_allocator();
  {
    VideoFrameQueue queue;
    int id = 0x12345678;
    unsigned int flags = 0x87654321U;
    assert(queue.peek(&id, &flags) == 0);
    assert(id == 0x12345678 && flags == 0x87654321U);
    assert(queue.peek(0, &flags) == 0);
    assert(queue.peek(&id, 0) == 0);

    const unsigned char first[] = {1, 2, 3};
    const unsigned char second[] = {7, 8};
    assert(queue.append(first, 3, 41, 0x11U) == 0);
    assert(queue.append(second, 2, 42, 0x22U) == 0);
    // Two node and two payload allocations for the two nonempty appends.
    assert(allocation_calls == 4);
    assert(queue.count() == 2);
    unsigned char *front = queue.peek(&id, &flags);
    assert(front == payload_pool[0] && id == 41 && flags == 0x11U);
    assert(front[0] == 1 && front[1] == 2 && front[2] == 3);
    assert(queue.peek(0, &flags) == front && flags == 0x11U);
    assert(queue.peek(&id, 0) == front && id == 41);

    queue.discard_head();
    assert(queue.count() == 1);
    unsigned char *next = queue.peek(&id, &flags);
    assert(next == payload_pool[1] && id == 42 && flags == 0x22U);
    assert(next[0] == 7 && next[1] == 8);
    queue.set_limit(0);
    assert(queue.count() == 0 && queue.head_ == 0 && queue.tail_ == 0);
    const int calls_before_limit = allocation_calls;
    unsigned char *payload = &untouched_payload;
    assert(queue.append(second, 2, 43, 0) == 0x14);
    assert(queue.allocate(&payload, 2, 43, 0) == 0x14);
    assert(payload == 0 && allocation_calls == calls_before_limit);
    queue.discard_head();
    assert(queue.count() == 0);
  }

  reset_allocator();
  {
    VideoFrameQueue destructor_queue;
    const unsigned char bytes[] = {5, 6};
    assert(destructor_queue.append(bytes, 2, 50, 0x44U) == 0);
    assert(destructor_queue.count() == 1);
  }
  assert(free_calls == 2);

  reset_allocator(2);
  {
    VideoFrameQueue failed_payload;
    unsigned char *payload = &untouched_payload;
    assert(failed_payload.allocate(&payload, 4, 9, 0) == 4);
    assert(payload == 0);
    // The original failure path discards before incrementing count; preserve
    // the resulting underflow and the still-stored tail pointer.
    assert(failed_payload.count() == -1);
    assert(failed_payload.head_ == 0 && failed_payload.tail_ != 0);
  }

  reset_allocator();
  {
    VideoFrameQueue zero_byte;
    unsigned char *payload = 0;
    assert(zero_byte.allocate(&payload, 0, 17, 0x33U) == 0);
    assert(zero_byte.count() == 1);
    assert(payload == &untouched_payload);
    assert(zero_byte.head_->bytes == &untouched_payload);
    assert(zero_byte.head_->byte_count == 0xA5A5A5A5U);
    zero_byte.clear();
    assert(zero_byte.count() == 0 && zero_byte.head_ == 0 && zero_byte.tail_ == 0);
  }
  return 0;
}
'''
    return (harness.replace("/*ACTUAL_QUEUE_DESTRUCTOR*/", destructor_body)
            .replace("/*ACTUAL_QUEUE_METHODS*/", ''.join(definitions)))


def _compile_and_run(tmp_path: pathlib.Path, compiler: str, source: str, name: str):
    source_path = tmp_path / f"{name}.cpp"
    binary_path = tmp_path / name
    source_path.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O0", str(source_path), "-o", str(binary_path)],
        capture_output=True,
        text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary_path)], capture_output=True, text=True)


def test_actual_video_queue_bodies_keep_fifo_and_legacy_error_behavior(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/video_queue.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/video.h").read_text(encoding="utf-8")
    assert "VideoFrameNode *head_;\n  VideoFrameNode *tail_;\n  int count_;\n  int limit_;" in header
    completed = _compile_and_run(tmp_path, compiler, _queue_harness(source, header), "video_queue")
    assert completed.returncode == 0, completed.stderr


@pytest.mark.parametrize("mutation", (
    "equality_gate",
    "optional_output",
    "limit_without_clear",
    "zero_byte_initialization",
    "append_allocation_count",
    "constructor_initialization",
    "empty_destructor",
))
def test_actual_video_queue_harness_rejects_behavior_damage(tmp_path, mutation):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/video_queue.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/video.h").read_text(encoding="utf-8")
    damaged = _queue_harness(source, header, mutation)
    completed = _compile_and_run(tmp_path, compiler, damaged, f"video_queue_{mutation}")
    assert completed.returncode != 0, mutation

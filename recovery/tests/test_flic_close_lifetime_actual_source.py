"""Host controls for Flic close ordering and natural subobject teardown."""

from pathlib import Path
import shutil
import subprocess
import textwrap

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/flic.cpp").is_file())
FLIC_CPP = ROOT / "src/flic.cpp"
FLIC_H = ROOT / "src/flic.h"


def _definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition {signature}")


def _class_definition(source, class_name, signature):
    class_start = source.index(f"class {class_name}")
    class_open = source.index("{", class_start)
    depth = 0
    class_end = None
    for index in range(class_open, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                class_end = index
                break
    assert class_end is not None, f"unterminated class {class_name}"
    member_start = source.index(signature, class_open, class_end)
    opening = source.index("{", member_start, class_end)
    depth = 0
    for index in range(opening, class_end + 1):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[member_start:index + 1]
    raise AssertionError(f"unterminated in-class definition {signature}")


def test_in_class_destructor_keeps_close_opaque_and_original_mapping_in_tu():
    source = FLIC_CPP.read_text()
    header = FLIC_H.read_text()
    destructor = _class_definition(header, "Flic", "~Flic() {")
    marker = source.index("// ORIGINAL: 0x00629DF0 ?close@Flic@@QAEXXZ")
    close = source.index("void Flic::close() {")
    assert marker < close
    assert destructor == "~Flic() {\n    close();\n  }"
    assert "~Flic() {" not in source
    assert "// ORIGINAL: 0x004327A0 ??1Flic@@QAE@XZ 0x004327A0-0x004327F8;0x00653420-0x00653443" in source
    assert "historical" in source[source.index("Historical visibility hypothesis"):source.index("Return Value: n/a", source.index("Historical visibility hypothesis"))].lower()
    assert source.count("// ORIGINAL: 0x00629DF0 ?close@Flic@@QAEXXZ") == 1
    assert "void (Flic::*close_method)()" not in source


def _host_source(close_definition, destructor_definition):
    host_class = textwrap.dedent(r"""
        #include <cstdio>
        #include <cstddef>
        #include <string>
        #include <vector>

        static std::vector<std::string> events;
        static void record(const char *name) { events.push_back(name); }
        static int tracked_fclose_count;
        static int tracked_fclose(FILE *file) {
            if (file != reinterpret_cast<FILE *>(1)) return -1;
            ++tracked_fclose_count;
            record("fclose");
            return 0;
        }
        #define fclose tracked_fclose

        struct Buffer {
            ~Buffer() { record("~Buffer"); }
        };
        struct Palette {
            ~Palette() { record("~Palette"); }
        };
        struct TrackedInt {
            const char *name;
            int value;
            explicit TrackedInt(const char *field) : name(field), value(0) {}
            TrackedInt &operator=(int next) {
                record(name);
                value = next;
                return *this;
            }
        };
        struct TrackedPointer {
            const char *name;
            void *value;
            explicit TrackedPointer(const char *field) : name(field), value(0) {}
            TrackedPointer &operator=(std::nullptr_t) {
                record(name);
                value = 0;
                return *this;
            }
        };
        struct TrackedFile {
            FILE *value;
            TrackedFile() : value(0) {}
            operator FILE *() const { return value; }
            TrackedFile &operator=(std::nullptr_t) {
                record("file_=null");
                value = 0;
                return *this;
            }
        };

        struct Flic {
            Buffer background_buffer_;
            TrackedInt origin_x_, origin_y_, decoder_flags_;
            TrackedInt frame_;
            TrackedPointer destination_buffer_;
            TrackedInt transparent_index_, palette_offset_, decoded_width_, decoded_height_;
            TrackedFile file_;
            Palette palette_;
            Flic() : origin_x_("origin_x_"), origin_y_("origin_y_"),
                     decoder_flags_("decoder_flags_"), frame_("frame_"),
                     destination_buffer_("destination_buffer_"),
                     transparent_index_("transparent_index_"),
                     palette_offset_("palette_offset_"), decoded_width_("decoded_width_"),
                     decoded_height_("decoded_height_") {}
            void close();
            @@FLIC_DTOR@@
        };
    """)
    embedded_destructor = destructor_definition.replace("\n", "\n    ")
    host_class = host_class.replace("@@FLIC_DTOR@@", embedded_destructor)
    return host_class + "\n" + close_definition + textwrap.dedent(r"""

        static bool equals(const std::vector<std::string> &actual,
                           const std::vector<std::string> &expected) {
            return actual == expected;
        }
        int main() {
            const std::vector<std::string> stores = {
                "origin_x_", "origin_y_", "decoder_flags_", "frame_",
                "destination_buffer_", "transparent_index_", "palette_offset_", "decoded_width_",
                "decoded_height_"};
            {
                Flic f;
                f.file_.value = reinterpret_cast<FILE *>(1);
                events.clear();
                f.close();
                std::vector<std::string> expected = stores;
                expected.push_back("fclose");
                expected.push_back("file_=null");
                if (!equals(events, expected) || tracked_fclose_count != 1 || f.file_.value
                    || f.origin_x_.value || f.origin_y_.value || f.decoder_flags_.value
                    || f.frame_.value || f.destination_buffer_.value || f.transparent_index_.value
                    || f.palette_offset_.value || f.decoded_width_.value
                    || f.decoded_height_.value)
                    return 1;
            }
            events.clear();
            {
                Flic f;
                f.file_.value = 0;
                events.clear();
                f.close();
                if (!equals(events, stores) || tracked_fclose_count != 1 || f.file_.value
                    || f.origin_x_.value || f.origin_y_.value || f.decoder_flags_.value
                    || f.frame_.value || f.destination_buffer_.value || f.transparent_index_.value
                    || f.palette_offset_.value || f.decoded_width_.value
                    || f.decoded_height_.value)
                    return 2;
            }
            events.clear();
            tracked_fclose_count = 0;
            {
                Flic f;
                f.file_.value = reinterpret_cast<FILE *>(1);
                events.clear();
            }
            std::vector<std::string> expected = stores;
            expected.push_back("fclose");
            expected.push_back("file_=null");
            expected.push_back("~Palette");
            expected.push_back("~Buffer");
            return equals(events, expected) && tracked_fclose_count == 1 ? 0 : 3;
        }
    """)


def test_actual_close_and_destructor_preserve_close_then_reverse_member_order(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    source = FLIC_CPP.read_text()
    close_definition = _definition(source, "void Flic::close() {")
    destructor_definition = _class_definition(
        FLIC_H.read_text(), "Flic", "~Flic() {")
    host_source = _host_source(close_definition, destructor_definition)
    cpp = tmp_path / "flic_lifetime.cpp"
    binary = tmp_path / "flic_lifetime"
    cpp.write_text(host_source)
    result = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                             str(cpp), "-o", str(binary)], capture_output=True, timeout=30)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    run = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert run.returncode == 0, run.stderr.decode(errors="replace")

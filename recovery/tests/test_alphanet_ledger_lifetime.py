"""Host lifetime controls over the actual AlphaNet ledger source bodies.

These fixtures check source-level virtual dispatch, cleanup and C++ unwinding.
They do not claim VC6 EH, target ABI, native compiler selection, or game-runtime
equivalence.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_game_logic_seed_batch_behavior import _masked_cpp


ROOT = Path(__file__).resolve().parents[2]


def _class_declaration(source: str, class_name: str) -> str:
    masked = _masked_cpp(source)
    match = re.search(r"\b(?:class|struct)\s+" + re.escape(class_name)
                      + r"\b[^;{]*\{", masked)
    assert match is not None, class_name
    opening = masked.index("{", match.start())
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                end = index + 1
                while end < len(masked) and masked[end].isspace():
                    end += 1
                assert end < len(masked) and masked[end] == ";"
                return source[match.start():end + 1]
    raise AssertionError("unterminated class " + class_name)


def _braced(source: str, masked: str, opening: int) -> str:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening:index + 1]
    raise AssertionError("unterminated C++ body")


def _inline_method(class_text: str, pattern: str):
    masked = _masked_cpp(class_text)
    matches = list(re.finditer(pattern, masked))
    bodies = []
    for match in matches:
        paren = masked.find(")", match.end())
        assert paren >= 0
        brace = masked.find("{", paren + 1)
        semi = masked.find(";", paren + 1)
        if brace >= 0 and (semi < 0 or brace < semi):
            bodies.append(_braced(class_text, masked, brace))
    return bodies


def _qualified_definitions(sources, pattern: str):
    found = []
    for source in sources:
        masked = _masked_cpp(source)
        for match in re.finditer(pattern, masked):
            paren = masked.find(")", match.end())
            if paren < 0:
                continue
            brace = masked.find("{", paren + 1)
            semi = masked.find(";", paren + 1)
            if brace >= 0 and (semi < 0 or brace < semi):
                found.append(source[match.start():brace].rstrip() + " "
                             + _braced(source, masked, brace))
    return found


def _one_definition(class_text: str, inline_pattern: str,
                    source_files, qualified_pattern: str):
    inline = _inline_method(class_text, inline_pattern)
    out_of_line = _qualified_definitions(
        [path.read_text(encoding="utf-8") for path in source_files],
        qualified_pattern,
    )
    assert len(inline) + len(out_of_line) == 1, (
        inline_pattern, len(inline), len(out_of_line))
    return (bool(inline), inline[0] if inline else out_of_line[0])


def actual_fragments():
    header_path = ROOT / "src/alphanet.h"
    records_path = ROOT / "src/alphanet_records.cpp"
    string_header_path = ROOT / "src/stringstruct.h"
    string_source_path = ROOT / "src/stringstruct.cpp"
    net_source_path = ROOT / "src/net_class.cpp"
    header = header_path.read_text(encoding="utf-8")
    records = records_path.read_text(encoding="utf-8")
    string_header = string_header_path.read_text(encoding="utf-8")
    string_source = string_source_path.read_text(encoding="utf-8")
    net_source = net_source_path.read_text(encoding="utf-8")

    base = _class_declaration(header, "AlphaNetStringStruct")
    derived = _class_declaration(header, "AlphaNetLedger")
    allocation_base = _class_declaration(string_header, "StringAllocationBase")
    entry = _class_declaration(header, "AlphaNetLedgerEntry")

    remove_all_inline, remove_all = _one_definition(
        base, r"\bvoid\s+remove_all\s*\(", [records_path],
        r"(?:void\s+)?AlphaNetStringStruct::remove_all\s*\(")
    base_dtor_inline, base_dtor = _one_definition(
        base, r"~AlphaNetStringStruct\s*\(", [records_path],
        r"AlphaNetStringStruct::~AlphaNetStringStruct\s*\(")
    derived_dtor_inline, derived_dtor = _one_definition(
        derived, r"~AlphaNetLedger\s*\(", [records_path],
        r"AlphaNetLedger::~AlphaNetLedger\s*\(")
    base_insert = _qualified_definitions(
        [records], r"int\s+AlphaNetStringStruct::on_insert\s*\(")
    derived_insert = _qualified_definitions(
        [records], r"int\s+AlphaNetLedger::on_insert\s*\(")
    base_hook = _qualified_definitions(
        [records], r"void\s+AlphaNetStringStruct::on_remove\s*\(")
    derived_hook = _qualified_definitions(
        [records], r"void\s+AlphaNetLedger::on_remove\s*\(")
    base_ctor = _qualified_definitions(
        [records], r"AlphaNetStringStruct::AlphaNetStringStruct\s*\(")
    assert (len(base_insert) == len(derived_insert) == len(base_hook)
            == len(derived_hook) == len(base_ctor) == 1)

    alloc_ctor = _inline_method(
        allocation_base, r"\bStringAllocationBase\s*\(")
    alloc_dtor = _qualified_definitions(
        [string_source], r"StringAllocationBase::~StringAllocationBase\s*\(")
    entry_delete = _qualified_definitions(
        [records], r"void\s+AlphaNetLedgerEntry::operator\s+delete\s*\(\s*void\s*\*")
    entry_delete = [definition for definition in entry_delete
                    if re.search(r"operator\s+delete\s*\(\s*void\s*\*\s*memory\s*\)",
                                 definition)]
    cleanup = _qualified_definitions(
        [net_source], r"void\s+cleanup_failed_net_construction\s*\(\s*void\s*\*")
    assert len(alloc_ctor) == len(alloc_dtor) == len(entry_delete) == len(cleanup) == 1

    return {
        "base": base,
        "derived": derived,
        "entry": entry,
        "allocation_base": allocation_base,
        "remove_all": remove_all,
        "remove_all_inline": remove_all_inline,
        "base_dtor": base_dtor,
        "base_dtor_inline": base_dtor_inline,
        "derived_dtor": derived_dtor,
        "derived_dtor_inline": derived_dtor_inline,
        "base_hook": base_hook[0],
        "derived_hook": derived_hook[0],
        "base_insert": base_insert[0],
        "derived_insert": derived_insert[0],
        "base_ctor": base_ctor[0],
        "alloc_ctor": alloc_ctor[0],
        "alloc_dtor": alloc_dtor[0],
        "entry_delete": entry_delete[0],
        "cleanup": cleanup[0],
    }


STUBS = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <vector>

struct Heap {
    void *get(size_t size) { return std::malloc(size); }
};
struct GUID { unsigned char bytes[16]; };
class NetAllocationNode;
class AlphaNetStringStruct;
static Heap *StringAllocationHeap = 0;
static std::vector<std::string> events;
static bool expect_freed_strings = true;
static int payload_dtor_count = 0;
static int observed_string_free_count = 0;
static void *mem_get(size_t size) { return std::malloc(size); }
void observed_free(void *memory) {
    assert(memory != 0);
    ++observed_string_free_count;
    std::free(memory);
}

ALLOCATION_BASE_DECL

class NetAllocationNode : public virtual StringAllocationBase {
public:
    char *session_name_;
    char *password_;
    GUID instance_guid_;
    uint32_t session_flags_;
    uint32_t max_players_;
    uint32_t current_players_;
    uint32_t field_28_;
    int id_;
    NetAllocationNode(int id, char *name, char *password)
        : session_name_(name), password_(password), session_flags_(0),
          max_players_(0), current_players_(0), field_28_(0), id_(id) {}
    static void *operator new(size_t size) { return std::malloc(size); }
    static void operator delete(void *memory) { std::free(memory); }
    virtual ~NetAllocationNode() {
        if (expect_freed_strings) {
            assert(session_name_ == 0 && password_ == 0);
        } else {
            assert(session_name_ != 0 && password_ != 0);
        }
        ++payload_dtor_count;
        events.push_back("payload " + std::to_string(id_));
    }
};

ENTRY_DECL
BASE_DECL
DERIVED_DECL

// The real entry has an implicit virtual destructor. This derived fixture adds
// observation only, checking the actual remove_all null-store before deletion.
struct ObservedEntry : AlphaNetLedgerEntry {
    ~ObservedEntry() {
        assert(payload_ == 0);
        events.push_back("entry " + std::to_string(id_));
    }
};

CLEANUP
ALLOCATION_BASE_DTOR
BASE_CTOR
BASE_INSERT
DERIVED_INSERT
BASE_HOOK
DERIVED_HOOK
REMOVE_ALL
BASE_DTOR
DERIVED_DTOR
ENTRY_DELETE
static AlphaNetLedgerEntry *make_entry(int id, AlphaNetLedgerEntry *next,
                                       int string_owner_mode) {
    char *name = 0;
    char *password = 0;
    if (string_owner_mode == 0) {
        const char n[] = "session";
        const char p[] = "password";
        name = static_cast<char *>(std::malloc(sizeof(n)));
        password = static_cast<char *>(std::malloc(sizeof(p)));
        assert(name != 0 && password != 0);
        std::memcpy(name, n, sizeof(n));
        std::memcpy(password, p, sizeof(p));
    } else {
        static char borrowed_name[] = "borrowed-session";
        static char borrowed_password[] = "borrowed-password";
        name = borrowed_name;
        password = borrowed_password;
    }
    NetAllocationNode *payload = new NetAllocationNode(id, name, password);
    void *storage = std::malloc(sizeof(ObservedEntry));
    assert(storage != 0);
    ObservedEntry *observed = ::new (storage) ObservedEntry;
    AlphaNetLedgerEntry *entry = observed;
    entry->id_ = id;
    entry->payload_ = payload;
    entry->next_ = next;
    entry->previous_ = 0;
    return entry;
}

static void append_two(AlphaNetStringStruct &list, int string_owner_mode) {
    AlphaNetLedgerEntry *second = make_entry(2, 0, string_owner_mode);
    AlphaNetLedgerEntry *first = make_entry(1, second, string_owner_mode);
    second->next_ = first;
    first->previous_ = second;
    second->previous_ = first;
    list.head_ = first;
    list.current_ = first;
    list.entry_count_ = 2;
    list.current_position_ = 1;
}

static void reset_observers() {
    events.clear();
    expect_freed_strings = true;
    payload_dtor_count = 0;
    observed_string_free_count = 0;
    StringAllocationHeap = 0;
}

static void test_derived_destruction() {
    reset_observers();
    Heap ledger_owner;
    Heap ambient_owner;
    StringAllocationHeap = &ledger_owner;
    {
        AlphaNetLedger list;
        assert(list.allocation_owner_ == &ledger_owner);
        assert(StringAllocationHeap == 0);
        append_two(list, 0);
        StringAllocationHeap = &ambient_owner;
    }
    assert(payload_dtor_count == 2);
    assert(observed_string_free_count == 4);
    assert(events.size() == 4);
    assert(events[0] == "payload 1" && events[1] == "entry 1");
    assert(events[2] == "payload 2" && events[3] == "entry 2");
    assert(StringAllocationHeap == &ledger_owner);
}

static void test_base_only_destruction() {
    reset_observers();
    expect_freed_strings = false;
    Heap base_owner;
    StringAllocationHeap = &base_owner;
    {
        AlphaNetStringStruct list;
        assert(list.allocation_owner_ == &base_owner);
        append_two(list, 1);
    }
    assert(payload_dtor_count == 2);
    assert(observed_string_free_count == 0);
    assert(events.size() == 4);
    assert(events[0] == "payload 1" && events[1] == "entry 1");
    assert(events[2] == "payload 2" && events[3] == "entry 2");
    assert(StringAllocationHeap == &base_owner);
}

struct Seeder {
    explicit Seeder(AlphaNetLedger &list) {
        append_two(list, 0);
        events.push_back("seeded");
    }
    ~Seeder() { events.push_back("seeder dtor"); }
};
struct ThrowAfterSeed {
    ThrowAfterSeed() {
        events.push_back("throwing member ctor");
        StringAllocationHeap = 0;
        throw 23;
    }
};
struct PrefixOwner {
    AlphaNetLedger ledger;
    Seeder seeder;
    ThrowAfterSeed failure;
    PrefixOwner() : ledger(), seeder(ledger), failure() {}
};

static void test_prefix_unwind() {
    reset_observers();
    Heap ledger_owner;
    StringAllocationHeap = &ledger_owner;
    try {
        PrefixOwner owner;
        assert(!"constructor should throw after seeding ledger");
    } catch (int value) {
        assert(value == 23);
    }
    assert(payload_dtor_count == 2);
    assert(observed_string_free_count == 4);
    assert(events.size() == 7);
    assert(events[0] == "seeded");
    assert(events[1] == "throwing member ctor");
    assert(events[2] == "seeder dtor");
    assert(events[3] == "payload 1");
    assert(events[4] == "entry 1");
    assert(events[5] == "payload 2");
    assert(events[6] == "entry 2");
    assert(StringAllocationHeap == &ledger_owner);
}

int main() {
    test_derived_destruction();
    test_base_only_destruction();
    test_prefix_unwind();
}
'''


def _harness(fragments):
    # The data-bearing declarations and tested algorithms come from src. The
    # fixture supplies only narrow external types and payload observers.
    derived_hook = re.sub(r"\bfree\s*\(", "observed_free(",
                          fragments["derived_hook"])
    return (STUBS.replace("ALLOCATION_BASE_DECL", fragments["allocation_base"])
            .replace("ENTRY_DECL", fragments["entry"])
            .replace("BASE_DECL", fragments["base"])
            .replace("DERIVED_DECL", fragments["derived"])
            .replace("CLEANUP", fragments["cleanup"])
            .replace("ALLOCATION_BASE_DTOR", fragments["alloc_dtor"])
            .replace("BASE_CTOR", fragments["base_ctor"])
            .replace("BASE_INSERT", fragments["base_insert"])
            .replace("DERIVED_INSERT", fragments["derived_insert"])
            .replace("BASE_HOOK", fragments["base_hook"])
            .replace("DERIVED_HOOK", derived_hook)
            .replace("REMOVE_ALL", "" if fragments["remove_all_inline"]
                    else fragments["remove_all"])
            .replace("BASE_DTOR", "" if fragments["base_dtor_inline"]
                    else fragments["base_dtor"])
            .replace("DERIVED_DTOR", "" if fragments["derived_dtor_inline"]
                    else fragments["derived_dtor"])
            .replace("ENTRY_DELETE", fragments["entry_delete"]))


def _compile_and_run(tmp_path: Path, compiler: str, source: str, name: str):
    source_path = tmp_path / (name + ".cpp")
    binary_path = tmp_path / name
    source_path.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary_path)], capture_output=True, text=True,
                          timeout=10, check=False)


def _omit_dtor_cleanup_once(destructor: str) -> str:
    mutated, count = re.subn(
        r"\bremove_all\s*\(\s*\)\s*;\s*current_position_\s*=\s*0\s*;",
        "current_position_ = 0;",
        destructor,
        count=1,
    )
    assert count == 1, ("expected one destructor cleanup/reset sequence", count)
    return mutated


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_ledger_lifetimes_and_prefix_unwind(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = _compile_and_run(tmp_path, compiler, _harness(actual_fragments()), "ledger")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (
    "derived_cleanup", "base_cleanup", "derived_hook", "owner_publication",
    "callback", "delete", "null_order",
))
def test_lifetime_controls_reject_compilable_omission_and_owner_mutants(
        compiler_name, tmp_path, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    fragments = actual_fragments()
    if damage == "derived_cleanup":
        target = "derived" if fragments["derived_dtor_inline"] else "derived_dtor"
        fragments[target] = _omit_dtor_cleanup_once(fragments[target])
    elif damage == "base_cleanup":
        target = "base" if fragments["base_dtor_inline"] else "base_dtor"
        fragments[target] = _omit_dtor_cleanup_once(fragments[target])
    elif damage == "derived_hook":
        fragments["derived_hook"] = fragments["base_hook"].replace(
            "AlphaNetStringStruct::on_remove", "AlphaNetLedger::on_remove", 1)
    elif damage in ("callback", "delete", "null_order"):
        target = "base" if fragments["remove_all_inline"] else "remove_all"
        method = fragments[target]
        old, new = {
            "callback": ("on_remove(payload);", "/* omitted callback */"),
            "delete": ("delete payload;", "/* omitted payload deletion */"),
            "null_order": ("head_->payload_ = 0;", "/* omitted payload null */"),
        }[damage]
        assert old in method
        fragments[target] = method.replace(old, new, 1)
    else:
        old = "StringAllocationHeap = allocation_owner_;"
        assert old in fragments["alloc_dtor"]
        fragments["alloc_dtor"] = fragments["alloc_dtor"].replace(
            old, "StringAllocationHeap = 0;", 1)
    result = _compile_and_run(tmp_path, compiler, _harness(fragments),
                              "ledger_mutant_" + damage)
    assert result.returncode != 0, damage + " mutant escaped lifetime controls"

"""Source-linked behavioral controls for the two typed network-list walks."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    for position in range(opening, len(text)):
        depth += (text[position] == "{") - (text[position] == "}")
        if depth == 0:
            return text[opening + 1:position]
    raise AssertionError("unterminated C++ body")


def class_method(source: str, class_name: str, method: str) -> str:
    class_start = source.index("class " + class_name + " :")
    class_body = braced(source, class_start)
    method_start = class_body.index(method)
    return method + " {" + braced(class_body, method_start) + "}"


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    return signature + " {" + braced(source, start) + "}"


HARNESS = r'''#include <cassert>
#include <vector>

static std::vector<int> events;
static void expect_events(const std::vector<int> &expected) { assert(events == expected); }

struct NetTextPayload {
    int id;
    explicit NetTextPayload(int value) : id(value) {}
    ~NetTextPayload() { events.push_back(100 + id); }
};
struct NetTextListEntry {
    NetTextPayload *payload;
    NetTextListEntry *next;
    NetTextListEntry(NetTextPayload *p, NetTextListEntry *n) : payload(p), next(n) {}
    ~NetTextListEntry() { assert(payload == 0); events.push_back(200 + id); }
    int id;
};
class NetTextListBase {
public:
    typedef NetTextListEntry Entry;
    typedef NetTextPayload Payload;
    Entry *head_ = 0;
    Entry *current_ = 0;
    int entry_count_ = 0;
    int current_position_ = 0;
    int callbacks = 0;
    int mutation = 0;
    Entry *replacement = 0;
    void on_remove(Payload *payload) {
        events.push_back(payload ? 10 + payload->id : 10);
        if (++callbacks != 1) return;
        if (mutation == 1) entry_count_ = 1;
        if (mutation == 2) head_ = replacement;
        if (mutation == 3) current_ = replacement;
    }
    NET_METHOD
};

struct NetAllocationNode {
    int id;
    explicit NetAllocationNode(int value) : id(value) {}
    ~NetAllocationNode() { events.push_back(300 + id); }
};
struct AlphaNetLedgerEntry {
    NetAllocationNode *payload_;
    AlphaNetLedgerEntry *next_;
    int id_;
    AlphaNetLedgerEntry(NetAllocationNode *p, AlphaNetLedgerEntry *n, int id)
        : payload_(p), next_(n), id_(id) {}
    ~AlphaNetLedgerEntry() { assert(payload_ == 0); events.push_back(400 + id_); }
};
class AlphaNetStringStruct {
public:
    typedef AlphaNetLedgerEntry Entry;
    typedef NetAllocationNode Payload;
    Entry *head_ = 0;
    Entry *current_ = 0;
    int entry_count_ = 0;
    int current_position_ = 0;
    int callbacks = 0;
    int mutation = 0;
    Entry *replacement = 0;
    void on_remove(Payload *payload) {
        events.push_back(payload ? 30 + payload->id : 30);
        if (++callbacks != 1) return;
        if (mutation == 1) entry_count_ = 1;
        if (mutation == 2) head_ = replacement;
        if (mutation == 3) current_ = replacement;
    }
    void remove_all();
};

ALPHA_REMOVE_ALL_DEFINITION

static void discard(NetTextListEntry *entry) {
    if (!entry) return;
    delete entry->payload;
    entry->payload = 0;
    delete entry;
}
static void discard(AlphaNetLedgerEntry *entry) {
    if (!entry) return;
    delete entry->payload_;
    entry->payload_ = 0;
    delete entry;
}

static void test_net_count_reload() {
    NetTextListBase list;
    NetTextListEntry *second = new NetTextListEntry(new NetTextPayload(2), 0);
    second->id = 2;
    NetTextListEntry *first = new NetTextListEntry(new NetTextPayload(1), second);
    first->id = 1;
    list.head_ = first; list.entry_count_ = 2; list.current_position_ = 9;
    list.mutation = 1;
    list.remove_all();
    expect_events(std::vector<int>{11, 101, 201});
    assert(list.callbacks == 1 && list.head_ == 0 && list.current_ == second);
    assert(list.entry_count_ == 0 && list.current_position_ == 0);
    discard(second);
}
static void test_net_head_reload() {
    NetTextListBase list;
    NetTextListEntry *second = new NetTextListEntry(new NetTextPayload(2), 0);
    second->id = 2;
    NetTextListEntry *first = new NetTextListEntry(new NetTextPayload(1), second);
    first->id = 1;
    NetTextListEntry *replacement = new NetTextListEntry(new NetTextPayload(3), 0);
    replacement->id = 3;
    NetTextPayload *replacement_payload = replacement->payload;
    list.head_ = first; list.entry_count_ = 2; list.mutation = 2;
    list.replacement = replacement;
    list.remove_all();
    expect_events(std::vector<int>{11, 101, 203, 12, 102, 202});
    assert(list.head_ == 0 && list.current_ == 0 && list.callbacks == 2);
    first->payload = 0; delete first;
    delete replacement_payload;
}
static void test_net_current_reload() {
    NetTextListBase list;
    NetTextListEntry *second = new NetTextListEntry(new NetTextPayload(2), 0);
    second->id = 2;
    NetTextListEntry *first = new NetTextListEntry(new NetTextPayload(1), second);
    first->id = 1;
    NetTextListEntry *replacement = new NetTextListEntry(new NetTextPayload(3), 0);
    replacement->id = 3;
    list.head_ = first; list.entry_count_ = 2; list.mutation = 3;
    list.replacement = replacement;
    list.remove_all();
    expect_events(std::vector<int>{11, 101, 201, 13, 103, 203});
    assert(list.head_ == 0 && list.current_ == 0 && list.callbacks == 2);
    discard(second);
}
static void test_net_guards_and_null_payload() {
    NetTextListBase empty;
    empty.entry_count_ = 3; empty.current_position_ = 7;
    empty.remove_all();
    assert(empty.entry_count_ == 3 && empty.current_position_ == 7);
    NetTextListEntry *zero = new NetTextListEntry(0, 0); zero->id = 4;
    empty.head_ = zero; empty.entry_count_ = 0; empty.current_position_ = 8;
    empty.remove_all();
    expect_events(std::vector<int>{});
    assert(empty.head_ == 0 && empty.entry_count_ == 0 && empty.current_position_ == 0);
    zero->payload = 0; delete zero;
    events.clear();
    NetTextListEntry *negative = new NetTextListEntry(0, 0); negative->id = 5;
    empty.head_ = negative; empty.entry_count_ = -1; empty.current_position_ = 6;
    empty.remove_all();
    assert(empty.head_ == 0 && empty.entry_count_ == 0 && empty.current_position_ == 0);
    negative->payload = 0; delete negative;
    events.clear();
    NetTextListEntry *null_payload = new NetTextListEntry(0, 0); null_payload->id = 6;
    empty.head_ = null_payload; empty.entry_count_ = 1;
    empty.remove_all();
    expect_events(std::vector<int>{10, 206});
    assert(empty.callbacks == 1 && empty.head_ == 0);
}

static void test_alpha_count_reload() {
    AlphaNetStringStruct list;
    AlphaNetLedgerEntry *second = new AlphaNetLedgerEntry(new NetAllocationNode(2), 0, 2);
    AlphaNetLedgerEntry *first = new AlphaNetLedgerEntry(new NetAllocationNode(1), second, 1);
    list.head_ = first; list.entry_count_ = 2; list.current_position_ = 9;
    list.mutation = 1;
    list.remove_all();
    expect_events(std::vector<int>{31, 301, 401});
    assert(list.callbacks == 1 && list.head_ == 0 && list.current_ == second);
    assert(list.entry_count_ == 0 && list.current_position_ == 0);
    discard(second);
}
static void test_alpha_head_reload() {
    AlphaNetStringStruct list;
    AlphaNetLedgerEntry *second = new AlphaNetLedgerEntry(new NetAllocationNode(2), 0, 2);
    AlphaNetLedgerEntry *first = new AlphaNetLedgerEntry(new NetAllocationNode(1), second, 1);
    AlphaNetLedgerEntry *replacement = new AlphaNetLedgerEntry(new NetAllocationNode(3), 0, 3);
    NetAllocationNode *replacement_payload = replacement->payload_;
    list.head_ = first; list.entry_count_ = 2; list.mutation = 2;
    list.replacement = replacement;
    list.remove_all();
    expect_events(std::vector<int>{31, 301, 403, 32, 302, 402});
    assert(list.head_ == 0 && list.current_ == 0 && list.callbacks == 2);
    first->payload_ = 0; delete first;
    delete replacement_payload;
}
static void test_alpha_current_reload() {
    AlphaNetStringStruct list;
    AlphaNetLedgerEntry *second = new AlphaNetLedgerEntry(new NetAllocationNode(2), 0, 2);
    AlphaNetLedgerEntry *first = new AlphaNetLedgerEntry(new NetAllocationNode(1), second, 1);
    AlphaNetLedgerEntry *replacement = new AlphaNetLedgerEntry(new NetAllocationNode(3), 0, 3);
    list.head_ = first; list.entry_count_ = 2; list.mutation = 3;
    list.replacement = replacement;
    list.remove_all();
    expect_events(std::vector<int>{31, 301, 401, 33, 303, 403});
    assert(list.head_ == 0 && list.current_ == 0 && list.callbacks == 2);
    discard(second);
}
static void test_alpha_guards_and_null_payload() {
    AlphaNetStringStruct empty;
    empty.entry_count_ = 3; empty.current_position_ = 7;
    empty.remove_all();
    assert(empty.entry_count_ == 3 && empty.current_position_ == 7);
    AlphaNetLedgerEntry *zero = new AlphaNetLedgerEntry(0, 0, 4);
    empty.head_ = zero; empty.entry_count_ = 0; empty.current_position_ = 8;
    empty.remove_all();
    expect_events(std::vector<int>{});
    assert(empty.head_ == 0 && empty.entry_count_ == 0 && empty.current_position_ == 0);
    zero->payload_ = 0; delete zero;
    events.clear();
    AlphaNetLedgerEntry *negative = new AlphaNetLedgerEntry(0, 0, 5);
    empty.head_ = negative; empty.entry_count_ = -1; empty.current_position_ = 6;
    empty.remove_all();
    assert(empty.head_ == 0 && empty.entry_count_ == 0 && empty.current_position_ == 0);
    negative->payload_ = 0; delete negative;
    events.clear();
    AlphaNetLedgerEntry *null_payload = new AlphaNetLedgerEntry(0, 0, 6);
    empty.head_ = null_payload; empty.entry_count_ = 1;
    empty.remove_all();
    expect_events(std::vector<int>{30, 406});
    assert(empty.callbacks == 1 && empty.head_ == 0);
}

int main() {
    test_net_count_reload(); events.clear();
    test_net_head_reload(); events.clear();
    test_net_current_reload(); events.clear();
    test_net_guards_and_null_payload(); events.clear();
    test_alpha_count_reload(); events.clear();
    test_alpha_head_reload(); events.clear();
    test_alpha_current_reload(); events.clear();
    test_alpha_guards_and_null_payload();
    return 0;
}
'''


def compile_and_run(tmp_path: Path, source: str, compiler: str) -> subprocess.CompletedProcess:
    source_path = tmp_path / "network-list-cleanup.cpp"
    binary_path = tmp_path / "network-list-cleanup"
    source_path.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary_path)], capture_output=True, text=True, timeout=10)


def source_methods():
    net_header = (ROOT / "src/net_class.h").read_text()
    alpha_header = (ROOT / "src/alphanet.h").read_text()
    net_source = (ROOT / "src/net_class.cpp").read_text()
    alpha_source = (ROOT / "src/alphanet_records.cpp").read_text()
    net_method = class_method(net_header, "NetTextListBase", "void remove_all()")
    alpha_class_start = alpha_header.index("class AlphaNetStringStruct :")
    alpha_class = alpha_header[alpha_class_start:]
    alpha_class_body = braced(alpha_class, 0)
    alpha_member = alpha_class_body.index("void remove_all()")
    alpha_brace = alpha_class_body.find("{", alpha_member)
    alpha_semicolon = alpha_class_body.find(";", alpha_member)
    inline = alpha_brace >= 0 and (alpha_semicolon < 0 or alpha_brace < alpha_semicolon)
    alpha_sources = [
        alpha_header,
        (ROOT / "src/alphanet.cpp").read_text(),
        alpha_source,
    ]
    alpha_definitions = []
    if inline:
        inline_method = "void remove_all() {" + braced(
            alpha_class_body, alpha_brace - 1
        ) + "}"
        alpha_definitions.append(inline_method.replace(
            "void remove_all()", "void AlphaNetStringStruct::remove_all()", 1))
    for source in alpha_sources[1:]:
        signature = "void AlphaNetStringStruct::remove_all()"
        if signature in source:
            alpha_definitions.append(function_body(source, signature))
    assert len(alpha_definitions) == 1, "expected one current AlphaNet remove_all definition"
    alpha_method = alpha_definitions[0]

    assert "while (index < entry_count_)" in net_method
    assert "for (int removed = 0; removed < entry_count_; ++removed)" in alpha_method
    assert "NetTextListBase::remove_all" not in net_source
    destructors = [function_body(net_source, "NetTextListBase::~NetTextListBase()")]
    for name in ("AlphaNetStringStruct", "AlphaNetLedger"):
        assert f"{name}::~{name}()" not in alpha_source
        destructors.append(class_method(alpha_header, name, f"~{name}()"))
    for destructor in destructors:
        assert "remove_all();" in destructor
        assert "current_position_ = 0;" in destructor
        assert destructor.index("remove_all();") < destructor.index("current_position_ = 0;")
    return net_method, alpha_method


def fixture(net_method: str, alpha_method: str) -> str:
    return (HARNESS.replace("NET_METHOD", net_method)
            .replace("ALPHA_REMOVE_ALL_DEFINITION", alpha_method))


def host_compilers():
    return [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


@pytest.mark.parametrize("compiler", host_compilers())
def test_actual_walks_preserve_reload_guards_and_delete_order(tmp_path, compiler):
    net_method, alpha_method = source_methods()
    result = compile_and_run(tmp_path, fixture(net_method, alpha_method), compiler)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", host_compilers())
@pytest.mark.parametrize("owner,mutation", [
    ("net", "count"), ("net", "head"), ("net", "current"), ("net", "delete_order"),
    ("alpha", "count"), ("alpha", "head"), ("alpha", "current"), ("alpha", "delete_order"),
])
def test_behavior_controls_reject_cached_state_and_swapped_delete_mutants(
        tmp_path, compiler, owner, mutation):
    net_method, alpha_method = source_methods()
    method = net_method if owner == "net" else alpha_method
    if mutation == "count":
        old = "while (index < entry_count_)" if owner == "net" else "removed < entry_count_; ++removed"
        new = "while (index < 2)" if owner == "net" else "removed < 2; ++removed"
        assert old in method
        mutated = method.replace(old, new, 1)
    elif mutation == "head":
        next_read = "current_ = head_->next;" if owner == "net" else "current_ = head_->next_;"
        payload_read = "Payload *const payload = head_->payload;" if owner == "net" else "Payload *payload = head_->payload_;"
        payload_field = "head_->payload" if owner == "net" else "head_->payload_"
        assert next_read in method and payload_read in method
        mutated = method.replace(next_read, "Entry *const doomed = head_;\n        " + next_read, 1)
        mutated = mutated.replace(payload_read, payload_read.replace("head_", "doomed"), 1)
        mutated = mutated.replace("delete head_;", "delete doomed;", 1)
        # The cleanup store is redirected too, modeling a cached node receiver.
        mutated = mutated.replace(payload_field + " = 0;", payload_field.replace("head_", "doomed") + " = 0;", 1)
    elif mutation == "current":
        next_read = "current_ = head_->next;" if owner == "net" else "current_ = head_->next_;"
        assert next_read in method and "head_ = current_;" in method
        mutated = method.replace(next_read, next_read + "\n          Entry *const saved_next = current_;", 1)
        mutated = mutated.replace("head_ = current_;", "head_ = saved_next;", 1)
    else:
        payload_delete = "delete payload;"
        node_delete = "delete head_;"
        assert payload_delete in method and node_delete in method
        mutated = method.replace(payload_delete, "delete_payload_later;", 1)
        mutated = mutated.replace(node_delete, payload_delete, 1)
        mutated = mutated.replace("delete_payload_later;", node_delete, 1)
    if owner == "net":
        net_method = mutated
    else:
        alpha_method = mutated
    result = compile_and_run(tmp_path, fixture(net_method, alpha_method), compiler)
    assert result.returncode != 0, (owner, mutation, "mutation escaped behavior controls")


def service_fixture():
    # Use the same independent 402970 callback/delete-order oracles for this
    # owner, while compiling its actual member definition and typed fields.
    # This projects list effects, not the generated scalar-delete ABI.
    net_method, _ = source_methods()
    source = (ROOT / "src/net_class.cpp").read_text()
    method = function_body(source, "void ServiceProviderList::remove_all()")
    assert "clear_owned_list" not in source
    assert "for (int removed = 0; removed < entry_count_; ++removed)" in method
    harness = (HARNESS.replace("AlphaNetStringStruct", "ServiceProviderList")
               .replace("AlphaNetLedgerEntry", "ServiceListEntry")
               .replace("NetAllocationNode", "ServiceStruct"))
    return harness.replace("NET_METHOD", net_method), method


@pytest.mark.parametrize("compiler", host_compilers())
@pytest.mark.parametrize("damage", ["", "count", "head", "current", "delete_order"])
def test_actual_service_member_cleanup_effects(tmp_path, compiler, damage):
    harness, method = service_fixture()
    if damage == "count":
        assert "removed < entry_count_" in method
        method = method.replace("removed < entry_count_", "removed < 2", 1)
    elif damage == "head":
        next_read = "current_ = head_->next_;"
        payload_read = "Payload *payload = head_->payload_;"
        assert next_read in method and payload_read in method
        method = method.replace(next_read, "Entry *const doomed = head_;\n" + next_read, 1)
        method = method.replace(payload_read, payload_read.replace("head_", "doomed"), 1)
        method = method.replace("head_->payload_ = 0;", "doomed->payload_ = 0;", 1)
        method = method.replace("delete head_;", "delete doomed;", 1)
    elif damage == "current":
        next_read = "current_ = head_->next_;"
        assert next_read in method and "head_ = current_;" in method
        method = method.replace(next_read, next_read + "\nEntry *const saved_next = current_;", 1)
        method = method.replace("head_ = current_;", "head_ = saved_next;", 1)
    elif damage == "delete_order":
        assert "delete payload;" in method and "delete head_;" in method
        method = method.replace("delete payload;", "delete_payload_later;", 1)
        method = method.replace("delete head_;", "delete payload;", 1)
        method = method.replace("delete_payload_later;", "delete head_;", 1)
    result = compile_and_run(
        tmp_path, harness.replace("ALPHA_REMOVE_ALL_DEFINITION", method), compiler)
    if damage:
        assert result.returncode < 0 and "Assertion" in result.stderr, (damage, result.stderr)
    else:
        assert result.returncode == 0, result.stderr

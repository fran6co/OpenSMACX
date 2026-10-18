"""Exercise the real ServiceProviderList allocator/operator/add source."""
from pathlib import Path
import os
import subprocess
HERE = Path(__file__).resolve().parent


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if (candidate / "src/net_class.cpp").exists():
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks src/net_class.cpp")
    for candidate in (HERE, *HERE.parents):
        if (candidate / "src/net_class.cpp").exists():
            return candidate
    raise AssertionError("cannot locate OpenSMACX source root")


def extract_definition(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and nxt == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and nxt == "*":
            block_comment = True
            index += 2
            continue
        if char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated product definition: {signature}")


def product_bodies():
    source = (repo_root() / "src/net_class.cpp").read_text()
    signatures = (
        "void *allocate_net_node(size_t size, Heap *owner)",
        "void cleanup_failed_net_construction(void *memory)",
        "void *ServiceStruct::operator new(size_t size, Heap *owner)",
        "void *ServiceStruct::operator new(size_t size, ServiceProviderList *list)",
        "void ServiceStruct::operator delete(void *memory, Heap *)",
        "void ServiceStruct::operator delete(void *memory, ServiceProviderList *)",
        "void ServiceStruct::operator delete(void *memory)",
        "void *ServiceListEntry::operator new(size_t size, Heap *owner)",
        "void *ServiceListEntry::operator new(size_t size, ServiceProviderList *list)",
        "void ServiceListEntry::operator delete(void *memory, Heap *)",
        "void ServiceListEntry::operator delete(void *memory, ServiceProviderList *)",
        "void ServiceListEntry::operator delete(void *memory)",
        "ServiceProviderList::ServiceProviderList()",
        "int ServiceProviderList::on_insert(ServiceStruct *)",
        "void ServiceProviderList::on_remove(ServiceStruct *)",
        "int ServiceProviderList::add(int id)",
    )
    return "\n\n".join(extract_definition(source, signature) for signature in signatures)


FIXTURE = r'''#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

enum { MB_OK = 0 };
class Heap;
class ServiceProviderList;
class ServiceStruct;
class ServiceListEntry;
static Heap *StringAllocationHeap;
static int fail_mem_call;
static int mem_calls;
static ServiceProviderList *watched_list;
static Heap *mutate_allocator_to;
static Heap *mutate_fallback_to;
static int mutate_after_get;
static int failure_snapshot;

class Heap {
public:
    int tag;
    int calls;
    int fail_call;
    size_t sizes[4];
    Heap(int identity) : tag(identity), calls(0), fail_call(0) {}
    void *get(size_t size);
};

class StringAllocationBase {
public:
    Heap *allocation_owner_;
    StringAllocationBase() : allocation_owner_(StringAllocationHeap) {
        StringAllocationHeap = 0;
    }
    virtual ~StringAllocationBase() {}
};

struct ServiceStruct : public virtual StringAllocationBase {
    char *name_;
    char provider_guid_[16];
    ServiceStruct() : name_(0) { memset(provider_guid_, 0, sizeof(provider_guid_)); }
    static void *operator new(size_t, Heap *) throw();
    static void *operator new(size_t, ServiceProviderList *) throw();
    static void operator delete(void *, Heap *);
    static void operator delete(void *, ServiceProviderList *);
    static void operator delete(void *);
};

struct ServiceListEntry : public virtual StringAllocationBase {
    int id_;
    ServiceStruct *payload_;
    ServiceListEntry *next_;
    ServiceListEntry *previous_;
    ServiceListEntry() : id_(0), payload_(0), next_(0), previous_(0) {}
    static void *operator new(size_t, Heap *) throw();
    static void *operator new(size_t, ServiceProviderList *) throw();
    static void operator delete(void *, Heap *);
    static void operator delete(void *, ServiceProviderList *);
    static void operator delete(void *);
};

class ServiceProviderList : public virtual StringAllocationBase {
public:
    virtual int on_insert(ServiceStruct *);
    virtual void on_remove(ServiceStruct *);
    ServiceProviderList();
    virtual ~ServiceProviderList() {}
    int add(int id);
    ServiceListEntry *head_;
    ServiceListEntry *current_;
    int entry_count_;
    int current_position_;
    Heap *allocator_;
};

void *mem_get(size_t size) {
    ++mem_calls;
    if (fail_mem_call == mem_calls) return 0;
    return malloc(size);
}
void MessageBoxA(void *, const char *, const char *, int) {
    printf("fatal snapshot=%d\n", failure_snapshot);
}
void *Heap::get(size_t size) {
    ++calls;
    sizes[calls - 1] = size;
    if (mutate_after_get == calls && watched_list != 0) {
        watched_list->allocator_ = mutate_allocator_to;
        if (mutate_fallback_to != 0)
            watched_list->allocation_owner_ = mutate_fallback_to;
    }
    if (fail_call == calls) return 0;
    return malloc(size);
}

/* PRODUCT ALLOCATION/OWNER BODIES */

class ProbeList : public ServiceProviderList {
public:
    int hook_calls;
    int hook_id;
    int hook_count;
    int hook_position;
    ServiceListEntry *hook_current;
    ProbeList() : hook_calls(0), hook_id(0), hook_count(0), hook_position(0),
        hook_current(0) {}
    int on_insert(ServiceStruct *payload) {
        ++hook_calls;
        hook_id = current_ == 0 ? -1 : current_->id_;
        hook_count = entry_count_;
        hook_position = current_position_;
        hook_current = current_;
        assert(payload != 0);
        return 91;
    }
};

static void reset_globals() {
    StringAllocationHeap = 0;
    fail_mem_call = mem_calls = 0;
    watched_list = 0;
    mutate_allocator_to = mutate_fallback_to = 0;
    mutate_after_get = 0;
    failure_snapshot = 0;
}

static void test_empty_explicit_and_owner_reload() {
    reset_globals();
    Heap first(1), second(2);
    ProbeList list;
    list.allocator_ = &first;
    watched_list = &list;
    mutate_after_get = 1;
    mutate_allocator_to = &second;
    assert(list.add(17) == 0);
    assert(first.calls == 1 && second.calls == 1);
    assert(first.sizes[0] == sizeof(ServiceListEntry));
    assert(second.sizes[0] == sizeof(ServiceStruct));
    assert(list.head_ != 0 && list.head_->id_ == 17);
    assert(list.head_->next_ == list.head_ && list.head_->previous_ == list.head_);
    assert(list.head_->payload_ != 0);
    assert(list.head_->StringAllocationBase::allocation_owner_ == &first);
    assert(list.head_->payload_->StringAllocationBase::allocation_owner_ == &second);
    assert(StringAllocationHeap == 0);
    assert(list.current_ == list.head_ && list.entry_count_ == 1);
    assert(list.current_position_ == 0);
    assert(list.hook_calls == 1 && list.hook_id == -1);
    assert(list.hook_current == 0 && list.hook_count == 0 && list.hook_position == 0);
}

static void test_nonempty_fallback_reloads_owner() {
    reset_globals();
    Heap entry_owner(3), payload_owner(4);
    ProbeList list;
    list.allocation_owner_ = &entry_owner;
    assert(list.add(1) == 0);
    ServiceListEntry *first = list.head_;
    assert(entry_owner.calls == 2);
    list.allocator_ = 0;
    watched_list = &list;
    mutate_after_get = 3;
    mutate_fallback_to = &payload_owner;
    assert(list.add(2) == 0);
    ServiceListEntry *second = list.head_->previous_;
    assert(entry_owner.calls == 3 && payload_owner.calls == 1);
    assert(second != first && second->id_ == 2 && second->next_ == list.head_);
    assert(second->previous_ == first && first->next_ == second);
    assert(second->StringAllocationBase::allocation_owner_ == &entry_owner);
    assert(second->payload_->StringAllocationBase::allocation_owner_ == &payload_owner);
    assert(list.hook_calls == 2 && list.hook_id == 2);
    assert(list.hook_current == second && list.hook_count == 1 && list.hook_position == 0);
    assert(list.current_ == second && list.entry_count_ == 2 && list.current_position_ == 1);
}

static void test_null_and_partial_failures() {
    reset_globals();
    ProbeList empty;
    fail_mem_call = 1;
    assert(empty.add(5) == 4);
    assert(empty.head_ == 0 && empty.current_ == 0 && empty.entry_count_ == 0);
    assert(StringAllocationHeap == 0 && empty.hook_calls == 0);

    reset_globals();
    ProbeList empty_payload;
    fail_mem_call = 2;
    assert(empty_payload.add(6) == 4);
    assert(empty_payload.head_ != 0 && empty_payload.head_->id_ == 6);
    assert(empty_payload.head_->next_ == empty_payload.head_);
    assert(empty_payload.head_->payload_ == 0);
    assert(empty_payload.current_ == 0 && empty_payload.entry_count_ == 0);
    assert(empty_payload.current_position_ == 0 && empty_payload.hook_calls == 0);

    reset_globals();
    ProbeList populated;
    assert(populated.add(7) == 0);
    ServiceListEntry *old = populated.head_;
    fail_mem_call = mem_calls + 1;
    assert(populated.add(8) == 4);
    assert(old->next_ == 0 && old->previous_ == old);
    assert(populated.head_ == old && populated.current_ == old);
    assert(populated.entry_count_ == 1 && populated.current_position_ == 0);
    assert(populated.hook_calls == 1);

    reset_globals();
    ProbeList payload_fail;
    assert(payload_fail.add(9) == 0);
    old = payload_fail.head_;
    fail_mem_call = mem_calls + 2;
    assert(payload_fail.add(10) == 4);
    ServiceListEntry *partial = old->previous_;
    assert(partial != old && partial->id_ == 10 && partial->payload_ == 0);
    assert(old->next_ == partial && partial->next_ == old && old->previous_ == partial);
    assert(payload_fail.current_ == partial && payload_fail.entry_count_ == 1);
    assert(payload_fail.current_position_ == 0 && payload_fail.hook_calls == 1);
}

static int fatal_child() {
    reset_globals();
    Heap heap(7);
    ProbeList list;
    list.allocator_ = &heap;
    watched_list = &list;
    failure_snapshot = 42;
    heap.fail_call = 1;
    (void)list.add(11);
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "fatal") == 0) return fatal_child();
    test_empty_explicit_and_owner_reload();
    test_nonempty_fallback_reloads_owner();
    test_null_and_partial_failures();
    return 0;
}
'''


def compile_fixture(tmp_path):
    source = product_bodies()
    assert source.count("? allocate_owned_node") == 0
    assert "Heap *const entry_allocator = allocator_;" in source
    assert source.count("Heap *const payload_allocator = allocator_;") == 2
    assert source.count("new(entry_allocator) ServiceListEntry") == 2
    assert source.count("new(payload_allocator) ServiceStruct") == 2
    assert source.count("new(this) ServiceListEntry") == 2
    assert source.count("new(this) ServiceStruct") == 2
    cpp = tmp_path / "service-provider-add.cpp"
    exe = tmp_path / "service-provider-add"
    anchor = "/* PRODUCT ALLOCATION/OWNER BODIES */"
    assert FIXTURE.count(anchor) == 1
    cpp.write_text(FIXTURE.replace(anchor, source))
    built = subprocess.run(
        ["c++", "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror=return-type",
         str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return exe


def test_product_service_provider_add_owners_and_partial_failure(tmp_path):
    exe = compile_fixture(tmp_path)
    run = subprocess.run([str(exe)], capture_output=True, text=True)
    assert run.returncode == 0, run.stdout + run.stderr


def test_product_service_provider_add_preserves_heap_fatal_policy(tmp_path):
    exe = compile_fixture(tmp_path)
    run = subprocess.run([str(exe), "fatal"], capture_output=True, text=True)
    assert run.returncode == 3
    assert "fatal snapshot=42" in run.stdout

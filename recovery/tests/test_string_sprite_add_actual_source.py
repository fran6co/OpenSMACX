"""Execute the product StringStruct/SpriteList allocator and add bodies."""
from pathlib import Path
import os
import subprocess
import shutil


HERE = Path(__file__).resolve().parent


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if (candidate / "src/stringstruct.cpp").exists():
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks src/stringstruct.cpp")
    for candidate in (HERE, *HERE.parents):
        if (candidate / "src/stringstruct.cpp").exists():
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
    root = repo_root()
    strings = (root / "src/stringstruct.cpp").read_text()
    string_header = (root / "src/stringstruct.h").read_text()
    sprites = (root / "src/spritebox.cpp").read_text()
    string_signatures = (
        "StringStruct::StringStruct()",
        "int StringStruct::on_insert(void *)",
        "void StringStruct::on_remove(void *)",
        "int StringStruct::add(int id)",
    )
    sprite_helper_signature = (
        "template<class Node>\nNode *allocate_sprite_node(Heap *explicit_owner, SpriteList *list)"
    )
    sprite_signatures = (
        "SpriteList::SpriteList()",
        "int SpriteList::on_insert(void *raw_payload)",
        "void SpriteList::on_remove(void *raw_payload)",
        "int SpriteList::add(int id)",
    )
    sprite_bodies = [extract_definition(sprites, sig) for sig in sprite_signatures]
    if sprite_helper_signature in sprites:
        sprite_bodies.insert(0, extract_definition(sprites, sprite_helper_signature))
    return "\n\n".join(
        [extract_definition(string_header, string_signatures[0])]
        + [extract_definition(strings, sig) for sig in string_signatures[1:]]
        + sprite_bodies
    )


FIXTURE = r'''#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

enum { MB_OK = 0 };
class Heap;
class StringStruct;
class SpriteList;
static Heap *StringAllocationHeap;
static int mem_calls;
static int fail_mem_call;
static size_t mem_sizes[16];
static StringStruct *watch_string;
static SpriteList *watch_sprite;
static Heap *mutate_allocator_to;
static Heap *mutate_fallback_to;
static int mutate_heap_call;
static char *mutate_pending_label_to;
static int fatal_kind;

class Heap {
public:
    int calls;
    int fail_call;
    size_t sizes[8];
    Heap() : calls(0), fail_call(0) {}
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

class ListPayloadBase : public virtual StringAllocationBase {
public:
    virtual ~ListPayloadBase() {}
};

struct SpriteStruct : public ListPayloadBase {
    void *sprite_;
    char *label_;
    unsigned int field_C_;
    unsigned int field_10_;
    unsigned int field_14_;
    SpriteStruct() : sprite_(0), label_(0), field_C_(0), field_10_(0), field_14_(0) {}
};

class StringStructPayload : public ListPayloadBase {
public:
    char *text;
    unsigned int field_8_;
    unsigned int field_C_;
    StringStructPayload() : text(0), field_8_(0), field_C_(0) {}
};

struct StringStructEntry : public virtual StringAllocationBase {
    int id;
    ListPayloadBase *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructEntry() : id(0), payload(0), next(0), previous(0) {}
    StringStructPayload *string_payload() { return static_cast<StringStructPayload *>(payload); }
    SpriteStruct *sprite_payload() { return static_cast<SpriteStruct *>(payload); }
};

class StringStruct : public virtual StringAllocationBase {
public:
    virtual int on_insert(void *);
    virtual void on_remove(void *);
    StringStruct();
    virtual ~StringStruct() {}
    int add(int id);
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    Heap *allocator_;
};

class SpriteListBase : public virtual StringAllocationBase {
public:
    virtual int on_insert(void *) { return 0; }
    virtual void on_remove(void *) {}
    SpriteListBase() : head_(0), current_(0), entry_count_(0), current_position_(0), allocator_(0) {}
    virtual ~SpriteListBase() {}
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    Heap *allocator_;
};

class SpriteList : public SpriteListBase {
public:
    SpriteList();
    int on_insert(void *);
    void on_remove(void *);
    int add(int id);
    virtual ~SpriteList() {}
    void *pending_sprite_;
    char *pending_label_;
    unsigned int field_24_;
};

void *mem_get(size_t size) {
    ++mem_calls;
    mem_sizes[mem_calls - 1] = size;
    if (fail_mem_call == mem_calls) return 0;
    return malloc(size);
}
void MessageBoxA(void *window, const char *text, const char *caption, int flags) {
    assert(window == 0 && flags == MB_OK);
    assert(strcmp(text, "out of heap space in new function") == 0);
    assert(strcmp(caption, "Aborting") == 0);
    printf("fatal kind=%d\n", fatal_kind);
}
void *Heap::get(size_t size) {
    ++calls;
    sizes[calls - 1] = size;
    if (mutate_heap_call == calls) {
        if (watch_string != 0) {
            watch_string->allocator_ = mutate_allocator_to;
            if (mutate_fallback_to != 0)
                watch_string->allocation_owner_ = mutate_fallback_to;
        }
        if (watch_sprite != 0) {
            watch_sprite->allocator_ = mutate_allocator_to;
            if (mutate_pending_label_to != 0)
                watch_sprite->pending_label_ = mutate_pending_label_to;
            if (mutate_fallback_to != 0)
                watch_sprite->allocation_owner_ = mutate_fallback_to;
        }
    }
    if (fail_call == calls) return 0;
    return malloc(size);
}

/* PRODUCT ALLOCATION AND ADD BODIES */

class ProbeString : public StringStruct {
public:
    int hooks;
    int hook_id;
    int hook_count;
    int hook_position;
    StringStructEntry *hook_current;
    ProbeString() : hooks(0), hook_id(-1), hook_count(-1), hook_position(-1), hook_current(0) {}
    int on_insert(void *payload) {
        ++hooks;
        hook_id = current_ == 0 ? -1 : current_->id;
        hook_count = entry_count_;
        hook_position = current_position_;
        hook_current = current_;
        assert(payload != 0);
        return 73;
    }
};

class ProbeSprite : public SpriteList {
public:
    int hooks;
    int hook_id;
    int hook_count;
    int hook_position;
    StringStructEntry *hook_current;
    ProbeSprite() : hooks(0), hook_id(-1), hook_count(-1), hook_position(-1), hook_current(0) {}
    int on_insert(void *payload) {
        ++hooks;
        hook_id = current_ == 0 ? -1 : current_->id;
        hook_count = entry_count_;
        hook_position = current_position_;
        hook_current = current_;
        assert(payload != 0);
        return 73;
    }
};

static void reset_state() {
    StringAllocationHeap = 0;
    mem_calls = fail_mem_call = 0;
    watch_string = 0;
    watch_sprite = 0;
    mutate_allocator_to = mutate_fallback_to = 0;
    mutate_heap_call = 0;
    mutate_pending_label_to = 0;
    fatal_kind = 0;
}

static void test_string_explicit_snapshot_and_reload() {
    reset_state();
    Heap first, second;
    ProbeString list;
    list.allocator_ = &first;
    watch_string = &list;
    mutate_heap_call = 1;
    mutate_allocator_to = &second;
    assert(list.add(31) == 0);
    assert(first.calls == 1 && second.calls == 1);
    assert(first.sizes[0] == sizeof(StringStructEntry));
    assert(second.sizes[0] == sizeof(StringStructPayload));
    assert(list.head_ && list.head_->id == 31 && list.head_->payload);
    assert(list.head_->next == list.head_ && list.head_->previous == list.head_);
    assert(list.head_->StringAllocationBase::allocation_owner_ == &first);
    assert(list.head_->payload->StringAllocationBase::allocation_owner_ == &second);
    assert(StringAllocationHeap == 0);
    assert(list.current_ == list.head_ && list.entry_count_ == 1 && list.current_position_ == 0);
    assert(list.hooks == 1 && list.hook_id == -1 && list.hook_current == 0);
    assert(list.hook_count == 0 && list.hook_position == 0);
}

static void test_string_fallback_owner_is_read_per_allocation() {
    reset_state();
    Heap first, second;
    ProbeString list;
    list.allocation_owner_ = &first;
    watch_string = &list;
    mutate_heap_call = 1;
    mutate_fallback_to = &second;
    assert(list.add(32) == 0);
    assert(first.calls == 1 && second.calls == 1);
    assert(list.head_->StringAllocationBase::allocation_owner_ == &first);
    assert(list.head_->payload->StringAllocationBase::allocation_owner_ == &second);
    assert(first.sizes[0] == sizeof(StringStructEntry));
    assert(second.sizes[0] == sizeof(StringStructPayload));
}

static void test_string_crt_allocations_use_typed_sizes_and_clear_handoff() {
    reset_state();
    ProbeString list;
    assert(list.add(35) == 0);
    assert(mem_calls == 2);
    assert(mem_sizes[0] == sizeof(StringStructEntry));
    assert(mem_sizes[1] == sizeof(StringStructPayload));
    assert(list.head_->StringAllocationBase::allocation_owner_ == 0);
    assert(list.head_->payload->StringAllocationBase::allocation_owner_ == 0);
    assert(StringAllocationHeap == 0);
}

static void test_string_nonempty_append_and_hook_order() {
    reset_state();
    Heap first, second;
    ProbeString list;
    list.allocator_ = &first;
    assert(list.add(33) == 0);
    StringStructEntry *old = list.head_;
    watch_string = &list;
    mutate_heap_call = 3;
    mutate_allocator_to = &second;
    assert(list.add(34) == 0);
    StringStructEntry *added = list.head_->previous;
    assert(first.calls == 3 && second.calls == 1);
    assert(first.sizes[2] == sizeof(StringStructEntry));
    assert(second.sizes[0] == sizeof(StringStructPayload));
    assert(added != old && added->id == 34 && added->payload);
    assert(old->next == added && added->next == list.head_ && added->previous == old);
    assert(added->StringAllocationBase::allocation_owner_ == &first);
    assert(added->payload->StringAllocationBase::allocation_owner_ == &second);
    assert(list.current_ == added && list.entry_count_ == 2 && list.current_position_ == 1);
    assert(list.hooks == 2 && list.hook_id == 34 && list.hook_current == added);
    assert(list.hook_count == 1 && list.hook_position == 0);
}

static void test_sprite_explicit_snapshot_and_reload() {
    reset_state();
    Heap first, second;
    ProbeSprite list;
    list.allocator_ = &first;
    watch_sprite = &list;
    mutate_heap_call = 1;
    mutate_allocator_to = &second;
    assert(list.add(41) == 0);
    assert(first.calls == 1 && second.calls == 1);
    assert(first.sizes[0] == sizeof(StringStructEntry));
    assert(second.sizes[0] == sizeof(SpriteStruct));
    assert(list.head_ && list.head_->id == 41 && list.head_->payload);
    assert(list.head_->next == list.head_ && list.head_->previous == list.head_);
    assert(list.head_->StringAllocationBase::allocation_owner_ == &first);
    assert(list.head_->payload->StringAllocationBase::allocation_owner_ == &second);
    assert(StringAllocationHeap == 0);
    assert(list.current_ == list.head_ && list.entry_count_ == 1 && list.current_position_ == 0);
    assert(list.hooks == 1 && list.hook_id == -1 && list.hook_current == 0);
    assert(list.hook_count == 0 && list.hook_position == 0);
}

static void test_sprite_fallback_owner_is_read_per_allocation() {
    reset_state();
    Heap first, second;
    ProbeSprite list;
    list.allocation_owner_ = &first;
    watch_sprite = &list;
    mutate_heap_call = 1;
    mutate_fallback_to = &second;
    assert(list.add(42) == 0);
    assert(first.calls == 1 && second.calls == 1);
    assert(list.head_->StringAllocationBase::allocation_owner_ == &first);
    assert(list.head_->payload->StringAllocationBase::allocation_owner_ == &second);
}

static void test_sprite_nonempty_append_and_hook_order() {
    reset_state();
    Heap first, second;
    ProbeSprite list;
    list.allocator_ = &first;
    assert(list.add(43) == 0);
    StringStructEntry *old = list.head_;
    watch_sprite = &list;
    mutate_heap_call = 3;
    mutate_allocator_to = &second;
    assert(list.add(44) == 0);
    StringStructEntry *added = list.head_->previous;
    assert(first.calls == 3 && second.calls == 1);
    assert(added != old && added->id == 44 && added->payload);
    assert(old->next == added && added->next == list.head_ && added->previous == old);
    assert(added->StringAllocationBase::allocation_owner_ == &first);
    assert(added->payload->StringAllocationBase::allocation_owner_ == &second);
    assert(list.current_ == added && list.entry_count_ == 2 && list.current_position_ == 1);
    assert(list.hooks == 2 && list.hook_id == 44 && list.hook_current == added);
    assert(list.hook_count == 1 && list.hook_position == 0);
}

static void test_string_null_failures_preserve_partial_effects() {
    reset_state();
    ProbeString first;
    fail_mem_call = 1;
    assert(first.add(51) == 4);
    assert(first.head_ == 0 && first.current_ == 0 && first.entry_count_ == 0);
    assert(first.hooks == 0 && StringAllocationHeap == 0);

    reset_state();
    ProbeString second;
    fail_mem_call = 2;
    assert(second.add(52) == 4);
    assert(second.head_ && second.head_->id == 52 && second.head_->payload == 0);
    assert(second.head_->next == second.head_ && second.head_->previous == second.head_);
    assert(second.current_ == 0 && second.entry_count_ == 0 && second.current_position_ == 0);
    assert(second.hooks == 0 && StringAllocationHeap == 0);

    reset_state();
    ProbeString third;
    assert(third.add(53) == 0);
    StringStructEntry *old = third.head_;
    fail_mem_call = mem_calls + 1;
    assert(third.add(54) == 4);
    assert(old->next == 0 && old->previous == old);
    assert(third.head_ == old && third.current_ == old);
    assert(third.entry_count_ == 1 && third.current_position_ == 0 && third.hooks == 1);

    reset_state();
    ProbeString fourth;
    assert(fourth.add(55) == 0);
    old = fourth.head_;
    fail_mem_call = mem_calls + 2;
    assert(fourth.add(56) == 4);
    StringStructEntry *partial = old->previous;
    assert(partial != old && partial->id == 56 && partial->payload == 0);
    assert(old->next == partial && partial->next == old && old->previous == partial);
    assert(fourth.current_ == partial && fourth.entry_count_ == 1);
    assert(fourth.current_position_ == 0 && fourth.hooks == 1);
}

static void test_sprite_label_is_reloaded_after_allocation() {
    reset_state();
    Heap heap;
    SpriteList list;
    char old_label[] = "old";
    char new_label[] = "new";
    int sprite;
    list.allocator_ = &heap;
    list.pending_sprite_ = &sprite;
    list.pending_label_ = old_label;
    list.field_24_ = 81;
    watch_sprite = &list;
    mutate_heap_call = 3;
    mutate_allocator_to = &heap;
    mutate_pending_label_to = new_label;
    assert(list.add(45) == 0);
    assert(heap.calls == 3 && heap.sizes[2] == 4);
    SpriteStruct *payload = list.head_->sprite_payload();
    assert(strcmp(payload->label_, "new") == 0);
    assert(list.pending_label_ == new_label);
    assert(payload->sprite_ == &sprite && payload->field_C_ == 81);
}

static void test_sprite_null_failures_preserve_partial_effects() {
    reset_state();
    ProbeSprite first;
    fail_mem_call = 1;
    assert(first.add(61) == 4);
    assert(first.head_ == 0 && first.current_ == 0 && first.entry_count_ == 0);
    assert(first.hooks == 0 && StringAllocationHeap == 0);

    reset_state();
    ProbeSprite second;
    fail_mem_call = 2;
    assert(second.add(62) == 4);
    assert(second.head_ && second.head_->id == 62 && second.head_->payload == 0);
    assert(second.head_->next == second.head_ && second.head_->previous == second.head_);
    assert(second.current_ == 0 && second.entry_count_ == 0 && second.current_position_ == 0);
    assert(second.hooks == 0 && StringAllocationHeap == 0);

    reset_state();
    ProbeSprite third;
    assert(third.add(63) == 0);
    StringStructEntry *old = third.head_;
    fail_mem_call = mem_calls + 1;
    assert(third.add(64) == 4);
    assert(old->next == 0 && old->previous == old);
    assert(third.head_ == old && third.current_ == old);
    assert(third.entry_count_ == 1 && third.current_position_ == 0 && third.hooks == 1);

    reset_state();
    ProbeSprite fourth;
    assert(fourth.add(65) == 0);
    old = fourth.head_;
    fail_mem_call = mem_calls + 2;
    assert(fourth.add(66) == 4);
    StringStructEntry *partial = old->previous;
    assert(partial != old && partial->id == 66 && partial->payload == 0);
    assert(old->next == partial && partial->next == old && old->previous == partial);
    assert(fourth.current_ == partial && fourth.entry_count_ == 1);
    assert(fourth.current_position_ == 0 && fourth.hooks == 1);
}

static int fatal_child(const char *which) {
    reset_state();
    Heap heap;
    if (strncmp(which, "fatal-string", 12) == 0) {
        ProbeString list;
        list.allocator_ = &heap;
        fatal_kind = 1;
        if (strcmp(which, "fatal-string-entry") == 0) {
            heap.fail_call = 1;
        } else if (strcmp(which, "fatal-string-payload") == 0) {
            heap.fail_call = 2;
        } else if (strcmp(which, "fatal-string-append-entry") == 0) {
            assert(list.add(70) == 0);
            heap.fail_call = heap.calls + 1;
        } else if (strcmp(which, "fatal-string-append-payload") == 0) {
            assert(list.add(70) == 0);
            heap.fail_call = heap.calls + 2;
        } else {
            assert(false);
        }
        (void)list.add(71);
    } else {
        ProbeSprite list;
        list.allocator_ = &heap;
        heap.fail_call = 1;
        fatal_kind = 2;
        (void)list.add(72);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1) return fatal_child(argv[1]);
    test_string_explicit_snapshot_and_reload();
    test_string_fallback_owner_is_read_per_allocation();
    test_string_crt_allocations_use_typed_sizes_and_clear_handoff();
    test_string_nonempty_append_and_hook_order();
    test_sprite_explicit_snapshot_and_reload();
    test_sprite_fallback_owner_is_read_per_allocation();
    test_sprite_nonempty_append_and_hook_order();
    test_sprite_label_is_reloaded_after_allocation();
    test_string_null_failures_preserve_partial_effects();
    test_sprite_null_failures_preserve_partial_effects();
    return 0;
}
'''


def compile_fixture(tmp_path, source=None):
    if source is None:
        source = product_bodies()
    string_source = (repo_root() / "src/stringstruct.cpp").read_text()
    string_add = extract_definition(string_source, "int StringStruct::add(int id)")
    assert "static_cast<StringStructEntry *>(0)" not in source
    assert "static_cast<StringStructPayload *>(0)" not in source
    assert "allocate_list_node" not in string_source
    assert string_add.count("Heap *const entry_allocator = allocator_;") == 1
    assert string_add.count("Heap *const payload_allocator = allocator_;") == 2
    assert string_add.count("::new(entry_memory) StringStructEntry") == 1
    assert string_add.count("::new(payload_memory) StringStructPayload") == 1
    assert string_add.count("::new(append_entry_memory) StringStructEntry") == 1
    assert string_add.count("::new(append_payload_memory) StringStructPayload") == 1
    anchor = "/* PRODUCT ALLOCATION AND ADD BODIES */"
    assert FIXTURE.count(anchor) == 1
    cpp = tmp_path / "string-sprite-add.cpp"
    exe = tmp_path / "string-sprite-add"
    cpp.write_text(FIXTURE.replace(anchor, source))
    built = subprocess.run(
        ["c++", "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror=return-type",
         str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return exe


def test_product_string_and_sprite_add_owner_timing_and_partial_failure(tmp_path):
    exe = compile_fixture(tmp_path)
    run = subprocess.run([str(exe)], capture_output=True, text=True)
    assert run.returncode == 0, run.stdout + run.stderr


def test_product_string_and_sprite_add_keep_heap_fatal_policy(tmp_path):
    exe = compile_fixture(tmp_path)
    for argument, kind in (
        ("fatal-string-entry", 1),
        ("fatal-string-payload", 1),
        ("fatal-string-append-entry", 1),
        ("fatal-string-append-payload", 1),
        ("fatal-sprite", 2),
    ):
        run = subprocess.run([str(exe), argument], capture_output=True, text=True)
        assert run.returncode == 3
        assert f"fatal kind={kind}" in run.stdout


def test_cached_pending_label_mutant_fails_allocator_callback(tmp_path):
    source = product_bodies()
    body = extract_definition(source, "int SpriteList::on_insert(void *raw_payload)")
    anchor = "payload->sprite_ = pending_sprite_;"
    assert body.count(anchor) == 1 and body.count("pending_label_") == 4
    cached = body.replace("pending_label_", "saved_label").replace(
        anchor, anchor + "\n    char *const saved_label = pending_label_;", 1)
    exe = compile_fixture(tmp_path, source.replace(body, cached, 1))
    run = subprocess.run([str(exe)], capture_output=True, text=True)
    assert run.returncode != 0, "cached label survived allocator mutation"
    assert 'strcmp(payload->label_, "new") == 0' in run.stderr


def test_sprite_insert_allocation_selection_receiver_and_destination_phases(tmp_path):
    """Boundary doubles expose read phases, not a claim strlen mutates objects."""
    root = repo_root()
    body = extract_definition((root / "src/spritebox.cpp").read_text(),
                              "int SpriteList::on_insert(void *raw_payload)")
    fixture = r"""
#include <cassert>
#include <cstring>
#include <cstdint>
#include <new>
class Heap { public: int calls; Heap(): calls(0) {} void *get(size_t); };
struct SpriteStruct { void *sprite_; char *label_; unsigned field_C_; };
class SpriteList {
public:
    Heap *allocator_;
    void *pending_sprite_;
    char *pending_label_;
    unsigned field_24_;
    int on_insert(void *);
};
static SpriteList *list;
static Heap *strlen_owner, *expected_owner;
static bool mutate_strlen, fail_allocation;
static int lengths, mem_calls, concats;
static char buffer[64], old_label[] = "old", new_label[] = "new";
static char *allocation, *expected_destination;
static size_t phase_strlen(const char *text) {
    assert(text == old_label); ++lengths;
    size_t result = std::strlen(text);
    if (mutate_strlen) list->allocator_ = strlen_owner;
    return result;
}
void *Heap::get(size_t bytes) {
    assert(this == expected_owner && bytes == 4); ++calls;
    if (fail_allocation) return 0;
    list->pending_label_ = new_label;
    return allocation;
}
static void *mem_get(size_t bytes) {
    assert(bytes == 4); ++mem_calls;
    if (fail_allocation) return 0;
    list->pending_label_ = new_label;
    return allocation;
}
static char *phase_strcat(char *destination, const char *source) {
    assert(destination == expected_destination && source == new_label);
    ++concats;
    return std::strcat(destination, source);
}
#define strlen phase_strlen
#define strcat phase_strcat
ACTUAL
#undef strcat
#undef strlen
static void reset(SpriteList &owner) {
    list = &owner; owner.allocator_ = 0;
    owner.pending_sprite_ = reinterpret_cast<void *>(7);
    owner.pending_label_ = old_label; owner.field_24_ = 81;
    mutate_strlen = fail_allocation = false;
    lengths = mem_calls = concats = 0;
    std::memset(buffer, 0, sizeof buffer);
    allocation = expected_destination = buffer;
}
int main() {
    SpriteList owner; SpriteStruct payload;
    Heap before, after;
    reset(owner); owner.allocator_ = &before;
    mutate_strlen = true; strlen_owner = expected_owner = &after;
    assert(owner.on_insert(&payload) == 0);
    assert(before.calls == 0 && after.calls == 1 && mem_calls == 0 && lengths == 1);
    assert(concats == 1 && std::strcmp(payload.label_, "new") == 0);
    assert(payload.sprite_ == reinterpret_cast<void *>(7) && payload.field_C_ == 81);

    reset(owner); mutate_strlen = true; strlen_owner = &after;
    assert(owner.on_insert(&payload) == 0);
    assert(mem_calls == 1 && after.calls == 1 && lengths == 1 && concats == 1);

    // Allocator-owned bytes may overlap the destination pointer's object
    // representation. Pin zeroes one byte then rereads that stored pointer.
    alignas(256) unsigned char slab[1024] = {};
    SpriteStruct *alias = new(slab + 384) SpriteStruct;
    reset(owner); owner.allocator_ = expected_owner = &before;
    allocation = reinterpret_cast<char *>(&alias->label_);
    uintptr_t address = reinterpret_cast<uintptr_t>(allocation);
    assert(*reinterpret_cast<unsigned char *>(&address) == (address & 255u));
    expected_destination = reinterpret_cast<char *>(address & ~uintptr_t(255));
    assert(owner.on_insert(alias) == 0);
    assert(alias->label_ == expected_destination && concats == 1);
    assert(std::strcmp(expected_destination, "new") == 0);

    reset(owner); owner.pending_label_ = 0;
    payload.field_C_ = 0xFEED;
    assert(owner.on_insert(&payload) == 0 && payload.label_ == 0 && payload.field_C_ == 81);
    assert(lengths == 0 && mem_calls == 0 && concats == 0);
    reset(owner); fail_allocation = true; payload.field_C_ = 0xFEED;
    assert(owner.on_insert(&payload) == 4 && payload.label_ == 0 && payload.field_C_ == 0xFEED);
    assert(lengths == 1 && mem_calls == 1 && concats == 0);
}
"""
    variants = [("actual", body, True)]
    length = "        const size_t length = strlen(pending_label_);\n"
    assert body.count(length) == 2
    hoisted = body.replace(length, "").replace(
        "    char *copy;", "    const size_t length = strlen(pending_label_);\n    char *copy;", 1)
    variants.append(("hoisted_length", hoisted, False))
    cached_owner = body.replace("if (allocator_ != 0)", "if (saved_owner != 0)", 1).replace(
        "allocator_->get(length + 1)", "saved_owner->get(length + 1)", 1).replace(
        "    char *copy;", "    Heap *const saved_owner = allocator_;\n    char *copy;", 1)
    variants.append(("cached_allocator", cached_owner, False))
    assert body.count("strcat(payload->label_, pending_label_);") == 1
    variants.append(("cached_destination", body.replace(
        "strcat(payload->label_, pending_label_);", "strcat(copy, pending_label_);", 1), False))
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        assert compiler, compiler_name + " required for allocation phase control"
        for name, candidate, success in variants:
            stem = compiler_name.replace("+", "p") + "-" + name
            source = tmp_path / (stem + ".cpp")
            source.write_text(fixture.replace("ACTUAL", candidate))
            binary = tmp_path / stem
            build = subprocess.run([compiler, "-std=c++11", "-O0", "-fsanitize=address,undefined",
                                    "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                                   capture_output=True, text=True)
            assert build.returncode == 0, build.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                                 env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0",
                                      "UBSAN_OPTIONS": "halt_on_error=1"})
            assert (run.returncode == 0) == success, stem + run.stderr

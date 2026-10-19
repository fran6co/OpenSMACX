"""Guard the direct, owner-aware allocations in the actual SpriteList body."""
from pathlib import Path
import os


def repo_root():
    configured = os.environ.get("OPENSMACX_TEST_SOURCE_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if (candidate / "src/spritebox.cpp").exists():
            return candidate
        raise AssertionError("OPENSMACX_TEST_SOURCE_ROOT lacks src/spritebox.cpp")
    here = Path(__file__).resolve().parent
    for candidate in (here, *here.parents):
        if (candidate / "src/spritebox.cpp").exists():
            return candidate
    raise AssertionError("cannot locate OpenSMACX source root")


def definition(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    index = brace
    while index < len(source):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def test_add_owns_both_typed_allocation_paths_and_original_order():
    source = (repo_root() / "src/spritebox.cpp").read_text()
    assert "allocate_sprite_node" not in source
    body = definition(source, "int SpriteList::add(int id)")

    assert body.count("Heap *const entry_allocator = allocator_;") == 1
    assert body.count("Heap *entry_owner = entry_allocator;") == 2
    assert body.count("if (entry_owner == 0) entry_owner = allocation_owner_;") == 2
    assert body.count("Heap *payload_owner = allocator_;") == 2
    assert body.count("if (payload_owner == 0) payload_owner = allocation_owner_;") == 2

    assert body.count("entry_owner->get(sizeof(StringStructEntry))") == 2
    assert body.count("mem_get(sizeof(StringStructEntry))") == 2
    assert body.count("payload_owner->get(sizeof(SpriteStruct))") == 2
    assert body.count("mem_get(sizeof(SpriteStruct))") == 2
    assert body.count("StringStructEntry *new_entry = 0;") == 2
    assert body.count("SpriteStruct *new_payload = 0;") == 2
    assert body.count("new_entry = ::new(entry_memory) StringStructEntry;") == 2
    assert body.count("new_payload = ::new(payload_memory) SpriteStruct;") == 2
    assert body.count("StringAllocationHeap = entry_owner;") == 2
    assert body.count("StringAllocationHeap = payload_owner;") == 2
    assert body.count('MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);') == 4
    assert body.count("exit(3);") == 4

    empty_branch, nonempty_branch = body.split("    } else {\n        Heap *entry_owner = entry_allocator;", 1)
    assert empty_branch.index("head_->previous = head_;") < empty_branch.index("Heap *payload_owner = allocator_;")
    assert nonempty_branch.index("head_->previous = head_->previous->next;") < nonempty_branch.index("Heap *payload_owner = allocator_;")
    assert nonempty_branch.index("current_->id = id;") < nonempty_branch.index("Heap *payload_owner = allocator_;")
    assert body.index("on_insert(head_->sprite_payload());") < body.index("current_ = head_;", body.index("on_insert(head_->sprite_payload());"))
    assert body.index("on_insert(current_->sprite_payload());") < body.index("current_position_ = entry_count_;")
    assert body.index("current_position_ = entry_count_;") < body.index("++entry_count_;")

    # Publish each allocation result only once, after successful construction
    # or CRT null, at the original partial-mutation point.
    assert body.count("head_ = new_entry;") == 1
    assert body.count("head_->previous->next = new_entry;") == 1
    assert body.count("head_->payload = new_payload;") == 1
    assert body.count("current_->payload = new_payload;") == 1
    assert body.index("head_ = new_entry;") < body.index("if (head_ == 0) return 4;")
    assert body.index("head_->previous->next = new_entry;") < body.index("if (head_->previous->next == 0) return 4;")
    assert body.index("head_->payload = new_payload;") < body.index("if (head_->payload == 0) return 4;")
    assert body.index("current_->payload = new_payload;") < body.index("if (current_->payload == 0) return 4;")
    assert "head_ = 0;" not in body
    assert "head_->previous->next = 0;" not in body
    assert "head_->payload = 0;" not in body
    assert "current_->payload = 0;" not in body


def test_compiler_generated_attach_binding_has_no_fake_source_body():
    from match_recovery.source import Mode, read_file
    root = Path(__file__).resolve().parents[2]
    record, = (r for r in read_file(root / 'src/spritebox.cpp') if r.address == 0x611920)
    assert record.mode is Mode.FILE
    assert not record.extract_error

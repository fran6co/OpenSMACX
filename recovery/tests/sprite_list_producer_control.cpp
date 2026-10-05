#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <new>

typedef unsigned int uint32_t;
enum { MB_OK = 0 };
static int fail_text_allocation;
static int fail_string_add;
static int string_add_calls;
static int string_add_id;
static int string_pending_matches;
class Heap;
static Heap *StringAllocationHeap;

class Heap {
public:
    void *get(size_t size) { return ::malloc(size); }
};

void *mem_get(size_t size) {
    if (fail_text_allocation && size > 8) return 0;
    return ::malloc(size);
}
void MessageBoxA(void *, const char *, const char *, int) {}

class StringAllocationBase {
public:
    Heap *allocation_owner_;
    StringAllocationBase() : allocation_owner_(StringAllocationHeap) {
        StringAllocationHeap = 0;
    }
    virtual ~StringAllocationBase() {}
    static void operator delete(void *memory) {
        if (StringAllocationHeap == 0) ::free(memory);
        StringAllocationHeap = 0;
    }
};

class ListPayloadBase : public virtual StringAllocationBase {
public:
    virtual ~ListPayloadBase() {}
};

struct Sprite {
    int identity;
};

struct SpriteStruct : public ListPayloadBase {
    Sprite *sprite_;
    char *label_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    static void operator delete(void *memory) {
        if (StringAllocationHeap == 0) ::free(memory);
        StringAllocationHeap = 0;
    }
};

class StringStructPayload : public ListPayloadBase {
public:
    char *text;
    uint32_t field_8_;
    uint32_t field_C_;
};

struct StringStructEntry : public virtual StringAllocationBase {
    int id;
    ListPayloadBase *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructEntry() : id(0), payload(0), next(0), previous(0) {}
    virtual ~StringStructEntry() {}
    static void operator delete(void *memory) {
        if (StringAllocationHeap == 0) ::free(memory);
        StringAllocationHeap = 0;
    }
    StringStructPayload *string_payload() {
        return static_cast<StringStructPayload *>(payload);
    }
    SpriteStruct *sprite_payload() {
        return static_cast<SpriteStruct *>(payload);
    }
};

class StringStruct : public virtual StringAllocationBase {
public:
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    Heap *allocator_;
    StringStruct() : head_(0), current_(0), entry_count_(0),
        current_position_(0), allocator_(0) {}
    virtual int on_insert(void *payload);
    virtual void on_remove(void *payload);
    virtual ~StringStruct() {}
};

class SpriteList : public StringStruct {
public:
    Sprite *pending_sprite_;
    char *pending_label_;
    uint32_t field_24_;
    SpriteList() : pending_sprite_(0), pending_label_(0), field_24_(0) {}
    virtual int on_insert(void *payload);
    virtual void on_remove(void *payload);
    int add(int id);
    virtual ~SpriteList() {}
};

class StringList : public StringStruct {
public:
    char *pending_text_;
    uint32_t field_20_;
    uint32_t field_24_;
    int add(int id) {
        ++string_add_calls;
        string_add_id = id;
        string_pending_matches = pending_text_ != 0;
        return fail_string_add;
    }
    StringList() : pending_text_(0), field_20_(0), field_24_(0) {}
};

class Dialog {
public:
    StringList item_list_;
};

class SpriteBox : public Dialog {
public:
    SpriteList list_;
    int sprite(Sprite *row_sprite, char *text, int id);
};

/* PRODUCT OWNER BODIES */

int main() {
    SpriteBox box;
    Sprite sprite = { 73 };
    char label[] = "builder";
    int result = box.sprite(&sprite, label, 41);
    assert(result == 0);
    assert(box.list_.entry_count_ == 1);
    assert(box.list_.head_ != 0);
    assert(box.list_.head_->id == 41);
    assert(box.list_.head_->payload != 0);
    SpriteStruct *payload = box.list_.head_->sprite_payload();
    assert(payload->sprite_ == &sprite);
    assert(payload->field_C_ == 0); // SpriteBox::sprite explicitly resets +0x24
    assert(payload->label_ == 0); // producer clears SpriteList pending_label_
    assert(string_add_calls == 1 && string_add_id == 41);
    assert(string_pending_matches);

    SpriteBox null_box;
    assert(null_box.sprite(0, label, 9) == 3);
    assert(null_box.list_.entry_count_ == 0);
    assert(string_add_calls == 1);

    SpriteBox partial_box;
    fail_string_add = 4;
    assert(partial_box.sprite(&sprite, label, 52) == 4);
    assert(partial_box.list_.entry_count_ == 1); // producer does not roll back
    assert(partial_box.list_.head_->id == 52);
    assert(string_add_calls == 2 && string_add_id == 52);

    fail_string_add = 0;
    return 0;
}

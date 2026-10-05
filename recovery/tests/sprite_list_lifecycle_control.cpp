#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define nullptr 0

typedef unsigned int uint32_t;
class Heap;
static Heap *StringAllocationHeap;
static char *tracked_text;
static int tracked_text_frees;
static int sprite_payload_destroys;
static int entry_destroys;

void host_free(void *memory) {
    if (memory == tracked_text) ++tracked_text_frees;
    ::free(memory);
}
#define free host_free

class Heap {
public:
    void *get(size_t size) { return ::malloc(size); }
};
void *mem_get(size_t size) { return ::malloc(size); }
void MessageBoxA(void *, const char *, const char *, int) {}
enum { MB_OK = 0 };

class StringAllocationBase {
public:
    Heap *allocation_owner_;
    StringAllocationBase() : allocation_owner_(StringAllocationHeap) {
        StringAllocationHeap = 0;
    }
    virtual ~StringAllocationBase() {}
    static void operator delete(void *memory) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
};

class ListPayloadBase : public virtual StringAllocationBase {};
class StringStructPayload : public ListPayloadBase {
public:
    char *text;
    uint32_t field_8_;
    uint32_t field_C_;
    StringStructPayload() : text(0), field_8_(0), field_C_(0) {}
};

struct Sprite {
    int width() const { return 1; }
    int height() const { return 1; }
    int transparent_index() const { return 0; }
};
struct SpriteStruct : public ListPayloadBase {
    Sprite *sprite_;
    char *label_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    SpriteStruct() : sprite_(0), label_(0), field_C_(0), field_10_(0), field_14_(0) {}
    virtual ~SpriteStruct() { ++sprite_payload_destroys; }
};

struct StringStructEntry : public virtual StringAllocationBase {
    int id;
    ListPayloadBase *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *string_payload() {
        return static_cast<StringStructPayload *>(payload);
    }
    SpriteStruct *sprite_payload() {
        return static_cast<SpriteStruct *>(payload);
    }
    StringStructEntry() : id(0), payload(0), next(0), previous(0) {}
    virtual ~StringStructEntry() { ++entry_destroys; }
    static void operator delete(void *memory) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
};

class StringStruct {
public:
    virtual int on_insert(void *payload);
    virtual void on_remove(void *payload);
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    Heap *allocator_;
    StringStruct() : head_(0), current_(0), entry_count_(0),
        current_position_(0), allocator_(0) {}
    virtual ~StringStruct();
};

class SpriteList : public StringStruct {
public:
    Sprite *pending_sprite_;
    char *pending_label_;
    uint32_t field_24_;
    SpriteList() : pending_sprite_(0), pending_label_(0), field_24_(0) {}
    virtual int on_insert(void *payload);
    virtual void on_remove(void *payload);
    void close();
    virtual ~SpriteList();
};

class Buffer {
public:
    void set_text_color(int, int, int, int) {}
    void set_text_color2(int, int, int, int) {}
    void set_text_color3(int, int, int, int) {}
    void set_font(int, int, int, int) {}
};
class GraphicWin : public Buffer {
public:
    void close() {}
};
class Dialog { public: void close() {} };
class Spot { public: void shutdown() {} };
class SpriteBox : public GraphicWin, public Dialog {
public:
    SpriteList list_;
    Spot spots_;
    uint32_t field_4_, field_8_, field_1C_, field_20_;
    uint32_t field_54_, field_58_, field_5C_, field_60_;
    uint32_t field_74_, field_78_, field_7C_, field_84_;
    void (*redraw_callback_)(Sprite *, GraphicWin *, int, int, int);
    void (*field_68_)(int);
    void (*left_down_callback_)(int);
    void (*left_up_callback_)(int);
    void (*double_click_callback_)(int);
    SpriteBox() : field_4_(0), field_8_(0), field_1C_(0), field_20_(0),
        field_54_(0), field_58_(0), field_5C_(0), field_60_(0),
        field_74_(0), field_78_(0), field_7C_(0), field_84_(0),
        redraw_callback_(0), field_68_(0), left_down_callback_(0),
        left_up_callback_(0), double_click_callback_(0) {}
    void close();
};

static uint32_t SpriteBoxDefaultField8;
static int DialogDefaults[36];
static int DialogDefaultFonts[3];

/* PRODUCT LIFECYCLE BODIES */

static StringStructEntry *make_sprite_entry(SpriteStruct *payload, int id) {
    StringStructEntry *entry = new StringStructEntry;
    entry->id = id;
    entry->payload = payload;
    entry->next = entry;
    entry->previous = entry;
    return entry;
}

int main() {
    const int before_payloads = sprite_payload_destroys;
    const int before_entries = entry_destroys;
    {
        SpriteList list;
        SpriteStruct *payload = new SpriteStruct;
        tracked_text = static_cast<char *>(::malloc(8));
        strcpy(tracked_text, "owned");
        payload->label_ = tracked_text;
        StringStructEntry *first = make_sprite_entry(payload, 7);
        StringStructEntry *second = make_sprite_entry(new SpriteStruct, 8);
        first->next = second;
        first->previous = second;
        second->next = first;
        second->previous = first;
        list.head_ = first;
        list.current_ = list.head_;
        list.entry_count_ = 2;
    }
    assert(tracked_text_frees == 0); // base-stage no-op hook, not SpriteList hook
    assert(sprite_payload_destroys == before_payloads + 2);
    assert(entry_destroys == before_entries + 2);
    host_free(tracked_text);
    tracked_text = 0;

    SpriteList ordinary;
    SpriteStruct *payload = new SpriteStruct;
    tracked_text = static_cast<char *>(::malloc(8));
    strcpy(tracked_text, "close");
    payload->label_ = tracked_text;
    ordinary.head_ = make_sprite_entry(payload, 8);
    ordinary.current_ = ordinary.head_;
    ordinary.entry_count_ = 1;
    ordinary.close();
    assert(tracked_text_frees == 2); // one leaked base-dtor label, one close label
    assert(ordinary.head_ == 0 && ordinary.entry_count_ == 0);
    tracked_text = 0;

    SpriteBox box;
    SpriteStruct *box_payload = new SpriteStruct;
    tracked_text = static_cast<char *>(::malloc(8));
    strcpy(tracked_text, "box");
    box_payload->label_ = tracked_text;
    box.list_.head_ = make_sprite_entry(box_payload, 9);
    box.list_.current_ = box.list_.head_;
    box.list_.entry_count_ = 1;
    box.close();
    assert(tracked_text_frees == 3);
    assert(box.list_.head_ == 0 && box.list_.entry_count_ == 0);
    tracked_text = 0;
    return 0;
}

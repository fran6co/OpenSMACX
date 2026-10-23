#include <stdint.h>

struct StringStructEntry;
struct StringStructPayload {
    StringStructEntry *owner;
    char *text;
    explicit StringStructPayload(StringStructEntry *entry);
    ~StringStructPayload();
};
struct StringStructEntry {
    StringStructEntry *previous;
    StringStructEntry *next;
    int id;
    void *payload;
    StringStructEntry() : previous(0), next(0), id(0), payload(0) {}
    ~StringStructEntry();
    StringStructPayload *string_payload() {
        return static_cast<StringStructPayload *>(payload);
    }
};

static int events[64];
static int event_count;
static int callback_count;
static int callback_count_delta;
static int callback_entry_count_delta;
static int callback_head_id;
static int callback_current_id;
static int callback_position;
static int callback_entry_count;
static int callback_saw_live_payload;
static int callback_saw_unlinked_neighbors;
static int payload_destroyed;
static int entry_destroyed;
static int payload_saw_owned_entry;
static StringStructEntry *watched_previous;
static StringStructEntry *watched_next;
static int measured_calls;
static int measure_mutates_width;
static int measure_mutates_height;
static int measure_result_override;
static int measurement_widths[16];
static int mutate_box_dimensions_in_callback;
static int callback_width_after;
static int callback_height_after;
static struct StringBox *callback_box;

struct StringList {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    StringList() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
    virtual ~StringList() {}
    virtual void on_remove(void *opaque_payload);
    StringStructPayload *current_entry() { return current_ ? current_->string_payload() : 0; }
    StringStructPayload *next_entry() {
        if (head_ != 0) {
            current_ = current_->next;
            ++current_position_;
            if (current_position_ == entry_count_) current_position_ = 0;
        }
        return current_entry();
    }
};

struct DibHeader { int biWidth; int biHeight; };
struct Dib { DibHeader bmiHeader; };
struct Scroll {
    int iSomeFlag_;
    int bar_thickness_;
    int range_maximum_;
    int paging_;
    int position_;
    int show_calls;
    int hide_calls;
    int set_pos_calls;
    int last_set_pos;
    Scroll() : iSomeFlag_(0), bar_thickness_(12), range_maximum_(0), paging_(0),
               position_(0), show_calls(0), hide_calls(0), set_pos_calls(0),
               last_set_pos(0) {}
    void show(int) { ++show_calls; }
    void hide() { ++hide_calls; }
    void set_pos(int value) { ++set_pos_calls; last_set_pos = value; position_ = value; }
};
struct StringBox {
    StringList stringList_;
    StringList *list_override_;
    Dib dib_;
    Scroll scroll_;
    unsigned int visible_rows_;
    int top_line_;
    int per_line_draw_;
    int visible_;
    int redraw_calls;
    StringBox() : list_override_(0), visible_rows_(0), top_line_(0),
                  per_line_draw_(0), visible_(0), redraw_calls(0) {
        dib_.bmiHeader.biWidth = 100;
        dib_.bmiHeader.biHeight = -48;
    }
    void add_fixup();
    void UNK1(int id);
    void UNK2(int first_bound, int second_bound);
    void clip_ids(unsigned long count);
    int is_visible() const { return visible_; }
    void redraw() { ++redraw_calls; }
    int text_line_height();
    int write_strings_height(StringList *list, int width, int flags);
    int wrap_height(char *text, int width);
};

static const int WinFlagVisible = 1;

StringStructPayload::StringStructPayload(StringStructEntry *entry)
    : owner(entry), text(0) {}
StringStructPayload::~StringStructPayload() {
    if (owner != 0 && owner->payload == this) ++payload_saw_owned_entry;
    delete[] text;
    events[event_count++] = 2;
    ++payload_destroyed;
}
StringStructEntry::~StringStructEntry() {
    events[event_count++] = 3;
    if (payload == 0) ++entry_destroyed;
}
void StringList::on_remove(void *opaque_payload) {
    StringStructPayload *payload = static_cast<StringStructPayload *>(opaque_payload);
    events[event_count++] = 1;
    ++callback_count;
    callback_count += callback_count_delta;
    entry_count_ += callback_entry_count_delta;
    callback_entry_count_delta = 0;
    callback_head_id = head_ ? head_->id : -999;
    callback_current_id = current_ ? current_->id : -999;
    callback_position = current_position_;
    callback_entry_count = entry_count_;
    callback_saw_live_payload = payload != 0 && payload->owner != 0 &&
                                payload->owner->payload == payload && payload->text != 0;
    callback_saw_unlinked_neighbors = payload != 0 && payload->owner != 0 &&
        payload->owner->previous == watched_previous && payload->owner->next == watched_next &&
        (watched_previous == payload->owner || watched_previous->next != payload->owner) &&
        (watched_next == payload->owner || watched_next->previous != payload->owner);
    if (mutate_box_dimensions_in_callback && callback_box != 0) {
        callback_box->dib_.bmiHeader.biWidth = callback_width_after;
        callback_box->dib_.bmiHeader.biHeight = callback_height_after;
    }
}

int StringBox::text_line_height() { return 12; }
int StringBox::write_strings_height(StringList *list, int width, int) {
    if (measured_calls < 16) measurement_widths[measured_calls] = width;
    ++measured_calls;
    if (measure_mutates_width && callback_box != 0) {
        measure_mutates_width = 0;
        callback_box->dib_.bmiHeader.biWidth = callback_width_after;
    }
    if (measure_mutates_height && callback_box != 0) {
        measure_mutates_height = 0;
        callback_box->dib_.bmiHeader.biHeight = callback_height_after;
    }
    return measure_result_override >= 0 ? measure_result_override : list->entry_count_ * 12;
}
int StringBox::wrap_height(char *, int) { return 12; }

/*SOURCE_METHODS*/

static void reset_observation() {
    event_count = callback_count = payload_destroyed = entry_destroyed = 0;
    payload_saw_owned_entry = 0;
    callback_count_delta = 0;
    callback_entry_count_delta = 0;
    mutate_box_dimensions_in_callback = 0;
    callback_width_after = callback_height_after = 0;
    callback_box = 0;
    callback_head_id = callback_current_id = callback_position = -999;
    callback_entry_count = -999;
    callback_saw_live_payload = callback_saw_unlinked_neighbors = 0;
    watched_previous = watched_next = 0;
    measured_calls = 0;
    measure_mutates_width = measure_mutates_height = 0;
    measure_result_override = -1;
}

static void make_ring(StringList &list, StringStructEntry **nodes,
                     const int *ids, int count, int cursor) {
    for (int index = 0; index < count; ++index) {
        nodes[index] = new StringStructEntry;
        nodes[index]->id = ids[index];
        nodes[index]->payload = new StringStructPayload(nodes[index]);
        nodes[index]->string_payload()->text = new char[2];
        nodes[index]->string_payload()->text[0] = 'x';
        nodes[index]->string_payload()->text[1] = 0;
    }
    for (int index = 0; index < count; ++index) {
        nodes[index]->previous = nodes[(index + count - 1) % count];
        nodes[index]->next = nodes[(index + 1) % count];
    }
    list.head_ = count ? nodes[0] : 0;
    list.current_ = count ? nodes[cursor] : 0;
    list.entry_count_ = count;
    list.current_position_ = cursor;
}

static int one_removal_checks() {
    reset_observation();
    StringBox box;
    StringStructEntry *nodes[3];
    const int ids[] = {4, 7, 9};
    make_ring(box.stringList_, nodes, ids, 3, 1);
    watched_previous = nodes[0];
    watched_next = nodes[2];
    box.UNK1(7);
    if (event_count != 3 || events[0] != 1 || events[1] != 2 || events[2] != 3) return 1;
    if (callback_count != 1 || callback_head_id != 4 || callback_current_id != 9 ||
        callback_position != 1 || callback_entry_count != 3 || !callback_saw_live_payload ||
        !callback_saw_unlinked_neighbors) return 2;
    if (box.stringList_.entry_count_ != 2 || box.stringList_.current_ != nodes[2] ||
        box.stringList_.current_position_ != 1 || box.stringList_.head_ != nodes[0]) return 3;
    if (payload_destroyed != 1 || entry_destroyed != 1 || payload_saw_owned_entry != 1) return 4;

    reset_observation();
    StringBox head_box;
    StringStructEntry *head_nodes[3];
    make_ring(head_box.stringList_, head_nodes, ids, 3, 0);
    watched_previous = head_nodes[2];
    watched_next = head_nodes[1];
    head_box.UNK1(4);
    if (callback_count != 1 || callback_head_id != 7 || callback_current_id != 7 ||
        callback_entry_count != 3 || !callback_saw_live_payload ||
        !callback_saw_unlinked_neighbors) return 14;
    return 0;
}

static int singleton_and_callback_count_checks() {
    reset_observation();
    StringBox singleton;
    StringStructEntry *one[1];
    const int one_id[] = {11};
    make_ring(singleton.stringList_, one, one_id, 1, 0);
    watched_previous = watched_next = one[0];
    singleton.UNK1(11);
    if (event_count != 3 || events[0] != 1 || events[1] != 2 || events[2] != 3 ||
        callback_count != 1 || !callback_saw_live_payload ||
        !callback_saw_unlinked_neighbors) return 5;
    if (singleton.stringList_.entry_count_ != 0 || singleton.stringList_.head_ != 0 ||
        singleton.stringList_.current_position_ != -1) return 6;

    reset_observation();
    StringBox changing;
    StringStructEntry *nodes[3];
    const int ids[] = {2, 4, 6};
    make_ring(changing.stringList_, nodes, ids, 3, 0);
    callback_count_delta = 4;
    callback_entry_count_delta = 1;
    changing.UNK2(3, 1); // unsigned interval retains ID 2 and skips final ID 6
    if (changing.stringList_.entry_count_ != 2 || callback_count != 10 ||
        payload_destroyed != 2 || entry_destroyed != 2 ||
        changing.stringList_.head_ != nodes[0] || nodes[0]->next != nodes[0] ||
        nodes[0]->previous != nodes[0]) return 7;
    return 0;
}

static int add_fixup_measurement_and_scroll_checks() {
    reset_observation();
    StringBox box;
    StringStructEntry *nodes[3];
    const int ids[] = {1, 2, 3};
    make_ring(box.stringList_, nodes, ids, 3, 2);
    box.visible_rows_ = 1;
    box.visible_ = 0;
    box.dib_.bmiHeader.biWidth = 100;
    box.dib_.bmiHeader.biHeight = -18;
    callback_box = &box;
    mutate_box_dimensions_in_callback = 1;
    callback_width_after = 77;
    callback_height_after = -6;
    box.add_fixup();
    if (box.dib_.bmiHeader.biWidth != 77 || box.dib_.bmiHeader.biHeight != -6) return 8;
    // Width is captured before the measuring callback; height is read live
    // after the callback when paging and visibility are recalculated.
    if (measured_calls != 4 || measurement_widths[0] != 100 ||
        measurement_widths[1] != 100 || measurement_widths[2] != 100 ||
        measurement_widths[3] != 100) return 9;
    if (box.stringList_.entry_count_ != 1 || box.stringList_.head_ != nodes[2] ||
        box.stringList_.current_ != nodes[2] || box.stringList_.current_position_ != 0 ||
        box.scroll_.range_maximum_ != 1 || box.scroll_.paging_ != -1 ||
        box.scroll_.show_calls != 1 || box.scroll_.hide_calls != 0) return 10;
    if (payload_destroyed != 2 || entry_destroyed != 2 || callback_count != 2 ||
        payload_saw_owned_entry != 2) return 11;
    if (event_count != 6 || events[0] != 1 || events[1] != 2 || events[2] != 3 ||
        events[3] != 1 || events[4] != 2 || events[5] != 3) return 12;

    reset_observation();
    StringBox show_box;
    StringStructEntry *show_nodes[3];
    make_ring(show_box.stringList_, show_nodes, ids, 3, 0);
    show_box.visible_rows_ = 3;
    show_box.dib_.bmiHeader.biWidth = 100;
    show_box.dib_.bmiHeader.biHeight = -30;
    show_box.add_fixup();
    if (show_box.scroll_.range_maximum_ != 3 || show_box.scroll_.paging_ != 1 ||
        show_box.scroll_.show_calls != 1 || show_box.scroll_.hide_calls != 0 ||
        measured_calls != 2 || measurement_widths[0] != 100 ||
        measurement_widths[1] != 100) return 13;
    return 0;
}

int main() {
    const int first = one_removal_checks();
    if (first) return first;
    const int second = singleton_and_callback_count_checks();
    if (second) return second;
    return add_fixup_measurement_and_scroll_checks();
}

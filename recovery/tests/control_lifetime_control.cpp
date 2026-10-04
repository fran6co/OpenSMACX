// Host behavior projection for product destructor bodies. This models event
// order and complete-object ownership; it is not target ABI/layout evidence.
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <new>

enum Event {
    LIST_CLOSE = 1, EDIT_CLOSE, CHECK_RESET, DIALOG_CLOSE, GRAPHIC_CLOSE,
    DIALOG_DTOR, GRAPHIC_DTOR, DEALLOCATE
};

static int events[64];
static int event_count;
static int dialog_dtors;
static int graphic_dtors;
static int deallocations;
static int failures;
static int track_deallocation;
static void *actual_deallocated_address;

static void record(int event) {
    if (event_count < 64) events[event_count++] = event;
    else ++failures;
}

static void reset_observations() {
    event_count = dialog_dtors = graphic_dtors = deallocations = 0;
    track_deallocation = 0;
    actual_deallocated_address = 0;
}

static void expect(bool condition, const char *message) {
    if (!condition) {
        std::cerr << message << "\n";
        ++failures;
    }
}

class GraphicWin {
public:
    virtual ~GraphicWin() { ++graphic_dtors; record(GRAPHIC_DTOR); }
    virtual void close();
};

class Dialog {
public:
    virtual ~Dialog() { ++dialog_dtors; record(DIALOG_DTOR); }
    virtual void close();
};

class ListBox : public virtual GraphicWin, public virtual Dialog {
public:
    void close() { record(LIST_CLOSE); }
    virtual ~ListBox();
};

class CheckBox : public virtual GraphicWin, public virtual Dialog {
public:
    CheckBox() : field_4_(7), field_8_(8), double_click_callback_(this),
                 checked_sprite_(this), unchecked_sprite_(this) {}
    virtual ~CheckBox();
    int field_4_;
    int field_8_;
    void *double_click_callback_;
    void *checked_sprite_;
    void *unchecked_sprite_;
};

static CheckBox *active_checkbox;
static int *check_sequence;
static int check_sequence_count;
static int default_sprite_1;
static int default_sprite_2;
void *CheckBoxDefault1 = &default_sprite_1;
void *CheckBoxDefault2 = &default_sprite_2;

void Dialog::close() {
    record(DIALOG_CLOSE);
    if (active_checkbox != 0) {
        CheckBox *box = active_checkbox;
        expect(box->field_4_ == 0 && box->field_8_ == 0 &&
               box->double_click_callback_ == 0 &&
               box->checked_sprite_ == CheckBoxDefault2 &&
               box->unchecked_sprite_ == CheckBoxDefault1,
               "CheckBox reset state must be established before base close");
        if (check_sequence_count < 4)
            check_sequence[check_sequence_count++] = 1;
    }
}

void GraphicWin::close() {
    record(GRAPHIC_CLOSE);
    if (active_checkbox != 0 && check_sequence_count < 4)
        check_sequence[check_sequence_count++] = 2;
}

class EditGroup : public virtual GraphicWin, public virtual Dialog {
public:
    void close() { record(EDIT_CLOSE); }
    virtual ~EditGroup();
};

// @@LISTBOX_DESTRUCTOR@@
// @@CHECKBOX_DESTRUCTOR@@
// @@EDITGROUP_DESTRUCTOR@@

static void check_base_order(const char *which) {
    expect(dialog_dtors == 1 && graphic_dtors == 1,
           which);
    expect(event_count >= 2 && events[event_count - 2] == DIALOG_DTOR &&
           events[event_count - 1] == GRAPHIC_DTOR,
           "virtual bases must be destroyed once, Dialog then GraphicWin");
}

void *operator new(std::size_t size) throw(std::bad_alloc) {
    void *p = std::malloc(size);
    if (p == 0) throw std::bad_alloc();
    return p;
}

void operator delete(void *p) throw() {
    if (track_deallocation) {
        ++deallocations;
        actual_deallocated_address = p;
        record(DEALLOCATE);
    }
    std::free(p);
}

static bool has_event(int event) {
    for (int i = 0; i < event_count; ++i)
        if (events[i] == event) return true;
    return false;
}

int main() {
    {
        reset_observations();
        { ListBox value; }
        expect(event_count >= 3 && events[0] == LIST_CLOSE,
               "ListBox destructor must close before virtual-base teardown");
        check_base_order("ListBox must destroy each virtual base once");
    }
    {
        reset_observations();
        { EditGroup value; }
        expect(event_count >= 3 && events[0] == EDIT_CLOSE,
               "EditGroup destructor must close before virtual-base teardown");
        check_base_order("EditGroup must destroy each virtual base once");
    }
    {
        reset_observations();
        int sequence[4] = {0, 0, 0, 0};
        check_sequence = sequence;
        check_sequence_count = 0;
        {
            CheckBox value;
            active_checkbox = &value;
        }
        active_checkbox = 0;
        expect(check_sequence_count == 2 && sequence[0] == 1 &&
               sequence[1] == 2,
               "CheckBox must reset, close Dialog then GraphicWin");
        expect(dialog_dtors == 1 && graphic_dtors == 1,
               "CheckBox stack destruction must destroy each virtual base once");
    }
    {
        reset_observations();
        int sequence[4] = {0, 0, 0, 0};
        check_sequence = sequence;
        check_sequence_count = 0;
        CheckBox *complete = new CheckBox;
        track_deallocation = 1;
        active_checkbox = complete;
        GraphicWin *base = static_cast<GraphicWin *>(complete);
        delete base;
        track_deallocation = 0;
        active_checkbox = 0;
        expect(dialog_dtors == 1 && graphic_dtors == 1 &&
               sequence[0] == 1 && sequence[1] == 2,
               "delete through GraphicWin must run CheckBox and virtual bases once");
        expect(deallocations == 1 && actual_deallocated_address == complete,
               "delete through GraphicWin must deallocate complete address once");
    }
    {
        reset_observations();
        EditGroup *complete = new EditGroup;
        track_deallocation = 1;
        Dialog *base = static_cast<Dialog *>(complete);
        delete base;
        track_deallocation = 0;
        expect(dialog_dtors == 1 && graphic_dtors == 1,
               "delete through Dialog must destroy EditGroup virtual bases once");
        expect(has_event(EDIT_CLOSE),
               "delete through Dialog must run EditGroup close");
        expect(deallocations == 1 && actual_deallocated_address == complete,
               "delete through Dialog must deallocate complete address once");
    }
    {
        reset_observations();
        ListBox *complete = new ListBox;
        track_deallocation = 1;
        GraphicWin *base = static_cast<GraphicWin *>(complete);
        delete base;
        track_deallocation = 0;
        expect(dialog_dtors == 1 && graphic_dtors == 1 &&
               deallocations == 1 && actual_deallocated_address == complete &&
               has_event(LIST_CLOSE),
               "delete through GraphicWin must complete ListBox teardown once");
    }
    {
        reset_observations();
        ListBox *complete = new ListBox;
        track_deallocation = 1;
        Dialog *base = static_cast<Dialog *>(complete);
        delete base;
        track_deallocation = 0;
        expect(dialog_dtors == 1 && graphic_dtors == 1 &&
               deallocations == 1 && actual_deallocated_address == complete &&
               has_event(LIST_CLOSE),
               "delete through Dialog must complete ListBox teardown once");
    }
    {
        reset_observations();
        int sequence[4] = {0, 0, 0, 0};
        check_sequence = sequence;
        check_sequence_count = 0;
        CheckBox *complete = new CheckBox;
        track_deallocation = 1;
        active_checkbox = complete;
        Dialog *base = static_cast<Dialog *>(complete);
        delete base;
        track_deallocation = 0;
        active_checkbox = 0;
        expect(dialog_dtors == 1 && graphic_dtors == 1 &&
               sequence[0] == 1 && sequence[1] == 2 && deallocations == 1 &&
               actual_deallocated_address == complete,
               "delete through Dialog must complete CheckBox teardown once");
    }
    {
        reset_observations();
        EditGroup *complete = new EditGroup;
        track_deallocation = 1;
        GraphicWin *base = static_cast<GraphicWin *>(complete);
        delete base;
        track_deallocation = 0;
        expect(dialog_dtors == 1 && graphic_dtors == 1 &&
               deallocations == 1 && actual_deallocated_address == complete &&
               has_event(EDIT_CLOSE),
               "delete through GraphicWin must complete EditGroup teardown once");
    }
    return failures == 0 ? 0 : 1;
}

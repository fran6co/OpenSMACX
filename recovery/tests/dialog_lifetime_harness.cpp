// Actual Dialog declaration/destructor and retained callback. Member/close
// doubles expose lifetime order; this is not a full-game exception oracle.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <new>
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
struct Font {};
class GraphicWin; // This bounded lifetime projection never invokes attachment.
#define MEASURED
int events[16], count, throw_close, throw_list;
void *deleting;
void event(int value) { assert(count < 16); events[count++] = value; }
struct Heap {
    Heap() { event(1); }
    ~Heap() { event(5); }
};
struct StringList {
    StringList() { event(2); if (throw_list) throw 12; }
    ~StringList() { event(4); }
};
// LIVE_DIALOG_DECL
Dialog::Dialog() {}
void Dialog::close() { event(3); if (throw_close) throw 13; }
// LIVE_DIALOG_DESTRUCTION
// LIVE_DIALOG_CALLBACK

struct DerivedDialog : Dialog {
    ~DerivedDialog() { event(7); }
};

void operator delete(void *memory) {
    if (memory == deleting) { event(6); deleting = 0; }
    free(memory);
}

void expect(const int *values, int size) {
    assert(count == size);
    for (int i = 0; i < size; ++i) assert(events[i] == values[i]);
}

int main() {
    const int normal[] = {1, 2, 3, 4, 5};
    const int freed[] = {1, 2, 3, 4, 5, 6};
    const int partial[] = {1, 2, 5};
    { Dialog dialog; }
    expect(normal, 5);

    count = 0;
    void *storage = malloc(sizeof(Dialog));
    assert(storage);
    Dialog *dialog = ::new(storage) Dialog;
    dialog_destructor_redirect(dialog, 0);
    expect(normal, 5);
    free(storage); // Non-deleting callback must not free the allocation.

    count = 0;
    storage = malloc(sizeof(DerivedDialog));
    assert(storage);
    DerivedDialog *derived = ::new(storage) DerivedDialog;
    dialog_destructor_redirect(derived, 0);
    expect(normal, 5); // Original entry destroys only the named Dialog base.
    free(storage);

    count = 0;
    storage = malloc(sizeof(Dialog));
    assert(storage);
    dialog = ::new(storage) Dialog;
    deleting = storage;
    delete dialog; // Actual compiler-generated scalar deleting destructor.
    expect(freed, 6);
    assert(!deleting);

    count = 0;
    throw_close = 1;
    storage = malloc(sizeof(Dialog));
    assert(storage);
    dialog = ::new(storage) Dialog;
    bool caught = false;
    try { dialog->~Dialog(); }
    catch (int value) { assert(value == 13); caught = true; }
    assert(caught);
    expect(normal, 5); // Both constructed members unwind exactly once.
    free(storage);
    throw_close = 0;

    count = 0;
    throw_list = 1;
    caught = false;
    try { Dialog incomplete; }
    catch (int value) { assert(value == 12); caught = true; }
    assert(caught);
    expect(partial, 3); // Failed list construction cleans up only the Heap.
    return 0;
}

// Host-only fixture for the actual AlphaNet::pick_service body. The Python
// runner extracts that definition verbatim from src/alphanet.cpp.
#include <assert.h>
#include <stddef.h>
#include <string.h>

struct ServiceStruct { char *name_; };
struct ServiceEntry { ServiceStruct *payload_; };

struct Heap { int marker; };
static Heap outer_heap = { 1 };
Heap *StringAllocationHeap = 0;
static int provider_system_allocations = 0;
static int provider_heap_allocations = 0;
static int provider_destructions = 0;

struct ServiceList {
  ServiceEntry *head_;
  ServiceEntry *current_;
  int entry_count_;
  int cursor_;
  Heap *allocation_owner_;
  ServiceList() : head_(0), current_(0), entry_count_(0), cursor_(-1),
                  allocation_owner_(StringAllocationHeap) {
    StringAllocationHeap = 0;
    assert(allocation_owner_ == 0);
  }
  ~ServiceList() {
    // Restoration must follow, not precede, the automatic list cleanup.
    assert(StringAllocationHeap == 0);
    ++provider_destructions;
  }
  int seek_pos(int position) {
    if (position > entry_count_ - 1) return 0;
    current_ = head_;
    if (position < -entry_count_) return 0;
    if (position < 0) position += entry_count_;
    cursor_ = position;
    current_ = head_ == 0 ? 0 : head_ + position;
    return 1;
  }
  ServiceStruct *next_entry() {
    ++cursor_;
    if (cursor_ == entry_count_) cursor_ = 0;
    current_ = head_ == 0 ? 0 : head_ + cursor_;
    return current_ == 0 ? 0 : current_->payload_;
  }
};

struct Dialogs {
  int item_count;
  int item_rows[12];
  char *item_names[12];
  Dialogs() : item_count(0) {}
  int item(char *name, int row) {
    assert(item_count < 12);
    item_rows[item_count] = row;
    item_names[item_count] = name;
    ++item_count;
    return 0;
  }
};

struct BasePop {
  Dialogs dialogs_;
  Dialogs &dialogs() { return dialogs_; }
  int start_result;
  int selection;
  int start_calls;
  int exec_calls;
  static int destroyed;
  static int destroyed_item_count;
  static int destroyed_item_rows[12];
  static char destroyed_item_names[12][32];
  BasePop() : start_result(0), selection(-1), start_calls(0), exec_calls(0) {}
  ~BasePop() {
    destroyed_item_count = dialogs_.item_count;
    for (int i = 0; i < dialogs_.item_count; ++i) {
      destroyed_item_rows[i] = dialogs_.item_rows[i];
      strncpy(destroyed_item_names[i], dialogs_.item_names[i], 31);
      destroyed_item_names[i][31] = '\0';
    }
    ++destroyed;
  }
  int start(const char *, const char *, int, int, int, int) {
    ++start_calls;
    return start_result;
  }
  int exec(int, int) { ++exec_calls; return selection; }
};
int BasePop::destroyed = 0;
int BasePop::destroyed_item_count = 0;
int BasePop::destroyed_item_rows[12] = { 0 };
char BasePop::destroyed_item_names[12][32] = { { 0 } };

static ServiceEntry fixture_entries[3];
static int fixture_count = 0;
static int fixture_poll_result = 0;
static int fixture_popup_start_result = 0;
static int fixture_selection = -1;
static int fixture_join_result = 0;
static int fixture_poll_calls = 0;
static int fixture_join_calls = 0;
static int fixture_popup_alloc_fail = 0;
static BasePop *last_popup = 0;
static ServiceStruct *last_join_payload = 0;
static char *const pbem_label = const_cast<char *>("PBEM");

char *label_get(int);
char *label_get(int) { return pbem_label; }

BasePop *PopupAllocHook() {
  if (fixture_popup_alloc_fail) return 0;
  last_popup = new BasePop;
  last_popup->start_result = fixture_popup_start_result;
  last_popup->selection = fixture_selection;
  return last_popup;
}

int poll_services(ServiceList *list) {
  ++fixture_poll_calls;
  if (list->allocation_owner_ == 0) ++provider_system_allocations;
  else ++provider_heap_allocations;
  list->head_ = fixture_count == 0 ? 0 : fixture_entries;
  list->entry_count_ = fixture_count;
  list->cursor_ = -1;
  return fixture_poll_result;
}

struct Net {
  int poll_services(ServiceList *list) { return ::poll_services(list); }
  int join_service(ServiceStruct *payload) {
    ++fixture_join_calls;
    last_join_payload = payload;
    return fixture_join_result;
  }
};

struct AlphaNet : Net {
  void *buffer_6E0_;
  AlphaNet() : buffer_6E0_(this) {}
  int pick_service(int *allow_pbem);
};

// @@PICK_SERVICE_BODY@@

static void reset_fixture() {
  if (provider_destructions != 0) assert(StringAllocationHeap == &outer_heap);
  StringAllocationHeap = &outer_heap;
  provider_system_allocations = 0;
  provider_heap_allocations = 0;
  provider_destructions = 0;
  fixture_count = 0;
  fixture_poll_result = 0;
  fixture_popup_start_result = 0;
  fixture_selection = -1;
  fixture_join_result = 0;
  fixture_poll_calls = 0;
  fixture_join_calls = 0;
  fixture_popup_alloc_fail = 0;
  last_popup = 0;
  last_join_payload = 0;
  BasePop::destroyed = 0;
  BasePop::destroyed_item_count = 0;
}

int main() {
  char service_a_name[] = "Provider A";
  char service_b_name[] = "Provider B";
  ServiceStruct service_a = { service_a_name };
  ServiceStruct service_b = { service_b_name };
  fixture_entries[0].payload_ = &service_a;
  fixture_entries[1].payload_ = &service_b;
  fixture_count = 2;
  AlphaNet net;

  reset_fixture();
  net.buffer_6E0_ = 0;
  int allow_pbem = 9;
  assert(net.pick_service(&allow_pbem) == 7);
  assert(allow_pbem == 0 && fixture_poll_calls == 0);

  reset_fixture();
  net.buffer_6E0_ = &net;
  fixture_popup_alloc_fail = 1;
  allow_pbem = 9;
  assert(net.pick_service(&allow_pbem) == 4);
  assert(allow_pbem == 0 && fixture_poll_calls == 0);

  reset_fixture();
  fixture_poll_result = 2;
  allow_pbem = 9;
  assert(net.pick_service(&allow_pbem) == 1);
  assert(allow_pbem == 0 && fixture_poll_calls == 1);
  assert(BasePop::destroyed == 1 && fixture_join_calls == 0);
  assert(provider_system_allocations == 1 && provider_heap_allocations == 0);

  reset_fixture();
  fixture_count = 2;
  fixture_selection = 1;
  fixture_join_result = 17;
  allow_pbem = 9;
  assert(net.pick_service(&allow_pbem) == 17);
  assert(allow_pbem == 0 && fixture_join_calls == 1);
  assert(last_join_payload == &service_b);
  assert(BasePop::destroyed_item_count == 3);
  assert(BasePop::destroyed_item_rows[0] == 0
         && BasePop::destroyed_item_rows[1] == 1);
  assert(strcmp(BasePop::destroyed_item_names[0], service_a_name) == 0);
  assert(strcmp(BasePop::destroyed_item_names[1], service_b_name) == 0);
  assert(BasePop::destroyed_item_rows[2] == 9999);
  assert(strcmp(BasePop::destroyed_item_names[2], "PBEM") == 0);
  assert(BasePop::destroyed == 1);

  reset_fixture();
  fixture_count = 2;
  fixture_selection = 9999;
  allow_pbem = 9;
  assert(net.pick_service(&allow_pbem) == 0);
  assert(allow_pbem == 1 && fixture_join_calls == 0);
  assert(BasePop::destroyed_item_count == 3);
  assert(BasePop::destroyed_item_rows[2] == 9999);
  assert(BasePop::destroyed == 1);

  reset_fixture();
  fixture_count = 2;
  fixture_selection = -1;
  allow_pbem = 9;
  assert(net.pick_service(&allow_pbem) == 1);
  assert(allow_pbem == 0 && fixture_join_calls == 0);
  assert(BasePop::destroyed == 1);

  reset_fixture();
  fixture_count = 2;
  fixture_selection = 9999;
  assert(net.pick_service(0) == 0);
  assert(fixture_join_calls == 0 && BasePop::destroyed == 1);

  assert(provider_destructions == 1 && StringAllocationHeap == &outer_heap);

  for (int bad_row = 0; bad_row < 2; ++bad_row) {
    reset_fixture();
    fixture_count = 2;
    fixture_selection = bad_row == 0 ? 2 : -3;
    fixture_join_result = 17;
    assert(net.pick_service(0) == 17);
    assert(last_join_payload == &service_a);
    assert(provider_destructions == 1 && StringAllocationHeap == &outer_heap);
  }
  reset_fixture();
  fixture_selection = 0;
  assert(net.pick_service(0) == 0);
  assert(fixture_join_calls == 1 && last_join_payload == 0);
  assert(provider_destructions == 1 && StringAllocationHeap == &outer_heap);

  return 0;
}

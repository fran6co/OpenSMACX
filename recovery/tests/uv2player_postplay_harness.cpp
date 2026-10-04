// Host fakes for the actual source member inserted by test_draft.py.
// Keep behavioral logic out of this file: the member body always comes from
// src/uv2player.cpp when --source is selected.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif
typedef unsigned int DWORD;
typedef char *LPSTR;
typedef int (__cdecl *BufferWriteRawCallback)(LPSTR, int, int, int);

static std::vector<std::string> events;
static void event(const char *name) { events.push_back(name); }
static int event_index(const char *name, int occurrence) {
  for (size_t i = 0; i < events.size(); ++i) {
    if (events[i] == name && occurrence-- == 0) return static_cast<int>(i);
  }
  return -1;
}

struct IDirectDrawSurface {};
struct ExtDirectDraw { IDirectDrawSurface *surface; void *clipper; };
static char help_file_storage[] = "host DatalinkHelpFile";
static char *DatalinkHelpFile = help_file_storage;

static void *allocator_owner = reinterpret_cast<void *>(0x4567);
static int list_live = 0;
static int list_dtor_count = 0;
static int list_clear_count = 0;
static int list_item_count = 0;
static int list_load_count = 0;
static int list_add_count = 0;
static int expected_list_items = 4;
static std::vector<std::string> list_mutations;

class StringAllocationBase {
 public:
  StringAllocationBase() : saved_owner_(allocator_owner) {
    allocator_owner = 0;
    event("allocator.enter");
  }
  virtual ~StringAllocationBase() {
    assert(allocator_owner == 0);
    allocator_owner = saved_owner_;
    event("allocator.leave");
  }
 private:
  void *saved_owner_;
};

class StringList {
 public:
  StringList() : saved_owner_(allocator_owner), count_(0) {
    allocator_owner = 0;
    ++list_live;
    event("list.ctor");
  }
  ~StringList() {
    assert(list_live == 1);
    // The real StringList destructor owns removal of any remaining entries.
    if (count_ != 0) {
      list_item_count += count_;
      count_ = 0;
      event("list.dtor.clear");
    }
    allocator_owner = saved_owner_;
    --list_live;
    ++list_dtor_count;
    event("list.dtor");
  }
  void remove_all() {
    ++list_clear_count;
    count_ = 0;
    event("list.clear");
  }
  int load(char *file, char *section, int flags,
           void (__cdecl *callback)(char *)) {
    assert(file == DatalinkHelpFile);
    assert(section != 0 && section[0] != '\0');
    assert(flags == 0 && callback == 0);
    ++list_load_count;
    ++count_;
    list_mutations.push_back(std::string("load:") + section);
    event("list.load");
    return 1;
  }
  int add_special(char *line) {
    assert(line != 0 && line[0] != '\0' && line[0] != '@');
    ++list_add_count;
    ++count_;
    list_mutations.push_back(std::string("add:") + line);
    event("list.add");
    return 1;
  }
  int item_count() const { return count_; }
 private:
  void *saved_owner_;
  int count_;
};

static int font_ctor_count = 0;
static int font_dtor_count = 0;
static int font_init_count = 0;
static int font_dtor_ids[3];
static char font_names[3][128];
static int font_heights[3];
static int font_styles[3];
class Font {
 public:
  Font() : id_(font_ctor_count++) { event("font.ctor"); }
  ~Font() {
    font_dtor_ids[font_dtor_count++] = id_;
    event("font.dtor");
  }
  int init(char *name, int height, int style) {
    assert(font_init_count < 3);
    strcpy(font_names[font_init_count], name);
    font_heights[font_init_count] = height;
    font_styles[font_init_count] = style;
    ++font_init_count;
    event("font.init");
    return 1;
  }
 private:
  int id_;
};

static int open_result = 0;
static int text_get_calls = 0;
static int text_item_calls = 0;
static int number_index = 0;
static int numbers[24];
static const char *names[3] = {"font-a.fot", "font-b.fot", "font-c.fot"};
static const char *help_lines[5] = {"@DATALINK", "inline one", "@UNITS", "inline two", 0};
static int help_line_index = 0;
static int text_close_count = 0;
static int parse_calls_after_failed_open = 0;

static int text_open(const char *format, const char *section) {
  assert(strcmp(format, "movlist") == 0);
  assert(section != 0 && section[0] != '\0');
  event("text.open");
  return open_result;
}
static char *text_get() {
  ++text_get_calls;
  event("text.get");
  if (open_result != 0) ++parse_calls_after_failed_open;
  if (text_get_calls >= 10) {
    const char *line = help_lines[help_line_index++];
    return const_cast<char *>(line);
  }
  return const_cast<char *>("");
}
static char *text_item() {
  ++text_item_calls;
  event("text.item");
  return const_cast<char *>(names[(text_item_calls - 1) % 3]);
}
static int text_item_number() {
  if (open_result != 0) ++parse_calls_after_failed_open;
  assert(number_index < 24);
  event("text.number");
  return numbers[number_index++];
}
static void text_close() { ++text_close_count; event("text.close"); }
static int init_count = 0;
static int buffer_font_count = 0;
static int color_calls = 0;
static int color_args[4][4];
static int height_call_count = 0;
static int height_width_arg = 0;
static int height_flags_arg = 0;
static int measured_height = 17;
static int write_count = 0;
static int write_args[4];
static int callback_sets = 0;
static BufferWriteRawCallback current_callback = 0;
static int draw_calls = 0;
static int draw_text_calls = 0;
static int expected_surface_marker;
static int expected_clipper_marker;

class Buffer {
 public:
  int init(int width, int height, int flags, ExtDirectDraw *draw) {
    assert(width == 640 && height == 480 && flags == 4);
    assert(draw != 0);
    assert(draw->surface == reinterpret_cast<IDirectDrawSurface *>(
                                &expected_surface_marker));
    assert(draw->clipper == &expected_clipper_marker);
    ++init_count;
    event("buffer.init");
    return 1;
  }
  int set_font(Font *one, Font *two, Font *three, Font *four) {
    assert(one != 0 && two != 0 && three != 0 && four == one);
    ++buffer_font_count;
    event("buffer.fonts");
    return 1;
  }
  void set_text_color(int a, int b, int c, int d) { colors(0, a,b,c,d); }
  void set_text_color2(int a, int b, int c, int d) { colors(1, a,b,c,d); }
  void set_text_color3(int a, int b, int c, int d) { colors(2, a,b,c,d); }
  void set_text_color_hyper(int a, int b, int c, int d) { colors(3, a,b,c,d); }
  int write_strings_height(StringList *list, int width, int flags) {
    assert(list != 0 && list->item_count() == expected_list_items);
    ++height_call_count;
    height_width_arg = width;
    height_flags_arg = flags;
    event("buffer.measure");
    return measured_height;
  }
  int write_strings(StringList *list, int x, int y, int height, int flags) {
    assert(list != 0 && list->item_count() == expected_list_items);
    ++write_count;
    write_args[0] = x; write_args[1] = y;
    write_args[2] = height; write_args[3] = flags;
    ++draw_calls;
    ++draw_text_calls;
    event("buffer.write");
    return 1;
  }
  void set_write_raw_callback(BufferWriteRawCallback callback) {
    current_callback = callback;
    ++callback_sets;
    event(callback ? "callback.set" : "callback.clear");
  }
 private:
  void colors(int index, int a, int b, int c, int d) {
    color_args[index][0] = a; color_args[index][1] = b;
    color_args[index][2] = c; color_args[index][3] = d;
    ++color_calls;
    event("buffer.color");
  }
};

class UV2Player {
 public:
  UV2Player() : write_delay_(0), primary_surface_(&expected_surface_marker),
                clipper_(&expected_clipper_marker) {}
  void show_post_movie_text();
  char movie_name_[0x104];
  DWORD write_delay_;
  void *primary_surface_;
  void *clipper_;
  Buffer buffer_;
};

static DWORD timer_values[16];
static int timer_value_count = 0;
static int timer_value_index = 0;
static unsigned int timeGetTime() {
  assert(timer_value_count > 0);
  unsigned int value;
  if (timer_value_index < timer_value_count) value = timer_values[timer_value_index++];
  else value = timer_values[timer_value_count - 1];
  event("timer");
  return value;
}
static int __cdecl uv2_player_writer_redirect(LPSTR, int, int, int) { return 0; }

// test_draft.py inserts the brace-balanced member extracted from src/uv2player.cpp.
// ACTUAL_MEMBER_GOES_HERE

static void reset_case() {
  assert(list_live == 0);
  events.clear();
  list_mutations.clear();
  list_dtor_count = list_clear_count = list_item_count = 0;
  list_load_count = list_add_count = 0;
  font_ctor_count = font_dtor_count = font_init_count = 0;
  for (int i = 0; i < 3; ++i) font_dtor_ids[i] = -1;
  text_get_calls = text_item_calls = number_index = help_line_index = 0;
  text_close_count = parse_calls_after_failed_open = 0;
  init_count = buffer_font_count = color_calls = 0;
  height_call_count = write_count = callback_sets = 0;
  current_callback = 0;
  draw_calls = draw_text_calls = 0;
  timer_value_count = timer_value_index = 0;
  measured_height = 17;
  expected_list_items = 4;
  for (int i = 0; i < 24; ++i) numbers[i] = 0;
}

static void prepare_success_values() {
  // Per font: line pitch, style, ascent, descent, secondary and tertiary.
  int values[24] = {
    20, 1, 11, 12, 13, 14,
    24, 2, 21, 22, 23, 24,
    28, 3, 31, 32, 33, 34,
    100, 200, 40, 60,  // bounds; endpoints are formed from these pairs
    9, 5               // write delay and unsigned timeout
  };
  for (int i = 0; i < 24; ++i) numbers[i] = values[i];
}

static void test_failed_open() {
  reset_case();
  open_result = 1;
  timer_values[0] = 0; timer_value_count = 1;
  UV2Player player;
  strcpy(player.movie_name_, "INTRO");
  player.show_post_movie_text();
  assert(text_get_calls == 0 && text_item_calls == 0 && number_index == 0);
  assert(parse_calls_after_failed_open == 0);
  assert(text_close_count == 0 && init_count == 0 && height_call_count == 0);
  assert(write_count == 0 && draw_calls == 0 && callback_sets == 0);
  assert(font_ctor_count == 3 && font_dtor_count == 3);
  assert(font_dtor_ids[0] == 2 && font_dtor_ids[1] == 1 && font_dtor_ids[2] == 0);
  assert(list_dtor_count == 1 && list_live == 0);
  assert(allocator_owner == reinterpret_cast<void *>(0x4567));
  assert(event_index("list.dtor", 0) < event_index("allocator.leave", 0));
  assert(event_index("allocator.leave", 0) < event_index("font.dtor", 0));
}

static void test_success() {
  reset_case();
  open_result = 0;
  prepare_success_values();
  timer_values[0] = 0xfffffffcu;
  timer_values[1] = 0xfffffffeu;
  timer_values[2] = 1u;
  timer_value_count = 3;
  UV2Player player;
  strcpy(player.movie_name_, "INTRO");
  player.show_post_movie_text();

  assert(text_get_calls == 14 && text_item_calls == 3 && number_index == 24);
  assert(text_close_count == 1);
  assert(font_ctor_count == 3 && font_dtor_count == 3 && font_init_count == 3);
  assert(strcmp(font_names[0], "font-a.fot") == 0);
  assert(strcmp(font_names[1], "font-b.fot") == 0);
  assert(strcmp(font_names[2], "font-c.fot") == 0);
  assert(font_heights[0] == 1 && font_styles[0] == 20);
  assert(font_heights[1] == 2 && font_styles[1] == 24);
  assert(font_heights[2] == 3 && font_styles[2] == 28);
  assert(event_index("font.ctor", 2) < event_index("text.open", 0));
  assert(event_index("text.open", 0) < event_index("font.init", 0));

  assert(list_load_count == 2 && list_add_count == 2);
  assert(list_mutations.size() == 4);
  assert(list_mutations[0] == "load:DATALINK");
  assert(list_mutations[1] == "add:inline one");
  assert(list_mutations[2] == "load:UNITS");
  assert(list_mutations[3] == "add:inline two");
  assert(text_close_count == 1);
  assert(init_count == 1 && buffer_font_count == 1 && color_calls == 4);
  assert(event_index("list.clear", 0) < event_index("list.load", 0));
  assert(event_index("text.close", 0) < event_index("buffer.init", 0));
  const int expected_colors[4][4] = {
    {21, 22, 13, 14}, {31, 32, 23, 24},
    {41, 42, 33, 34}, {21, 22, 13, 14}
  };
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      assert(color_args[row][col] == expected_colors[row][col]);
    }
  }
  assert(height_call_count == 1 && height_width_arg == 40 && height_flags_arg == 0);
  // Native bounds are origin plus extent. The text block is vertically
  // centered in [top, bottom), and the original width is passed to drawing.
  assert(write_count == 1 && write_args[0] == 100 && write_args[1] == 221);
  assert(write_args[2] == 40 && write_args[3] == 0);
  assert(callback_sets == 2 && current_callback == 0);
  assert(event_index("callback.set", 0) < event_index("buffer.write", 0));
  assert(event_index("buffer.write", 0) < event_index("callback.clear", 0));
  assert(event_index("buffer.write", 0) < event_index("timer", 0));
  assert(event_index("timer", 0) < event_index("list.dtor", 0));
  assert(timer_value_index == 3);
  assert(list_dtor_count == 1 && list_live == 0);
  assert(list_item_count == 4);
  assert(allocator_owner == reinterpret_cast<void *>(0x4567));
  assert(event_index("list.dtor", 0) < event_index("allocator.leave", 0));
  assert(event_index("allocator.leave", 0) < event_index("font.dtor", 0));
}

static void test_zero_timeout_and_empty_list() {
  reset_case();
  open_result = 0;
  prepare_success_values();
  numbers[23] = 0;
  help_lines[0] = 0;
  expected_list_items = 0;
  timer_values[0] = 100u;
  timer_values[1] = 100u;
  timer_value_count = 2;
  UV2Player player;
  strcpy(player.movie_name_, "EMPTY");
  player.show_post_movie_text();
  assert(list_load_count == 0 && list_add_count == 0);
  assert(list_clear_count == 1);
  assert(height_call_count == 1 && write_count == 1);
  assert(timer_value_index == 2);
  assert(list_live == 0 && allocator_owner == reinterpret_cast<void *>(0x4567));
  help_lines[0] = "@DATALINK";
}

int main() {
  test_failed_open();
  test_success();
  test_zero_timeout_and_empty_list();
  reset_case();
  open_result = 0;
  prepare_success_values();
  numbers[21] = 10;
  numbers[23] = 0;
  timer_values[0] = timer_values[1] = 0;
  timer_value_count = 2;
  UV2Player player;
  strcpy(player.movie_name_, "NEGATIVE");
  player.show_post_movie_text();
  assert(write_args[1] == 197); // (10 - 17)/2 truncates toward zero.

  puts("UV2 postplay host controls passed");
  return 0;
}

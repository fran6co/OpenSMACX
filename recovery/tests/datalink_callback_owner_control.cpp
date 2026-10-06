// Host behavior control from the actual callback-row declarations and methods.
#include <cassert>
#include <cstddef>
#define __cdecl
// @@DECLARATIONS@@

int help_calls = 0;
int help_value = 0;
int text_calls = 0;
void *text_output = 0;
int text_value = 0;
int text_extra = 0;
char output_storage;

void record_help(int value) {
  ++help_calls;
  help_value = value;
}

void record_text(void *output, int value, int extra) {
  ++text_calls;
  text_output = output;
  text_value = value;
  text_extra = extra;
}

// @@BODIES@@

int main() {
  assert(sizeof(DatalinkEntryCallbacks) == 2 * sizeof(DatalinkHelpCallback));
  assert(sizeof(DatalinkHelpCallback) == sizeof(DatalinkTextCallback));
  assert(offsetof(DatalinkEntryCallbacks, help) == 0);
  assert(offsetof(DatalinkEntryCallbacks, append_text) == sizeof(DatalinkHelpCallback));

  DatalinkEntryCallbacks callbacks = {0, 0};
  callbacks.invoke_help(19);
  callbacks.invoke_text(&output_storage, 23, 29);
  assert(help_calls == 0 && text_calls == 0);

  callbacks.set_callbacks(record_help, record_text);
  callbacks.invoke_help(-17);
  callbacks.invoke_text(&output_storage, 41, 53);
  assert(help_calls == 1 && help_value == -17);
  assert(text_calls == 1 && text_output == &output_storage);
  assert(text_value == 41 && text_extra == 53);

  // Each field is updated independently; null resets must clear a
  // previously installed callback rather than preserve stale state.
  callbacks.set_callbacks(0, record_text);
  callbacks.invoke_help(61);
  callbacks.invoke_text(&output_storage, 67, 71);
  assert(help_calls == 1 && help_value == -17);
  assert(text_calls == 2 && text_value == 67 && text_extra == 71);
  callbacks.set_callbacks(record_help, 0);
  callbacks.invoke_help(73);
  callbacks.invoke_text(&output_storage, 79, 83);
  assert(help_calls == 2 && help_value == 73);
  assert(text_calls == 2);
  callbacks.set_callbacks(0, 0);
  assert(callbacks.help == 0 && callbacks.append_text == 0);
  callbacks.invoke_help(89);
  callbacks.invoke_text(&output_storage, 97, 101);
  assert(help_calls == 2 && text_calls == 2);
}

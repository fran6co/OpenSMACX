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

  callbacks.help = record_help;
  callbacks.append_text = record_text;
  callbacks.invoke_help(-17);
  callbacks.invoke_text(&output_storage, 41, 53);
  assert(help_calls == 1 && help_value == -17);
  assert(text_calls == 1 && text_output == &output_storage);
  assert(text_value == 41 && text_extra == 53);
}

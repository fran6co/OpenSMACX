// Recovery leaf tests: dialog_and_strings.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus one LEAF_CASE line at the bottom, and
// bumping this family's count in tests/leaf/leaf_case_manifest.h.
#include "leaf_shared.h"

#define LEAF_FAMILY "dialog_and_strings"

namespace {

void test_log_lifecycle_and_wrappers() {
    const int failures_before = failures;
    int stage_failures = failures;
    static_assert(sizeof(Log) == 8, "Log fixture requires the legacy layout");
    alignas(Log) uint8_t storage[sizeof(Log) + 32];
    uint8_t expected[sizeof(storage)];
    LPSTR null_file = nullptr;
    const BOOL zero_state = 0;

    seed_storage(storage, expected, sizeof(storage));
    write_at(expected, 16, null_file);
    write_at(expected, 16 + 4, zero_state);
    auto *log = new (storage + 16) Log;
    expect(log == reinterpret_cast<Log *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Log default constructor", storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log default-constructor expectations failed\n");
    }
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    const BOOL preserved_state = static_cast<BOOL>(0x13572468U);
    write_at_volatile(storage, 16 + 4, preserved_state);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16, null_file);
    env_open_calls = 0;
    log = new (storage + 16) Log(nullptr);
    expect(log == reinterpret_cast<Log *>(storage + 16));
    expect(env_open_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Log null constructor", storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log null-constructor expectations failed\n");
    }
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    write_at_volatile(storage, 16 + 4, preserved_state);
    char file_name[] = "fixture.log";
    env_open_calls = 0;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    env_open_source = nullptr;
    env_open_mode = nullptr;
    log = new (storage + 16) Log(file_name);
    LPSTR allocated_file = nullptr;
    std::memcpy(&allocated_file, storage + 16, sizeof(allocated_file));
    expect(allocated_file != nullptr);
    if (allocated_file) {
        expect(std::strcmp(allocated_file, file_name) == 0);
    }
    expect(env_open_calls == 1);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    expect(env_open_source && std::strcmp(env_open_source, file_name) == 0);
    expect(env_open_mode && std::strcmp(env_open_mode, "wt") == 0);
    expect(std::memcmp(storage + 16 + 4, &preserved_state, sizeof(preserved_state)) == 0);
    if (failures != stage_failures) {
        BOOL actual_state = 0;
        std::memcpy(&actual_state, storage + 16 + 4, sizeof(actual_state));
        std::fprintf(stderr,
            "Log filename-constructor expectations failed: file=%p calls=%d source=%s mode=%s state=0x%x\n",
            allocated_file, env_open_calls, env_open_source ? env_open_source : "(null)",
            env_open_mode ? env_open_mode : "(null)", static_cast<unsigned int>(actual_state));
    }
    stage_failures = failures;

    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16, null_file);
    tracked_free_pointer = allocated_file;
    tracked_free_calls = 0;
    log->~Log();
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Log destructor", storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log destructor expectations failed\n");
    }
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    std::memcpy(expected, storage, sizeof(storage));
    mem_get_calls = 0;
    env_open_calls = 0;
    expect(log->init(nullptr) == 16);
    expect(mem_get_calls == 0);
    expect(env_open_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    char *old_file = static_cast<char *>(std::malloc(16));
    std::strcpy(old_file, "old.log");
    write_at(storage, 16, old_file);
    write_at(storage, 16 + 4, preserved_state);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16, null_file);
    mem_get_scripted = true;
    mem_get_calls = 0;
    mem_get_results[0] = nullptr;
    tracked_free_pointer = old_file;
    tracked_free_calls = 0;
    expect(log->init("new.log") == 4);
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 8);
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    write_at(storage, 16 + 4, preserved_state);
    char *failed_open_file = static_cast<char *>(std::malloc(9));
    mem_get_calls = 0;
    mem_get_results[0] = failed_open_file;
    env_open_calls = 0;
    env_open_fails = true;
    expect(log->init("fail.log") == 6);
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 9);
    expect(std::strcmp(failed_open_file, "fail.log") == 0);
    expect(env_open_calls == 1);
    expect(env_open_mode && std::strcmp(env_open_mode, "wt") == 0);
    std::memcpy(expected, storage, sizeof(storage));
    tracked_free_pointer = failed_open_file;
    tracked_free_calls = 0;
    log->~Log();
    expect_tracked_free_calls(1);
    write_at(expected, 16, null_file);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    write_at(storage, 16 + 4, preserved_state);
    char *successful_file = static_cast<char *>(std::malloc(12));
    mem_get_calls = 0;
    mem_get_results[0] = successful_file;
    env_open_calls = 0;
    env_open_fails = false;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    expect(log->init("success.log") == 0);
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 12);
    expect(std::strcmp(successful_file, "success.log") == 0);
    expect(env_open_calls == 1);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    std::memcpy(expected, storage, sizeof(storage));
    tracked_free_pointer = successful_file;
    tracked_free_calls = 0;
    log->~Log();
    expect_tracked_free_calls(1);
    write_at(expected, 16, null_file);
    expect_storage_bytes(storage, expected, sizeof(storage));
    mem_get_scripted = false;
    mem_get_results[0] = nullptr;
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    LPSTR file_pointer = file_name;
    write_at(storage, 16, file_pointer);
    std::memcpy(expected, storage, sizeof(storage));
    Logging = reinterpret_cast<Log *>(storage + 16);
    env_open_calls = 0;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    env_open_source = nullptr;
    env_open_mode = nullptr;
    log_reset();
    expect(env_open_calls == 1);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    expect(env_open_source == file_name);
    expect(env_open_mode && std::strcmp(env_open_mode, "wt") == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log reset expectations failed\n");
    }
    stage_failures = failures;

    log_set_state(FALSE);
    const BOOL disabled = TRUE;
    write_at(expected, 16 + 4, disabled);
    expect_storage_bytes(storage, expected, sizeof(storage));
    log_set_state(INT_MIN);
    write_at(expected, 16 + 4, zero_state);
    expect_storage_bytes(storage, expected, sizeof(storage));

    static BOOL logging_disabled = FALSE;
    IsLoggingDisabled = &logging_disabled;
    capture_closed_file = true;
    struct LogOutputFixture {
        bool hex;
        LPCSTR second;
        LPCSTR expected_output;
    };
    const LogOutputFixture output_fixtures[] = {
        {false, "second", "first second 1 -2 3\n"},
        {false, nullptr, "first 1 -2 3\n"},
        {true, "second", "first second 0001 abcd ffffffff\n"},
        {true, nullptr, "first 0001 abcd ffffffff\n"},
    };
    const auto invoke_log_output = [](const LogOutputFixture &fixture) {
        if (fixture.hex) {
            if (fixture.second) {
                log_say_hex("first", fixture.second, 1, 0xABCD, -1);
            } else {
                log_say_hex("first", 1, 0xABCD, -1);
            }
        } else if (fixture.second) {
            log_say("first", fixture.second, 1, -2, 3);
        } else {
            log_say("first", 1, -2, 3);
        }
    };
    for (const LogOutputFixture &fixture : output_fixtures) {
        env_open_calls = 0;
#if defined(__MINGW32__)
        env_close_calls = 0;
#endif
        env_open_source = nullptr;
        env_open_mode = nullptr;
        closed_file_output[0] = 0;
        invoke_log_output(fixture);
        expect(env_open_calls == 1);
        expect(env_open_source == file_name);
        expect(env_open_mode && std::strcmp(env_open_mode, "at") == 0);
#if defined(__MINGW32__)
        expect(env_close_calls == 1);
        expect(std::strcmp(closed_file_output, fixture.expected_output) == 0);
#endif
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    env_open_fails = true;
    for (const LogOutputFixture &fixture : output_fixtures) {
        env_open_calls = 0;
#if defined(__MINGW32__)
        env_close_calls = 0;
#endif
        env_open_source = nullptr;
        env_open_mode = nullptr;
        invoke_log_output(fixture);
        expect(env_open_calls == 1);
        expect(env_open_source == file_name);
        expect(env_open_mode && std::strcmp(env_open_mode, "at") == 0);
#if defined(__MINGW32__)
        expect(env_close_calls == 0);
#endif
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    env_open_fails = false;

    const BOOL disabled_state = TRUE;
    for (const LogOutputFixture &fixture : output_fixtures) {
        for (int gate = 0; gate < 3; ++gate) {
            write_at(storage, 16, file_pointer);
            write_at(storage, 16 + 4, zero_state);
            logging_disabled = FALSE;
            if (gate == 0) {
                write_at(storage, 16 + 4, disabled_state);
            } else if (gate == 1) {
                logging_disabled = TRUE;
            } else {
                write_at(storage, 16, null_file);
            }
            uint8_t gate_expected[sizeof(storage)];
            std::memcpy(gate_expected, storage, sizeof(storage));
            env_open_calls = 0;
            invoke_log_output(fixture);
            expect(env_open_calls == 0);
            expect_storage_bytes(storage, gate_expected, sizeof(storage));
        }
    }
    logging_disabled = FALSE;
    write_at(storage, 16, file_pointer);
    write_at(storage, 16 + 4, zero_state);
    capture_closed_file = false;
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log state expectations failed\n");
    }
    stage_failures = failures;
    write_at(storage, 16, null_file);
    log->~Log();

    alignas(Log) static uint8_t exit_storage[sizeof(Log)]{};
    Logging = reinterpret_cast<Log *>(exit_storage);
    write_at_volatile(exit_storage, 4, preserved_state);
    env_open_calls = 0;
    atexit_calls = 0;
    atexit_callback = nullptr;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    log_logging();
    LPSTR global_file = nullptr;
    std::memcpy(&global_file, exit_storage, sizeof(global_file));
    expect(global_file && std::strcmp(global_file, "logfile.txt") == 0);
    expect(std::memcmp(exit_storage + 4, &preserved_state, sizeof(preserved_state)) == 0);
    expect(env_open_calls == 1);
    expect(atexit_calls == 1);
    expect(atexit_callback == log_logging_exit);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    log_logging_exit();
    std::memcpy(&global_file, exit_storage, sizeof(global_file));
    expect(global_file == nullptr);
    expect(std::memcmp(exit_storage + 4, &preserved_state, sizeof(preserved_state)) == 0);
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log global-lifecycle expectations failed\n");
    }
    new (exit_storage) Log;
    if (failures != failures_before) {
        std::fprintf(stderr, "Log lifecycle fixture failed\n");
    }
}

void seed_dialog(uint8_t *storage, uint8_t *expected) {
    seed_storage(storage, expected, sizeof(Dialog) + 32);
}

void test_dialog_id_to_pos() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;

    seed_dialog(storage, expected);
    DialogEntry *null_entry = nullptr;
    const int stale_position = -7;
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xD0, stale_position);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->id_to_pos(123) == stale_position);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    auto *invalid_entry = reinterpret_cast<DialogEntry *>(1);
    const int negative_count = INT_MIN;
    write_at(storage, 16 + 0xC4, invalid_entry);
    write_at(storage, 16 + 0xCC, negative_count);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    const int zero = 0;
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog_id_to_pos_redirect(dialog, nullptr, INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, invalid_entry);
    write_at(storage, 16 + 0xCC, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->id_to_pos(INT_MIN) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 30;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    seed_dialog(storage, expected);
    DialogEntry *head = &entries[0].entry;
    const int three = 3;
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    DialogEntry *second = &entries[1].entry;
    const int one = 1;
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->id_to_pos(20) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[2].entry.id = 20;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->id_to_pos(20) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[1].entry.id = 21;
    entries[2].entry.id = 30;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    DialogEntry *third = &entries[2].entry;
    const int two = 2;
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->id_to_pos(30) == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, three);
    expect(dialog_id_to_pos_redirect(dialog, nullptr, -1) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->id_to_pos(INT_MIN) == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[1].entry.next = invalid_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->id_to_pos(-1) == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[0].entry.id = INT_MAX;
    entries[0].entry.next = invalid_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->id_to_pos(INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    // A single-entry MISS is the only shape that observes the `count > 0`
    // guard: the match-at-position-zero case above leaves entry_position_ and
    // current_entry_ exactly as the pre-loop stores set them, so skipping the
    // loop entirely (guard mutated to `count > 1`) produces identical state.
    // A miss walks once - position 1, cursor advanced to the entry's next -
    // which the skipped loop cannot reproduce.
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->id_to_pos(77) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_set_selected_id() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;
    DialogEntry *null_entry = nullptr;

    seed_dialog(storage, expected);
    const int stale_position = -17;
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xD0, stale_position);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xEC, stale_position);
    dialog->set_selected_id(123);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    auto *invalid_entry = reinterpret_cast<DialogEntry *>(1);
    const int negative_count = INT_MIN;
    const int zero = 0;
    write_at(storage, 16 + 0xC4, invalid_entry);
    write_at(storage, 16 + 0xCC, negative_count);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, zero);
    write_at(expected, 16 + 0xEC, zero);
    dialog_set_selected_id_redirect(dialog, nullptr, INT_MAX);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 20;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    seed_dialog(storage, expected);
    DialogEntry *head = &entries[0].entry;
    DialogEntry *second = &entries[1].entry;
    const int three = 3;
    const int one = 1;
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    write_at(expected, 16 + 0xEC, one);
    dialog->set_selected_id(20);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[0].entry.id = 10;
    entries[1].entry.id = 11;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = invalid_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    const int two = 2;
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, two);
    write_at(expected, 16 + 0xEC, two);
    dialog->set_selected_id(-1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_get_selected_id() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;

    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 30;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    entries[0].entry.previous = &entries[2].entry;
    entries[1].entry.previous = &entries[0].entry;
    entries[2].entry.previous = &entries[1].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    DialogEntry *head = &entries[0].entry;
    DialogEntry *second = &entries[1].entry;
    DialogEntry *third = &entries[2].entry;
    const int zero = 0;
    const int one = 1;
    const int two = 2;
    const int three = 3;

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, third);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->get_selected_id() == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -1);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog_get_selected_id_redirect(dialog, nullptr) == 30);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -3);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // position == 0 is the only value that distinguishes `position < 0` from
    // `position <= 0` (or `< 1`): both branches leave the cursor at the head
    // and return its ID, but the negative branch normalizes the position by
    // adding the count, so entry_position_ ends at 3 instead of 0. Only the
    // full-object byte compare sees the difference.
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, three);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->get_selected_id() == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -4);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    DialogEntry *null_entry = nullptr;
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xCC, zero);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, zero);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->get_selected_id() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xCC, zero);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -1);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, null_entry);
    expect(dialog->get_selected_id() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, INT_MIN);
    std::memcpy(expected, storage, sizeof(storage));
    const int normalized_min = INT_MIN + 3;
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, normalized_min);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, INT_MIN);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, INT_MIN);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_pos_to_id() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;
    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3] = {};
    uint8_t entries_expected[sizeof(entries)];
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 30;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    entries[0].entry.previous = &entries[2].entry;
    entries[1].entry.previous = &entries[0].entry;
    entries[2].entry.previous = &entries[1].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    DialogEntry *head = &entries[0].entry;
    DialogEntry *second = &entries[1].entry;
    DialogEntry *third = &entries[2].entry;
    const int three = 3;
    const int selected_sentinel = 0x12345678;

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int one = 1;
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->pos_to_id(1) == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int zero = 0;
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->pos_to_id(0) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int two = 2;
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->pos_to_id(2) == 30);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog_pos_to_id_redirect(dialog, nullptr, -1) == 30);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->pos_to_id(3) == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    expect(dialog->pos_to_id(-4) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    DialogEntry *null_entry = nullptr;
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, null_entry);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->pos_to_id(0) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int normalized_min = INT_MIN + 3;
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, normalized_min);
    expect(dialog->pos_to_id(INT_MIN) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, INT_MIN);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->pos_to_id(INT_MIN) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_font_and_color_setters() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;

    // The twelve color slots interleave by tier - primary at 0x7C/0x88/0x94/
    // 0xA0, secondary at 0x80/0x8C/0x98/0xA4, tertiary at 0x84/0x90/0x9C/
    // 0xA8 - so each setter must stride by 0xC, not fill a contiguous block.
    // Distinct sentinels per argument catch both dropped and misrouted
    // stores; the full-object compare catches writes to a sibling tier.
    struct ColorCase {
        void (Dialog::*member)(int, int, int, int);
        void (__fastcall *redirect)(Dialog *, void *, int, int, int, int);
        size_t offsets[4];
    };
    const ColorCase color_cases[] = {
        {&Dialog::set_dialog_text_color, dialog_set_text_color_redirect,
         {0x7C, 0x88, 0x94, 0xA0}},
        {&Dialog::set_dialog_text_color2, dialog_set_text_color2_redirect,
         {0x80, 0x8C, 0x98, 0xA4}},
        {&Dialog::set_dialog_text_color3, dialog_set_text_color3_redirect,
         {0x84, 0x90, 0x9C, 0xA8}},
    };
    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};
    for (const ColorCase &test : color_cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_dialog(storage, expected);
            std::memcpy(expected, storage, sizeof(storage));
            for (int slot = 0; slot < 4; ++slot) {
                write_at(expected, 16 + test.offsets[slot], colors[slot]);
            }
            if (adapter) {
                test.redirect(dialog, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (dialog->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // set_dialog_font reads only Font::is_initialized(), an inline check of
    // font_obj_ against null. Font's data is private with no offset pins, so
    // the fixtures are raw storage with font_obj_ written at 0x08 per the
    // declared layout (int unk_1_, BOOL is_fot_set_, HFONT font_obj_) - no
    // constructed Font is required or wanted here.
    alignas(Font) uint8_t initialized_font[sizeof(Font)];
    alignas(Font) uint8_t uninitialized_font[sizeof(Font)];
    std::memset(initialized_font, 0xA5, sizeof(initialized_font));
    std::memset(uninitialized_font, 0xA5, sizeof(uninitialized_font));
    const HFONT font_object = reinterpret_cast<HFONT>(0x1234U);
    const HFONT null_font_object = nullptr;
    write_at(initialized_font, 0x08, font_object);
    write_at(uninitialized_font, 0x08, null_font_object);
    auto *primary = reinterpret_cast<Font *>(initialized_font);
    auto *unready = reinterpret_cast<Font *>(uninitialized_font);
    auto *secondary = reinterpret_cast<Font *>(0x11111111U);
    auto *tertiary = reinterpret_cast<Font *>(0x22222222U);

    // Null primary: error 3, nothing written at all.
    seed_dialog(storage, expected);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->set_dialog_font(nullptr, secondary, tertiary) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Initialized primary: all three stored, success.
    seed_dialog(storage, expected);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x70, primary);
    write_at(expected, 16 + 0x74, secondary);
    write_at(expected, 16 + 0x78, tertiary);
    expect(dialog->set_dialog_font(primary, secondary, tertiary) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Uninitialized primary: font1_ is left alone, but the secondary and
    // tertiary slots are still stored and the call still succeeds.
    seed_dialog(storage, expected);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x74, secondary);
    write_at(expected, 16 + 0x78, tertiary);
    expect(dialog_set_font_redirect(
               dialog, nullptr, unready, secondary, tertiary) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void test_string_struct_current_accessors() {
    alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
    uint8_t expected[sizeof(storage)];
    auto *strings = new (storage + 16) StringStruct;
    StringStructEntry *null_entry = nullptr;
    auto *invalid_entry = reinterpret_cast<StringStructEntry *>(1);

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, null_entry);
    write_at(storage, 16 + 0x0C, invalid_entry);
    std::memcpy(expected, storage, sizeof(storage));
    expect(strings->current_id() == 0);
    expect(string_struct_current_entry_redirect(strings, nullptr) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        StringStructEntry entry;
        uint8_t suffix[16];
    } guarded;
    uint8_t entry_expected[sizeof(guarded)];
    seed_storage(reinterpret_cast<uint8_t *>(&guarded), entry_expected, sizeof(guarded));
    guarded.entry.id = INT_MIN;
    guarded.entry.payload = INT_MAX;
    std::memcpy(entry_expected, &guarded, sizeof(guarded));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, invalid_entry);
    StringStructEntry *current = &guarded.entry;
    write_at(storage, 16 + 0x0C, current);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    expect(string_struct_current_id_redirect(strings, nullptr) == INT_MIN);
    expect(strings->current_entry() == INT_MAX);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(&guarded), entry_expected, sizeof(guarded));
}

void test_string_struct_next_entry() {
    alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
    uint8_t expected[sizeof(storage)];
    auto *strings = new (storage + 16) StringStruct;
    StringStructEntry *null_entry = nullptr;
    auto *invalid_entry = reinterpret_cast<StringStructEntry *>(1);

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, null_entry);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        StringStructEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.payload = INT_MIN;
    entries[1].entry.payload = 0;
    entries[2].entry.payload = INT_MAX;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    StringStructEntry *head = &entries[0].entry;
    StringStructEntry *second = &entries[1].entry;
    StringStructEntry *third = &entries[2].entry;
    const int three = 3;
    const int one = 1;
    const int two = 2;
    const int zero = 0;

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, three);
    write_at(storage, 16 + 0x14, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, one);
    expect(string_struct_next_entry_redirect(strings, nullptr) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, third);
    write_at(storage, 16 + 0x10, three);
    write_at(storage, 16 + 0x14, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->next_entry() == INT_MIN);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, invalid_entry);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, zero);
    write_at(storage, 16 + 0x14, -1);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, zero);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, INT_MIN);
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_string_struct_seek_id() {
    alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
    uint8_t expected[sizeof(storage)];
    auto *strings = new (storage + 16) StringStruct;
    StringStructEntry *null_entry = nullptr;
    auto *invalid_entry = reinterpret_cast<StringStructEntry *>(1);

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, null_entry);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    expect(strings->seek_id(INT_MIN) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, invalid_entry);
    write_at(storage, 16 + 0x0C, null_entry);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, invalid_entry);
    const int zero = 0;
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        StringStructEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = INT_MIN;
    entries[1].entry.id = 0;
    entries[2].entry.id = INT_MAX;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    StringStructEntry *head = &entries[0].entry;
    StringStructEntry *second = &entries[1].entry;
    const int one = 1;
    const int three = 3;

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, zero);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, three);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC, second);
    write_at(expected, 16 + 0x14, one);
    expect(string_struct_seek_id_redirect(strings, nullptr, zero) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(INT_MIN) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    StringStructEntry *third = &entries[2].entry;
    const int two = 2;
    write_at(expected, 16 + 0x0C, third);
    write_at(expected, 16 + 0x14, two);
    expect(strings->seek_id(INT_MAX) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    entries[2].entry.id = zero;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, one);
    expect(strings->seek_id(zero) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    entries[2].entry.id = INT_MAX;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, three);
    expect(strings->seek_id(17) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[2].entry.next = null_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, null_entry);
    write_at(expected, 16 + 0x14, three);
    expect(strings->seek_id(17) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    // A single-entry list distinguishes `entry_count_ > 0` from `> 1`: with
    // three entries above, both guards evaluate true and the loop runs either
    // way, so a mutated bound survived undetected. Only count == 1 makes the
    // guard itself observable.
    struct GuardedEntry single_entry_storage;
    uint8_t single_entry_expected[sizeof(single_entry_storage)];
    seed_storage(reinterpret_cast<uint8_t *>(&single_entry_storage),
                 single_entry_expected, sizeof(single_entry_storage));
    single_entry_storage.entry.id = 42;
    single_entry_storage.entry.next = &single_entry_storage.entry;
    std::memcpy(single_entry_expected, &single_entry_storage,
                sizeof(single_entry_storage));
    StringStructEntry *single_head = &single_entry_storage.entry;

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, single_head);
    write_at(storage, 16 + 0x10, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, single_head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(42) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(reinterpret_cast<uint8_t *>(&single_entry_storage),
                         single_entry_expected, sizeof(single_entry_storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, single_head);
    write_at(storage, 16 + 0x10, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, single_head);
    write_at(expected, 16 + 0x14, one);
    expect(strings->seek_id(17) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(reinterpret_cast<uint8_t *>(&single_entry_storage),
                         single_entry_expected, sizeof(single_entry_storage));
}

}  // namespace

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(89, test_log_lifecycle_and_wrappers);
LEAF_CASE(92, test_dialog_id_to_pos);
LEAF_CASE(93, test_dialog_set_selected_id);
LEAF_CASE(94, test_dialog_get_selected_id);
LEAF_CASE(95, test_dialog_pos_to_id);
LEAF_CASE(100, test_dialog_font_and_color_setters);
LEAF_CASE(96, test_string_struct_current_accessors);
LEAF_CASE(97, test_string_struct_next_entry);
LEAF_CASE(98, test_string_struct_seek_id);
}  // namespace

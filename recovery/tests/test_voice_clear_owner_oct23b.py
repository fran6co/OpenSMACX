"""Host controls for receiver and Net::close ordering; not linked-game proof."""

from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

import pytest

from test_application_link_recovery import ROOT, recovered_harness_source


NET_CLOSE_HOST = r'''
struct Net;
Net *active_net_for_close;
int net_receiver_hook_count, net_queue_hook_count;
int net_queue_order[3];
int expect_stale_null_receiver;
void observe_net_receiver_hook();

struct NetDirectPlayDouble { void Close() {} void Release() {} };
NetDirectPlayDouble *NetDirectPlay;
struct NetAllocationNodeDouble { char *session_name_, *password_; };
struct NetPlayerDouble {
    int field_0_, field_4_, field_8_, flags_14_;
};
struct NetThreadInfoDouble {
    CRITICAL_SECTION receive_lock_;
    void *events_[5];
    void *direct_play_, *owner_, *buffer_;
    unsigned long thread_id_;
    int lock_initialized_;
};
struct VoiceTxDouble { void unload() {} };
struct Net {
    VoiceTxDouble voice_tx_;
    int poll_timeout_E0_, local_player_id_, host_player_id_, poll_time_DC_, field_6DC_;
    NetThreadInfoDouble thread_info_;
    NetAllocationNodeDouble allocation_node_;
    void *buffer_6E0_;
    int field_6E4_, field_6FC_, voice_flags_D8_;
    NetPlayerDouble players_[16];
    JackalVoiceRx jackal_voice_rx_;
    unsigned int voice_key_D4_;
    NetFifo queue_e8_, queue_10c_, queue_130_;
    unsigned char service_provider_guid_[16];
    int field_E4_, voice_hotkey_48_, voice_hotkey_4c_, voice_target_50_, field_54_;
    void close();
};

void SetEvent(void *) {}
void CloseHandle(void *) {}
void Sleep(unsigned long) {}

void observe_net_receiver_hook() {
    assert(active_net_for_close);
    ++net_receiver_hook_count;
    // D4 is retained through receiver callbacks; the earlier D8 clear has
    // already happened. Mutating both makes post-clear stores observable.
    assert(active_net_for_close->voice_key_D4_ == 0x12345678);
    assert(active_net_for_close->voice_flags_D8_ == 0);
    active_net_for_close->voice_key_D4_ = 0xA5A5A5A5;
    active_net_for_close->voice_flags_D8_ = 0x5A5A;
}

void observe_net_queue_close(NetFifo *queue) {
    if (!active_net_for_close) return;
    assert(net_queue_hook_count < 3);
    assert(!active_net_for_close->jackal_voice_rx_.head_);
    if (expect_stale_null_receiver) {
        assert(!active_net_for_close->jackal_voice_rx_.head_);
        assert(active_net_for_close->jackal_voice_rx_.count_ == 7);
        assert(active_net_for_close->jackal_voice_rx_.cursor_index_ == 9);
    } else {
        assert(!active_net_for_close->jackal_voice_rx_.count_);
        assert(!active_net_for_close->jackal_voice_rx_.cursor_index_);
    }
    assert(active_net_for_close->voice_key_D4_ == 0);
    assert(active_net_for_close->voice_flags_D8_ == 0);
    int index = queue == &active_net_for_close->queue_e8_ ? 0 :
                queue == &active_net_for_close->queue_10c_ ? 1 :
                queue == &active_net_for_close->queue_130_ ? 2 : -1;
    assert(index == net_queue_hook_count);
    net_queue_order[net_queue_hook_count++] = index;
    active_queue = queue;
}

void check_net_receiver_close(int scenario) {
    Net net;
    net.thread_info_.lock_initialized_ = 0;
    net.allocation_node_.session_name_ = net.allocation_node_.password_ = 0;
    net.buffer_6E0_ = 0;
    for (int event_index = 0; event_index != 5; ++event_index)
        net.thread_info_.events_[event_index] = 0;
    net.voice_key_D4_ = 0x12345678;
    net.voice_flags_D8_ = 0x7777;
    net.jackal_voice_rx_.head_ = 0;
    net.jackal_voice_rx_.cursor_ = 0;
    const bool populated = scenario == 1;
    const bool stale_null = scenario == 2;
    net.jackal_voice_rx_.count_ = populated ? 1 : (stale_null ? 7 : 0);
    net.jackal_voice_rx_.cursor_index_ = populated ? 2 : (stale_null ? 9 : 0);
    VoiceReceiverEntry untouched;
    if (stale_null) net.jackal_voice_rx_.cursor_ = &untouched;
    VoiceReceiverEntry *entry = 0;
    Payload *payload = 0;
    if (populated) {
        entry = new VoiceReceiverEntry;
        payload = new Payload(2);
        payloads[2] = payload;
        entry->payload_ = static_cast<VoiceRxAllocation *>(payload);
        entry->next_ = entry->previous_ = entry;
        net.jackal_voice_rx_.head_ = entry;
        net.jackal_voice_rx_.cursor_ = entry;
    }
    net_receiver_hook_count = net_queue_hook_count = 0;
    for (int order_index = 0; order_index != 3; ++order_index)
        net_queue_order[order_index] = -1;
    expect_stale_null_receiver = stale_null;
    active_net_for_close = &net;
    net.close();
    active_net_for_close = 0;
    active_queue = 0;
    assert(net_receiver_hook_count == (populated ? 1 : 0));
    assert(net_queue_hook_count == 3);
    assert(net_queue_order[0] == 0 && net_queue_order[1] == 1 && net_queue_order[2] == 2);
    assert(net.voice_key_D4_ == 0 && net.voice_flags_D8_ == 0);
    assert(!net.jackal_voice_rx_.head_);
    if (stale_null) {
        assert(net.jackal_voice_rx_.count_ == 7);
        assert(net.jackal_voice_rx_.cursor_index_ == 9);
        assert(net.jackal_voice_rx_.cursor_ == &untouched);
    } else {
        assert(!net.jackal_voice_rx_.count_ && !net.jackal_voice_rx_.cursor_index_);
    }
    if (populated) assert(destroyed[2] == 1);
    payloads[2] = 0;
}
'''


def _compile_and_run(tmp_path, monkeypatch, source):
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment
    from match_recovery.processes import run_bounded

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("the leased project compiler and Wine32 are required")
    tmp_path.mkdir(parents=True, exist_ok=True)
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    source_path = tmp_path / "receiver_net_close.cpp"
    source_path.write_text(source)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Oy-")
    (tmp_path / "receiver_net_close.obj").write_bytes(compile_object(source_path, command, flags))
    result = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                          "/subsystem:console", "/incremental:no", "/out:receiver_net_close.exe",
                          "receiver_net_close.obj", "kernel32.lib"], cwd=tmp_path, timeout=60)
    assert result.returncode == 0, (result.stdout + result.stderr).decode(errors="replace")
    with tempfile.TemporaryFile() as diagnostics:
        result = subprocess.run(["wine32", str(tmp_path / "receiver_net_close.exe")],
                                env=environment(), stdout=diagnostics, stderr=diagnostics,
                                timeout=30)
        diagnostics.seek(0)
        output = diagnostics.read().decode(errors="replace")
    return result.returncode, output


def _source_with_actual_bodies():
    harness = recovered_harness_source("receiver_cleanup_harness.cpp", [], declarations=[
        ("// LIVE_CLEAR_BODY", "src/net_class.cpp", "void JackalVoiceRx::clear() {", "__DEFINITION__"),
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_OWNER_PUBLICATION", "src/stringstruct.cpp", "StringAllocationBase::~StringAllocationBase() {",
         "// ORIGINAL: 0x00401000 ??0StringStruct@@QAE@H@Z"),
        ("// LIVE_VOICE_RX_ALLOCATION", "src/net_class.h", "class VoiceRxAllocation :",
         "static_assert(sizeof(VoiceRxAllocation)"),
        ("// LIVE_RECEIVER_DECLARATIONS", "src/net_class.h", "struct VoiceReceiverEntry :",
         "// The list subobject at Net+0x72C"),
        ("// LIVE_QUEUE_DECLARATIONS", "src/net_fifo.h", "class NetFifo {", None),
        ("// LIVE_QUEUE_CONSTRUCTOR", "src/net_fifo.cpp", "NetFifo::NetFifo() {", "// ORIGINAL: 0x00633A50"),
        ("// LIVE_QUEUE_CLOSE", "src/net_fifo.cpp", "void NetFifo::close() {", "// ORIGINAL: 0x00633AB0"),
        ("// LIVE_QUEUE_GET", "src/net_fifo.cpp",
         "int NetFifo::get(void *data, unsigned int *key, int *type,\n                 unsigned int *len)",
         "// Additional original operations"),
        ("// LIVE_SCRATCH_STORAGE", "src/net_class.cpp", "unsigned char NetGetScratch[",
         "namespace {\n// Shared wire views for the sender and receiver."),
        ("// LIVE_QUEUE_DESTRUCTOR", "src/net_fifo.cpp", "NetFifo::~NetFifo() {", "NetFifo::NetFifo() {"),
    ])
    source = (ROOT / "src/net_class.cpp").read_text()
    close = extract_definition(source, "void Net::close() {")
    # Inject a queue observer into the actual queue close body while retaining
    # every original operation and order.
    qsource = (ROOT / "src/net_fifo.cpp").read_text()
    qclose = extract_definition(qsource, "void NetFifo::close() {")
    assert harness.count(qclose) == 1
    harness = harness.replace(qclose, "void observe_net_queue_close(NetFifo *);\n" + qclose.replace(
        "void NetFifo::close() {", "void NetFifo::close() {\n    observe_net_queue_close(this);") )
    # Net::close's three call edges are independently observed by each actual
    # NetFifo::close invocation; no application Net wrapper is substituted.
    # The observer uses the queue receiver to establish the order.
    harness = harness.replace("int main() {", NET_CLOSE_HOST + "\n" + close + "\nint main() {")
    harness = harness.replace("void JackalVoiceRx::on_remove(void *payload) { observe_removal(this, payload); }",
                              "struct Net; extern Net *active_net_for_close; "
                              "void observe_net_receiver_hook();\n"
                              "void JackalVoiceRx::on_remove(void *payload) { observe_removal(this, payload); "
                              "if (active_net_for_close) observe_net_receiver_hook(); }")
    harness = harness.replace("    StringAllocationHeap = 0;\n    return 0;",
                              "    StringAllocationHeap = 0;\n"
                              "    hook_count = destruction_count = 0;\n"
                              "    check_net_receiver_close(0);\n"
                              "    check_net_receiver_close(1);\n"
                              "    check_net_receiver_close(2);\n"
                              "    return 0;")
    return harness, close


def extract_definition(text, declaration):
    # Deliberately duplicate the tiny lexical selector here so the test module
    # remains independently runnable even if imported by another pytest path.
    assert text.count(declaration) == 1
    start = text.index(declaration)
    opening = text.index("{", start)
    depth, state, i = 0, "code", opening
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/": state, i = "line", i + 2; continue
            if char == "/" and nxt == "*": state, i = "block", i + 2; continue
            if char == '"': state = "string"
            elif char == "'": state = "char"
            elif char == "{": depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0: return text[start:i + 1]
        elif state == "line":
            if char == "\n": state = "code"
        elif state == "block":
            if char == "*" and nxt == "/": state, i = "code", i + 2; continue
        else:
            if char == "\\": i += 2; continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"): state = "code"
        i += 1
    raise AssertionError("unterminated definition: " + declaration)


def test_actual_receiver_and_close_bodies_observe_post_clear_resets(tmp_path, monkeypatch):
    source, _ = _source_with_actual_bodies()
    code, output = _compile_and_run(tmp_path, monkeypatch, source)
    assert code == 0, output


@pytest.mark.parametrize("field", ["voice_key_D4_", "voice_flags_D8_"])
def test_close_reset_omission_and_early_store_are_compiled_damage_controls(
        tmp_path, monkeypatch, field):
    baseline, close = _source_with_actual_bodies()
    reset = f"    {field} = 0;"
    assert close.count(reset) == (1 if field == "voice_key_D4_" else 2)
    occurrence = close.rfind(reset) if field == "voice_flags_D8_" else close.index(reset)
    omitted = close[:occurrence] + close[occurrence + len(reset):]
    omission_source = baseline.replace(close, omitted)
    code, _ = _compile_and_run(tmp_path / "omit", monkeypatch, omission_source)
    assert code != 0

    early = close.replace("    if (jackal_voice_rx_.head_ != 0) {",
                          reset + "\n    if (jackal_voice_rx_.head_ != 0) {", 1)
    if field == "voice_flags_D8_":
        # The first D8 zero remains in its original place; move only the late
        # second zero before the callback-capable receiver teardown.
        late = close.rfind(reset)
        early = close[:late] + close[late + len(reset):]
        early = early.replace("    if (jackal_voice_rx_.head_ != 0) {",
                              reset + "\n    if (jackal_voice_rx_.head_ != 0) {", 1)
    else:
        early = close.replace(reset, "", 1).replace(
            "    if (jackal_voice_rx_.head_ != 0) {",
            reset + "\n    if (jackal_voice_rx_.head_ != 0) {", 1)
    early_source = baseline.replace(close, early)
    code, _ = _compile_and_run(tmp_path / "early", monkeypatch, early_source)
    assert code != 0

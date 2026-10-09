"""Focused controls for Net's inline service-provider GUID owner."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start:offset + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_provider_guid_source_owner_and_original_consumer_order() -> None:
    header = (ROOT / "src/net_class.h").read_text()
    source = (ROOT / "src/net_class.cpp").read_text()
    assert "GUID service_provider_guid_;  // 0x6EC" in header
    assert "_GUID *guid_6E8_;" in header
    constructor = _body(source, "Net::Net() {")
    close = _body(source, "void Net::close() {")
    join = _body(source, "int Net::join_service(ServiceStruct *service)")
    create = _body(source, "NetAllocationNode *Net::create_session(")
    assert "service_provider_guid_" not in constructor
    assert close.index("queue_130_.close();") < close.index(
        "memset(&service_provider_guid_, 0, sizeof(service_provider_guid_));"
    )
    assert "GUID *provider = &service->provider_guid_;" in join
    assert "service_provider_guid_ = *provider;" in join
    assert "DirectPlayCreate(provider," in join
    serial = create.index("&service_provider_guid_, &NetSerialServiceGuid")
    modem = create.index("&service_provider_guid_, &NetModemServiceGuid")
    assert serial < modem
    assert "guid_6E8_" in create and "&service_provider_guid_" in create


def test_actual_net_close_zeros_guid_after_queue_teardown(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    source = (ROOT / "src/net_class.cpp").read_text()
    close = _body(source, "void Net::close() {")
    harness = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
struct GUID { uint32_t a, b, c, d; };
struct DP { int Close() { return 0; } int Release() { return 0; } };
struct Thread { void *receive_lock_[32]{}; void *events_[5]{}; DP *direct_play_{};
  void *owner_{}, *buffer_{}; uint32_t thread_id_{}, lock_initialized_{}; };
struct Node { char *session_name_{}, *password_{}; };
struct Voice { void unload() {} };
struct Rx { void *head_{}; void clear() { head_ = 0; } };
struct GUID;
static GUID *observed_guid;
static int queue_guid_observations;
struct Queue { int closes{}; void close(); };
struct Player { uint32_t field_0_{}, field_4_{}, field_8_{}, field_C_{};
  uint32_t stamp_10_{}; unsigned char flags_14_{}; };
struct Net {
  Voice voice_tx_; Thread thread_info_; Node allocation_node_; Rx jackal_voice_rx_;
  Queue queue_e8_, queue_10c_, queue_130_;
  uint32_t poll_timeout_E0_{}, field_760_{}, field_764_{}, poll_time_DC_{}, field_6DC_{};
  void *buffer_6E0_{}; uint32_t field_6E4_{}, field_6FC_{}, voice_flags_D8_{};
  Player players_[16]{}; GUID service_provider_guid_;
  uint32_t field_E4_{}, voice_hotkey_48_{}, voice_hotkey_4c_{};
  unsigned long voice_target_50_{}; uint32_t field_54_{};
  void close();
};
void Queue::close() { ++closes; if (observed_guid->a != 0) ++queue_guid_observations; }
DP *NetDirectPlay;
void EnterCriticalSection(void *) {} void LeaveCriticalSection(void *) {}
void SetEvent(void *) {} void Sleep(int) {} void DeleteCriticalSection(void *) {}
void CloseHandle(void *) {}
'''
    # The recovered body uses the SDK's Win32 return types, but ignores these
    # results; host stubs preserve the statements while keeping the test local.
    harness += "\n" + close + r'''
int main() {
  Net net{};
  net.service_provider_guid_ = {1, 2, 3, 4};
  observed_guid = &net.service_provider_guid_;
  net.field_6E4_ = 0;
  net.close();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(
      &net.service_provider_guid_);
  for (unsigned i = 0; i != sizeof(GUID); ++i) assert(bytes[i] == 0);
  assert(net.queue_e8_.closes == 1 && net.queue_10c_.closes == 1
         && net.queue_130_.closes == 1);
  assert(queue_guid_observations == 3);
  assert(net.field_E4_ == 1 && net.field_6FC_ == 0xC8);
}
'''
    cpp, binary = tmp_path / "net_close.cpp", tmp_path / "net_close"
    cpp.write_text(harness)
    built = subprocess.run([compiler, "-std=c++11", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, check=False)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr

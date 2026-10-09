"""Host controls for the shared-scratch pointer returned by Net::get."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]

HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>

static unsigned char NetGetScratch[64];
static void *NetDirectPlay;

class NetFifo {
public:
  int result;
  const unsigned char *payload;
  unsigned int payload_size;
  unsigned int key, size;
  bool write_outputs;
  int calls;
  NetFifo() : result(0), payload(0), payload_size(0), key(0), size(0),
              write_outputs(false), calls(0) {}
  int get(void *scratch, unsigned int *out_key, int *, unsigned int *out_size) {
    ++calls;
    if (payload != 0) memcpy(scratch, payload, payload_size);
    if (write_outputs && out_key != 0) *out_key = key;
    if (write_outputs && out_size != 0) *out_size = size;
    return result;
  }
};
class Net {
public:
  NetFifo queue_10c_;
  void *get(unsigned long *, unsigned long *);
};
class NetDaemon : public Net {
public:
  char *seen;
  unsigned long seen_size;
  int seen_sender, calls;
  NetDaemon() : seen(0), seen_size(0), seen_sender(0), calls(0) {}
  int process_message(char *message, unsigned long size, int sender) {
    seen = message; seen_size = size; seen_sender = sender; ++calls; return 0;
  }
  int receive();
};
static NetDaemon NetDaemonNet;
'''


def _compiled_test(get_body: str, receive_body: str, root: Path) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler required for actual-source control"
    main = r'''int main() {
  Net net;
  unsigned long key = 0xAA, size = 0xBB;
  unsigned char first[] = {1, 2, 3};
  unsigned char second[] = {9, 8};
  NetDirectPlay = &net;

  assert(net.get(&key, &size) == 0);                 // empty FIFO leaves outputs
  assert(key == 0xAA && size == 0xBB && net.queue_10c_.calls == 1);
  NetDirectPlay = 0;
  assert(net.get(&key, &size) == 0 && net.queue_10c_.calls == 1);
  assert(key == 0xAA && size == 0xBB);
  NetDirectPlay = &net;
  assert(net.get(0, &size) == 0 && net.queue_10c_.calls == 1);
  assert(key == 0xAA && size == 0xBB);

  net.queue_10c_.result = 0;                         // copied packet, but null result
  net.queue_10c_.write_outputs = true;
  net.queue_10c_.payload = first; net.queue_10c_.payload_size = sizeof(first);
  net.queue_10c_.key = 0x1234; net.queue_10c_.size = 3;
  assert(net.get(&key, &size) == 0);
  assert(key == 0x1234 && size == 3 && memcmp(NetGetScratch, first, 3) == 0);

  net.queue_10c_.result = 7;                         // null payload exposes stale scratch
  net.queue_10c_.payload = 0; net.queue_10c_.key = 0x5678; net.queue_10c_.size = 99;
  void *shared = net.get(&key, &size);
  assert(shared == NetGetScratch && key == 0x5678 && size == 99);
  assert(memcmp(shared, first, 3) == 0);

  net.queue_10c_.payload = second; net.queue_10c_.payload_size = sizeof(second);
  void *again = net.get(&key, &size);
  assert(again == shared && memcmp(shared, second, 2) == 0);

  NetDaemonNet.queue_10c_.result = 3;
  NetDaemonNet.queue_10c_.write_outputs = true;
  NetDaemonNet.queue_10c_.payload = first; NetDaemonNet.queue_10c_.payload_size = sizeof(first);
  NetDaemonNet.queue_10c_.key = 0xCAFEBABE; NetDaemonNet.queue_10c_.size = 17;
  assert(NetDaemonNet.receive() == 1);
  assert(NetDaemonNet.calls == 1 && NetDaemonNet.seen == reinterpret_cast<char *>(NetGetScratch));
  assert((uint32_t)NetDaemonNet.seen_size == 17);
  assert((uint32_t)NetDaemonNet.seen_sender == 0xCAFEBABE);
  NetDaemonNet.queue_10c_.result = 0;
  assert(NetDaemonNet.receive() == 0 && NetDaemonNet.calls == 1);
}
'''
    source = HARNESS + "\n" + get_body + "\n" + receive_body + "\n" + main
    with tempfile.TemporaryDirectory(dir="/tmp") as temp:
        cpp = Path(temp) / "net_get_control.cpp"
        exe = Path(temp) / "net_get_control"
        cpp.write_text(source)
        build = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
                               text=True, capture_output=True)
        if build.returncode != 0:
            return build
        run = subprocess.run([str(exe)], text=True, capture_output=True)
        return run


def test_actual_net_get_pointer_and_receive_forwarding_controls() -> None:
    net_source = "\n".join(row.region for row in read_file(ROOT / "src/net_class.cpp"))
    daemon_source = "\n".join(row.region for row in read_file(ROOT / "src/netdaemon.cpp"))
    get_body = _definition(net_source, "void *Net::get(unsigned long *a, unsigned long *b)")
    receive_body = _definition(daemon_source, "int NetDaemon::receive()")
    result = _compiled_test(get_body, receive_body, ROOT)
    assert result.returncode == 0, result.stdout + result.stderr

    # A status-shaped regression must fail the executable controls: returning
    # a pointer on empty/result-zero paths breaks both null and identity cases.
    mutant = get_body.replace("return result ? NetGetScratch : 0;",
                              "return result ? 0 : NetGetScratch;")
    assert mutant != get_body
    damaged = _compiled_test(mutant, receive_body, ROOT)
    assert damaged.returncode != 0
    forwarding_mutant = receive_body.replace(
        "process_message(static_cast<char *>(result)",
        "process_message(static_cast<char *>(0)",
    )
    assert forwarding_mutant != receive_body
    damaged_forwarding = _compiled_test(get_body, forwarding_mutant, ROOT)
    assert damaged_forwarding.returncode != 0


def test_net_get_declaration_and_all_mapped_consumers_are_pointer_typed() -> None:
    header = (ROOT / "src/net_class.h").read_text()
    daemon = (ROOT / "src/netdaemon.cpp").read_text()
    assert "void *get(unsigned long *a, unsigned long *b);" in header
    assert daemon.count("NetDaemonNet.Net::get(") == 52
    assert not __import__("re").search(
        r"\b(?:const\s+)?int\s+\w+\s*=\s*NetDaemonNet\.Net::get\(|"
        r"while\s*\(\s*int\s+\w+\s*=\s*NetDaemonNet\.Net::get\(", daemon
    )
    assert "unsigned long id;" not in daemon

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <new>
#include <type_traits>

using std::int8_t;
using std::int32_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

/* ACTUAL_NETWIN_DECLARATIONS */

static_assert(std::is_trivially_default_constructible<NetWinLobbyRow>::value,
              "wire lobby row remains POD");
static_assert(std::is_trivially_default_constructible<NetWinSessionBlock>::value,
              "wire session block remains POD");
static_assert(
    std::is_trivially_default_constructible<NetWinSessionOptionsState>::value,
    "wire session options remain POD");
static_assert(
    std::is_trivially_default_constructible<NetWinLobbySinglePacket>::value,
    "single-row packet stays uninitialized by default");
static_assert(
    std::is_trivially_default_constructible<NetWinLobbyFullPacket>::value,
    "full-row packet stays uninitialized by default");
static_assert(
    std::is_trivially_default_constructible<NetWinSessionOptionsPacket>::value,
    "options packet stays uninitialized by default");

static void test_session_options_constructor_stores_only_observed_fields() {
  alignas(NetWinSessionOptionsStorage)
      unsigned char storage[sizeof(NetWinSessionOptionsStorage)];
  std::memset(storage, 0xA5, sizeof(storage));
  NetWinSessionOptionsStorage *options =
      new (storage) NetWinSessionOptionsStorage;
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(options);

  assert(bytes[0] == 0xFF);
  assert(bytes[1] == 4);
  assert(bytes[2] == 1);
  assert(bytes[3] == 0xA5);
  assert(bytes[4] == 0);
  assert(bytes[5] == 0);
  for (std::size_t i = 6; i <= 10; ++i) assert(bytes[i] == 1);
  assert(bytes[11] == 0xFF);
  for (std::size_t i = 12; i < sizeof(storage); ++i) assert(bytes[i] == 0xA5);
}

static void test_session_data_constructor_stores_only_observed_fields() {
  alignas(NetWinSessionDataStorage)
      unsigned char storage[sizeof(NetWinSessionDataStorage)];
  std::memset(storage, 0xA5, sizeof(storage));
  NetWinSessionDataStorage *session =
      new (storage) NetWinSessionDataStorage;
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(session);

  for (std::size_t i = 0; i < sizeof(uint32_t); ++i) assert(bytes[i] == 0);
  for (std::size_t i = sizeof(uint32_t); i < sizeof(storage); ++i) {
    if (i == 0x24) assert(bytes[i] == 0);
    else assert(bytes[i] == 0xA5);
  }
}

static void test_lobby_constructor_preserves_text_and_tail_bytes() {
  alignas(NetWinLobbyRowsStorage) unsigned char guarded[
      sizeof(uint32_t) + sizeof(NetWinLobbyRowsStorage) + sizeof(uint32_t)];
  std::memset(guarded, 0xA5, sizeof(guarded));
  unsigned char *storage = guarded + sizeof(uint32_t);
  std::memset(storage, 0xA5, sizeof(NetWinLobbyRowsStorage));
  NetWinLobbyRowsStorage *lobby = new (storage) NetWinLobbyRowsStorage;
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(lobby);

  for (std::size_t row = 0; row < 8; ++row) {
    const std::size_t base = row * sizeof(NetWinLobbyRow);
    assert(bytes[base + 0] == 0xFF);
    assert(bytes[base + 1] == 0);
    assert(bytes[base + 2] == 0);
    assert(bytes[base + 3] == 0xFF);
    assert(bytes[base + 4] == 2);
    for (std::size_t i = 5; i < offsetof(NetWinLobbyRow, owner_); ++i) {
      assert(bytes[base + i] == 0xA5);
    }
    for (std::size_t i = offsetof(NetWinLobbyRow, owner_);
         i < sizeof(NetWinLobbyRow); ++i) {
      assert(bytes[base + i] == 0);
    }
  }
  for (std::size_t i = 0; i < sizeof(uint32_t); ++i) {
    assert(guarded[i] == 0xA5);
    assert(guarded[sizeof(guarded) - sizeof(uint32_t) + i] == 0xA5);
  }
}

template <typename Packet>
static void test_packet_default_initialization_leaves_poison() {
  alignas(Packet) unsigned char storage[sizeof(Packet)];
  std::memset(storage, 0xA5, sizeof(storage));
  (void)new (storage) Packet;
  for (std::size_t i = 0; i < sizeof(storage); ++i) assert(storage[i] == 0xA5);
}

int main() {
  test_session_options_constructor_stores_only_observed_fields();
  test_session_data_constructor_stores_only_observed_fields();
  test_lobby_constructor_preserves_text_and_tail_bytes();
  test_packet_default_initialization_leaves_poison<NetWinLobbySinglePacket>();
  test_packet_default_initialization_leaves_poison<NetWinLobbyFullPacket>();
  test_packet_default_initialization_leaves_poison<NetWinSessionOptionsPacket>();
}

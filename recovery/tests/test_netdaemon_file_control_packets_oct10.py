"""Native file-transfer producer/receiver offsets; not full-game acceptance."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text, write

ROOT = Path(__file__).resolve().parents[2]
BASE = "88651d37c8f5880a39d8ac12d37ea5f3f284dea9"
# Exact authorized declaration/builder substitutions, not a whole-function waiver.
EDITS = (
    (r"""static_assert(offsetof(NetDaemonFileIndexRecord, filename) == 0x18,
              "native file-list filename offset");

struct NetDaemonFileChunkPacket {
    uint16_t opcode;          // +0
    uint16_t reserved_2;      // +2
""", r"""static_assert(offsetof(NetDaemonFileIndexRecord, filename) == 0x18,
              "native file-list filename offset");

// Native F05 writes its path at +0x18 (0x52EE83..0x52EEAA).
// Only opcode and path are populated; the intervening bytes stay untouched.
struct NetDaemonFilePathPacket {
    uint16_t opcode;
    uint8_t reserved_2[0x16];
    char path[0x104];
};
static_assert(offsetof(NetDaemonFilePathPacket, path) == 0x18,
              "native transfer path offset");
static_assert(sizeof(NetDaemonFilePathPacket) == 0x11C,
              "native transfer path packet extent");

// F0F writes source_player at +4; F07 writes state at +8, not at +2.
// Each operation leaves all other payload/header bytes untouched.
struct NetDaemonFileControlPacket {
    uint16_t opcode;
    uint16_t reserved_2;
    int32_t source_player;
    int32_t state;
    uint8_t reserved_C[0x14];
};
static_assert(offsetof(NetDaemonFileControlPacket, source_player) == 4,
              "native transfer acknowledgement player offset");
static_assert(offsetof(NetDaemonFileControlPacket, state) == 8,
              "native transfer completion state offset");
static_assert(sizeof(NetDaemonFileControlPacket) == 0x20,
              "native transfer control packet extent");

struct NetDaemonFileChunkPacket {
    uint16_t opcode;          // +0
    uint16_t reserved_2;      // +2
"""),
    (r"""                                static_cast<int>(sender));
        }

        unsigned char announce[0x20]; // native sends 0x20 bytes
        const uint16_t announce_type = 0xF0F;
        memcpy(announce, &announce_type, sizeof announce_type);
        memcpy(announce + 2, &local_index, sizeof local_index);
        NetDaemonNet.Net::send(announce, 0x20,
            static_cast<unsigned long>(field_764_), 1);

        SaveNameBuffer[0] = '\0';
""", r"""                                static_cast<int>(sender));
        }

        NetDaemonFileControlPacket announce;
        announce.opcode = 0xF0F;
        announce.source_player = local_index;
        NetDaemonNet.Net::send(&announce, 0x20,
            static_cast<unsigned long>(field_764_), 1);

        SaveNameBuffer[0] = '\0';
"""),
    (r"""
        NetDaemonFileTransferPath[0] = '\0';
        strcat(NetDaemonFileTransferPath, relative_path);
        uint8_t path_message[0x11C];
        const uint16_t path_type = 0xF05;
        memcpy(path_message, &path_type, sizeof path_type);
        path_message[2] = '\0';
        strcat(reinterpret_cast<char *>(path_message + 2),
               NetDaemonFileTransferPath);
        send_message(reinterpret_cast<char *>(path_message), 0x11C, 0);
        NetDaemonFileTransferAckMask = 0;
        if (send_mask != 0) {
            do {
""", r"""
        NetDaemonFileTransferPath[0] = '\0';
        strcat(NetDaemonFileTransferPath, relative_path);
        NetDaemonFilePathPacket path_message;
        path_message.opcode = 0xF05;
        path_message.path[0] = '\0';
        strcat(path_message.path, NetDaemonFileTransferPath);
        send_message(reinterpret_cast<char *>(&path_message), 0x11C, 0);
        NetDaemonFileTransferAckMask = 0;
        if (send_mask != 0) {
            do {
"""),
    (r"""            } while (FindNextFileA(find_handle, &found_file) != 0);
        }

        uint16_t list_done[0x10];
        list_done[0] = 0xF07;
        const int list_done_state = 2;
        memcpy(reinterpret_cast<unsigned char *>(list_done) + 2,
               &list_done_state, sizeof list_done_state);
        send_message(reinterpret_cast<char *>(list_done), 0x20, 0);
        FindClose(find_handle);

        for (;;) {
""", r"""            } while (FindNextFileA(find_handle, &found_file) != 0);
        }

        NetDaemonFileControlPacket list_done;
        list_done.opcode = 0xF07;
        list_done.state = 2;
        send_message(reinterpret_cast<char *>(&list_done), 0x20, 0);
        FindClose(find_handle);

        for (;;) {
"""),
    (r"""                ++slot;
            if (slot > NetDaemonNet.player_count_) break;
        }
        uint16_t transfer_done[0x10];
        transfer_done[0] = 0xF07;
        const int transfer_done_state = 3;
        memcpy(reinterpret_cast<unsigned char *>(transfer_done) + 2,
               &transfer_done_state, sizeof transfer_done_state);
        send_message(reinterpret_cast<char *>(transfer_done), 0x20, 0);
    }

cleanup:
""", r"""                ++slot;
            if (slot > NetDaemonNet.player_count_) break;
        }
        NetDaemonFileControlPacket transfer_done;
        transfer_done.opcode = 0xF07;
        transfer_done.state = 3;
        send_message(reinterpret_cast<char *>(&transfer_done), 0x20, 0);
    }

cleanup:
"""),
)


def restore_file_control_packet_migration(source):
    from test_netdaemon_deletion_initialization_oct10 import restore_deletion_initialization_migration
    source = restore_deletion_initialization_migration(source)
    for before, after in reversed(EDITS):
        assert source.count(after) == 1
        source = source.replace(after, before, 1)
    return source


def test_only_four_packet_builders_and_plain_declarations_change():
    path = ROOT / "src/netdaemon.cpp"
    current = path.read_text()
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    assert restore_file_control_packet_migration(current) == old
    rows = read_text(current, path)
    before = {row.address: row for row in read_text(old, path)}
    assert {row.address for row in rows} == set(before)
    assert write(current, rows) == current
    for row in rows:
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(before[row.address], field.name)
        def lessons(record):
            return [(x.kind, x.key, x.prose, tuple(text for _, text in x.lines))
                    for x in record.origin]
        assert lessons(row) == lessons(before[row.address])


def test_pinned_payload_offsets_and_advancing_chunk_file_index():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin

    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    for address, encoding in (
        (0x52EE83, "66c78530fbffff050f"),
        (0x52EF42, "66c745940f0f"),
        (0x52EF48, "895d98"),
        (0x52F0E8, "66c74594070f"),
        (0x52F0EE, "c7459c02000000"),
        (0x52F42D, "66c74594070f"),
        (0x52F433, "c7459c03000000"),
        (0x52F185, "33db"),
        (0x52F21F, "895f08"),
        (0x52F399, "8b5dec"),
        (0x52F3A2, "43"),
        (0x52F3AB, "895dec"),
        (0x52F3B1, "0f8ce5fdffff"),
    ):
        expected = bytes.fromhex(encoding)
        assert image.read(Span(address, address + len(expected))) == expected
    assert -0x4B8 - (-0x4D0) == 0x18
    assert -0x68 - (-0x6C) == 4
    assert -0x64 - (-0x6C) == 8


/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

// The two bytes after opcode are observed unwritten in the native stack record;
// keep them unresolved and do not value-initialize the local object.
struct MessageDataRecord {
    unsigned short opcode_;
    unsigned char unresolved_2_[2];
    int faction_;
    uint32_t timestamp_;
    int sequence_;
    int payload_10_;
    int payload_14_;
    int payload_18_;
    int payload_1C_;
};

static_assert(sizeof(MessageDataRecord) == 0x20,
              "message_data stack record width from native stores");
static_assert(offsetof(MessageDataRecord, faction_) == 0x04,
              "message_data faction field offset");
static_assert(offsetof(MessageDataRecord, timestamp_) == 0x08,
              "message_data timestamp field offset");
static_assert(offsetof(MessageDataRecord, sequence_) == 0x0C,
              "message_data sequence field offset");
static_assert(offsetof(MessageDataRecord, payload_10_) == 0x10,
              "message_data first payload field offset");
static_assert(offsetof(MessageDataRecord, payload_14_) == 0x14,
              "message_data second payload field offset");
static_assert(offsetof(MessageDataRecord, payload_18_) == 0x18,
              "message_data third payload field offset");
static_assert(offsetof(MessageDataRecord, payload_1C_) == 0x1C,
              "message_data fourth payload field offset");

struct MessageVehicleRecord {
    unsigned short opcode_;
    unsigned char unresolved_2_[2];
    int32_t faction_;
    uint32_t time_;
    int32_t sequence_;
    int32_t field_10_;
    int32_t field_14_;
};

static_assert(sizeof(MessageVehicleRecord) == 0x18,
              "vehicle message native transmitted extent");
static_assert(offsetof(MessageVehicleRecord, faction_) == 0x04,
              "vehicle message faction offset");
static_assert(offsetof(MessageVehicleRecord, time_) == 0x08,
              "vehicle message timestamp offset");
static_assert(offsetof(MessageVehicleRecord, sequence_) == 0x0C,
              "vehicle message sequence offset");
static_assert(offsetof(MessageVehicleRecord, field_10_) == 0x10,
              "vehicle message first payload offset");
static_assert(offsetof(MessageVehicleRecord, field_14_) == 0x14,
              "vehicle message second payload offset");

struct MessageLockRecord {
    unsigned short opcode_;
    unsigned char unresolved_2_[2];
    int32_t faction_;
    uint32_t time_;
    int32_t sequence_;
    int32_t field_10_;
    int32_t field_14_;
    int32_t field_18_;
    int32_t field_1C_;
    int32_t field_20_;
    int32_t field_24_;
};

static_assert(sizeof(MessageLockRecord) == 0x28,
              "lock message native transmitted extent");
static_assert(offsetof(MessageLockRecord, faction_) == 0x04,
              "lock message faction offset");
static_assert(offsetof(MessageLockRecord, time_) == 0x08,
              "lock message timestamp offset");
static_assert(offsetof(MessageLockRecord, sequence_) == 0x0C,
              "lock message sequence offset");
static_assert(offsetof(MessageLockRecord, field_10_) == 0x10,
              "lock message first payload offset");
static_assert(offsetof(MessageLockRecord, field_14_) == 0x14,
              "lock message second payload offset");
static_assert(offsetof(MessageLockRecord, field_18_) == 0x18,
              "lock message third payload offset");
static_assert(offsetof(MessageLockRecord, field_1C_) == 0x1C,
              "lock message fourth payload offset");
static_assert(offsetof(MessageLockRecord, field_20_) == 0x20,
              "lock message fifth payload offset");
static_assert(offsetof(MessageLockRecord, field_24_) == 0x24,
              "lock message sixth payload offset");

struct MessageBaseTextRecord {
    unsigned short opcode_;
    unsigned char unresolved_2_[2];
    int32_t faction_;
    uint32_t time_;
    int32_t sequence_;
    int32_t payload_10_;
    char text_[24];
};

static_assert(sizeof(MessageBaseTextRecord) == 0x2C,
              "base text message native transmitted extent");
static_assert(offsetof(MessageBaseTextRecord, faction_) == 0x04,
              "base text message faction offset");
static_assert(offsetof(MessageBaseTextRecord, time_) == 0x08,
              "base text message timestamp offset");
static_assert(offsetof(MessageBaseTextRecord, sequence_) == 0x0C,
              "base text message sequence offset");
static_assert(offsetof(MessageBaseTextRecord, payload_10_) == 0x10,
              "base text message payload offset");
static_assert(offsetof(MessageBaseTextRecord, text_) == 0x14,
              "base text message text offset");

// Native automatic wire records. Bytes 2..3 remain unwritten in all three
// builders; the raw bulk sender also leaves data_[8..31] unwritten. These
// objects must not be value-initialized merely to make the wire deterministic.
struct MessageBigDataRecord {
    unsigned short opcode_;
    unsigned char unresolved_2_[2];
    int faction_;
    uint32_t timestamp_;
    int sequence_;
    int data_[32];
};

struct MessageLandmarkRecord {
    unsigned short opcode_;
    unsigned char unresolved_2_[2];
    int faction_;
    uint32_t timestamp_;
    int sequence_;
    int payload_10_;
    int payload_14_;
    int payload_18_;
    int payload_1C_;
    char text_[32];
};

static_assert(sizeof(MessageBigDataRecord) == 0x90,
              "bulk-data native transmitted extent");
static_assert(offsetof(MessageBigDataRecord, faction_) == 0x04,
              "bulk-data faction word");
static_assert(offsetof(MessageBigDataRecord, timestamp_) == 0x08,
              "bulk-data clock word");
static_assert(offsetof(MessageBigDataRecord, sequence_) == 0x0C,
              "bulk-data sequence word");
static_assert(offsetof(MessageBigDataRecord, data_) == 0x10,
              "bulk-data thirty-two-word payload");

static_assert(sizeof(MessageLandmarkRecord) == 0x40,
              "landmark native transmitted extent");
static_assert(offsetof(MessageLandmarkRecord, faction_) == 0x04,
              "landmark faction word");
static_assert(offsetof(MessageLandmarkRecord, timestamp_) == 0x08,
              "landmark clock word");
static_assert(offsetof(MessageLandmarkRecord, sequence_) == 0x0C,
              "landmark sequence word");
static_assert(offsetof(MessageLandmarkRecord, payload_10_) == 0x10,
              "landmark first payload word");
static_assert(offsetof(MessageLandmarkRecord, payload_14_) == 0x14,
              "landmark second payload word");
static_assert(offsetof(MessageLandmarkRecord, payload_18_) == 0x18,
              "landmark third payload word");
static_assert(offsetof(MessageLandmarkRecord, payload_1C_) == 0x1C,
              "landmark fourth payload word");
static_assert(offsetof(MessageLandmarkRecord, text_) == 0x20,
              "landmark optional text start");

// The synchronization packet's serialized header prefix is 0x24 bytes. It is
// a field map over borrowed bytes, not the type or extent of the allocated
// packet. In particular, the sender leaves +2..+3 and +0x20..+0x23 unwritten,
// so neither view constructs or copies a complete prefix object. The first
// sixteen bytes are the header every message carries: send_message tests the
// flag word and, for 0x4000 messages, stamps the clock at +8.
struct SyncHeaderPrefix {
    int16_t flags;
    unsigned char unresolved_2[2];
    uint32_t faction;
    uint32_t timestamp;
    uint32_t sequence;
    int16_t opcode;
    int16_t index;
    int32_t argument_2;
    int32_t argument_3;
    int32_t argument_4;
    unsigned char unresolved_20[4];
};

static_assert(sizeof(SyncHeaderPrefix) == 0x24,
              "synchronization serialized header prefix width");
static_assert(offsetof(SyncHeaderPrefix, flags) == 0x00,
              "synchronization flags offset");
static_assert(offsetof(SyncHeaderPrefix, unresolved_2) == 0x02,
              "synchronization unwritten bytes 2..3 offset");
static_assert(offsetof(SyncHeaderPrefix, faction) == 0x04,
              "synchronization faction offset");
static_assert(offsetof(SyncHeaderPrefix, timestamp) == 0x08,
              "synchronization send-clock offset");
static_assert(offsetof(SyncHeaderPrefix, sequence) == 0x0C,
              "synchronization sequence offset");
static_assert(offsetof(SyncHeaderPrefix, opcode) == 0x10,
              "synchronization inner opcode offset");
static_assert(offsetof(SyncHeaderPrefix, index) == 0x12,
              "synchronization signed chunk index offset");
static_assert(offsetof(SyncHeaderPrefix, argument_2) == 0x14,
              "synchronization argument 2 offset");
static_assert(offsetof(SyncHeaderPrefix, argument_3) == 0x18,
              "synchronization argument 3 offset");
static_assert(offsetof(SyncHeaderPrefix, argument_4) == 0x1C,
              "synchronization argument 4 offset");
static_assert(offsetof(SyncHeaderPrefix, unresolved_20) == 0x20,
              "synchronization unwritten bytes 20..23 offset");

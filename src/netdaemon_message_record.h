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
    int16_t opcode_;
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
    int16_t opcode_;
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
    int16_t opcode_;
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

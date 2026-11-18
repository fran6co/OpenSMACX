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

// The game entities JACKAL.LIB references directly. In the image these are the
// library's only references into game objects that are not COMDAT copies of
// inline functions:
// - ExitTurnLoop (0x009B2068, the game's bss): Net's send paths set it.
// - WinBubbleWindow (0x009B22F0, the game's MultiDebug window): Win::on_char
//   toggles it on the "mdebug" key sequence.
// - The sound module (Sound, VoiceRx/VoiceTx, VideoSoundState, Wave_Device
//   and WaveDeviceGlobal, get_sound_version). The game defines it, and
//   JACKAL.LIB is compiled against the same sound.h / sound device.h; the
//   image's own "sound header files used by jackal" checks say so.
// Jackal sources that need them include this header rather than the game's
// headers wholesale.
#include "sound.h"
#include "wave_device.h"
#include "multidebug.h"

extern int ExitTurnLoop;  // 0x009B2068, defined in src/game.cpp

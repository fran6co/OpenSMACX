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
#include "stdafx.h"
#include <mmsystem.h>
#include "original_seam.h"
#include "ambience.h"
#include "sound.h"   // Sound::set_type, reached by cast
#include "fx.h"
#include "wave.h"
#include "general.h"  // filefind_get
#include "faction.h"
#include "game.h"
#include "strings.h"

// RECOVERY_OBJECT: v1 0x0074DAA0 ?FactionMusic@@3PAVFactionAmbience@@A
FactionAmbience *FactionMusic = nullptr;
// RECOVERY_OBJECT: v1 0x00683BF4 ?LastAmbienceVehicle@@3HA
int LastAmbienceVehicle = -1;

// These exact cells are source-owned here; the transition marker's wider
// external lifetime census remains open for 0x0074C5E4.
// RECOVERY_OBJECT: v1 0x00683BF0 ?AmbienceCurrentEffect@@3HA
int AmbienceCurrentEffect = -1;
// RECOVERY_OBJECT: v1 0x0074C5E4 ?AmbienceTransitionMarker@@3HA
int AmbienceTransitionMarker = 0;

// ORIGINAL: 0x00446B40 ?ambience@@YAHH@Z 0x00446B40-0x00446CD6
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/00446b40.cpp", "recovery/tasks/ambience-faction-art-owner-oct5.json", "pinned complete original spans; fresh native owner compilation and actual-source behavior controls"], "reason": "Complete semantic recovery through genuine source-owned members, globals and lifetime operations; retain original defects and historical obligations. Current byte restoration remains unfinished."}
// symbol    ?ambience@@YAHH@Z
// TRIED: guard bit 0x800, no-op-if-unchanged, in-range switch dispatch on *DAT_0074daa0's vtable (14 cases mapping to slots verified against the raw jump table at 0x446CD8, not Ghidra's summary, which collapsed two distinct case targets), else stop the old FX entry (slot 0x28), start the new one (slot 0x5c), revert on failure (slot 0x20), and FX::play() the result. Landed at 0.89 similarity; diverges at #108 where case 0xc8 and case 0xd0 - same vtable slot, genuinely two separate blocks in the original (`mov eax`/`call [eax+..]` vs `mov edx`/`call [edx+..]`) - compile to one shared jump target here. Tried plain int* stride (`g_00749cf8 + id*27`) against explicit byte-offset char* arithmetic for the FX table addressing; neither changed the divergence.
int __cdecl ambience(int ambience_id) {
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x800) == 0) {
        return ambience_id;
    }

    if (AmbienceCurrentEffect == ambience_id) {
        return AmbienceCurrentEffect;
    }

    if (ambience_id >= 0xC8 && ambience_id < 0xD6) {
        if (FactionMusic == nullptr) {
            return ambience_id;
        }

        switch (ambience_id) {
        case 0xC8: FactionMusic->general(); break;
        case 0xC9: FactionMusic->tech(); break;
        case 0xCA: FactionMusic->popup1(); break;
        case 0xCB: FactionMusic->eot(); break;
        case 0xCC: FactionMusic->hostility(); break;
        case 0xCD: FactionMusic->basewin_show(); break;
        case 0xCE: FactionMusic->basewin_hide(); break;
        case 0xCF: FactionMusic->terraform(); break;
        case 0xD0: FactionMusic->general(); break;
        case 0xD1: FactionMusic->new_base(); break;
        case 0xD2: FactionMusic->energy_resources(); break;
        case 0xD3: FactionMusic->design_window_show(); break;
        case 0xD4: FactionMusic->design_window_hide(); break;
        case 0xD5: FactionMusic->base_liberated(); break;
        }
        return ambience_id;
    }

    AmbienceTransitionMarker = -1;
    const int previous_effect = AmbienceCurrentEffect;
    if (previous_effect >= 0) {
        g_FX.effect(previous_effect).fade();
    }

    AmbienceCurrentEffect = ambience_id;
    if (ambience_id >= 0) {
        if (g_FX.effect(ambience_id).is_playing() != 0) {
            g_FX.effect(AmbienceCurrentEffect).stop();
        }
        g_FX.play(AmbienceCurrentEffect);
    }

    return AmbienceCurrentEffect;
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004457E0 ?load_music@@YAXH@Z 0x004457E0-0x00445BA4;0x00654CF0-0x00654D52 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/004457e0.cpp", "src/ambience.h", "src/midi.h", "src/fx.h"], "reason": "Original constructor staging, actual event tables and compiler-call contracts identify the genuine audio owner operation; compatible historical seed spans and claims remain available for restoration, not an acceptance verdict."}
void __cdecl load_music(int faction_id) {
    StringTemp[0] = 0;
    strcat(StringTemp, "fx\\");
    if (FactionMusic) {
        delete FactionMusic;
        FactionMusic = nullptr;
    }
    const char *const name = Players[faction_id].search_key;
    FactionAmbience *music;
    if (stricmp(name, "gaians") == 0) {
        strcat(StringTemp, "gset1.amb");
        music = new GAmbience;
    } else if (stricmp(name, "hive") == 0) {
        strcat(StringTemp, "mset1.amb");
        music = new MAmbience;
    } else if (stricmp(name, "univ") == 0) {
        strcat(StringTemp, "uset2.amb");
        music = new UAmbience;
    } else if (stricmp(name, "morgan") == 0) {
        strcat(StringTemp, "mset1.amb");
        music = new MAmbience;
    } else if (stricmp(name, "spartans") == 0) {
        strcat(StringTemp, "sset1.amb");
        music = new SAmbience;
    } else if (stricmp(name, "believers") == 0) {
        strcat(StringTemp, "bset1.amb");
        music = new BAmbience;
    } else if (stricmp(name, "peace") == 0) {
        strcat(StringTemp, "gset1.amb");
        music = new GAmbience;
    } else {
        strcat(StringTemp, "aset1.amb");
        music = new AAmbience;
    }
    const uint32_t preferences = GameDataStateStorage.fields_.game_preferences_;
    FactionMusic = music;
    if ((preferences & 0x800) && music) {
        music->load(StringTemp);
        FactionMusic->begin();
    }
}

// Original447DD0 individually reads/writes each four-byte sequencer cell.
// RECOVERY_OBJECT: v1 0x00683C30 _AAmbiencePass
static int AAmbiencePass = 6;             //683C30
// RECOVERY_OBJECT: v1 0x00683C34 _AAmbienceRandomRange
static int AAmbienceRandomRange = 1;      //683C34
// RECOVERY_OBJECT: v1 0x00683C38 _AAmbiencePhase
static int AAmbiencePhase = 3;            //683C38
// RECOVERY_OBJECT: v1 0x00683C3C _AAmbienceCents
static int AAmbienceCents = -600;         //683C3C
// RECOVERY_OBJECT: v1 0x00683C40 _AAmbienceCentsStep
static int AAmbienceCentsStep = 200;      //683C40
// RECOVERY_OBJECT: v1 0x00683C44 _AAmbienceLevel
static int AAmbienceLevel = 90;           //683C44
// RECOVERY_OBJECT: v1 0x00683C48 _AAmbienceTrack0
static int AAmbienceTrack0 = 18;          //683C48
// RECOVERY_OBJECT: v1 0x00683C4C _AAmbienceTrack1
static int AAmbienceTrack1 = 21;          //683C4C
// RECOVERY_OBJECT: v1 0x00683C50 _AAmbienceTrack2
static int AAmbienceTrack2 = 24;          //683C50
// RECOVERY_OBJECT: v1 0x0074DAE8 _AAmbiencePhase3Cents
static int AAmbiencePhase3Cents = 0;      //74DAE8
// RECOVERY_OBJECT: v1 0x0074DAF0 _AAmbienceAlternate
static int AAmbienceAlternate = 0;        //74DAF0

// RECOVERY_OBJECT: v1 0x0074DAD4 _BAmbienceHostility
static uint8_t BAmbienceHostility = 0;    //74DAD4
// RECOVERY_OBJECT: v1 0x00683C18 _BAmbienceAmbientPass
static int BAmbienceAmbientPass = 3;      //683C18
// RECOVERY_OBJECT: v1 0x0074DAD8 _BAmbienceMusicPass
static int BAmbienceMusicPass = 0;        //74DAD8
// RECOVERY_OBJECT: v1 0x0074DADC _BAmbienceTrackPass
static int BAmbienceTrackPass = 0;        //74DADC
// RECOVERY_OBJECT: v1 0x00683C1C _BAmbienceAlternate
static uint8_t BAmbienceAlternate = 1;    //683C1C

// ORIGINAL: 0x00447940 ?general@BAmbience@@QAEXXZ 0x00447940-0x00447AA9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00447940.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?general@BAmbience@@UAEXXZ
void BAmbience::general() {
    field_74_ += 500;
    if (field_74_ > 500) field_74_ = -1000;
    const uint8_t hostility = BAmbienceHostility;
    ++BAmbienceAmbientPass;
    ++BAmbienceMusicPass;
    ++BAmbienceTrackPass;
    if (hostility) {
        const bool playing = Midi::is_trackset_playing(4);
        if (playing) return;
        BAmbienceHostility = playing;
    }
    if (field_6D_) {
        const int track = field_78_++;
        play_trackset(track);
        if (field_78_ > 22) field_78_ = 20;
        if (BAmbienceAmbientPass >= 3) {
            play_trackset(field_7C_);
            BAmbienceAmbientPass = 0;
        }
        field_7C_ += 2;
        if (field_7C_ == 21) field_7C_ = 16;
        if (field_7C_ == 22) field_7C_ = 16;
    }
    if (BAmbienceAmbientPass >= 3) {
        const int track = BAmbienceAlternate ? 24 : 25;
        xpose_trackset(track, field_74_);
        if (field_70_ > 1) {
            xpose_trackset(11, field_74_);
            play_trackset(11);
        }
        play_trackset(track);
        BAmbienceAlternate = !BAmbienceAlternate;
    }
    if (BAmbienceMusicPass >= 5) {
        xpose_trackset(13, field_74_);
        play_trackset(13);
        BAmbienceMusicPass = 0;
    }
}

// ORIGINAL: 0x00447AF0 ?hostility@BAmbience@@QAEXXZ 0x00447AF0-0x00447B2D FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00447af0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?hostility@BAmbience@@UAEXXZ
void BAmbience::hostility() {
    if (!BAmbienceHostility) {
        BAmbienceHostility = 1;
        xpose_trackset(15, field_74_);
        play_trackset(15);
        play_trackset(27);
    }
}

// ORIGINAL: 0x00447DB0 ?begin@AAmbience@@QAEXXZ 0x00447DB0-0x00447DC6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/00447db0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?begin@AAmbience@@UAEXXZ
void AAmbience::begin() {
    play();
    play_trackset(3);
}

// ORIGINAL: 0x00447DD0 ?general@AAmbience@@QAEXXZ 0x00447DD0-0x00448273 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00447dd0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?general@AAmbience@@UAEXXZ
void AAmbience::general() {
    ++AAmbiencePass;
    if (AAmbiencePhase == 1) {
        if (AAmbiencePass % 7 != 0) goto finish_phase;
        xpose_trackset(14, AAmbienceCents);
        xpose_trackset(15, AAmbienceCents);
        xpose_trackset(16, AAmbienceCents);
        xpose_trackset(13, AAmbienceCents);
        AAmbienceCents += AAmbienceCentsStep;
        play_trackset(14);
        play_trackset(2);
        play_trackset(1);
        if (AAmbienceCents >= 300 || AAmbienceCents < -900)
            AAmbienceCentsStep = -AAmbienceCentsStep;
        if (AAmbiencePass > 10) play_trackset(15);
        if (AAmbiencePass > 20) play_trackset(16);
        if (AAmbiencePass > 25) play_trackset(13);
    }
    if (AAmbiencePhase == 2) {
        play_trackset(27);
        AAmbienceCents += AAmbienceCentsStep;
        if (AAmbienceCents >= 100 || AAmbienceCents < -600)
            AAmbienceCentsStep = -AAmbienceCentsStep;
        AAmbienceLevel += 10;
        AAmbienceAlternate ^= 1;
        if (AAmbienceAlternate) play_trackset(AAmbienceTrack1);
        else play_trackset(AAmbienceTrack0);
        if (AAmbienceTrack2 != 25) play_trackset(AAmbienceTrack2);
        if (++AAmbienceTrack0 > 20) AAmbienceTrack0 = 18;
        if (++AAmbienceTrack1 > 23) AAmbienceTrack1 = 21;
        if (++AAmbienceTrack2 > 26) AAmbienceTrack2 = 24;
    }
    if (AAmbiencePhase == 3) {
        AAmbiencePhase3Cents += AAmbienceCentsStep;
        if (AAmbiencePhase3Cents >= 100 || AAmbiencePhase3Cents < -600)
            AAmbienceCentsStep = -AAmbienceCentsStep;
        xpose_trackset(8, AAmbiencePhase3Cents);
        xpose_trackset(3, AAmbiencePhase3Cents);
        xpose_trackset(8, AAmbiencePhase3Cents);
        xpose_trackset(6, AAmbiencePhase3Cents);
        xpose_trackset(5, AAmbiencePhase3Cents);
        xpose_trackset(2, AAmbiencePhase3Cents);
        xpose_trackset(4, AAmbiencePhase3Cents);
        if (AAmbiencePass == 5 || AAmbiencePass == 15) play_trackset(5);
        if (AAmbienceRandomRange > 5) AAmbienceRandomRange = 1;
        else ++AAmbienceRandomRange;
        unsigned int track = timeGetTime() % static_cast<unsigned int>(AAmbienceRandomRange);
        if (track == 3 || (track == 8 && AAmbiencePhase3Cents >= 0)) ++track;
        if (!is_trackset_playing(track)) play_trackset(track);
    }
finish_phase:
    if (AAmbiencePass < 40) return;
    if (AAmbiencePhase == 1) {
        stop_trackset(13);
        stop_trackset(15);
        stop_trackset(16);
        AAmbiencePhase = 2;
        sub_4c7f70(14, 0);
        AAmbiencePass = 0;
        return;
    }
    if (AAmbiencePhase == 2) {
        stop_trackset(18);
        stop_trackset(19);
        stop_trackset(20);
        stop_trackset(21);
        stop_trackset(22);
        stop_trackset(23);
        stop_trackset(24);
        stop_trackset(25);
        stop_trackset(26);
        stop_trackset(27);
        sub_4c7f70(14, 1);
        play_trackset(3);
        AAmbiencePhase = 3;
        AAmbiencePass = 0;
        return;
    }
    if (AAmbiencePhase == 3) {
        stop_trackset(1);
        stop_trackset(2);
        stop_trackset(3);
        stop_trackset(4);
        stop_trackset(5);
        stop_trackset(6);
        stop_trackset(7);
        stop_trackset(8);
        AAmbiencePhase = 1;
    }
    AAmbiencePass = 0;
}

// ORIGINAL: 0x004C8530 ?load@Ambience@@QAEHPBD@Z 0x004C8530-0x004C85EF FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/unrecovered/004c8530.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// TRIED: reused-parameter-slot byte/garbage-upper-bits trick for the sub_4c7cc0 arg (UB, not attempted); passed the plain flag bit instead. Ambience reinterpreted as Sound* for the trailing Sound::load call (single-inheritance-at-0 evidence: ecx=edi unchanged across the call). First divergence #3 mov/push.
// symbol    ?load@Ambience@@UAEHPBD@Z
int Ambience::load(const char *filename) {
    char path[256];
    strcpy(path, filename);
    const char *const found = filefind_get(path);
    if (!found) return 10;
    if (device_) unload();
    if (!device_) {
        if (!WaveDeviceReleaseGuard) return 1;
        const int error = (WaveDeviceCreateSlot())(&device_, found, 5);
        if (error) return error;
    }
    // The mode5 factory produces the same Sound/Midi object chain used by
    // Midi::set_mode. The low-byte argument is the only part consumed by
    // every recursive4C7CC0 stage; no speculative engine-overlay fields.
    static_cast<Midi *>(device_)->set_mode(field_54_ & 1);
    return Sound::load(found);
}

// ORIGINAL: 0x004C85F0 ?unload@Ambience@@QAEXXZ 0x004C85F0-0x004C861C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/004c85f0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?unload@Ambience@@UAEHXZ
int Ambience::unload() {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate) delegate->slot005();
    detach();
    device_ = nullptr;
    flags_40_ &= ~1u;
    return 0;
}

UAmbience::UAmbience() {}
// ORIGINAL: 0x004478D0 ??0BAmbience@@QAE@XZ 0x004478D0-0x00447911 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/004478d0.cpp", "src/ambience.h", "src/midi.h", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004478d0.cpp"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ??0BAmbience@@QAE@XZ
BAmbience::BAmbience() {
    field_70_ = 1;
    field_74_ = 0;
    field_78_ = 20;
    field_7C_ = 15;
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447220 ?begin@FactionAmbience@@QAEXXZ 0x00447220-0x00447221 BYTE_EXACT
// symbol    ?begin@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?begin@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::begin() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447230 ?tech@FactionAmbience@@QAEXXZ 0x00447230-0x00447231 BYTE_EXACT
// symbol    ?tech@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?tech@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::tech() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447240 ?terraform@FactionAmbience@@QAEXXZ 0x00447240-0x00447241 BYTE_EXACT
// symbol    ?terraform@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?terraform@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::terraform() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447250 ?production@FactionAmbience@@QAEXXZ 0x00447250-0x00447251 BYTE_EXACT
// symbol    ?production@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?production@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::production() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447260 ?general@FactionAmbience@@QAEXXZ 0x00447260-0x00447261 BYTE_EXACT
// symbol    ?general@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?general@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::general() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472B0 ?new_base@FactionAmbience@@QAEXXZ 0x004472B0-0x004472B1 BYTE_EXACT
// symbol    ?new_base@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?new_base@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::new_base() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472C0 ?popup1@FactionAmbience@@QAEXXZ 0x004472C0-0x004472C1 BYTE_EXACT
// symbol    ?popup1@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?popup1@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::popup1() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472D0 ?eot@FactionAmbience@@QAEXXZ 0x004472D0-0x004472D1 BYTE_EXACT
// symbol    ?eot@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?eot@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::eot() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472E0 ?hostility@FactionAmbience@@QAEXXZ 0x004472E0-0x004472E1 BYTE_EXACT
// symbol    ?hostility@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?hostility@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::hostility() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472F0 ?energy_resources@FactionAmbience@@QAEXXZ 0x004472F0-0x004472F1 BYTE_EXACT
// symbol    ?energy_resources@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?energy_resources@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::energy_resources() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447300 ?base_liberated@FactionAmbience@@QAEXXZ 0x00447300-0x00447301 BYTE_EXACT
// symbol    ?base_liberated@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?base_liberated@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::base_liberated() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447690 ?tech@UAmbience@@QAEXXZ 0x00447690-0x00447691 BYTE_EXACT
// symbol    ?tech@UAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?tech@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::tech() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004476A0 ?popup1@UAmbience@@QAEXXZ 0x004476A0-0x004476A1 BYTE_EXACT
// symbol    ?popup1@UAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?popup1@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::popup1() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004476B0 ?eot@UAmbience@@QAEXXZ 0x004476B0-0x004476B1 BYTE_EXACT
// symbol    ?eot@UAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?eot@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::eot() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447C60 ?tech@GAmbience@@QAEXXZ 0x00447C60-0x00447C61 BYTE_EXACT
// symbol    ?tech@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?tech@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::tech() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CA0 ?production@GAmbience@@QAEXXZ 0x00447CA0-0x00447CA1 BYTE_EXACT
// symbol    ?production@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?production@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::production() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CB0 ?popup1@GAmbience@@QAEXXZ 0x00447CB0-0x00447CB1 BYTE_EXACT
// symbol    ?popup1@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?popup1@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::popup1() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CC0 ?eot@GAmbience@@QAEXXZ 0x00447CC0-0x00447CC1 BYTE_EXACT
// symbol    ?eot@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?eot@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::eot() {
}





















/*
Purpose: Build the ambience. The original constructs in four vtable stages -
         0x0066E444 at 0x004C848A, 0x0066E3C0 at 0x004C84AC, 0x0066E538 at
         0x004C84EF, then its own 0x0066E664 at 0x004C850F - but only CRT
         memsets run between them, so the three intermediate installs are
         unobservable and, like the registered SEH frame, are omitted, keeping
         only the final Ambience publication. A device dispatch of the 0x3E8
         default through slot 0 at 0x004C84DB is provably dead - the device
         slot at 0x3C is zeroed at 0x004C84B8 and the memset in between covers
         0x40..0x43 only, so the reload at 0x004C84D4 cannot see anything else
         - and is omitted under the same policy. The net field state: full
         volume, zeroed regions, a 1000ms default at 0x38, flag dword 8, flag
         byte 1, and Sound::set_type run with type 5. Unlike Wave, whose 0x54
         clear is four bytes wide, this one is a single byte, so 0x55..0x57
         keep whatever the storage held - and nothing at or above 0x58 is
         written at all, which is what pins the shared base's extent there.
// ORIGINAL: 0x004C8460 ??0Ambience@@QAE@XZ 0x004C8460-0x004C852D;0x004C8450-0x004C8457;0x00659F4C-0x00659F5E
// TRIED: dropping `volatile` off `object`/`bytes` measured 0/72
//   agreeing both ways (the function diverges at instruction 0, the
//   omitted SEH prologue, before reaching these stores), but it shrank
//   the compiled body from 31 to 25 instructions - the optimiser folds
//   the documented `object[0x40/4] |= 8` / `bytes[0x54] |= 1`
//   read-modify-writes into plain immediate stores, which the
//   Verification note above says the image does NOT do (it genuinely
//   reloads at 0x004C84FA/0x004C84FC). Reverted; volatile is load-bearing
//   here, not incidental.
// symbol    ??0Ambience@@QAE@XZ
// size      230 bytes
// prototype void (__thiscall ??0Ambience@@QAE@XZ)(Ambience* this)
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C61E0 0x006465F0
// indirect  0x004C84E2
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
Verification note: 0x0066E664 has exactly one writer in the whole image, this
         constructor, and no reader on the teardown path, which is why the
         already-recovered ~Ambience at 0x004C7670 starts its descent one
         stage lower at 0x0066E538. That is not a missing stage: 0x0066E664
         differs from 0x0066E538 in three slots only - the deleting destructor
         at 0x08, load at 0x10 and unload at 0x14 - and 0x004C7670 is the
         base-level destructor, reached from both slot 0x08 of 0x0066E664 and
         the global's ??__F thunk at 0x004455E0 without a vtable store of its
         own, MSVC having elided a store that 0x004C7670 overwrites on its
         first instruction. The two bodies are consistent.
Verification note: the sweep leaves exactly two survivors and both are
         equivalent mutants that no assertion can separate, because the
         original's own shape makes them so. Widening the 0x0C..0x2F loop by
         one iteration lands on 0x30, which the very next statement stores
         zero to anyway; and seeding the 0x54 byte with 1 instead of 0 washes
         out under the `|= 1` two statements later. Both are transcribed the
         way the bytes read - a memset then a separate store, and a memset
         then a read-modify-write - rather than folded into whatever would
         make the sweep look cleaner.
Verification note: the base-level constructor at 0x004C75B0 is byte-identical
         to this one through the 0x0066E538 publication and then stops - it
         neither publishes 0x0066E664 nor calls set_type. Recovering it later
         must duplicate this shape rather than share a helper with it, or a
         mutation sweep of either loses its signal.
*/
Ambience::Ambience() {
    set_type(5);
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447270 ?basewin_show@FactionAmbience@@QAEXXZ 0x00447270-0x00447275 BYTE_EXACT
// symbol    ?basewin_show@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?basewin_show@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_show() {
    field_6C_ = 1;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447280 ?basewin_hide@FactionAmbience@@QAEXXZ 0x00447280-0x00447285 BYTE_EXACT
// symbol    ?basewin_hide@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?basewin_hide@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_hide() {
    field_6C_ = 0;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447290 ?design_window_show@FactionAmbience@@QAEXXZ 0x00447290-0x00447295 BYTE_EXACT
// symbol    ?design_window_show@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?design_window_show@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_show() {
    field_6D_ = 1;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x004472A0 ?design_window_hide@FactionAmbience@@QAEXXZ 0x004472A0-0x004472A5 BYTE_EXACT
// symbol    ?design_window_hide@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?design_window_hide@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_hide() {
    field_6D_ = 0;
}


/*
Purpose: GAmbience raises the flag byte only when it is CLEAR.

             mov al,[ecx+0x6C] / test al,al / jne done
             mov byte [ecx+0x6C],1 / done: ret

         The guard is not redundant. Its sibling basewin_hide below stores
         unconditionally, and an unconditional store here would agree for the
         values 0 and 1 and differ for every other: a byte holding 5 keeps 5,
         where `field_6C_ = 1` would overwrite it.
// ORIGINAL: 0x00447BE0 ?basewin_show@GAmbience@@QAEXXZ 0x00447BE0-0x00447BEC BYTE_EXACT
// symbol    ?basewin_show@GAmbience@@UAEXXZ
// size      12 bytes
// prototype void (__thiscall ?basewin_show@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_show() {
    if (field_6C_ == 0) {
        field_6C_ = 1;
    }
}


/*
Purpose: GAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447BF0 ?basewin_hide@GAmbience@@QAEXXZ 0x00447BF0-0x00447BF5 BYTE_EXACT
// symbol    ?basewin_hide@GAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?basewin_hide@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_hide() {
    field_6C_ = 0;
}


/*
Purpose: Run the shared Ambience constructor, publish this variant's vtable,
         and zero the fields this variant owns.

             push esi / mov esi,ecx / call ??0Ambience / xor eax,eax
             mov [esi],0x66c0f8 / mov [esi+0x58],eax ... [esi+0x64],eax
             mov byte [esi+0x6c],al / mov byte [esi+0x6d],al
             mov [esi+0x68],eax / mov eax,esi / pop esi / ret

         The four variants share this shape and differ only in the vtable and
         in how far the tail runs: FactionAmbience stops at 0x68, MAmbience and
         GAmbience add 0x70, SAmbience adds 0x70 and 0x74. That tail is what
         evidences each variant's extent, and it is why the four bodies are
         written out rather than routed through one helper - sharing a body
         would lose the mutation signal that keeps them distinct, exactly as
         the note on 0x004C75B0 in this file already warns.

         The base constructor runs FIRST and publishes the base vtable; the
         store at [this] then overwrites it. Ordering them the other way would
         leave the base's vtable in place.

         0x6c and 0x6d are BYTES. 0x6e and 0x6f are not written, so whatever
         the storage held survives there.

// ORIGINAL: 0x004471F0 ??0FactionAmbience@@QAE@XZ 0x004471F0-0x00447219
// symbol    ??0FactionAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: the vtable store must NOT be `volatile`. A volatile store is a scheduling barrier for VC6, so the `xor eax, eax` that feeds the seven zero field stores could not be hoisted above it - 13 of 15, with the image's `xor eax, eax` / `mov [esi], vtable` pair in the wrong order. Dropping the qualifier gives 15 of 15, BYTE_EXACT. The store stays alive without it because the seven field stores that follow are not dead, so nothing lets VC6 prove the object unobserved. Measured the same 15 of 15 with the `object[0x000 / 4]` idiom and with the zero stores written before the vtable store, so only the qualifier matters.
//   The catalogued name is the mangled CONSTRUCTOR, `??0FactionAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      41 bytes
// prototype void (__thiscall ??0FactionAmbience@@QAE@XZ)(FactionAmbience* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
FactionAmbience::FactionAmbience() {
    // The LEVER/construct-method notes above describe the retired spelling;
    // the compiler now owns base construction, table staging and unwinding.
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
}


/*
Purpose: As 004471F0, with the MAmbience vtable and one more field.
// ORIGINAL: 0x00447310 ??0MAmbience@@QAE@XZ 0x00447310-0x0044733C
// symbol    ??0MAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
//   The catalogued name is the mangled CONSTRUCTOR, `??0MAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      44 bytes
// prototype void (__thiscall ??0MAmbience@@QAE@XZ)(MAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
MAmbience::MAmbience() {
    // Historical construct-method notes above do not describe this real ctor.
    field_70_ = 0;
}


/*
Purpose: As 004471F0, with the SAmbience vtable and two more fields.
// ORIGINAL: 0x00447850 ??0SAmbience@@QAE@XZ 0x00447850-0x0044787F
// symbol    ??0SAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
//   The catalogued name is the mangled CONSTRUCTOR, `??0SAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      47 bytes
// prototype void (__thiscall ??0SAmbience@@QAE@XZ)(SAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
SAmbience::SAmbience() {
    // Historical construct-method notes above do not describe this real ctor.
    field_70_ = 0;
    field_74_ = 0;
}


/*
Purpose: As 004471F0, with the GAmbience vtable and one more field.
// ORIGINAL: 0x00447B90 ??0GAmbience@@QAE@XZ 0x00447B90-0x00447BBC
// symbol    ??0GAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
//   The catalogued name is the mangled CONSTRUCTOR, `??0GAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      44 bytes
// prototype void (__thiscall ??0GAmbience@@QAE@XZ)(GAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
GAmbience::GAmbience() {
    // Historical construct-method notes above do not describe this real ctor.
    field_70_ = 0;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Ambience g_AMBIENCE;  // 0x0074DA40
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// THE AMBIENCE SEQUENCER STATE. Every one of these is an ordinary int the
// image keeps in .data at a fixed address, and the four variants below read
// and write them as one shared bank. They are named from what the bodies do
// with them - the increment, the wrap point, the virtual slot the value
// feeds - because no catalogue row covers any of them. The binding spelling
// is the one the codegen needs: a dereferenced const pointer folds to the
// absolute address, where an `extern T` definition would not.
//
//   AmbienceClockArmed   0x0074C5E8  byte, set once, gates the first clock read
//   AmbienceClock        0x0074D8EC  the last clock reading
//   AmbienceGeneralTick  0x00683C14  clamped to 18, incremented once a pass
//   MAmbienceStep        0x00683BF8  play_trackset argument, cycles 5..7
//   MAmbienceStepBase    0x00683BFC  the closing xpose_trackset argument
//   MAmbiencePass        0x0074DAAC  the five-second pass counter
//   MAmbienceCents       0x0074DAB0  +150 a pass, wraps +600 back to -600
//   UAmbienceStep        0x00683C00  play_trackset argument, wraps +600 to -600
//   UAmbienceStepBase    0x00683C04  play_trackset argument, fixed
//   UAmbienceStepEvery   0x00683C08  the pass count UAmbience waits for
//   UAmbienceTrackset    0x00683C0C  toggles between 4 and 5
//   UAmbiencePass0       0x0074DAB4  wraps at 9
//   UAmbiencePass1       0x0074DAB8  wraps at 6
//   UAmbienceCents       0x0074DABC  +100 a pass
//   UAmbiencePass2       0x0074DAC0  compared against UAmbienceStepEvery
//   UAmbiencePass3       0x0074DAC4  wraps at 15
//   SAmbienceCents       0x0074DAC8  +100 a pass, wraps +400 back to -300
//   SAmbiencePass0       0x0074DACC  wraps at 1
//   SAmbiencePass1       0x0074DAD0  wraps at 7
// RECOVERY_OBJECT: v1 0x0074C5E8 _AmbienceClockArmed
static uint8_t AmbienceClockArmed = 0; // Original 0x0074C5E8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074D8EC _AmbienceClock
static int AmbienceClock = 0; // Original 0x0074D8EC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C14 _AmbienceGeneralTick
static int AmbienceGeneralTick = 17; // Original 0x00683C14, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683BF8 _MAmbienceStep
static int MAmbienceStep = 5; // Original 0x00683BF8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683BFC _MAmbienceStepBase
static int MAmbienceStepBase = 1; // Original 0x00683BFC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAAC _MAmbiencePass
static int MAmbiencePass = 0; // Original 0x0074DAAC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAB0 _MAmbienceCents
static int MAmbienceCents = 0; // Original 0x0074DAB0, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C00 _UAmbienceStep
static int UAmbienceStep = 2; // Original 0x00683C00, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C04 _UAmbienceStepBase
static int UAmbienceStepBase = 3; // Original 0x00683C04, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C08 _UAmbienceStepEvery
static int UAmbienceStepEvery = 10; // Original 0x00683C08, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C0C _UAmbienceTrackset
static int UAmbienceTrackset = 5; // Original 0x00683C0C, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAB4 _UAmbiencePass0
static int UAmbiencePass0 = 0; // Original 0x0074DAB4, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAB8 _UAmbiencePass1
static int UAmbiencePass1 = 0; // Original 0x0074DAB8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DABC _UAmbienceCents
static int UAmbienceCents = 0; // Original 0x0074DABC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAC0 _UAmbiencePass2
static int UAmbiencePass2 = 0; // Original 0x0074DAC0, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAC4 _UAmbiencePass3
static int UAmbiencePass3 = 0; // Original 0x0074DAC4, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAC8 _SAmbienceCents
static int SAmbienceCents = 0; // Original 0x0074DAC8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DACC _SAmbiencePass0
static int SAmbiencePass0 = 0; // Original 0x0074DACC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAD0 _SAmbiencePass1
static int SAmbiencePass1 = 0; // Original 0x0074DAD0, independently read initial bytes.



// ORIGINAL: 0x00447360 ?general@MAmbience@@QAEXXZ 0x00447360-0x0044746A FILE
// symbol    ?general@MAmbience@@UAEXXZ
// size      266 bytes
// prototype void (__thiscall ?general@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044737E 0x0044738B 0x00447399 0x004473E2 0x004473FA 0x00447461
void MAmbience::general() {
    DWORD (WINAPI *const get_time)() = timeGetTime;
    if ((AmbienceClockArmed & 1) == 0) {
        AmbienceClockArmed |= 1;
        AmbienceClock = get_time();
    }
    if (!is_trackset_playing(0x64)) {
        unsigned int now = get_time();
        unsigned int elapsed = now - static_cast<unsigned int>(AmbienceClock);
        int pass = MAmbiencePass + 1;
        MAmbiencePass = pass;
        if (elapsed >= 0x1388 && pass >= 5) {
            MAmbiencePass = 0;
            xpose_trackset(MAmbienceStep, MAmbienceCents);
            int step = (MAmbienceStep)++;
            play_trackset(step);
            if (MAmbienceStep > 7) {
                MAmbienceStep = 5;
            }
            MAmbienceCents += 0x96;
            if (MAmbienceCents > 0x258) {
                MAmbienceCents = -600;
            }
            AmbienceClock = now;
            field_70_ += 0x64;
            if (field_70_ > 0x1f4) {
                field_70_ = -700;
            }
            xpose_trackset(MAmbienceStepBase, field_70_);
        }
    }
}

// ORIGINAL: 0x00447570 ?general@UAmbience@@QAEXXZ 0x00447570-0x00447684 FILE
// symbol    ?general@UAmbience@@UAEXXZ
// size      276 bytes
// prototype void (__thiscall ?general@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004475D1 0x004475EB 0x00447620 0x00447650 0x00447675
void UAmbience::general() {
    if (field_6C_ != 0) {
        return;
    }
    ++UAmbiencePass3;
    ++UAmbiencePass0;
    ++UAmbiencePass1;
    ++UAmbiencePass2;
    if (UAmbiencePass3 == 0xf) {
        UAmbienceTrackset = (UAmbienceTrackset == 4) + 4;
        play_trackset(UAmbienceTrackset);
        UAmbiencePass3 = 0;
    }
    if (UAmbiencePass0 == 9) {
        play_trackset(0);
        UAmbiencePass0 = 0;
    }
    if (UAmbiencePass1 == 6) {
        UAmbienceStep = (UAmbienceStep != 2) + 1;
        xpose_trackset(UAmbienceStep, UAmbienceCents);
        UAmbienceCents += 0x64;
        if (UAmbienceStep > 0x258) {
            UAmbienceStep = -600;
        }
        play_trackset(UAmbienceStep);
        UAmbiencePass1 = 0;
    }
    if (UAmbiencePass2 == UAmbienceStepEvery) {
        play_trackset(UAmbienceStepBase);
        UAmbiencePass2 = 0;
    }
}

/*
Purpose: Increment the dword field at 0x58.
         Emitted by tools/generate_field_accessors.py from

             inc dword ptr [ecx + 0x58] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// ORIGINAL: 0x004476E0 ?tech@SAmbience@@QAEXXZ 0x004476E0-0x004476E4 BYTE_EXACT
// symbol    ?tech@SAmbience@@UAEXXZ
// size      4 bytes
// prototype void (__thiscall ?tech@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Original SAmbience table 0x0066BB58 slot76 selects this member increment,
// not the empty FactionAmbience::tech. The raw-accessor prose above is history.
void SAmbience::tech() {
    ++field_58_;
}

// ORIGINAL: 0x004476F0 ?general@SAmbience@@QAEXXZ 0x004476F0-0x004477A9 FILE
// symbol    ?general@SAmbience@@UAEXXZ
// LEVER: the delegate vtable - declaring Sound's 33 + the sequencer's 42 on
//   Ambience (ambience.h slots 33..74) is what turned the VCall shims into
//   real `call dword ptr [eax+0xNNN]` dispatches and brought this body from
//   an unverifiable artifact to 51 of 51 instructions structurally.
// TRIED: clamp through a named local read once (`int tick = *p; if (tick >
//   0x12) { tick = 0x11; *p = tick; }`) - the image holds the clamped tick in
//   ecx across the store AND the xpose argument, this spelling still lands it
//   in eax, 1 of 51. Plain `if (*p > 0x12) *p = 0x11;` gives the immediate
//   store the image does not emit. Also open: `(*p)++ > n` compiles
//   `lea ecx,[eax+1]` where the image has `mov edx,eax; inc eax`, and the
//   trailing `++*p` compiles load/inc/store where the image has one
//   `inc dword ptr [0x683c14]`. Register-preference plateau, not a semantic
//   one - same 51 instructions, same calls, same order.
// size      185 bytes
// prototype void (__thiscall ?general@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447727 0x00447738 0x00447767 0x00447773

void SAmbience::general() {
    // Read once and clamped in the local: the image keeps the clamped value
    // live in ecx across the clamp store AND the xpose_trackset argument that
    // follows it (0x004476FE `mov ecx, 0x11` -> 0x00447703 store ->
    // 0x00447724 `push ecx`), and only reloads the field for play_trackset,
    // after the call has possibly overwritten it.
    int tick = AmbienceGeneralTick;
    if (tick > 0x12) {
        tick = 0x11;
        AmbienceGeneralTick = tick;
    }
    if ((SAmbiencePass0)++ > 1) {
        xpose_trackset(tick, SAmbienceCents);
        play_trackset(AmbienceGeneralTick);
        SAmbiencePass0 = 0;
    }
    if ((SAmbiencePass1)++ > 7) {
        xpose_trackset(0x10, SAmbienceCents);
        play_trackset(0x10);
        SAmbiencePass1 = 0;
    }
    SAmbienceCents += 0x64;
    if (SAmbienceCents > 0x190) {
        SAmbienceCents = -300;
    }
    ++AmbienceGeneralTick;
}

// ORIGINAL: 0x004477B0 ?hostility@SAmbience@@QAEXXZ 0x004477B0-0x00447831 FILE BYTE_EXACT
// symbol    ?hostility@SAmbience@@UAEXXZ
// LEVER: field signedness. field_70_ and field_74_ are int32_t, not
//   uint32_t - the image compares each against a positive bound with `jle`
//   and wraps it downward past zero, and `uint32_t` spelled `jbe` twice for a
//   2-instruction loss. Together with the delegate vtable on Ambience this
//   took the homed artifact from 16 of 46 to 46 of 46.
// LEVER: the artifact's `VCall2 *self = (VCall2 *)this` pun and its
//   `field_70_ + 1` / store local are both gone - the member form with the
//   declared virtuals is byte-identical and carries no semantic debt.
// size      129 bytes
// prototype void (__thiscall ?hostility@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004477B7 0x004477D7 0x00447802 0x00447810
void SAmbience::hostility() {
    if (is_trackset_playing(0x64)) {
        return;
    }
    if (field_70_ == 0) {
        field_70_ = 0x64;
    }
    if (is_trackset_playing(field_70_)) {
        return;
    }
    field_70_ += 1;
    if (field_70_ > 0x67) {
        field_70_ = 0x65;
    }
    xpose_trackset(field_70_, field_74_);
    play_trackset(field_70_);
    field_74_ += 0x64;
    if (field_74_ > 0x12c) {
        field_74_ = 0xfffffed4;
    }
}


// THE SLOT-7 DISPATCH. Sound declares play(unsigned long) before play(), and
// VC6 gives same-name overloads their slots in REVERSE declaration order, so
// `reinterpret_cast<SlotSeven *>(this)->slot007()` lands one slot low. Sound's table is not this file's to move,
// so the four begin() variants reach slot 7 through a named holder instead -
// the class exists only to spell the convention and is never instantiated.

// ORIGINAL: 0x00447340 ?begin@MAmbience@@QAEXXZ 0x00447340-0x00447356 BYTE_EXACT
// symbol    ?begin@MAmbience@@UAEXXZ
// size      22 bytes
// prototype void (__thiscall ?begin@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447347 0x00447351
// 0x00447340  ?begin@MAmbience@@QAEXXZ  ->  ?begin@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::begin() {
    this->play_trackset(1);
    play();
}


// ORIGINAL: 0x00447470 ?tech@MAmbience@@QAEXXZ 0x00447470-0x0044748F BYTE_EXACT
// symbol    ?tech@MAmbience@@UAEXXZ
// size      31 bytes
// prototype void (__thiscall ?tech@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00447477 0x00447487
// 0x00447470  ?tech@MAmbience@@QAEXXZ  ->  ?tech@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::tech() {
    if (!this->is_trackset_playing(0x64)) {
        this->play_trackset(0x69);
    }
}

// ORIGINAL: 0x00447490 ?energy_resources@MAmbience@@QAEXXZ 0x00447490-0x004474AF BYTE_EXACT
// symbol    ?energy_resources@MAmbience@@UAEXXZ
// size      31 bytes
// prototype void (__thiscall ?energy_resources@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447497 0x004474A7
// 0x00447490  ?energy_resources@MAmbience@@QAEXXZ  ->  ?energy_resources@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::energy_resources() {
    if (!this->is_trackset_playing(0x64)) {
        this->play_trackset(4);
    }
}

// ORIGINAL: 0x004474B0 ?production@MAmbience@@QAEXXZ 0x004474B0-0x004474DF FILE BYTE_EXACT
// symbol    ?production@MAmbience@@UAEXXZ
// size      47 bytes
// prototype void (__thiscall ?production@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004474B7 0x004474CB 0x004474D7
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004474B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004474b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?production@MAmbience@@QAEXXZ  at 0x004474B0  (47 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void MAmbience::production() {
    Ambience *const self = this;
    if (!self->is_trackset_playing(0x64)) {
        self->xpose_trackset(4, field_70_);
        self->play_trackset(4);
    }
}


// ORIGINAL: 0x004474E0 ?new_base@MAmbience@@QAEXXZ 0x004474E0-0x00447507 BYTE_EXACT
// symbol    ?new_base@MAmbience@@UAEXXZ
// size      39 bytes
// prototype void (__thiscall ?new_base@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004474E7 0x004474F3 0x004474FF
// 0x004474E0  ?new_base@MAmbience@@QAEXXZ  ->  ?new_base@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::new_base() {
    // Unconditional, with no slot-70 gate - unlike tech() and
    // energy_resources() on this same class. And 0x64 here is the slot-67
    // argument, not the threshold it is elsewhere in the family.
    this->stop_trackset(4);
    this->stop_trackset(0x69);
    this->play_trackset(0x64);
}

// ORIGINAL: 0x00447510 ?begin@UAmbience@@QAEXXZ 0x00447510-0x00447526 BYTE_EXACT
// symbol    ?begin@UAmbience@@UAEXXZ
// size      22 bytes
// prototype void (__thiscall ?begin@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447515 0x0044751E
// 0x00447510  ?begin@UAmbience@@QAEXXZ  ->  ?begin@UAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void UAmbience::begin() {
    play();
    this->play_trackset(0);
}

// ORIGINAL: 0x00447530 ?basewin_show@UAmbience@@QAEXXZ 0x00447530-0x0044754A FILE BYTE_EXACT
// symbol    ?basewin_show@UAmbience@@UAEXXZ
// size      26 bytes
// prototype void (__thiscall ?basewin_show@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044753E
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447530
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447530/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?basewin_show@UAmbience@@QAEXXZ  at 0x00447530  (26 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void UAmbience::basewin_show() {
    if (!field_6C_) {
        this->play_trackset(6);
        field_6C_ = 1;
    }
}


// ORIGINAL: 0x00447550 ?basewin_hide@UAmbience@@QAEXXZ 0x00447550-0x00447563 BYTE_EXACT
// symbol    ?basewin_hide@UAmbience@@UAEXXZ
// size      19 bytes
// prototype void (__thiscall ?basewin_hide@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447557
// 0x00447550  ?basewin_hide@UAmbience@@QAEXXZ  ->  ?basewin_hide@UAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void UAmbience::basewin_hide() {
    this->stop_trackset(6);
    field_6C_ = 0;
}

// ORIGINAL: 0x004476C0 ?begin@SAmbience@@QAEXXZ 0x004476C0-0x004476D6 BYTE_EXACT
// symbol    ?begin@SAmbience@@UAEXXZ
// size      22 bytes
// prototype void (__thiscall ?begin@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004476C5 0x004476CE
// 0x004476C0  ?begin@SAmbience@@QAEXXZ  ->  ?begin@SAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SAmbience::begin() {
    play();
    this->play_trackset(0x11);
}


// ORIGINAL: 0x00447840 ?production@SAmbience@@QAEXXZ 0x00447840-0x0044784B BYTE_EXACT
// symbol    ?production@SAmbience@@UAEXXZ
// size      11 bytes
// prototype void (__thiscall ?production@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447844
// 0x00447840  ?production@SAmbience@@QAEXXZ  ->  ?production@SAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SAmbience::production() {
    this->play_trackset(0xf);
}

// ORIGINAL: 0x00447880 ?base_liberated@SAmbience@@QAEXXZ 0x00447880-0x004478C3 FILE BYTE_EXACT
// symbol    ?base_liberated@SAmbience@@UAEXXZ
// size      67 bytes
// prototype void (__thiscall ?base_liberated@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447887 0x00447893 0x0044789F 0x004478AB 0x004478BB
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447880
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447880/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_liberated@SAmbience@@QAEXXZ  at 0x00447880  (67 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void SAmbience::base_liberated() {
    Ambience *const self = this;
    self->stop_trackset(0x65);
    self->stop_trackset(0x66);
    self->stop_trackset(0x67);
    if (!self->is_trackset_playing(0x64)) {
        self->play_trackset(0x64);
    }
}

// ORIGINAL: 0x00447BC0 ?begin@GAmbience@@QAEXXZ 0x00447BC0-0x00447BDD BYTE_EXACT
// symbol    ?begin@GAmbience@@UAEXXZ
// size      29 bytes
// prototype void (__thiscall ?begin@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447BC5 0x00447BD5
// 0x00447BC0  ?begin@GAmbience@@QAEXXZ  ->  ?begin@GAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void GAmbience::begin() {
    play();
    // Alone among the six begin() overrides, this one also stores its mode.
    field_68_ = 0x18;
    this->play_trackset(0x18);
}

// Fixed-slot bindings carried from 00447c00.cpp
// AmbienceClockArmed's sibling at 0x00683C20: UAmbience's own one-shot
// gate, compared against 0x18 and never written here.
// RECOVERY_OBJECT: v1 0x00683C20 _UAmbienceGate
static int UAmbienceGate = 24; // Original 0x00683C20, independently read initial bytes.

// ORIGINAL: 0x00447C00 ?general@GAmbience@@QAEXXZ 0x00447C00-0x00447C58 FILE BYTE_EXACT
// symbol    ?general@GAmbience@@UAEXXZ
// size      88 bytes
// prototype void (__thiscall ?general@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00447C0F
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447C00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@GAmbience@@QAEXXZ  at 0x00447C00  (88 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::general() {
    const uint32_t arg68 = field_68_;

    this->xpose_trackset(arg68, field_70_);

    if (UAmbienceGate == 0x18) {
        field_70_ += 300;
        if (field_70_ > 600) {
            field_70_ = -600;
        }
        return;
    }
    field_70_ += 700;
    if (field_70_ > 1200) {
        field_70_ = -700;
    }
}

// Fixed-slot bindings carried from 00447c70.cpp
// GAmbience's play_trackset argument at 0x00683C24: cycled 7..9.
// RECOVERY_OBJECT: v1 0x00683C24 _GAmbienceStep
static int GAmbienceStep = 7; // Original 0x00683C24, independently read initial bytes.

// ORIGINAL: 0x00447C70 ?terraform@GAmbience@@QAEXXZ 0x00447C70-0x00447C9A FILE BYTE_EXACT
// symbol    ?terraform@GAmbience@@UAEXXZ
// size      42 bytes
// prototype void (__thiscall ?terraform@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447C79
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447C70
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447c70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?terraform@GAmbience@@QAEXXZ  at 0x00447C70  (42 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::terraform() {
    this->play_trackset(GAmbienceStep);
    if (++GAmbienceStep > 9) {
        GAmbienceStep = 7;
    }
}

// Fixed-slot bindings carried from 00447cd0.cpp
// SAmbience's one-shot flag at 0x00683C28, set to 1 on entry.
// RECOVERY_OBJECT: v1 0x00683C28 _SAmbienceArmed
static int SAmbienceArmed = 1; // Original 0x00683C28, independently read initial bytes.

// ORIGINAL: 0x00447CD0 ?new_base@GAmbience@@QAEXXZ 0x00447CD0-0x00447D07 FILE BYTE_EXACT
// symbol    ?new_base@GAmbience@@UAEXXZ
// size      55 bytes
// prototype void (__thiscall ?new_base@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447CE5 0x00447CFF
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447CD0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447cd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?new_base@GAmbience@@QAEXXZ  at 0x00447CD0  (55 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::new_base() {
    SAmbienceArmed = 1;

    this->stop_trackset(field_68_);
    field_68_ = 0x19;
    field_70_ = 0;
    this->play_trackset(0x19);
}

// ORIGINAL: 0x00447920 ?begin@BAmbience@@QAEXXZ 0x00447920-0x00447936 BYTE_EXACT
// symbol    ?begin@BAmbience@@UAEXXZ
void BAmbience::begin() {
    play_trackset(0x1A);
    play();
}

// ORIGINAL: 0x00447AB0 ?design_window_show@BAmbience@@QAEXXZ 0x00447AB0-0x00447AB5 BYTE_EXACT
// symbol    ?design_window_show@BAmbience@@UAEXXZ
void BAmbience::design_window_show() {
    field_6D_ = 1;
}

// ORIGINAL: 0x00447AC0 ?design_window_hide@BAmbience@@QAEXXZ 0x00447AC0-0x00447AEB BYTE_EXACT
// symbol    ?design_window_hide@BAmbience@@UAEXXZ
void BAmbience::design_window_hide() {
    field_6D_ = 0;
    stop_trackset(0x14);
    stop_trackset(0x15);
    stop_trackset(0x16);
}

// ORIGINAL: 0x00447B30 ?new_base@BAmbience@@QAEXXZ 0x00447B30-0x00447B58 BYTE_EXACT
// symbol    ?new_base@BAmbience@@UAEXXZ
void BAmbience::new_base() {
    const int reported = field_74_;
    field_70_ += 1;
    xpose_trackset(0x0D, reported);
    play_trackset(0x0D);
}

// ORIGINAL: 0x00447B60 ?base_liberated@BAmbience@@QAEXXZ 0x00447B60-0x00447B8B BYTE_EXACT
// symbol    ?base_liberated@BAmbience@@UAEXXZ
void BAmbience::base_liberated() {
    stop_trackset(0x1B);
    if (!is_trackset_playing(0x64)) {
        play_trackset(0x64);
    }
}

/*
Purpose: Increment the dword field at 0x58.
         Emitted by tools/generate_field_accessors.py from

             inc dword ptr [ecx + 0x58] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// ORIGINAL: 0x00448280 ?tech@AAmbience@@QAEXXZ 0x00448280-0x00448284 BYTE_EXACT
// symbol    ?tech@AAmbience@@UAEXXZ
// size      4 bytes
// prototype void (__thiscall ?tech@AAmbience@@QAEXXZ)(AAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AAmbience::tech() {
    ++field_58_;
}

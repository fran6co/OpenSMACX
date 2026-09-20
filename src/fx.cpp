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
// ORIGINAL: 0x004482D0 ??0Effect@@QAE@XZ 0x004482D0-0x004482E2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original FX construction at0x004482A0 passes0x004482D0 as its per-element constructor for97elements of0x6Cbytes.", "Existing seedsrc/recovered/units/004482d0.cpp identifies span0x004482D0-0x004482E2; original instructions call Wave::Wave at0x004C66E0 then store the Effect virtual table at0x0066B7A4."], "reason": "Implicit Effect constructor emitted for the real FX Effect array; Wave base construction and derived virtual-table installation are compiler-owned."}
#include "fx.h"
#include "basepop.h"
#include "popup.h"

// The implicit Effect constructor at 0x004482D0 installs the derived table;
// its Wave tail includes FX::init's virtual pitch target at slot 38.
// RECOVERY_GENERATED: v1 vtable 0x0066B7A4 ? ??_7Effect@@6B@ 0x004482D0

/*
Purpose: Construct the 97 contained Effect objects, including compiler-owned
         cleanup of already-constructed elements if construction throws.
// ORIGINAL: 0x004482A0 ??0FX@@QAE@XZ 0x004482A0-0x004482BB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ??0FX@@QAE@XZ)(FX* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006457C2
Return Value: n/a (the constructor ABI returns the object pointer in eax)
Status: Complete
*/
FX::FX() {
}


/*
Purpose: Destroy the contained Effect objects in reverse construction order.
// ORIGINAL: 0x00448290 ??1FX@@QAE@XZ 0x00448290-0x004482A0 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??1FX@@QAE@XZ)(FX* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
FX::~FX() {
}


/*
// ORIGINAL: 0x00446A90 ?stop@FX@@QAEXH@Z 0x00446A90-0x00446AA9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?stop@FX@@QAEXH@Z)(FX* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AA2
Status: Complete
*/
void FX::stop(int a1) {
    effects_[a1].stop();
}

/*
// ORIGINAL: 0x00446AB0 ?release@FX@@QAEXH@Z 0x00446AB0-0x00446AC9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?release@FX@@QAEXH@Z)(FX* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AC2
Status: Complete
*/
void FX::release(int a1) {
    effects_[a1].release();
}

/*
// ORIGINAL: 0x00446AD0 ?fade@FX@@QAEXH@Z 0x00446AD0-0x00446AE9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?fade@FX@@QAEXH@Z)(FX* this, int)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AE2
Status: Complete
*/
void FX::fade(int a1) {
    effects_[a1].fade();
}

// FX publishes these existing owners; they are defined in their domain TUs.
extern uint32_t BasePopDefaultBlock2E64[70];
extern uint32_t AutoSoundDefaults[37];
extern Wave g_OK_WAVE, g_ALPHAMENU_WAVE, g_SCOOT_WAVE, g_PASSOVER_WAVE;
extern Wave g_MENU_UP_WAVE, g_MENU_DOWN_WAVE;

// Original FX initialization copies 70 words from 0x00683AD8. The shared
// text-block type supplies the extent and callback-visible field meanings.
// RECOVERY_OBJECT: v2 0x00683AD8 0x00683BF0 _FxPopupSoundDefaults 0x00445CD0
const BasePopTextBlock FxPopupSoundDefaults = {"", -1, 127, 50, 0, 0};

// Initialize the effect and CPU-voice banks in original call order. Qualified
// Wave calls were direct in the executable; Effect[4].set_pitch is virtual.
// ORIGINAL: 0x00445CD0 ?init@FX@@QAEXXZ 0x00445CD0-0x004468E1
// RECOVERY_BINDING: v1 {"evidence": ["Existing seed src/unrecovered/00445cd0.cpp preserves original span445CD0..4468E1 and is retained as a hint.", "Original instructions store this+28EC=-1, publish callback4456B0, copy70words from683AD8, initialize/group the Effect array and global Wave objects, and reset37words at9BC080. The sole indirect call446034 targets Effect[4] virtual set_pitch at slot38."], "reason": "Initialize the real effect and CPU-voice banks and publish source-owned popup/audio defaults; replaces the executable-address forwarder."}
void FX::init() {
    field_28EC_ = 0xffffffff;
    BasePopDefaultField3094 = popup_wave_callback;
    memcpy(BasePopDefaultBlock2E64, &FxPopupSoundDefaults, sizeof(FxPopupSoundDefaults));

    g_OK_WAVE.Wave::load("fx\\ok.wav", 0);
    g_OK_WAVE.Wave::set_volume(0x3C);
    WaveDeviceGlobal.add_to_group(1, &g_OK_WAVE);
    effects_[0].init("fx\\psi attack.wav", 0);
    WaveDeviceGlobal.add_to_group(1, &effects_[0]);
    effects_[1].init("fx\\energy1.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[1]);
    effects_[2].init("projectile1.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[2]);
    effects_[3].init("projectile2.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[3]);
    effects_[56].init("fx\\mindworms", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[56]);
    effects_[59].init("fx\\planet buster lead.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[59]);
    effects_[60].init("fx\\planet buster.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[60]);
    effects_[57].init("fx\\cpu turn complete", 0x80);
    WaveDeviceGlobal.add_to_group(0, &effects_[57]);
    effects_[58].init("fx\\cpu new data", 0x80);
    WaveDeviceGlobal.add_to_group(0, &effects_[58]);
    effects_[61].init("fx\\psi blow", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[61]);
    effects_[62].init("fx\\boom", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[62]);
    effects_[64].init("fx\\infantry", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[64]);
    effects_[65].init("fx\\military land", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[65]);
    effects_[66].init("fx\\non military land", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[66]);
    effects_[67].init("fx\\grav ship", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[67]);
    effects_[68].init("fx\\hover", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[68]);
    effects_[69].init("fx\\military ship", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[69]);
    effects_[70].init("fx\\ship", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[70]);
    effects_[71].init("fx\\jet", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[71]);
    effects_[72].init("fx\\engine2", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[72]);
    effects_[75].init("fx\\missile", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[75]);
    effects_[4].init("wpn laser2.wav", 0xC0);
    effects_[4].set_pitch(1100);
    WaveDeviceGlobal.add_to_group(1, &effects_[4]);
    effects_[73].init("wpn particle.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[73]);
    effects_[74].init("wpn gatlin laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[74]);
    effects_[76].init("wpn chaos gun.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[76]);
    effects_[77].init("wpn fusion laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[77]);
    effects_[78].init("wpn tachyon bolt.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[78]);
    effects_[79].init("wpn plasma shard.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[79]);
    effects_[80].init("wpn quantum laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[80]);
    effects_[81].init("wpn graviton gun.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[81]);
    effects_[85].init("wpn Resonance Laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[85]);
    effects_[86].init("wpn res bolt.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[86]);
    effects_[87].init("wpn string disruptor.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[87]);
    effects_[90].init("wpn spore launcher.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[90]);
    effects_[88].init("wpn singularity laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[88]);
    effects_[89].init("wpn missile launcher.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[89]);
    effects_[82].init("wpn conventional payload.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[82]);
    effects_[83].init("wpn tetonic payload.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[83]);
    effects_[84].init("wpn fungal payload.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[84]);
    effects_[91].init("alien infantry.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[91]);
    effects_[92].init("alien non military.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[92]);
    effects_[93].init("alien ogre.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[93]);
    effects_[94].init("spore launcher.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[94]);
    effects_[95].init("sea lurker.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[95]);
    effects_[96].init("isle of the deep.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[96]);
    memset(AutoSoundDefaults, 0, sizeof(AutoSoundDefaults));

    g_CPU_WAVES[0].Wave::init("fx\\CPU pod recovered.wav", 0x80);
    g_CPU_WAVES[1].Wave::init("fx\\CPU data pod recovered.wav", 0x80);
    g_CPU_WAVES[2].Wave::init("fx\\CPU invalid command.wav", 0x80);
    g_CPU_WAVES[3].Wave::init("fx\\CPU invalid base site.wav", 0x80);
    g_CPU_WAVES[4].Wave::init("fx\\CPU invalid terraform.wav", 0x80);
    g_CPU_WAVES[5].Wave::init("fx\\CPU improved already.wav", 0x80);
    g_CPU_WAVES[6].Wave::init("fx\\CPU lacks ability.wav", 0x80);
    g_CPU_WAVES[7].Wave::init("fx\\CPU combat units only.wav", 0x80);
    g_CPU_WAVES[8].Wave::init("fx\\CPU nocando.wav", 0x80);
    g_CPU_WAVES[9].Wave::init("fx\\CPU insufficient energy.wav", 0x80);
    g_CPU_WAVES[10].Wave::init("fx\\CPU nn already linked.wav", 0x80);
    g_CPU_WAVES[11].Wave::init("fx\\CPU PT insufficient exp.wav", 0x80);
    g_CPU_WAVES[12].Wave::init("fx\\CPU system error.wav", 0x80);
    g_CPU_WAVES[13].Wave::init("fx\\CPU combat report.wav", 0x80);
    g_CPU_WAVES[14].Wave::init("fx\\CPU artifact lost.wav", 0x80);
    g_CPU_WAVES[15].Wave::init("fx\\CPU faction gone.wav", 0x80);
    g_CPU_WAVES[16].Wave::init("fx\\CPU indigenous lifeforms.wav", 0x80);
    g_CPU_WAVES[17].Wave::init("fx\\CPU improvement destroyed.wav", 0x80);
    g_CPU_WAVES[18].Wave::init("fx\\CPU enemy pt gone.wav", 0x80);
    g_CPU_WAVES[20].Wave::init("fx\\CPU terraform complete.wav", 0x80);
    g_CPU_WAVES[21].Wave::init("fx\\CPU mission accomplished.wav", 0x80);
    g_CPU_WAVES[22].Wave::init("fx\\CPU pt compromised.wav", 0x80);
    g_CPU_WAVES[23].Wave::init("fx\\CPU mission aborted.wav", 0x80);
    g_CPU_WAVES[24].Wave::init("fx\\CPU resource shortfall.wav", 0x80);
    g_CPU_WAVES[25].Wave::init("fx\\CPU prod complete.wav", 0x80);
    g_CPU_WAVES[26].Wave::init("fx\\CPU max population.wav", 0x80);
    g_CPU_WAVES[27].Wave::init("fx\\CPU new orders.wav", 0x80);
    g_CPU_WAVES[28].Wave::init("fx\\CPU drone riots.wav", 0x80);
    g_CPU_WAVES[29].Wave::init("fx\\CPU drone riots over.wav", 0x80);
    g_CPU_WAVES[30].Wave::init("fx\\CPU golden age begun.wav", 0x80);
    g_CPU_WAVES[32].Wave::init("fx\\CPU request confirmation.wav", 0x80);
    g_CPU_WAVES[33].Wave::init("fx\\CPU time up.wav", 0x80);
    g_CPU_WAVES[34].Wave::init("fx\\CPU units engaged.wav", 0x80);
    g_CPU_WAVES[35].Wave::init("fx\\CPU are you certain1.wav", 0x80);
    g_CPU_WAVES[36].Wave::init("fx\\CPU project initiated.wav", 0x80);
    g_CPU_WAVES[37].Wave::init("fx\\CPU project complete.wav", 0x80);
    g_CPU_WAVES[38].Wave::init("fx\\CPU enemy project initiated.wav", 0x80);
    g_CPU_WAVES[39].Wave::init("fx\\CPU unit lost.wav", 0x80);
    g_CPU_WAVES[40].Wave::init("fx\\CPU nutrient resources.wav", 0x80);
    g_CPU_WAVES[42].Wave::init("fx\\CPU mineral resources.wav", 0x80);
    g_CPU_WAVES[41].Wave::init("fx\\CPU energy resources.wav", 0x80);
    g_CPU_WAVES[43].Wave::init("fx\\CPU please don't go.wav", 0x80);
    g_CPU_WAVES[44].Wave::init("fx\\CPU review your options.wav", 0x80);
    for (int index = 0; index < 45; ++index) {
        WaveDeviceGlobal.add_to_group(0, &g_CPU_WAVES[index]);
    }
    g_ALPHAMENU_WAVE.Wave::load("fx\\amenu2.wav", 0x11);
    WaveDeviceGlobal.add_to_group(1, &g_ALPHAMENU_WAVE);
    g_ALPHAMENU_WAVE.Sound::set_pan(-32);
    g_ALPHAMENU_WAVE.Wave::set_volume(0x64);
    g_SCOOT_WAVE.Wave::load("fx\\scoot.wav", 0x11);
    WaveDeviceGlobal.add_to_group(1, &g_SCOOT_WAVE);
    g_PASSOVER_WAVE.Wave::load("fx\\passover.wav", 0x11);
    g_PASSOVER_WAVE.Wave::set_volume(0x3C);
    WaveDeviceGlobal.add_to_group(1, &g_PASSOVER_WAVE);
    g_MENU_UP_WAVE.Wave::load("fx\\menu up.wav", 0x10);
    WaveDeviceGlobal.add_to_group(1, &g_MENU_UP_WAVE);
    g_MENU_UP_WAVE.Wave::set_pitch(0xC8);
    g_MENU_UP_WAVE.Wave::set_volume(0x2D);
    g_MENU_DOWN_WAVE.Wave::load("fx\\menu down.wav", 0x10);
    WaveDeviceGlobal.add_to_group(1, &g_MENU_DOWN_WAVE);
    g_MENU_DOWN_WAVE.Wave::set_volume(0x64);
    g_MENU_DOWN_WAVE.Wave::set_pitch(0x12C);
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// RECOVERY_OBJECT: v1 0x00749CF8 ?g_FX@@3VFX@@A
FX g_FX;
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

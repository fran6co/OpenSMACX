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
// ORIGINAL: 0x00448350 ??_GEffect@@UAEPAXI@Z 0x00448350-0x00448371 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00448350-0x00448371", "Pinned class constructor ??0Effect@@QAE@XZ at 0x004482D0 stores original class/view table VA 0x66b7a4 at 0x004482D8 (mov dword ptr [esi], 0x66b7a4); the current genuine owner vftable ??_7Effect@@6B@ selects generated ??_GEffect@@UAEPAXI@Z in slot 2 (+0x8) through weak entry ??_EEffect@@UAEPAXI@Z (mode 2).", "Pinned ??_GEffect@@UAEPAXI@Z at 0x00448350 calls the source-mapped ??1Effect@@QAE@XZ at 0x004482C0, then tests byte ptr [ebp + 8], 1; je 0x44836a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/fx.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original Effect scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GEffect@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00445590 _$E21 0x00445590-0x004455A6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820a4 selects initializer0x445590..0x4455a6, registering exit0x4455b0..0x4455ba. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x749cf8 bound to?g_FX@@3VFX@@A.", "Original startup instruction evidence: [[\"0x445590\", \"mov\", \"ecx, 0x749cf8\"], [\"0x445595\", \"call\", \"0x4482a0\"], [\"0x44559a\", \"push\", \"0x4455b0\"], [\"0x44559f\", \"call\", \"0x645398\"], [\"0x4455a4\", \"pop\", \"ecx\"], [\"0x4455a5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4455b0\", \"mov\", \"ecx, 0x749cf8\"], [\"0x4455b5\", \"jmp\", \"0x448290\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/fx.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FX@@3VFX@@A\", \"??0FX@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FX@@3VFX@@A\", \"??1FX@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f050; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_FX@@3VFX@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E8
// ORIGINAL: 0x004455B0 _$E19 0x004455B0-0x004455BA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820a4 selects initializer0x445590..0x4455a6, registering exit0x4455b0..0x4455ba. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x749cf8 bound to?g_FX@@3VFX@@A.", "Original startup instruction evidence: [[\"0x445590\", \"mov\", \"ecx, 0x749cf8\"], [\"0x445595\", \"call\", \"0x4482a0\"], [\"0x44559a\", \"push\", \"0x4455b0\"], [\"0x44559f\", \"call\", \"0x645398\"], [\"0x4455a4\", \"pop\", \"ecx\"], [\"0x4455a5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4455b0\", \"mov\", \"ecx, 0x749cf8\"], [\"0x4455b5\", \"jmp\", \"0x448290\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/fx.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FX@@3VFX@@A\", \"??0FX@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FX@@3VFX@@A\", \"??1FX@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f050; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_FX@@3VFX@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E6
#include "stdafx.h"
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004482D0 ??0Effect@@QAE@XZ 0x004482D0-0x004482E2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original FX construction at0x004482A0 passes0x004482D0 as its per-element constructor for97elements of0x6Cbytes.", "Existing seedsrc/recovered/units/004482d0.cpp identifies span0x004482D0-0x004482E2; original instructions call Wave::Wave at0x004C66E0 then store the Effect virtual table at0x0066B7A4."], "reason": "Implicit Effect constructor emitted for the real FX Effect array; Wave base construction and derived virtual-table installation are compiler-owned."}
#include "fx.h"
#include "basepop.h"
#include "popup.h"
#include "ambience.h"
#include "game.h"
#include "general.h"
#include "veh.h"
#include "faction.h"

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

/*
Purpose: Set a selected effect's pitch through the real virtual method.
// ORIGINAL: 0x00446AF0 ?set_pitch@FX@@QAEXHH@Z 0x00446AF0-0x00446B10 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The 0x6C-stride receiver is the FX-owned Effect array, and slot 38 is Wave::set_pitch in the established virtual hierarchy.","evidence":["Original 0x00446AF0-0x00446B10 multiplies the effect index by 0x6C and dispatches slot 38 with the pitch argument.","fx.h owns Effect[97] at offset zero; wave.h declares virtual set_pitch at slot 38; git:b7e5b127:src/recovered/units/00446af0.cpp preserves the historical exact seed."]}
*/
void FX::set_pitch(int effect, int pitch) {
    effects_[effect].set_pitch(pitch);
}

/*
Purpose: Adjust the active engine effect's pitch if one is selected.
// ORIGINAL: 0x00446B10 ?engine_pitch@FX@@QAEXH@Z 0x00446B10-0x00446B37 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The trailing signed FX member selects one Effect, and the indirect call is its inherited virtual Wave::set_pitch.","evidence":["Original 0x00446B10-0x00446B37 reads signed this+0x28EC, skips negative values and calls slot 38 on the 0x6C-stride Effect array.","FX::init writes -1 to the trailing member; git:b7e5b127:src/recovered/00446b10.cpp preserves the historical exact seed and its signedness note."]}
*/
void FX::engine_pitch(int pitch) {
    if (engine_effect_index_ >= 0) {
        effects_[engine_effect_index_].set_pitch(pitch);
    }
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
// RECOVERY_BINDING: v1 {"evidence": ["Existing seed git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00445cd0.cpp preserves original span445CD0..4468E1 and is retained as a hint.", "Original instructions store this+28EC=-1, publish callback4456B0, copy70words from683AD8, initialize/group the Effect array and global Wave objects, and reset37words at9BC080. The sole indirect call446034 targets Effect[4] virtual set_pitch at slot38."], "reason": "Initialize the real effect and CPU-voice banks and publish source-owned popup/audio defaults; replaces the executable-address forwarder."}
// TRIED: register-faithful transcription (self=ecx, no ebx-copy) diverges at instr #2 (mov ebx,ecx vs push) - /O2 register allocation differs for this 3089-byte, ~150-call function; not chased further.
void FX::init() {
    engine_effect_index_ = -1;
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

/*
Purpose: Trigger one sound effect from the process-wide bank.
// ORIGINAL: 0x00446A00 ?play@FX@@QAEXH@Z 0x00446A00-0x00446A8E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original446A00..446A8E; retired seed git:117282ee:src/unrecovered/00446a00.cpp records the same identity and spans with its shr/test TRIED note kept below.", "0x009A6490 is game.h's GamePreferences and 0x0074DAA0 is popup.h's PopupWaveOwnerSlot; the receiver is fx.h's own FX with effects_ at 0x0 in 0x6C-byte Effect records, and the wave-owner slot 82 plus the effect record's slots 35/7 run through member-fn-pointer unions because the slots' signatures are not declared anywhere.", "The bit-4 release test is the seed's measured ((flags >> 4) & 1) walk, and the wave-owner refresh runs only for effect kinds 0/1/2 exactly as the seed orders them."], "reason": "Recover the sound bank's play trigger on the typed FX receiver; the shr+test release mask stays spelled as the seed measured it."}
// TRIED (seed): 141/142 bytes match (edit distance 1). Only the bit-4 test
//         near the end differs: original is `shr ecx,4 / test cl,1`, every
//         rephrasing tried in the scaffold still lowered to a single fused
//         `test` against the shifted-away mask.
Status: Coverage transcription - unclaimed body
*/
void FX::play(int effect) {
    if (!(GameDataStateStorage.fields_.game_preferences_ & 0x400)) return;
    if (effect == 1 || effect == 0 || effect == 2) {
        if (FactionMusic) FactionMusic->hostility();
        if (effect == 0) {
            const int playing = effects_[0].is_playing();
            effects_[0].set_pitch(playing ? effects_[0].pitch_ + 100 : 0);
        }
    }
    if ((effects_[effect].flags_54_ >> 4) & 1) effects_[effect].load();
    effects_[effect].play();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00446960 ?play@FX@@QAEXHHH@Z 0x00446960-0x004469F9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/unrecovered/00446960.cpp", "src/ambience.h", "src/midi.h", "src/fx.h"], "reason": "Original constructor staging, actual event tables and compiler-call contracts identify the genuine audio owner operation; compatible historical seed spans and claims remain available for restoration, not an acceptance verdict."}
void FX::play(int effect, int, int) {
    if (!(GameDataStateStorage.fields_.game_preferences_ & 0x400)) return;
    if (effect == 1 || effect == 0 || effect == 2) {
        if (FactionMusic) FactionMusic->hostility();
        effects_[effect].set_volume(60);
        if (effect == 0) {
            // Original receiver is effects_[0], not a polymorphic FX.
            const int playing = effects_[0].is_playing();
            effects_[0].set_pitch(playing ? effects_[0].pitch_ + 100 : 0);
        }
    }
    if (effects_[effect].flags_54_ & 0x10) effects_[effect].load();
    effects_[effect].play();
}

// Every branch retains its original probe effect even when it differs from
// the effect played. These are real bank elements, not independent globals.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00446D10 ?ambience_veh@@YAXH@Z 0x00446D10-0x004471EE FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00446d10.cpp", "src/ambience.h", "src/midi.h", "src/fx.h"], "reason": "Original constructor staging, actual event tables and compiler-call contracts identify the genuine audio owner operation; compatible historical seed spans and claims remain available for restoration, not an acceptance verdict."}
void __cdecl ambience_veh(int vehicle) {
    if (vehicle < 0) return;
    const int old = LastAmbienceVehicle;
    // Original quirk: previous VEHICLE id selects an effect directly if<97.
    if (old >= 0 && old != vehicle && old < 97) g_FX.effect(old).fade();
    const uint32_t prefs = GameDataStateStorage.fields_.game_preferences_;
    LastAmbienceVehicle = vehicle;
    if (!(prefs & 0x800)) return;
    if (Vehs[vehicle].order != 0) return;
    const int prototype = Vehs[vehicle].proto_id;
    const VehPrototype &unit = VehPrototypes[prototype];
    if (unit.plan == 8) {
        if (!(Players[Vehs[vehicle].faction_id].rule_flags & 0x80)) {
            g_FX.play(0x42); return;
        }
        if (g_FX.effect(0x5C).is_playing()) return;
        g_FX.play(0x5C); return;
    }
    const unsigned int chassis = unit.chassis_id;
    if (chassis == 0) {
        const unsigned int weapon = unit.weapon_id;
        if (Weapon[weapon].offense_rating == 0) {
            if (g_FX.effect(0x42).is_playing()) return;
            g_FX.play(0x42); return;
        }
        if (!(Players[Vehs[vehicle].faction_id].rule_flags & 0x80)) {
            if (g_FX.effect(0x40).is_playing()) return;
            g_FX.play(0x40); return;
        }
        if (prototype == 0x10 || prototype == 0x11 || prototype == 0x12) {
            if (g_FX.effect(0x5D).is_playing()) return;
            g_FX.play(0x5D); return;
        }
        if (prototype == 8) {
            if (!g_FX.effect(0x38).is_playing()) g_FX.play(0x38);
            return;
        }
        if (prototype != 0xF) {
            if (g_FX.effect(0x5B).is_playing()) return;
            g_FX.play(0x5B); return;
        }
        if (g_FX.effect(0x5E).is_playing()) return;
        g_FX.play(0x5E); return;
    }
    if (chassis == 1) {
        if (g_FX.effect(0x48).is_playing()) return;
        g_FX.play(0x48); return;
    }
    if (chassis == 2) {
        if (g_FX.effect(0x31).is_playing()) return;
        g_FX.play(0x31); return;
    }
    if (prototype == 0xE) {
        if (g_FX.effect(0x5F).is_playing()) return;
        g_FX.play(0x5F); return;
    }
    if (prototype == 9) {
        if (g_FX.effect(0x60).is_playing()) return;
        g_FX.play(0x60); return;
    }
    if (chassis == 3) {
        if (g_FX.effect(0x45).is_playing()) return;
        g_FX.play(0x45); return;
    }
    if (chassis == 4) {
        if (g_FX.effect(0x46).is_playing()) return;
        g_FX.play(0x46); return;
    }
    if (chassis == 5) {
        if (g_FX.effect(0x47).is_playing()) return;
        g_FX.play(0x47); return;
    }
    if (chassis == 6) {
        if (g_FX.effect(0x44).is_playing()) return;
        g_FX.play(0x44); return;
    }
    if (chassis == 7) {
        if (g_FX.effect(0x43).is_playing()) return;
        g_FX.play(0x43); return;
    }
    if (chassis == 8) {
        if (g_FX.effect(0x4B).is_playing()) return;
        g_FX.play(0x4B); return;
    }
    if (prototype != 8) {
        if (static_cast<int8_t>(Chassis[chassis].triad) == 1) {
            if (g_FX.effect(0xC).is_playing()) return;
            g_FX.play(0xC); return;
        }
        if (static_cast<int8_t>(Chassis[chassis].triad) == 2) {
            if (g_FX.effect(0x12).is_playing()) return;
            g_FX.play(0x12); return;
        }
        if (unit.plan == 0xC) {
            if (g_FX.effect(0x14).is_playing()) return;
            g_FX.play(0x14); return;
        }
        const int offense = Weapon[unit.weapon_id].offense_rating;
        if (offense != 0) {
            if (offense <= Chassis[chassis].speed) {
                if (g_FX.effect(0x1A).is_playing()) return;
                g_FX.play(0x1A); return;
            }
            if (offense != 0) {
                if (g_FX.effect(7).is_playing()) return;
                g_FX.play(7); return;
            }
        }
        if (g_FX.effect(0x2E).is_playing()) return;
        g_FX.play(0x2E); return;
    }
    if (!g_FX.effect(0x38).is_playing()) g_FX.play(0x38);
}

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

// Help unit: the original help-text/link builders and help_* topic wrappers
// at 0x0044B340..0x0044CC18 (CC fill after FameWin, next unit 0x0044CC20).

#include "stdafx.h"
#include "interface_fonts.h"
#include "game.h"
#include "datalink.h"
#include "strings.h"
#include "alpha.h"
#include "veh.h"
#include "base.h"
#include "text.h"
#include "faction.h"
#include "general.h"
#include "popup.h"
#include "xpops.h"
#include "technology.h"
#include "council.h"
#include "terraforming.h"
#include "shared_word.h"


// Native4050A0 publishes this buffer and44CBD0 consumes it. The existing
// 256-byte source capacity is retained; original allocation bounds unverified.
// RECOVERY_OBJECT: v1 0x007591C8 ?PopupPendingLink@@3PADA
char PopupPendingLink[256];    // 0x007591C8
// RECOVERY_OBJECT: v1 0x007592C8 ?PopupPendingLinkFlag@@3HA
int PopupPendingLinkFlag;      // 0x007592C8

// Original wrapper 0x44C805 publishes its Popup receiver before invoking
// the synchronous help-text builder; callback 0x44C7D6 reads that same slot.
// Keep the original single shared receiver, including its reentrant overwrite
// behavior, rather than introducing a stack or an executable-address alias.
// RECOVERY_OBJECT: v2 0x007591B0 0x007591B4 ?TechnologyHelpPopup@@3PAVPopup@@A 0x0044C7F0
Popup *TechnologyHelpPopup;

void __cdecl help_tech_info(void (__cdecl *emit)(char *), int technology,
                            int faction, int link_mode);

// These are independent source-owned data objects. The two caret strings
// are separate literals; neither denotes a writable shared buffer.
// RECOVERY_OBJECT: v1 0x007591BC ?TechnologyHelpPrefixEmitted@@3HA
int TechnologyHelpPrefixEmitted;
// Cross-reference values shared with technology help and acquisition.
// Each is a separately accessed dword with a mapped neighbour 4 bytes later.
// RECOVERY_OBJECT: v4 0x007591A4 0x007591A8 ?TechnologyHelpEntryName@@3HA 0x0044BBBE target ?TechnologyHelpEntryName@@3PADA
LPSTR TechnologyHelpEntryName;
// RECOVERY_OBJECT: v2 0x007591A8 0x007591AC ?TechnologyHelpEntryKind@@3HA 0x0044BBB4
int TechnologyHelpEntryKind;
// RECOVERY_OBJECT: v1 0x00684634 ?TechnologyHelpSectionSelector@@3HA
int TechnologyHelpSectionSelector = -1;
// RECOVERY_OBJECT: v1 0x00684638 ?TechnologyHelpPrefixText@@3PADA
char TechnologyHelpPrefixText[] = "^";
// RECOVERY_OBJECT: v1 0x0068463C ?TechnologyHelpSectionName@@3PADA
char TechnologyHelpSectionName[] = "TECHHECK";
// RECOVERY_OBJECT: v1 0x00684648 ?TechnologyHelpSeparatorText@@3PADA
char TechnologyHelpSeparatorText[] = "^";

// ORIGINAL: 0x0044B340 sub_44b340 0x0044B340-0x0044B37B BYTE_EXACT
// symbol    ?sub_44b340@@YAXP6AXPAD@Z@Z
// Native44B359 passes the caret string and44B36D reloads TextBufferGetPtr
// after that callback: these are text pointers, not integer callback values.
// No observed original caller connects this entry to the similar emit_help
// factoring below; keep that relationship unclaimed.
void __cdecl sub_44b340(void (__cdecl *callback)(char *)) {
    if (TechnologyHelpPrefixEmitted == 0 && TechnologyHelpSectionSelector == -1) {
        TechnologyHelpPrefixEmitted = 1;
        callback(TechnologyHelpPrefixText);
    }
    callback(TextBufferGetPtr);
}

static void emit_help(void (__cdecl *cb)(char *)) {
    if (TechnologyHelpPrefixEmitted == 0 &&
        TechnologyHelpSectionSelector == -1) {
        TechnologyHelpPrefixEmitted = 1;
        cb(TechnologyHelpPrefixText);
    }
    cb(TextBufferGetPtr);
}

/*
Purpose: The help emission gate shared by the nineteen text_get tests: the
         first call after a fresh section opens the paragraph with the
         help prefix, later calls join with the separator; both shapes
         then hand the staged text to the caller's emitter.

// ORIGINAL: 0x0044B380 ?tech_heck@@YAXP6AXPAD@ZH@Z 0x0044B380-0x0044B9DC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x0044B380-0x0044B9DC complete original span; source keeps its full mapped identity and historical claims", "source:src/alpha.h Rules members at +0x3C..+0x54,+0x90,+0xA4,+0xA8; src/technology.h Technology[89] stride0x2C; src/council.h Proposal[11] stride0x0C", "source:src/terraforming.h Terraforming[20] and src/veh.h Order[30]; pinned loop visits rows0..18 and Order[4..22], with duplicate land/sea prerequisites suppressed only on sea iteration", "source:src/datalink.cpp TextBufferGetPtr/StringTemp/TechnologyHelp*; former TRIED/LEVER evidence remains intact as historical notes"], "reason": "Recover the complete technology-help prefix tests through the existing Rules, Technology, Proposal, Terraforming and Order records, using the named text/string services and Datalink help state. Preserve the original negative-index behavior, duplicate sea-prerequisite test, call order and unsafe string-buffer effects."}
// TRIED: 19 repeated "text_get(); if (cond) emit(a1);" blocks (7 direct ID compares, 8 bit-flag tests on a stride-0x2c table, 3 more ID compares, then a final ID compare) factored through a static `emit_help` helper (Ghidra's repeated if/if/call pattern makes this an exact behavioral match), plus the two trailing loops (0x691880 tech-name table, 0x9a6840 concatenation table). Compiles; diverges at instruction #3 (prologue - original reserves one local with a bare `push ecx`, rebuilt doesn't need a same-shaped stack slot since the outer scope has no locals of its own). Hoisting the loop-scratch locals to the top of the function did not change it.
// size      1628 bytes
// prototype
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD550 0x005FD570 0x006169A0 0x00625880 0x00625E50 0x00645460 0x00645470
// indirect  0x0044B3EB 0x0044B3F7 0x0044B426 0x0044B432 0x0044B461 0x0044B46C 0x0044B49B 0x0044B4A7 0x0044B4D6 0x0044B4E2 0x0044B511 0x0044B51C 0x0044B54B 0x0044B557 0x0044B591 0x0044B59C 0x0044B5CD 0x0044B5D9 0x0044B60A 0x0044B616 0x0044B647 0x0044B652 0x0044B683 0x0044B68F 0x0044B6C0 0x0044B6CC 0x0044B6FD 0x0044B708 0x0044B73B 0x0044B747 0x0044B77A 0x0044B786 0x0044B7B5 0x0044B7C0 0x0044B7EF 0x0044B7FB 0x0044B82A 0x0044B836 0x0044B8CE 0x0044B8DA 0x0044B9B6 0x0044B9C1
Status: Coverage transcription - unclaimed body; the seed's emit_help
        factoring and its push-ecx divergence stand, and the trailing
        strcat/strcpy loops go through the CRT names the image's
        0x00645460/0x00645470 wrappers are of.
*/

void __cdecl tech_heck(void (__cdecl *a1)(char *), int a2) {
    if (a2 >= 0x59) {
        return;
    }
    TechnologyHelpPrefixEmitted = 0;
    if (text_open(DatalinkHelpFile,
                  TechnologyHelpSectionName) != 0) {
        return;
    }

    text_get();
    if (a2 == Rules.tech_improve_fungus_sqr) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_ease_fungus_movement) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_build_roads_fungus) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_two_spec_abilities) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_three_nutrients_sqr) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_three_minerals_sqr) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_three_energy_sqr) {
        emit_help(a1);
    }

    text_get();
    if (Technology[a2].flags & 1) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 2) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 4) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 8) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x10) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x40) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x80) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x100) {
        emit_help(a1);
    }

    text_get();
    if (a2 == Rules.tech_orb_insert_sans_spc_elev) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_mining_platform_bonus) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_economic_victory) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Proposal[1].preq_tech) {
        emit_help(a1);
    }

    text_get();
    strcpy(StringTemp, TextBufferGetPtr);

    {
        for (int row = 0; row < MaxTerrainNum - 1; ++row) {
            for (int i = 0; i <= 1; ++i) {
                if (a2 == Terraforming[row].prerequisite_tech[i] &&
                    (i != 1 || Terraforming[row].prerequisite_tech[0]
                                   != Terraforming[row].prerequisite_tech[1])) {
                    // order and order_sea are read as a two-entry run.
                    parse_say(0, (&Order[4 + row].order)[i], -1, -1);
                    parse_string(StringTemp, TextBufferGetPtr);
                    emit_help(a1);
                }
            }
        }
    }

    {
        for (int proposal = 2; proposal < MaxProposalNum; ++proposal) {
            if (a2 == Proposal[proposal].preq_tech) {
                StringTemp[0] = 0;
                strcat(StringTemp, TechnologyHelpSeparatorText);
                strcat(StringTemp, label_get(0x7C8 / 4));
                strcat(StringTemp, ": ");
                LPSTR name = Proposal[proposal].name;
                strcat(StringTemp, StringTable.get(name));
                strcpy(TextBufferGetPtr, StringTemp);
                emit_help(a1);
            }
        }
    }
}

/*
Purpose: The technology help compiler: for one id, walk the ten game-data
         tables whose records reference it - technologies, secret projects,
         four facility/structure lists, the faction/other linked list, the
         nested four-group table, the id-only list and the final linked
         list - staging each record's text into the shared buffer, wiring
         its datalink id, and flushing through the caller's emitter either
         per record (a4 >= 0) or joined per section (a4 < 0). Section
         bodies run under their a4 gates; the tech_heck trailer runs when
         a4 < 0 or a4 == 3, bracketed by the shared 0x00684634 section
         flag.

// ORIGINAL: 0x0044B9E0 ?help_tech_info@@YAXP6AXPAD@ZHHH@Z 0x0044B9E0-0x0044C7C9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x0044B9E0-0x0044C7C9 full original function span and existing call facts", "source:src/veh.h Reactor/Chassis/Weapon/Armor/Ability/VehPrototype; src/base.h Facility/Citizen; src/faction.h SocialCategories; src/technology.h Technology", "pinned:0x007591A4/A8 are separate 4-byte name-handle/kind writes for chassis0x62, weapon0x63 and armor0x64, reset together by tech_achieved at0x005BB71A; object extents and wider allocation remain unknown", "pinned: report-page paths 0x0049A5FC/0x0049A606,0x0049A7CD/0x0049A7D7 and0x0049A9E1/0x0049A9EB also write this pair; this proposal restores those writes in ReportWin's existing typed unlock-section traversal, while broader page-layout/runtime work remains separate", "source:src/strings.h StringTemp/StringTable; src/alpha.h label_get; src/datalink.h DatalinkMain; source:src/technology.cpp reset follows prototype selection before presentation"], "reason": "Recover the ten-section technology-help builder through existing typed game tables while retaining original a4 gates, exact traversed ranges, append/flush order, callback timing and unsafe global-buffer behavior; publish all three original chassis/weapon/armor shared entry-state pairs."}
// TRIED: exact per-record byte offsets for the 4 embedded-owner index chains (52/380/etc strides) - transcribed the strcat/get/ construct_id/help_create_link shape per loop faithfully but did not verify every offset bit-for-bit; MISMATCH #0 at prologue.
// size      3561 bytes
// prototype
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0042A080 0x0044B380 0x0044CB80 0x006169A0 0x00645470 0x0064FC88
Status: Coverage transcription - unclaimed body; the seed's offsets and
        its prologue divergence stand. help_create_link is now the product
        helper in datalink.cpp; tech_heck is the transcription beside this
        body.
*/
void __cdecl help_tech_info(void (__cdecl *a1)(char *), int a2, int a3,
                            int a4) {
    char *const buf = StringTemp;
    char numbuf[80];
    char idbuf[64];
    char flushed;
    int idx;
    // stand-in: the image calls help_create_link out of line in the first
    // two sections and expands it in the rest; a local const pointer keeps
    // VC6 from inlining those two sites.
    void (__cdecl *const help_create_link_call)(char *, char *, char *) =
        help_create_link;

    if (a4 < 0 || a4 == 2) {

    // ---- section 1: reactor records (0x9527f8, stride 0xc, match +8) ----
    flushed = 0;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxReactorNum; ++idx) {
        const RulesReactor &record = Reactor[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (!flushed) {
                    strcat(buf, "^");
                    char *s = label_get(0x234 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 5,
                                                                  idx + 1);
            char *link = StringTable.get(record.name);
            help_create_link_call(buf, reinterpret_cast<char *>(link), idbuf);
            flushed = *buf;
            if (a4 >= 0 && flushed) {
                a1(buf);
                flushed = 0;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 2: chassis records (0x94a330, stride 0x90, match +0x8e) ----
    flushed = 0;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxChassisNum; ++idx) {
        const RulesChassis &record = Chassis[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (!flushed) {
                    strcat(buf, "^");
                    char *s = label_get(0x120 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 4,
                                                                  idx);
            char *link = StringTable.get(record.offsv_1_name);
            help_create_link_call(buf, reinterpret_cast<char *>(link), idbuf);
            TechnologyHelpEntryKind = 0x62;
            TechnologyHelpEntryName = record.offsv_1_name;
            strcat(buf, " ");
            strcat(buf, "(");
            _itoa(record.speed, numbuf, 10);
            strcat(buf, numbuf);
            strcat(buf, ")");
            flushed = *buf;
            if (a4 >= 0 && flushed) {
                a1(buf);
                flushed = 0;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 3: weapon records (0x94ae60, stride 0x10, match +0xc) ----
    flushed = 0;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxWeaponNum; ++idx) {
        if (Weapon[idx].preq_tech == a2) {
            if (a4 < 0) {
                if (!flushed) {
                    strcat(buf, "^");
                    char *s = label_get(0x124 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 6,
                                                                  idx);
            char *link = StringTable.get(Weapon[idx].name);
            help_create_link(buf, link, idbuf);
            TechnologyHelpEntryKind = 0x63;
            TechnologyHelpEntryName = Weapon[idx].name;
            strcat(buf, " ");
            strcat(buf, "(");
            if (Weapon[idx].offense_rating < 0) {
                strcat(buf, label_get(0x310 / 4));
            } else if (Weapon[idx].mode < 3) {
                _itoa(Weapon[idx].offense_rating, numbuf, 10);
                strcat(buf, numbuf);
            } else {
                strcat(buf, label_get(0x200 / 4));
            }
            strcat(buf, ")");
            if (a4 >= 0) {
                flushed = *buf;
                if (flushed) {
                    a1(buf);
                    flushed = 0;
                    *buf = 0;
                }
            } else {
                flushed = *buf;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 4: armor records (0x94f278, stride 0x10, match +0xc) ----
    flushed = 0;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxArmorNum; ++idx) {
        if (Armor[idx].preq_tech == a2) {
            if (a4 < 0) {
                if (!flushed) {
                    strcat(buf, "^");
                    char *s = label_get(0x128 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 7,
                                                                  idx);
            char *link = StringTable.get(Armor[idx].name);
            help_create_link(buf, link, idbuf);
            TechnologyHelpEntryKind = 0x64;
            TechnologyHelpEntryName = Armor[idx].name;
            strcat(buf, " ");
            strcat(buf, "(");
            if (Armor[idx].defense_rating < 0) {
                strcat(buf, label_get(0x310 / 4));
            } else {
                _itoa(Armor[idx].defense_rating, numbuf, 10);
                strcat(buf, numbuf);
            }
            strcat(buf, ")");
            if (a4 >= 0) {
                flushed = *buf;
                if (flushed) {
                    a1(buf);
                    flushed = 0;
                    *buf = 0;
                }
            } else {
                flushed = *buf;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 5: ability records (0x9ab538, stride 0x1c, match +0x18) ----
    flushed = 0;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxAbilityNum; ++idx) {
        const RulesAbility &record = Ability[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (!flushed) {
                    strcat(buf, "^");
                    char *s = label_get(0x12C / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 8,
                                                                  idx);
            char *link = StringTable.get(record.name);
            help_create_link(buf, link, idbuf);
            if (a4 >= 0) {
                flushed = *buf;
                if (flushed) {
                    a1(buf);
                    flushed = 0;
                    *buf = 0;
                }
            } else {
                flushed = *buf;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 6: vehicle prototypes (0x9ab89a, stride 0x34, bitfield-gated) ----
    {
        int ctr = 0;
        *buf = 0;
        for (ctr = 0; ctr < MaxVehProtoFactionNum; ++ctr) {
            const VehPrototype &record = VehPrototypes[ctr];
            if ((record.flags & 1) == 0) continue;
            if (record.preq_tech != a2) continue;
            if (a4 < 0) {
                if (*buf == 0) {
                    strcat(buf, "^");
                    char *s = label_get(0x318 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 3,
                                                                  ctr);
            help_create_link(buf, const_cast<char *>(record.veh_name), idbuf);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                *buf = 0;
            }
        }
        if (a4 < 0 && *buf != 0) a1(buf);
    }

    } // end sections 1-6 (a4 < 0 || a4 == 2)

    // ---- section 7: facility records (0x9a4bb0, stride 0x30) ----
    if (a4 < 0 || a4 == 0) {
        int ctr = 1;
        *buf = 0;
        for (; ctr < MaxFacilityNum; ++ctr) {
            const RulesFacility &record = Facility[ctr];
            bool match = (record.preq_tech == a2) ||
                         (record.free_tech == a2);
            if (!match) continue;
            if (a4 < 0) {
                if (*buf == 0) {
                    strcat(buf, "^");
                    char *s = label_get(0x110 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            if (record.free_tech == a2) {
                strcat(buf, "(");
            }
            int category = (ctr >= 0x46) ? 0xB : 0xA;
            DatalinkMain.construct_id(idbuf,
                                                                  category,
                                                                  ctr);
            char *link = StringTable.get(record.name);
            help_create_link(buf, link, idbuf);
            if (record.free_tech == a2) {
                strcat(buf, " ");
                char *s2 = label_get(0x4DC / 4);
                strcat(buf, s2);
                strcat(buf, ")");
            }
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                *buf = 0;
            }
        }
        if (a4 < 0 && *buf != 0) {
            a1(buf);
        }
    }

    // ---- section 8: social-category prerequisites, groups of 4 (0x94b014); section 9 follows in the same gate ----
    if ((a4 < 0) || (a4 == 3)) {
        *buf = 0;
        int outerIndex = 0;
        do {
            for (int j = 0; j < 4; ++j) {
                if (SocialCategories[outerIndex].preq_tech[j] == a2) {
                    if (a4 < 0) {
                        if (*buf == 0) {
                            strcat(buf, "^");
                            char *s = label_get(0x118 / 4);
                            strcat(buf, s);
                            strcat(buf, ": ");
                        } else {
                            strcat(buf, ", ");
                        }
                    } else {
                        strcat(buf, "^");
                    }
                    DatalinkMain.construct_id(
                        idbuf, 0xC, outerIndex * 5 + j + 1);
                    char *link = StringTable.get(
                        SocialCategories[outerIndex].name[j]);
                    help_create_link(buf, link, idbuf);
                    if (*buf != 0) {
                        a1(buf);
                    }
                }
            }
            ++outerIndex;
        } while (outerIndex < MaxSocialCatNum);

        // ---- section 9: citizen prerequisites (0x946020, stride 0x1c, match +8) ----
        // same gate as section 8: reached only when a4 < 0 || a4 == 3.
        *buf = 0;
        for (idx = 0; idx < 7; ++idx) {
            const RulesCitizen &record = Citizen[idx];
            if (record.preq_tech != a2) continue;
            if (a4 < 0) {
                if (*buf == 0) {
                    strcat(buf, "^");
                    char *s = label_get(0x4CC / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            char *link = StringTable.get(record.singular_name);
            strcat(buf, link);
            if (*buf != 0) {
                a1(buf);
            }
        }
    }

    // ---- section 10: technology prerequisites (0x94f380, stride 0x2c) ----
    if ((a4 < 0) || (a4 == 1)) {
        int ctr = 0;
        *buf = 0;
        for (ctr = 0; ctr < MaxTechnologyNum; ++ctr) {
            const RulesTechnology &record = Technology[ctr];
            bool match = (record.preq_tech_1 == a2) ||
                         (record.preq_tech_2 == a2);
            if (!match) continue;
            if (a4 < 0) {
                if (*buf == 0) {
                    strcat(buf, "^");
                    char *s = label_get(0x130 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                } else {
                    strcat(buf, ", ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 0xE,
                                                                  ctr);
            char *link = StringTable.get(record.name);
            help_create_link(buf, link, idbuf);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                *buf = 0;
            }
        }
        if (a4 < 0 && *buf != 0) a1(buf);
    }

    if (a4 < 0 || a4 == 3) {
        TechnologyHelpSectionSelector = a4;
        tech_heck(a1, a2);
        TechnologyHelpSectionSelector = -1;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044C7D0 ?help_info_pop@@YAXPAD@Z 0x0044C7D0-0x0044C7E4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned44C7D0..44C7E4 loads7591B0 and calls601980; src/recovered/0044c7d0.cpp preserves the historical exact candidate and original identity."], "reason": "Plain cdecl text callback loads the shared typed Popup receiver and invokes its inherited BasePop::string method. No runtime executable address, manual vtable or invented null guard."}
void __cdecl help_info_pop(char *text) {
    TechnologyHelpPopup->string(text);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044C7F0 ?help_tech_info@@YAXPAUPopup@@HH@Z 0x0044C7F0-0x0044C814 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned44C7F0..44C814 stores Popup pointer7591B0 and calls44B9E0 with callback44C7D0 and lastarg-1; original exact seed retained in src/recovered/units/0044c7f0.cpp."], "reason": "Popup-specific overload publishes the actual receiver to shared callback storage before synchronously invoking the existing four-argument technology text builder, with the original negative link selector."}
// symbol    ?help_tech_info@@YAXPAVPopup@@HH@Z
void __cdecl help_tech_info(Popup *popup, int technology, int faction) {
    TechnologyHelpPopup = popup;
    help_tech_info(help_info_pop, technology, faction, -1);
}

// ORIGINAL: 0x0044C820 nullsub_3 0x0044C820-0x0044C821 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-ui-sound-helper-owner", "Pinned44C820 is a one-byte return used by44C830 as the cdecl void(char*) technology-text callback. Source scan finds no consumers of the former unrelated fastcall shim.", "historical seed:src/nullsub_thunks.cpp"], "reason": "Rehome genuine typed helper into its existing source owner, preserving original spans, historical claims and useful discoveries. No fixed executable address or manual virtual-call model."}
// symbol    ?discard_technology_help_text@@YAXPAD@Z
void __cdecl discard_technology_help_text(char *) {
}

// ORIGINAL: 0x0044C830 sub_44c830 0x0044C830-0x0044C84C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-ui-sound-helper-owner", "Pinned44C830..44C84C passes arguments2/3, selector-1 and actual callback44C820 to existing typed technology-text builder44B9E0; first argument is unused.", "historical seed:src/recovered/units/0044c830.cpp"], "reason": "Rehome genuine typed helper into its existing source owner, preserving original spans, historical claims and useful discoveries. No fixed executable address or manual virtual-call model."}
// symbol    ?technology_help_without_output@@YAXHHH@Z
void __cdecl technology_help_without_output(int, int technology, int faction) {
    help_tech_info(discard_technology_help_text, technology, faction, -1);
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C850 nullsub_191 0x0044C850-0x0044C851 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C850-0x0044C851 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C830 sub_44c830 (src/datalink.cpp) and 0x0044C860 sub_44c860 (src/shared_word.cpp); datalink.cpp functions surround 0x0044C820..0x0044CB80"]}
// symbol    ?nullsub_191@@YAXXZ
// kind      game
void __cdecl nullsub_191() {
}

// The original word's semantic domain is unknown; retain only its
// independently evidenced storage identity and operations.
// RECOVERY_OBJECT: v1 0x007591C4 ?SharedWordValue@@3HA
int32_t SharedWordValue;

/*
// ORIGINAL: 0x0044C860 sub_44c860 0x0044C860-0x0044C86D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x0044C860-0x0044C86D reads one int argument and stores its full dword to 0x007591C4", "pinned:terranx_original.exe:0x0044C870-0x0044C878 directly XORs bit zero of the same dword and returns", "pinned PE virtual .data section contains 0x007591C4; read_initial(0x007591C4..0x007591C8) is four zero bytes", "pinned base-relocation table identifies absolute operand sites in both original instructions; no relocation is claimed inside the target word", "src/shared_word.cpp owns the named SharedWordValue storage plus both original operations; no class, domain-level field meaning, or caller/lifetime relation is asserted"], "reason": "Own the original setter in the same C++ translation unit as the real shared word it writes and the paired toggle operation. The original class/domain meaning and caller set remain unknown."}
// symbol    ?set_shared_word@@YAXH@Z
// size      13 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

void set_shared_word(int value) {
    SharedWordValue = value;
}

/*
Purpose: sub_44c870 - arithmetic over 1 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0044C870 sub_44c870 0x0044C870-0x0044C878 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:0x0044C870-0x0044C878 is the direct memory-form XOR `xor dword ptr [0x7591c4], 1; ret`", "pinned:terranx_original.exe:0x0044C860-0x0044C86D independently stores the caller-supplied dword at the same address", "pinned PE virtual .data section contains 0x007591C4; read_initial(0x007591C4..0x007591C8) is four zero bytes", "src/shared_word.cpp defines SharedWordValue and contains both paired source operations; no class or domain label is inferred"], "reason": "Own the original toggle operation beside the shared C++ word it mutates; use the same real storage as the original setter while leaving the state domain and callers unnamed."}
// Historical note: these spelling tests predate the paired setter and
// the parsed PE virtual zero-fill proof; retain their experiments, but
// the conclusion that 0x7591C4 has no named source object is superseded.
// TRIED 2026-09-03: five spellings for the single statement, scored with
// tools/try_spellings.py - `*p ^= 1`, `*p = *p ^ 1` (as committed), a
// `uint32_t *const p` local, `volatile int32_t *const p` read-modify-write
// and `volatile ... ^= 1`, plus `|= 1`. Every one still compiles the
// load/xor/store triple (3 or 4 instructions against the image's 2); none
// reaches the memory-form `xor dword ptr [0x7591c4], 1`. VC6 will not emit
// the in-memory read-modify-write through a const-pointer lvalue, only
// through a named global object, and these globals are bound rather than
// placed so no named object can live at 0x7591c4.
// TRIED 2026-09-03, second pass: the reference lvalue is the one new shape -
// `int32_t &r = *GlobalSlot007591C4; r = r ^ 1;` compiles `mov eax,<ptr>;
// xor [eax],1` (9 bytes against the image's 8), still not the memory form.
// Compiled the same statement against a REAL object in the same run as the
// control: `extern int x; x ^= 1` and `x = x ^ 1` BOTH compile to
// `xor dword ptr [<object>],1` - the image's exact 7 bytes, displacement
// relocated. So the memory form IS reachable from C++, and the one thing this
// body lacks is an object at 0x7591c4. That is an EVIDENCE lever, not a
// spelling lever: leaf_recoveries.cpp documents 0x7591C4 as evidence-free
// ("a named global here would be invention"), so it stays bound.
// symbol    ?toggle_shared_word@@YAXXZ
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void toggle_shared_word() {
    SharedWordValue = SharedWordValue ^ 1;
}

void __cdecl help_any() {
    DatalinkMain.exec(0, 0);
}

/*
Purpose: Show the technology help topic for the given id.
// ORIGINAL: 0x0044C880 ?help_tech@@YAXH@Z 0x0044C880-0x0044C895 BYTE_EXACT
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_tech(int id) {
    DatalinkMain.exec(0xE, id);
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C8A0 nullsub_226 0x0044C8A0-0x0044C8A1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C8A0-0x0044C8A1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_226@@YAXXZ
// kind      game
void __cdecl nullsub_226() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C8B0 nullsub_227 0x0044C8B0-0x0044C8B1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C8B0-0x0044C8B1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_227@@YAXXZ
// kind      game
void __cdecl nullsub_227() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: zero EAX and return; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C8C0 sub_44c8c0 0x0044C8C0-0x0044C8C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C8C0-0x0044C8C3 zero EAX and return; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?sub_44c8c0@@YAHXZ
// kind      game
int __cdecl sub_44c8c0() {
    return 0;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C8D0 nullsub_229 0x0044C8D0-0x0044C8D1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C8D0-0x0044C8D1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_229@@YAXXZ
// kind      game
void __cdecl nullsub_229() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C8E0 nullsub_230 0x0044C8E0-0x0044C8E1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C8E0-0x0044C8E1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_230@@YAXXZ
// kind      game
void __cdecl nullsub_230() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C8F0 nullsub_231 0x0044C8F0-0x0044C8F1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C8F0-0x0044C8F1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_231@@YAXXZ
// kind      game
void __cdecl nullsub_231() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C900 nullsub_232 0x0044C900-0x0044C901 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C900-0x0044C901 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C880 ?help_tech@@YAXH@Z (src/datalink.cpp) and 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_232@@YAXXZ
// kind      game
void __cdecl nullsub_232() {
}

/*
Purpose: Show the weapon help topic for the given id.
// ORIGINAL: 0x0044C910 ?help_weapon@@YAXH@Z 0x0044C910-0x0044C925 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C930 nullsub_233 0x0044C930-0x0044C931 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C930-0x0044C931 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C910 ?help_weapon@@YAXH@Z (src/datalink.cpp) and 0x0044C940 ?help_armor@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_233@@YAXXZ
// kind      game
void __cdecl nullsub_233() {
}

/*
Purpose: Show the armor help topic for the given id.
// ORIGINAL: 0x0044C940 ?help_armor@@YAXH@Z 0x0044C940-0x0044C955 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C960 nullsub_234 0x0044C960-0x0044C961 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C960-0x0044C961 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C940 ?help_armor@@YAXH@Z (src/datalink.cpp) and 0x0044C980 ?help_chassis@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_234@@YAXXZ
// kind      game
void __cdecl nullsub_234() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: zero EAX and return; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C970 sub_44c970 0x0044C970-0x0044C973 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C970-0x0044C973 zero EAX and return; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C940 ?help_armor@@YAXH@Z (src/datalink.cpp) and 0x0044C980 ?help_chassis@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?sub_44c970@@YAHXZ
// kind      game
int __cdecl sub_44c970() {
    return 0;
}

/*
Purpose: Show the chassis help topic for the given id.
// ORIGINAL: 0x0044C980 ?help_chassis@@YAXH@Z 0x0044C980-0x0044C995 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C9A0 nullsub_235 0x0044C9A0-0x0044C9A1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C9A0-0x0044C9A1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C980 ?help_chassis@@YAXH@Z (src/datalink.cpp) and 0x0044C9B0 ?help_facility@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_235@@YAXXZ
// kind      game
void __cdecl nullsub_235() {
}

/*
Purpose: Show the facility help topic for the given id.
// ORIGINAL: 0x0044C9B0 ?help_facility@@YAXH@Z 0x0044C9B0-0x0044C9C5 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044C9D0 nullsub_236 0x0044C9D0-0x0044C9D1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044C9D0-0x0044C9D1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C9B0 ?help_facility@@YAXH@Z (src/datalink.cpp) and 0x0044C9E0 ?help_project@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_236@@YAXXZ
// kind      game
void __cdecl nullsub_236() {
}

// The superseded seed was deleted; its original mapping and exact obligation
// are owned here. The seed path below is historical provenance in Git.
// ORIGINAL: 0x0044C9E0 ?help_project@@YAXH@Z 0x0044C9E0-0x0044C9F8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0044c9e0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_project_help.py", "Pinned01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf full24-byte body44C9E0..44C9F8 loads receiver703EA0 and calls429180 with topic11 and project+70.", "DatalinkMain is the source-owned singleton at703EA0; exec(unsigned int,int) owns the referenced behavior."], "reason": "Rehome historical project-help seed through the real DatalinkMain singleton and typed exec call, preserving its existing byte obligation. No fixed executable receiver, new placeholder or claim demotion."}
void __cdecl help_project(int project_id) {
    DatalinkMain.exec(0xB, project_id + 0x46);
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CA00 nullsub_237 0x0044CA00-0x0044CA01 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CA00-0x0044CA01 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044C9E0 ?help_project@@YAXH@Z (src/datalink.cpp) and 0x0044CA10 ?help_abil@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_237@@YAXXZ
// kind      game
void __cdecl nullsub_237() {
}

/*
Purpose: Show the secret project/ability help topic for the given id.
// ORIGINAL: 0x0044CA10 ?help_abil@@YAXH@Z 0x0044CA10-0x0044CA25 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CA30 nullsub_238 0x0044CA30-0x0044CA31 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CA30-0x0044CA31 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CA10 ?help_abil@@YAXH@Z (src/datalink.cpp) and 0x0044CA40 ?help_social@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_238@@YAXXZ
// kind      game
void __cdecl nullsub_238() {
}

/*
Purpose: Show the social engineering help topic for the given id.
// ORIGINAL: 0x0044CA40 ?help_social@@YAXH@Z 0x0044CA40-0x0044CA55 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CA60 nullsub_239 0x0044CA60-0x0044CA61 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CA60-0x0044CA61 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CA40 ?help_social@@YAXH@Z (src/datalink.cpp) and 0x0044CA70 ?help_faction@@YAXH@Z (src/datalink.cpp)"]}
// symbol    ?nullsub_239@@YAXXZ
// kind      game
void __cdecl nullsub_239() {
}

/*
Purpose: Show the faction help topic for the given id.
// ORIGINAL: 0x0044CA70 ?help_faction@@YAXH@Z 0x0044CA70-0x0044CA85 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x0044CA90 sub_44ca90 0x0044CA90-0x0044CABA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/0044ca90.cpp", "task:popup-linked-dialog-helper-owners", "Pinned44CA90 reads Players[faction].filename with59C stride, literal DATALINKS68469C and typed caption-label wrapper5BF330; original caller and helper name remain unestablished."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?popup_faction_datalinks@@YAHH@Z
int __cdecl popup_faction_datalinks(int faction_id) {
    // Native44CA90 uses this exact NUL-terminated label. Both comparison
    // profiles emit its independent ten-byte literal COMDAT in this TU.
    // RECOVERY_LITERAL: v1 0x0068469C 0x006846A6 ??_C@_09COKO@DATALINKS?$AA@ 0x0044CA90
    return X_pop(Players[faction_id].filename, "DATALINKS",
                               0);
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CAC0 nullsub_240 0x0044CAC0-0x0044CAC1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CAC0-0x0044CAC1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CA90 sub_44ca90 (src/popup.cpp) and 0x0044CAD0 ?help_veh@@YAXH@Z (src/datalink.cpp); datalink.cpp help_* functions surround it"]}
// symbol    ?nullsub_240@@YAXXZ
// kind      game
void __cdecl nullsub_240() {
}

/*
Purpose: Show the vehicle help topic for the given id.
// ORIGINAL: 0x0044CAD0 ?help_veh@@YAXH@Z 0x0044CAD0-0x0044CAE5 BYTE_EXACT
// body      src/datalink.h
// size      21 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CAF0 nullsub_241 0x0044CAF0-0x0044CAF1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CAF0-0x0044CAF1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CAD0 ?help_veh@@YAXH@Z (src/datalink.cpp) and 0x0044CB30 ?help_any@@YAXXZ (src/datalink.cpp)"]}
// symbol    ?nullsub_241@@YAXXZ
// kind      game
void __cdecl nullsub_241() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CB00 nullsub_242 0x0044CB00-0x0044CB01 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CB00-0x0044CB01 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CAD0 ?help_veh@@YAXH@Z (src/datalink.cpp) and 0x0044CB30 ?help_any@@YAXXZ (src/datalink.cpp)"]}
// symbol    ?nullsub_242@@YAXXZ
// kind      game
void __cdecl nullsub_242() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CB10 nullsub_243 0x0044CB10-0x0044CB11 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CB10-0x0044CB11 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CAD0 ?help_veh@@YAXH@Z (src/datalink.cpp) and 0x0044CB30 ?help_any@@YAXXZ (src/datalink.cpp)"]}
// symbol    ?nullsub_243@@YAXXZ
// kind      game
void __cdecl nullsub_243() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0044CB20 nullsub_244 0x0044CB20-0x0044CB21 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0044CB20-0x0044CB21 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0044CAD0 ?help_veh@@YAXH@Z (src/datalink.cpp) and 0x0044CB30 ?help_any@@YAXXZ (src/datalink.cpp)"]}
// symbol    ?nullsub_244@@YAXXZ
// kind      game
void __cdecl nullsub_244() {
}

// ORIGINAL: 0x0044CB30 ?help_any@@YAXXZ 0x0044CB30-0x0044CB3F BYTE_EXACT
// size      15 bytes
// prototype
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00429180
// 0x0044CB30  ?help_any@@YAXXZ  ->  ?help_any@@YAXXZ
//
// HOMING NOTE: the tree names the singleton, so the body spells it the way
// datalink.h already declares it - `DatalinkMain`, the object at 0x00703EA0 -
// rather than binding the address again under an anonymous name.

// Provisional selected-entry state; pinned7591AC initially contains zero.
// RECOVERY_OBJECT: v1 0x007591AC ?DatalinkEntrySelection@@3HA
int DatalinkEntrySelection = 0;

// ORIGINAL: 0x0044CB40 sub_44cb40 0x0044CB40-0x0044CB56 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-interface-callback-owner", "Pinned44CB40..44CB56 updates the signed index at7591AC, preserving its value for negative input. Queue590DC0 uses the selected matching entry. Name is provisional use-site evidence, not recovered canonical name.", "historical seed:src/recovered/0044cb40.cpp"], "reason": "Recover typed UI helper in its genuine source owner, retaining original identity, spans and historical obligations; remove superseded fixed-address seed."}
// symbol    ?datalink_entry_selection@@YAHH@Z
int __cdecl datalink_entry_selection(int selected_index) {
    if (selected_index < 0) {
        selected_index = DatalinkEntrySelection;
    }
    DatalinkEntrySelection = selected_index;
    return selected_index;
}

/*
Purpose: Show an arbitrary help topic/index pair, for callers that already
         hold the topic id rather than going through one of the typed
         help_* wrappers above.
// ORIGINAL: 0x0044CB60 ?help_topic@@YAXIH@Z 0x0044CB60-0x0044CB77 BYTE_EXACT
// body      src/help.cpp
// size      23 bytes
// prototype void (__cdecl ?help_topic@@YAXIH@Z)(DatalinkID id, int entryID)
// callers   9   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// HISTORICAL: this body was in datalink.h as `MEASURED inline`. The earlier
// mixed inline/call-site observation remains unverified; this ordinary
// definition is a bounded visibility experiment, not proof of original TU
// placement or of every original inline site.
void __cdecl help_topic(unsigned int topic, int index) {
    DatalinkMain.exec(topic, index);
}

/*
Purpose: Append one clickable cross-reference to a caller-owned help buffer.
         The original strings at 0x00683544/40/3C are "$LINK<", "=" and
         ">", respectively; the caller supplies visible text and its id.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044CB80 ?help_create_link@@YAXPAD00@Z 0x0044CB80-0x0044CBC2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0068353C-0x0068354A contains the original markup strings >, = and $LINK<", "source:src/recovered/units/0044cb80.cpp preserves five strcat calls and the original 0x0044CB80 span", "source:src/leaf_recoveries.cpp help builders call this free helper"], "reason": "Home the Datalink-family link formatter as ordinary C++ markup construction in the product TU, replacing the empty leaf placeholder while retaining the original identity."}
// size      66 bytes
Return Value: n/a
Status: Product body; measure before granting a byte claim.
*/
void __cdecl help_create_link(char *out, char *label, char *id) {
// The help-link grammar uses these prefix, separator and closing tokens in order.
// Bounds cover the literal payload, not surrounding allocation or lifetime.
// RECOVERY_LITERAL: v1 0x00683544 0x0068354B ??_C@_06OJDK@$LINK?$DM?$AA@ 0x0044CB80
    strcat(out, "$LINK<");
    strcat(out, label);
// RECOVERY_LITERAL: v1 0x00683540 0x00683542 ??_C@_01KPOD@?$DN?$AA@ 0x0044CB80
    strcat(out, "=");
    strcat(out, id);
// RECOVERY_LITERAL: v1 0x0068353C 0x0068353E ??_C@_01FAFK@?$DO?$AA@ 0x0044CB80
    strcat(out, ">");
}

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x0044CBD0 sub_44cbd0 0x0044CBD0-0x0044CC18 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/0044cbd0.cpp", "task:popup-linked-dialog-helper-owners", "Pinned44CBD0 clears pending flag before each two-argument X_pop5BF310. Existing Popup::on_link_clicked produces link and flag; caller/name remain provisional."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?popup_pending_link_dialog@@YAXPBD@Z
void __cdecl popup_pending_link_dialog(const char *label) {
    PopupPendingLinkFlag = 0;
    X_pop(label, 0);
    while (PopupPendingLinkFlag != 0) {
        PopupPendingLinkFlag = 0;
        X_pop(PopupPendingLink, 0);
    }
}

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

// Application-owned Sprite objects, separate from the class implementation.
// The constructor stays out of this translation unit so natural global
// initialization calls the shared Sprite constructor, as in the original.
// The binding evidence below retains pre-split compiler-local helper names;
// the explicit symbol facts identify the current emitted helpers. No original
// source filename or complete cross-owner startup order is inferred here.
// These Sprite lifetimes remain outside the interface/assets CRT cohort.


// ORIGINAL: 0x00404440 _$E26 0x00404440-0x00404456 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682008 selects init 0x00404440-0x00404456, registering fini 0x00404460-0x0040446A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7130; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E26 via .CRT$XCU and registers _$E24, with relocations naming ?g_UNUSED_SPRITE_VAR02@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR02; no callback body is authored."}
// symbol    _$E4
// ORIGINAL: 0x00404460 _$E24 0x00404460-0x0040446A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682008 selects init 0x00404440-0x00404456, registering fini 0x00404460-0x0040446A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7130; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E26 via .CRT$XCU and registers _$E24, with relocations naming ?g_UNUSED_SPRITE_VAR02@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR02; no callback body is authored."}
// symbol    _$E2
// ORIGINAL: 0x00404470 _$E31 0x00404470-0x00404486 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068200C selects init 0x00404470-0x00404486, registering fini 0x00404490-0x0040449A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A72E0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E31 via .CRT$XCU and registers _$E29, with relocations naming ?g_UNUSED_SPRITE_VAR11@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR11; no callback body is authored."}
// symbol    _$E9
// ORIGINAL: 0x00404490 _$E29 0x00404490-0x0040449A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068200C selects init 0x00404470-0x00404486, registering fini 0x00404490-0x0040449A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A72E0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E31 via .CRT$XCU and registers _$E29, with relocations naming ?g_UNUSED_SPRITE_VAR11@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR11; no callback body is authored."}
// symbol    _$E7
// ORIGINAL: 0x004044A0 _$E36 0x004044A0-0x004044B6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682010 selects init 0x004044A0-0x004044B6, registering fini 0x004044C0-0x004044CA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A71F0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E36 via .CRT$XCU and registers _$E34, with relocations naming ?g_UNUSED_SPRITE_VAR06@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR06; no callback body is authored."}
// symbol    _$E14
// ORIGINAL: 0x004044C0 _$E34 0x004044C0-0x004044CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682010 selects init 0x004044A0-0x004044B6, registering fini 0x004044C0-0x004044CA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A71F0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E36 via .CRT$XCU and registers _$E34, with relocations naming ?g_UNUSED_SPRITE_VAR06@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR06; no callback body is authored."}
// symbol    _$E12
// ORIGINAL: 0x004044D0 _$E41 0x004044D0-0x004044E6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682014 selects init 0x004044D0-0x004044E6, registering fini 0x004044F0-0x004044FA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7280; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E41 via .CRT$XCU and registers _$E39, with relocations naming ?g_UNUSED_SPRITE_VAR09@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR09; no callback body is authored."}
// symbol    _$E19
// ORIGINAL: 0x004044F0 _$E39 0x004044F0-0x004044FA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682014 selects init 0x004044D0-0x004044E6, registering fini 0x004044F0-0x004044FA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7280; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E41 via .CRT$XCU and registers _$E39, with relocations naming ?g_UNUSED_SPRITE_VAR09@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR09; no callback body is authored."}
// symbol    _$E17
// ORIGINAL: 0x00404500 _$E46 0x00404500-0x00404516 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682018 selects init 0x00404500-0x00404516, registering fini 0x00404520-0x0040452A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A74C0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E46 via .CRT$XCU and registers _$E44, with relocations naming ?g_UNUSED_SPRITE_VAR21@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR21; no callback body is authored."}
// symbol    _$E24
// ORIGINAL: 0x00404520 _$E44 0x00404520-0x0040452A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682018 selects init 0x00404500-0x00404516, registering fini 0x00404520-0x0040452A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A74C0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E46 via .CRT$XCU and registers _$E44, with relocations naming ?g_UNUSED_SPRITE_VAR21@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR21; no callback body is authored."}
// symbol    _$E22
// ORIGINAL: 0x00404530 _$E51 0x00404530-0x00404546 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068201C selects init 0x00404530-0x00404546, registering fini 0x00404550-0x0040455A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A71C0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E51 via .CRT$XCU and registers _$E49, with relocations naming ?g_UNUSED_SPRITE_VAR05@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR05; no callback body is authored."}
// symbol    _$E29
// ORIGINAL: 0x00404550 _$E49 0x00404550-0x0040455A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068201C selects init 0x00404530-0x00404546, registering fini 0x00404550-0x0040455A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A71C0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E51 via .CRT$XCU and registers _$E49, with relocations naming ?g_UNUSED_SPRITE_VAR05@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR05; no callback body is authored."}
// symbol    _$E27
// ORIGINAL: 0x00404560 _$E56 0x00404560-0x00404576 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682020 selects init 0x00404560-0x00404576, registering fini 0x00404580-0x0040458A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7250; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E56 via .CRT$XCU and registers _$E54, with relocations naming ?g_UNUSED_SPRITE_VAR08@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR08; no callback body is authored."}
// symbol    _$E34
// ORIGINAL: 0x00404580 _$E54 0x00404580-0x0040458A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682020 selects init 0x00404560-0x00404576, registering fini 0x00404580-0x0040458A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7250; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E56 via .CRT$XCU and registers _$E54, with relocations naming ?g_UNUSED_SPRITE_VAR08@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR08; no callback body is authored."}
// symbol    _$E32
// ORIGINAL: 0x00404590 _$E61 0x00404590-0x004045A6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682024 selects init 0x00404590-0x004045A6, registering fini 0x004045B0-0x004045BA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7190; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E61 via .CRT$XCU and registers _$E59, with relocations naming ?g_UNUSED_SPRITE_VAR04@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR04; no callback body is authored."}
// symbol    _$E39
// ORIGINAL: 0x004045B0 _$E59 0x004045B0-0x004045BA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682024 selects init 0x00404590-0x004045A6, registering fini 0x004045B0-0x004045BA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7190; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E61 via .CRT$XCU and registers _$E59, with relocations naming ?g_UNUSED_SPRITE_VAR04@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR04; no callback body is authored."}
// symbol    _$E37
// ORIGINAL: 0x004045C0 _$E66 0x004045C0-0x004045D6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682028 selects init 0x004045C0-0x004045D6, registering fini 0x004045E0-0x004045EA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7100; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E66 via .CRT$XCU and registers _$E64, with relocations naming ?g_UNUSED_SPRITE_VAR01@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR01; no callback body is authored."}
// symbol    _$E44
// ORIGINAL: 0x004045E0 _$E64 0x004045E0-0x004045EA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682028 selects init 0x004045C0-0x004045D6, registering fini 0x004045E0-0x004045EA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7100; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E66 via .CRT$XCU and registers _$E64, with relocations naming ?g_UNUSED_SPRITE_VAR01@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR01; no callback body is authored."}
// symbol    _$E42
// ORIGINAL: 0x004045F0 _$E71 0x004045F0-0x00404606 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068202C selects init 0x004045F0-0x00404606, registering fini 0x00404610-0x0040461A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7430; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E71 via .CRT$XCU and registers _$E69, with relocations naming ?g_UNUSED_SPRITE_VAR18@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR18; no callback body is authored."}
// symbol    _$E49
// ORIGINAL: 0x00404610 _$E69 0x00404610-0x0040461A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068202C selects init 0x004045F0-0x00404606, registering fini 0x00404610-0x0040461A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7430; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E71 via .CRT$XCU and registers _$E69, with relocations naming ?g_UNUSED_SPRITE_VAR18@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR18; no callback body is authored."}
// symbol    _$E47
// ORIGINAL: 0x00404620 _$E76 0x00404620-0x00404636 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682030 selects init 0x00404620-0x00404636, registering fini 0x00404640-0x0040464A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7160; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E76 via .CRT$XCU and registers _$E74, with relocations naming ?g_UNUSED_SPRITE_VAR03@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR03; no callback body is authored."}
// symbol    _$E54
// ORIGINAL: 0x00404640 _$E74 0x00404640-0x0040464A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682030 selects init 0x00404620-0x00404636, registering fini 0x00404640-0x0040464A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7160; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E76 via .CRT$XCU and registers _$E74, with relocations naming ?g_UNUSED_SPRITE_VAR03@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR03; no callback body is authored."}
// symbol    _$E52
// ORIGINAL: 0x00404650 _$E81 0x00404650-0x00404666 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682034 selects init 0x00404650-0x00404666, registering fini 0x00404670-0x0040467A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7490; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E81 via .CRT$XCU and registers _$E79, with relocations naming ?g_UNUSED_SPRITE_VAR20@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR20; no callback body is authored."}
// symbol    _$E59
// ORIGINAL: 0x00404670 _$E79 0x00404670-0x0040467A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682034 selects init 0x00404650-0x00404666, registering fini 0x00404670-0x0040467A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7490; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E81 via .CRT$XCU and registers _$E79, with relocations naming ?g_UNUSED_SPRITE_VAR20@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR20; no callback body is authored."}
// symbol    _$E57
// ORIGINAL: 0x00404680 _$E86 0x00404680-0x00404696 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682038 selects init 0x00404680-0x00404696, registering fini 0x004046A0-0x004046AA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A73D0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E86 via .CRT$XCU and registers _$E84, with relocations naming ?g_UNUSED_SPRITE_VAR16@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR16; no callback body is authored."}
// symbol    _$E64
// ORIGINAL: 0x004046A0 _$E84 0x004046A0-0x004046AA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682038 selects init 0x00404680-0x00404696, registering fini 0x004046A0-0x004046AA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A73D0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E86 via .CRT$XCU and registers _$E84, with relocations naming ?g_UNUSED_SPRITE_VAR16@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR16; no callback body is authored."}
// symbol    _$E62
// ORIGINAL: 0x004046B0 _$E91 0x004046B0-0x004046C6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068203C selects init 0x004046B0-0x004046C6, registering fini 0x004046D0-0x004046DA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7370; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E91 via .CRT$XCU and registers _$E89, with relocations naming ?g_UNUSED_SPRITE_VAR14@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR14; no callback body is authored."}
// symbol    _$E69
// ORIGINAL: 0x004046D0 _$E89 0x004046D0-0x004046DA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068203C selects init 0x004046B0-0x004046C6, registering fini 0x004046D0-0x004046DA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7370; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E91 via .CRT$XCU and registers _$E89, with relocations naming ?g_UNUSED_SPRITE_VAR14@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR14; no callback body is authored."}
// symbol    _$E67
// ORIGINAL: 0x004046E0 _$E96 0x004046E0-0x004046F6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682040 selects init 0x004046E0-0x004046F6, registering fini 0x00404700-0x0040470A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A74F0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E96 via .CRT$XCU and registers _$E94, with relocations naming ?g_UNUSED_SPRITE_VAR22@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR22; no callback body is authored."}
// symbol    _$E74
// ORIGINAL: 0x00404700 _$E94 0x00404700-0x0040470A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682040 selects init 0x004046E0-0x004046F6, registering fini 0x00404700-0x0040470A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A74F0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E96 via .CRT$XCU and registers _$E94, with relocations naming ?g_UNUSED_SPRITE_VAR22@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR22; no callback body is authored."}
// symbol    _$E72
// ORIGINAL: 0x00404710 _$E101 0x00404710-0x00404726 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682044 selects init 0x00404710-0x00404726, registering fini 0x00404730-0x0040473A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A72B0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E101 via .CRT$XCU and registers _$E99, with relocations naming ?g_UNUSED_SPRITE_VAR10@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR10; no callback body is authored."}
// symbol    _$E79
// ORIGINAL: 0x00404730 _$E99 0x00404730-0x0040473A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682044 selects init 0x00404710-0x00404726, registering fini 0x00404730-0x0040473A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A72B0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E101 via .CRT$XCU and registers _$E99, with relocations naming ?g_UNUSED_SPRITE_VAR10@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR10; no callback body is authored."}
// symbol    _$E77
// ORIGINAL: 0x00404740 _$E106 0x00404740-0x00404756 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682048 selects init 0x00404740-0x00404756, registering fini 0x00404760-0x0040476A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A73A0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E106 via .CRT$XCU and registers _$E104, with relocations naming ?g_UNUSED_SPRITE_VAR15@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR15; no callback body is authored."}
// symbol    _$E84
// ORIGINAL: 0x00404760 _$E104 0x00404760-0x0040476A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682048 selects init 0x00404740-0x00404756, registering fini 0x00404760-0x0040476A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A73A0; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E106 via .CRT$XCU and registers _$E104, with relocations naming ?g_UNUSED_SPRITE_VAR15@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR15; no callback body is authored."}
// symbol    _$E82
// ORIGINAL: 0x00404770 _$E111 0x00404770-0x00404786 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068204C selects init 0x00404770-0x00404786, registering fini 0x00404790-0x0040479A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7340; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E111 via .CRT$XCU and registers _$E109, with relocations naming ?g_UNUSED_SPRITE_VAR13@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR13; no callback body is authored."}
// symbol    _$E89
// ORIGINAL: 0x00404790 _$E109 0x00404790-0x0040479A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068204C selects init 0x00404770-0x00404786, registering fini 0x00404790-0x0040479A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7340; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E111 via .CRT$XCU and registers _$E109, with relocations naming ?g_UNUSED_SPRITE_VAR13@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR13; no callback body is authored."}
// symbol    _$E87
// ORIGINAL: 0x004047A0 _$E116 0x004047A0-0x004047B6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682050 selects init 0x004047A0-0x004047B6, registering fini 0x004047C0-0x004047CA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7400; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E116 via .CRT$XCU and registers _$E114, with relocations naming ?g_UNUSED_SPRITE_VAR17@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR17; no callback body is authored."}
// symbol    _$E94
// ORIGINAL: 0x004047C0 _$E114 0x004047C0-0x004047CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682050 selects init 0x004047A0-0x004047B6, registering fini 0x004047C0-0x004047CA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7400; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E116 via .CRT$XCU and registers _$E114, with relocations naming ?g_UNUSED_SPRITE_VAR17@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR17; no callback body is authored."}
// symbol    _$E92
// ORIGINAL: 0x004047D0 _$E121 0x004047D0-0x004047E6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682054 selects init 0x004047D0-0x004047E6, registering fini 0x004047F0-0x004047FA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7460; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E121 via .CRT$XCU and registers _$E119, with relocations naming ?g_UNUSED_SPRITE_VAR19@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR19; no callback body is authored."}
// symbol    _$E99
// ORIGINAL: 0x004047F0 _$E119 0x004047F0-0x004047FA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682054 selects init 0x004047D0-0x004047E6, registering fini 0x004047F0-0x004047FA; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7460; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E121 via .CRT$XCU and registers _$E119, with relocations naming ?g_UNUSED_SPRITE_VAR19@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR19; no callback body is authored."}
// symbol    _$E97
// ORIGINAL: 0x00404800 _$E126 0x00404800-0x00404816 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682058 selects init 0x00404800-0x00404816, registering fini 0x00404820-0x0040482A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7310; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E126 via .CRT$XCU and registers _$E124, with relocations naming ?g_UNUSED_SPRITE_VAR12@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR12; no callback body is authored."}
// symbol    _$E104
// ORIGINAL: 0x00404820 _$E124 0x00404820-0x0040482A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682058 selects init 0x00404800-0x00404816, registering fini 0x00404820-0x0040482A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7310; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E126 via .CRT$XCU and registers _$E124, with relocations naming ?g_UNUSED_SPRITE_VAR12@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR12; no callback body is authored."}
// symbol    _$E102
// ORIGINAL: 0x00404830 _$E131 0x00404830-0x00404846 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068205C selects init 0x00404830-0x00404846, registering fini 0x00404850-0x0040485A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7220; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E131 via .CRT$XCU and registers _$E129, with relocations naming ?g_UNUSED_SPRITE_VAR07@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR07; no callback body is authored."}
// symbol    _$E109
// ORIGINAL: 0x00404850 _$E129 0x00404850-0x0040485A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x0068205C selects init 0x00404830-0x00404846, registering fini 0x00404850-0x0040485A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x006A7220; Sprite constructor 0x005E37E0 and teardown tail-jump 0x005E3820.", "Current owner physical COFF selects _$E131 via .CRT$XCU and registers _$E129, with relocations naming ?g_UNUSED_SPRITE_VAR07@@3VSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_UNUSED_SPRITE_VAR07; no callback body is authored."}
// symbol    _$E107
// ORIGINAL: 0x00471350 _$E966 0x00471350-0x00471374 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682548 selects init 0x00471350-0x00471374, registering fini 0x00471380-0x00471394; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x007F67C8; array count 1, stride 0x2C, Sprite constructor/destructor 0x005E37E0/0x00406850 and vector iterators 0x006457C2/0x006456E4.", "Current owner physical COFF selects _$E966 via .CRT$XCU and registers _$E964, with relocations naming ?g_IFACE_GREEN_RIGHT_ARROW_SPRITE@@3PAVSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_IFACE_GREEN_RIGHT_ARROW_SPRITE; no callback body is authored."}
// symbol    _$E114
// ORIGINAL: 0x00471380 _$E964 0x00471380-0x00471394 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682548 selects init 0x00471350-0x00471374, registering fini 0x00471380-0x00471394; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x007F67C8; array count 1, stride 0x2C, Sprite constructor/destructor 0x005E37E0/0x00406850 and vector iterators 0x006457C2/0x006456E4.", "Current owner physical COFF selects _$E966 via .CRT$XCU and registers _$E964, with relocations naming ?g_IFACE_GREEN_RIGHT_ARROW_SPRITE@@3PAVSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_IFACE_GREEN_RIGHT_ARROW_SPRITE; no callback body is authored."}
// symbol    _$E112
// ORIGINAL: 0x0052DAC0 _$E971 0x0052DAC0-0x0052DAE4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682630 selects init 0x0052DAC0-0x0052DAE4, registering fini 0x0052DAF0-0x0052DB04; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x0093AA70; array count 12, stride 0x2C, Sprite constructor/destructor 0x005E37E0/0x00406850 and vector iterators 0x006457C2/0x006456E4.", "Current owner physical COFF selects _$E971 via .CRT$XCU and registers _$E969, with relocations naming ?g_CURSOR_SPRITES@@3PAVSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_CURSOR_SPRITES; no callback body is authored."}
// symbol    _$E119
// ORIGINAL: 0x0052DAF0 _$E969 0x0052DAF0-0x0052DB04 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682630 selects init 0x0052DAC0-0x0052DAE4, registering fini 0x0052DAF0-0x0052DB04; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x0093AA70; array count 12, stride 0x2C, Sprite constructor/destructor 0x005E37E0/0x00406850 and vector iterators 0x006457C2/0x006456E4.", "Current owner physical COFF selects _$E971 via .CRT$XCU and registers _$E969, with relocations naming ?g_CURSOR_SPRITES@@3PAVSprite@@A; genuine C++ global declaration owns the lifetime, including inline scalar constructor emission.", "Current local helper names are compiler-output interpretations, not original debug names. Individual lifetime binding does not prove full initialization order, linked retained selection, data-layout completeness or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_CURSOR_SPRITES; no callback body is authored."}
// symbol    _$E117

#include "stdafx.h"
#include "sprite.h"

// Canonical mapping for the inline destructor emitted by these object lifetimes.
/*
// ORIGINAL: 0x00406850 ??1Sprite@@QAE@XZ 0x00406850-0x00406855 BYTE_EXACT
// body      src/sprite.h
// size      5 bytes
// prototype
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// Pinned original init -> registered cleanup pairs, in CRT order. Each init
// loads the stated global into ECX, calls Sprite::Sprite (0x005E37E0), pushes
// the paired cleanup and calls _atexit (0x00645398). Each cleanup loads that
// same global into ECX and tail-jumps to Sprite::close (0x005E3820). These are
// lifetime identities only; no allocation ends are asserted here.
// 0x00404440 -> 0x006A7130 -> 0x00404460
// 0x00404470 -> 0x006A72E0 -> 0x00404490
// 0x004044A0 -> 0x006A71F0 -> 0x004044C0
// 0x004044D0 -> 0x006A7280 -> 0x004044F0
// 0x00404500 -> 0x006A74C0 -> 0x00404520
// 0x00404530 -> 0x006A71C0 -> 0x00404550
// 0x00404560 -> 0x006A7250 -> 0x00404580
// 0x00404590 -> 0x006A7190 -> 0x004045B0
// 0x004045C0 -> 0x006A7100 -> 0x004045E0
// 0x004045F0 -> 0x006A7430 -> 0x00404610
// 0x00404620 -> 0x006A7160 -> 0x00404640
// 0x00404650 -> 0x006A7490 -> 0x00404670
// 0x00404680 -> 0x006A73D0 -> 0x004046A0
// 0x004046B0 -> 0x006A7370 -> 0x004046D0
// 0x004046E0 -> 0x006A74F0 -> 0x00404700
// 0x00404710 -> 0x006A72B0 -> 0x00404730
// 0x00404740 -> 0x006A73A0 -> 0x00404760
// 0x00404770 -> 0x006A7340 -> 0x00404790
// 0x004047A0 -> 0x006A7400 -> 0x004047C0
// 0x004047D0 -> 0x006A7460 -> 0x004047F0
// 0x00404800 -> 0x006A7310 -> 0x00404820
// 0x00404830 -> 0x006A7220 -> 0x00404850
// RECOVERY_OBJECT: v1 0x006A7130 ?g_UNUSED_SPRITE_VAR02@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR02;  // 0x006A7130
// RECOVERY_OBJECT: v1 0x006A72E0 ?g_UNUSED_SPRITE_VAR11@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR11;  // 0x006A72E0
// RECOVERY_OBJECT: v1 0x006A71F0 ?g_UNUSED_SPRITE_VAR06@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR06;  // 0x006A71F0
// RECOVERY_OBJECT: v1 0x006A7280 ?g_UNUSED_SPRITE_VAR09@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR09;  // 0x006A7280
// RECOVERY_OBJECT: v1 0x006A74C0 ?g_UNUSED_SPRITE_VAR21@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR21;  // 0x006A74C0
// RECOVERY_OBJECT: v1 0x006A71C0 ?g_UNUSED_SPRITE_VAR05@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR05;  // 0x006A71C0
// RECOVERY_OBJECT: v1 0x006A7250 ?g_UNUSED_SPRITE_VAR08@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR08;  // 0x006A7250
// RECOVERY_OBJECT: v1 0x006A7190 ?g_UNUSED_SPRITE_VAR04@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR04;  // 0x006A7190
// RECOVERY_OBJECT: v1 0x006A7100 ?g_UNUSED_SPRITE_VAR01@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR01;  // 0x006A7100
// RECOVERY_OBJECT: v1 0x006A7430 ?g_UNUSED_SPRITE_VAR18@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR18;  // 0x006A7430
// RECOVERY_OBJECT: v1 0x006A7160 ?g_UNUSED_SPRITE_VAR03@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR03;  // 0x006A7160
// RECOVERY_OBJECT: v1 0x006A7490 ?g_UNUSED_SPRITE_VAR20@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR20;  // 0x006A7490
// RECOVERY_OBJECT: v1 0x006A73D0 ?g_UNUSED_SPRITE_VAR16@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR16;  // 0x006A73D0
// RECOVERY_OBJECT: v1 0x006A7370 ?g_UNUSED_SPRITE_VAR14@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR14;  // 0x006A7370
// RECOVERY_OBJECT: v1 0x006A74F0 ?g_UNUSED_SPRITE_VAR22@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR22;  // 0x006A74F0
// RECOVERY_OBJECT: v1 0x006A72B0 ?g_UNUSED_SPRITE_VAR10@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR10;  // 0x006A72B0
// RECOVERY_OBJECT: v1 0x006A73A0 ?g_UNUSED_SPRITE_VAR15@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR15;  // 0x006A73A0
// RECOVERY_OBJECT: v1 0x006A7340 ?g_UNUSED_SPRITE_VAR13@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR13;  // 0x006A7340
// RECOVERY_OBJECT: v1 0x006A7400 ?g_UNUSED_SPRITE_VAR17@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR17;  // 0x006A7400
// RECOVERY_OBJECT: v1 0x006A7460 ?g_UNUSED_SPRITE_VAR19@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR19;  // 0x006A7460
// RECOVERY_OBJECT: v1 0x006A7310 ?g_UNUSED_SPRITE_VAR12@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR12;  // 0x006A7310
// RECOVERY_OBJECT: v1 0x006A7220 ?g_UNUSED_SPRITE_VAR07@@3VSprite@@A
Sprite g_UNUSED_SPRITE_VAR07;  // 0x006A7220

// RECOVERY_OBJECT: v2 0x007F67C8 0x007F67F4 ?g_IFACE_GREEN_RIGHT_ARROW_SPRITE@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_GREEN_RIGHT_ARROW_SPRITE[1];
// RECOVERY_OBJECT: v2 0x0093AA70 0x0093AC80 ?g_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_CURSOR_SPRITES[12];
// Original CRT initializer 0x0044CCA0 constructs three 0x2C-byte Sprites;
// 0x0044CCD0 destroys the same array. DipEdit::init references elements 1
// and 2 at 0x00779504 / 0x00779530. Initial storage is separate from lifetime.
// Original CRT vector initializers 0x44CCF0,0x44CD40,...,0x44CF20 each
// construct 51 Sprites at stride 0x2C; their exits use the same extents.
// Each range is initially zero. Construction and asset extraction are separate.
// The eight original single-object initializers at 0x44CFC0..0x44D110
// call Sprite's constructor, whose final DWORD store is at +0x28.
// These 0x2C-byte ranges are initially zero; the loader supplies pixels.
// Original CRT 0044EB90 (table 006822DC) constructs 6 Sprite object(s);
// exit 0044EBC0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EBE0 (table 006822E0) constructs 9 Sprite object(s);
// exit 0044EC10 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EC30 (table 006822E4) constructs 2 Sprite object(s);
// exit 0044EC60 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EC80 (table 006822E8) constructs 1 Sprite object(s);
// exit 0044ECA0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044ECB0 (table 006822EC) constructs 1 Sprite object(s);
// exit 0044ECD0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044ECE0 (table 006822F0) constructs 1 Sprite object(s);
// exit 0044ED00 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044ED10 (table 006822F4) constructs 1 Sprite object(s);
// exit 0044ED30 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044ED40 (table 006822F8) constructs 1 Sprite object(s);
// exit 0044ED60 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044ED70 (table 006822FC) constructs 1 Sprite object(s);
// exit 0044ED90 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EDA0 (table 00682300) constructs 1 Sprite object(s);
// exit 0044EDC0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EDD0 (table 00682304) constructs 1 Sprite object(s);
// exit 0044EDF0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EE00 (table 00682308) constructs 1 Sprite object(s);
// exit 0044EE20 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EE30 (table 0068230C) constructs 1 Sprite object(s);
// exit 0044EE50 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EE60 (table 00682310) constructs 6 Sprite object(s);
// exit 0044EE90 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EEB0 (table 00682314) constructs 15 Sprite object(s);
// exit 0044EEE0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EF00 (table 00682318) constructs 4 Sprite object(s);
// exit 0044EF30 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EF50 (table 0068231C) constructs 6 Sprite object(s);
// exit 0044EF80 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EFA0 (table 00682320) constructs 5 Sprite object(s);
// exit 0044EFD0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044EFF0 (table 00682324) constructs 5 Sprite object(s);
// exit 0044F020 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F040 (table 00682328) constructs 5 Sprite object(s);
// exit 0044F070 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F090 (table 0068232C) constructs 12 Sprite object(s);
// exit 0044F0C0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F0E0 (table 00682330) constructs 6 Sprite object(s);
// exit 0044F110 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F130 (table 00682334) constructs 2 Sprite object(s);
// exit 0044F160 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F180 (table 00682338) constructs 6 Sprite object(s);
// exit 0044F1B0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F1D0 (table 0068233C) constructs 1 Sprite object(s);
// exit 0044F1F0 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F200 (table 00682340) constructs 1 Sprite object(s);
// exit 0044F220 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F230 (table 00682344) constructs 1 Sprite object(s);
// exit 0044F250 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F260 (table 00682348) constructs 1 Sprite object(s);
// exit 0044F280 uses the same extent. Initial zero storage is not lifetime proof.
// Original CRT 0044F290 (table 0068234C) constructs 2 Sprite object(s);
// exit 0044F2C0 uses the same extent. Initial zero storage is not lifetime proof.
// CRT 44F2E0 constructs152 Sprites; 44F310 destroys the same 0x2C-stride bank.
// CRT 44F330/44F360 construct/destroy112 Sprites: fourteen for each faction.
// Original CRT 0x682358 constructs 16 Sprite(s) via 0x44F380
// and registers 0x44F3B0 for teardown; element extent is 0x2C.
// 0x0075B450, 0x2c stride
// Original CRT 0x68235C constructs 32 Sprite(s) via 0x44F3D0
// and registers 0x44F400 for teardown; element extent is 0x2C.
// 0x007A72A0, 0x2c stride
// 0x0078CAD0, 0x2c stride
// 0x007765C0, 0x2c stride
// 0x00776420, 0x2c stride
// 0x00779B78, 0x2c stride
// 0x007AD1B0, 0x2c stride
// 0x00798CD0, 0x2c stride
// 0x0075B180, 0x2c stride
// 0x0075B710, 0x2c stride
// Original CRT 0x682380 constructs 1 Sprite(s) via 0x44F6A0
// and registers 0x44F6C0 for teardown; element extent is 0x2C.
// 0x00791F98
// Original CRT 0x682384 constructs 4 Sprite(s) via 0x44F6D0
// and registers 0x44F700 for teardown; element extent is 0x2C.
// 0x0075AE20, 0x2c stride
// Original CRT 0x682388 constructs 2 Sprite(s) via 0x44F720
// and registers 0x44F750 for teardown; element extent is 0x2C.
// 0x0075AF68, 0x2c stride
// Original CRT 0x68238C constructs 1 Sprite(s) via 0x44F770
// and registers 0x44F790 for teardown; element extent is 0x2C.
// 0x0075AD88
// Original CRT 0x682390 constructs 1 Sprite(s) via 0x44F7A0
// and registers 0x44F7C0 for teardown; element extent is 0x2C.
// 0x0078A520
// Original CRT 0x682394 constructs 2 Sprite(s) via 0x44F7D0
// and registers 0x44F800 for teardown; element extent is 0x2C.
// 0x0075AF10, 0x2c stride
// Original CRT 0x682398 constructs 1 Sprite(s) via 0x44F820
// and registers 0x44F840 for teardown; element extent is 0x2C.
// 0x00776728
// CRT 44F850/44F880 construct/destroy144 Sprites at stride0x2C.
// CRT 44F8A0/44F8D0 construct/destroy64 Sprites. The loader's moving
// bottom-row sentinel7AC0C0 is not the end of this object's storage.
// Loader454D10 identifies these owners; the pinned CRT table6823A4..6823C8
// selects their constructor wrappers in this order. Each uses Sprite2C;
// array wrappers pass the stated count/stride to the CRT vector helpers.
// Constructor/destructor pairs below prove original bounds/lifetime intent,
// not current linked initializer ordering or successful artwork loading.
// Native ctor44F8F0 / dtor44F910, one Sprite.
// 0x00779200
// Native ctor44F920 / dtor44F940, one Sprite.
// 0x0076DC30
// Native ctor44F950 / dtor44F970, one Sprite.
// 0x0078A4A0
// Native ctor44F980 / dtor44F9A0, one Sprite.
// 0x00787E70
// Native ctor44F9B0 / dtor44F9D0, one Sprite.
// 0x007AC000
// Native ctor44F9E0 / dtor44FA00, one Sprite.
// 0x00779570
// Native ctor44FA10 / dtor44FA40, 89 Sprites at stride2C.
// 0x00759E28, 0x2c stride
// Native ctor44FA60 / dtor44FA90, 70 Sprites at stride2C.
// 0x00787200, 0x2c stride
// Native ctor44FAB0 / dtor44FAE0, 64 Sprites at stride2C.
// 0x00759320, 0x2c stride
// Original 0x44FB00/0x44FB30 construct/destroy 3 elements at stride 0x2C.
// Original CRT 0x6823CC constructs 12 Sprite(s) via 0x44FB50
// and registers 0x44FB80 for teardown; element extent is 0x2C.
// 0x00791D88, 0x2c stride
// Original CRT 0x6823D0 constructs 12 Sprite(s) via 0x44FBA0
// and registers 0x44FBD0 for teardown; element extent is 0x2C.
// 0x0078DE30, 0x2c stride
// Original CRT 0x6823D4 constructs 1 Sprite(s) via 0x44FBF0
// and registers 0x44FC10 for teardown; element extent is 0x2C.
// 0x0079A5E8
// Original CRT 0x6823D8 constructs 1 Sprite(s) via 0x44FC20
// and registers 0x44FC40 for teardown; element extent is 0x2C.
// 0x007796B0
// Original 0x44FC50/0x44FC80 construct/destroy 2 elements at stride 0x2C.
// Original init 0x0044FCA0 and teardown 0x0044FCD0 both name
// this constructed array run: 8 elements, stride 0x2C; names remain provisional.
// 0x007A7940, 0x2c stride
// Original 0x450380/0x4503B0 construct/destroy 3 elements at stride 0x2C.
// Original init 0x00450910 and teardown 0x00450940 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x007A68F0, 0x2c stride
// Original init 0x00450960 and teardown 0x00450990 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x00789A90, 0x2c stride
// Original init 0x004509B0 and teardown 0x004509E0 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x00792160, 0x2c stride
// Original init 0x00450A00 and teardown 0x00450A30 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x0075B0C8, 0x2c stride
// Original init 0x00450A50 and teardown 0x00450A80 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x0075B8C8, 0x2c stride
// Original init 0x00450AA0 and teardown 0x00450AD0 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x00787ED0, 0x2c stride
// Original init 0x00450AF0 and teardown 0x00450B20 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x007ACE80, 0x2c stride
// Original init 0x00450B40 and teardown 0x00450B70 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x007920A8, 0x2c stride
// Original init 0x00450B90 and teardown 0x00450BC0 both name
// this constructed array run: 3 elements, stride 0x2C; names remain provisional.
// 0x007ACF08, 0x2c stride
// Original init 0x00450D90 and teardown 0x00450DC0 both name
// this constructed array run: 27 elements, stride 0x2C; names remain provisional.
// 0x0077A5C8, 0x2c stride
// Original 0x471350/0x471380 use a one-element vector at stride 0x2C.
// 0x0093AA70, 0x2c stride


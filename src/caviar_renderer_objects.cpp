/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

// Vehicle renderer lifetime is later than model, terrain and sprite assets.
// ORIGINAL: 0x004BF6E0 _$E21 0x004BF6E0-0x004BF6F6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x006825FC selects init 0x004BF6E0-0x004BF6F6, registering fini 0x004BF700-0x004BF70A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x008CC828; scalar Caviar constructor/destructor 0x00616DA0/0x00617020.", "Physical startup-root, registered-cleanup and global relocations identify this source-owned object in src/caviar_renderer_objects.cpp. The adjacent symbol fact names the compiler-emitted operation. Out-of-line class methods supply construction and teardown; no handwritten lifecycle callback or fixed-address object is substituted.", "Current helper names are compiler-output interpretations, not original debug symbols. These individual bindings do not establish full initialization order, allocation extents, linked retained selection or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_VEHDRAW_CAVIAR; no callback body is authored."}
// symbol    _$E4
// ORIGINAL: 0x004BF700 _$E19 0x004BF700-0x004BF70A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x006825FC selects init 0x004BF6E0-0x004BF6F6, registering fini 0x004BF700-0x004BF70A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x008CC828; scalar Caviar constructor/destructor 0x00616DA0/0x00617020.", "Physical startup-root, registered-cleanup and global relocations identify this source-owned object in src/caviar_renderer_objects.cpp. The adjacent symbol fact names the compiler-emitted operation. Out-of-line class methods supply construction and teardown; no handwritten lifecycle callback or fixed-address object is substituted.", "Current helper names are compiler-output interpretations, not original debug symbols. These individual bindings do not establish full initialization order, allocation extents, linked retained selection or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_VEHDRAW_CAVIAR; no callback body is authored."}
// symbol    _$E2

#include "stdafx.h"
#include "caviar.h"

// Original CRT slots 0x00682104..0x00682288 construct all 98 model-data
// globals before the renderer at slot 0x006825FC. Preserve that relative
// lifetime order here; this does not establish whole-game startup ordering.
// turn_start addresses this renderer at 0x008CC828 and its light/shadow
// vectors at +0x60/+0x6C. It is the existing C++ static owner, not an alias.
// RECOVERY_OBJECT: v1 0x008CC828 ?g_VEHDRAW_CAVIAR@@3VCaviar@@A
Caviar g_VEHDRAW_CAVIAR;


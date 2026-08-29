/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License, either version 3
 * of the License, or (at your option) any later version.
 */
// ARCHIVED LIFECYCLE THUNKS - the dynamic initializers (??__E) and atexit
// teardowns (??__F) of the globals that became REAL OBJECTS defined in
// src/init_thunks.cpp (2026-08-29). The image registers these thunks in its
// CRT dynamic-initializer table - confirmed at runtime under winedbg: the
// walker (return address 0x00644EEB) calls them before WinMain, walking the
// pointer run at 0x682568..0x6826D0. This tree's CRT now performs the same
// lifecycle for the real globals through its own private thunks, so these
// bodies have no caller here; they are preserved as the byte-exact record of
// the image's construction and exit sequence, one per global, and the
// addresses each one managed are named in init_thunks.h.
//
// A VERIFICATION ARTIFACT, not product source: this file is not in the
// build, and the bodies reference the objects at their image addresses.

/*
Purpose: Dynamic initializer for g_CAVIAR_BUFFER_2; registers its ??__F teardown.
// ORIGINAL: 0x00616AE0 ??__Eg_CAVIAR_BUFFER_2@@YAXXZ 0x00616AE0-0x00616AF6 BYTE_EXACT
// symbol    ?construct_caviar_buffer_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_caviar_buffer_2() {
    g_CAVIAR_BUFFER_2.Buffer::Buffer();
    atexit(teardown_g_caviar_buffer_2);
}

/*
Purpose: Dynamic initializer for g_CAVIAR_BUFFER_1; registers its ??__F teardown.
// ORIGINAL: 0x00616AA0 ??__Eg_CAVIAR_BUFFER_1@@YAXXZ 0x00616AA0-0x00616AB6 BYTE_EXACT
// symbol    ?construct_caviar_buffer_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_caviar_buffer_1() {
    g_CAVIAR_BUFFER_1.Buffer::Buffer();
    atexit(teardown_g_caviar_buffer_1);
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_3; registers its ??__F teardown.
// ORIGINAL: 0x00613810 ??__Eg_FILEWIN_SPRITE_3@@YAXXZ 0x00613810-0x00613826 BYTE_EXACT
// symbol    ?construct_filewin_sprite_3@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_3() {
    g_FILEWIN_SPRITE_3.Sprite::Sprite();
    atexit(teardown_g_filewin_sprite_3);
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_2; registers its ??__F teardown.
// ORIGINAL: 0x006137D0 ??__Eg_FILEWIN_SPRITE_2@@YAXXZ 0x006137D0-0x006137E6 BYTE_EXACT
// symbol    ?construct_filewin_sprite_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_2() {
    g_FILEWIN_SPRITE_2.Sprite::Sprite();
    atexit(teardown_g_filewin_sprite_2);
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_1; registers its ??__F teardown.
// ORIGINAL: 0x00613790 ??__Eg_FILEWIN_SPRITE_1@@YAXXZ 0x00613790-0x006137A6 BYTE_EXACT
// symbol    ?construct_filewin_sprite_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_1() {
    g_FILEWIN_SPRITE_1.Sprite::Sprite();
    atexit(teardown_g_filewin_sprite_1);
}

/*
Purpose: Dynamic initializer for g_CHECKBOX_SPRITE_2; registers its ??__F teardown.
// ORIGINAL: 0x0060E630 ??__Eg_CHECKBOX_SPRITE_2@@YAXXZ 0x0060E630-0x0060E646 BYTE_EXACT
// symbol    ?construct_checkbox_sprite_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_checkbox_sprite_2() {
    g_CHECKBOX_SPRITE_2.Sprite::Sprite();
    atexit(teardown_g_checkbox_sprite_2);
}

/*
Purpose: Dynamic initializer for g_CHECKBOX_SPRITE_1; registers its ??__F teardown.
// ORIGINAL: 0x0060E5F0 ??__Eg_CHECKBOX_SPRITE_1@@YAXXZ 0x0060E5F0-0x0060E606 BYTE_EXACT
// symbol    ?construct_checkbox_sprite_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_checkbox_sprite_1() {
    g_CHECKBOX_SPRITE_1.Sprite::Sprite();
    atexit(teardown_g_checkbox_sprite_1);
}

/*
Purpose: Dynamic initializer for g_RADIOBUTTON_SPRITE_2; registers its ??__F teardown.
// ORIGINAL: 0x0060D0A0 ??__Eg_RADIOBUTTON_SPRITE_2@@YAXXZ 0x0060D0A0-0x0060D0B6 BYTE_EXACT
// symbol    ?construct_radiobutton_sprite_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radiobutton_sprite_2() {
    g_RADIOBUTTON_SPRITE_2.Sprite::Sprite();
    atexit(teardown_g_radiobutton_sprite_2);
}

/*
Purpose: Dynamic initializer for g_RADIOBUTTON_SPRITE_1; registers its ??__F teardown.
// ORIGINAL: 0x0060D060 ??__Eg_RADIOBUTTON_SPRITE_1@@YAXXZ 0x0060D060-0x0060D076 BYTE_EXACT
// symbol    ?construct_radiobutton_sprite_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radiobutton_sprite_1() {
    g_RADIOBUTTON_SPRITE_1.Sprite::Sprite();
    atexit(teardown_g_radiobutton_sprite_1);
}

/*
Purpose: Dynamic initializer for g_WIN_BUFFER; registers its ??__F teardown.
// ORIGINAL: 0x005EB350 ??__Eg_WIN_BUFFER@@YAXXZ 0x005EB350-0x005EB366 BYTE_EXACT
// symbol    ?construct_win_buffer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_win_buffer() {
    g_WIN_BUFFER.Buffer::Buffer();
    atexit(teardown_g_win_buffer);
}

/*
Purpose: Dynamic initializer for g_BUFFER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x005D71D0 ??__Eg_BUFFER_SPRITE@@YAXXZ 0x005D71D0-0x005D71E6 BYTE_EXACT
// symbol    ?construct_buffer_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_buffer_sprite() {
    g_BUFFER_SPRITE.Sprite::Sprite();
    atexit(teardown_g_buffer_sprite);
}

/*
Purpose: Dynamic initializer for g_CONSOLE_TIMER; registers its ??__F teardown.
// ORIGINAL: 0x0050E980 ??__Eg_CONSOLE_TIMER@@YAXXZ 0x0050E980-0x0050E996 BYTE_EXACT
// symbol    ?construct_console_timer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006161D0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_console_timer() {
    g_CONSOLE_TIMER.Time::Time();
    atexit(destroy_console_timer);
}

/*
Purpose: Dynamic initializer for g_WAVE_DEVICE; registers its ??__F teardown.
// ORIGINAL: 0x004C5C50 ??__Eg_WAVE_DEVICE@@YAXXZ 0x004C5C50-0x004C5C66 BYTE_EXACT
// symbol    ?construct_wave_device@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C4DD0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_device() {
    g_WAVE_DEVICE.Wave_Device::Wave_Device();
    atexit(destroy_wave_device);
}

/*
Purpose: Dynamic initializer for g_VEHDRAW_CAVIAR; registers its ??__F teardown.
// ORIGINAL: 0x004BF6E0 ??__Eg_VEHDRAW_CAVIAR@@YAXXZ 0x004BF6E0-0x004BF6F6 BYTE_EXACT
// symbol    ?construct_vehdraw_caviar@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616DA0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vehdraw_caviar() {
    g_VEHDRAW_CAVIAR.Caviar::Caviar();
    atexit(destroy_vehdraw_caviar);
}

/*
Purpose: Dynamic initializer for g_NEWTECHWIN_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x00483880 ??__Eg_NEWTECHWIN_SPRITES@@YAXXZ 0x00483880-0x004838A4 BYTE_EXACT
// symbol    ?construct_newtechwin_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_newtechwin_sprites() {
    VectorCtorIterator(g_NEWTECHWIN_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x004838B0));
}

/*
Purpose: ??__Fg_CAVIAR_BUFFER_2@@YAXXZ - tear down the global at 0x9b96b0 through
         ??1Buffer@@QAE@XZ, at most once, gated on bit 2 of the flag byte
         at 0x9b9104.
// ORIGINAL: 0x00616B00 ??__Fg_CAVIAR_BUFFER_2@@YAXXZ 0x00616B00-0x00616B1F
// symbol    ?teardown_g_caviar_buffer_2@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_caviar_buffer_2() {
    const uint8_t flags = *TeardownFlags009B9104;
    if ((2 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B9104 = static_cast<uint8_t>(flags | 2);
    g_CAVIAR_BUFFER_2.Buffer::~Buffer();
}

/*
Purpose: ??__Fg_CAVIAR_BUFFER_1@@YAXXZ - tear down the global at 0x9b9108 through
         ??1Buffer@@QAE@XZ, at most once, gated on bit 1 of the flag byte
         at 0x9b9104.
// ORIGINAL: 0x00616AC0 ??__Fg_CAVIAR_BUFFER_1@@YAXXZ 0x00616AC0-0x00616ADF
// symbol    ?teardown_g_caviar_buffer_1@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_caviar_buffer_1() {
    const uint8_t flags = *TeardownFlags009B9104;
    if ((1 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B9104 = static_cast<uint8_t>(flags | 1);
    g_CAVIAR_BUFFER_1.Buffer::~Buffer();
}

/*
Purpose: ??__Fg_FILEWIN_SPRITE_3@@YAXXZ - tear down the global at 0x9b9078 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 4 of the flag byte
         at 0x9b9014.
// ORIGINAL: 0x00613830 ??__Fg_FILEWIN_SPRITE_3@@YAXXZ 0x00613830-0x0061384F
// symbol    ?teardown_g_filewin_sprite_3@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_filewin_sprite_3() {
    const uint8_t flags = *TeardownFlags009B9014;
    if ((4 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B9014 = static_cast<uint8_t>(flags | 4);
    g_FILEWIN_SPRITE_3.close();
}

/*
Purpose: ??__Fg_FILEWIN_SPRITE_2@@YAXXZ - tear down the global at 0x9b9018 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 2 of the flag byte
         at 0x9b9014.
// ORIGINAL: 0x006137F0 ??__Fg_FILEWIN_SPRITE_2@@YAXXZ 0x006137F0-0x0061380F
// symbol    ?teardown_g_filewin_sprite_2@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_filewin_sprite_2() {
    const uint8_t flags = *TeardownFlags009B9014;
    if ((2 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B9014 = static_cast<uint8_t>(flags | 2);
    g_FILEWIN_SPRITE_2.close();
}

/*
Purpose: ??__Fg_FILEWIN_SPRITE_1@@YAXXZ - tear down the global at 0x9b9048 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 1 of the flag byte
         at 0x9b9014.
// ORIGINAL: 0x006137B0 ??__Fg_FILEWIN_SPRITE_1@@YAXXZ 0x006137B0-0x006137CF
// symbol    ?teardown_g_filewin_sprite_1@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_filewin_sprite_1() {
    const uint8_t flags = *TeardownFlags009B9014;
    if ((1 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B9014 = static_cast<uint8_t>(flags | 1);
    g_FILEWIN_SPRITE_1.close();
}

/*
Purpose: ??__Fg_CHECKBOX_SPRITE_2@@YAXXZ - tear down the global at 0x9b8f90 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 2 of the flag byte
         at 0x9b8f58.
// ORIGINAL: 0x0060E650 ??__Fg_CHECKBOX_SPRITE_2@@YAXXZ 0x0060E650-0x0060E66F
// symbol    ?teardown_g_checkbox_sprite_2@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_checkbox_sprite_2() {
    const uint8_t flags = *TeardownFlags009B8F58;
    if ((2 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B8F58 = static_cast<uint8_t>(flags | 2);
    g_CHECKBOX_SPRITE_2.close();
}

/*
Purpose: ??__Fg_CHECKBOX_SPRITE_1@@YAXXZ - tear down the global at 0x9b8f60 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 1 of the flag byte
         at 0x9b8f58.
// ORIGINAL: 0x0060E610 ??__Fg_CHECKBOX_SPRITE_1@@YAXXZ 0x0060E610-0x0060E62F
// symbol    ?teardown_g_checkbox_sprite_1@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_checkbox_sprite_1() {
    const uint8_t flags = *TeardownFlags009B8F58;
    if ((1 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B8F58 = static_cast<uint8_t>(flags | 1);
    g_CHECKBOX_SPRITE_1.close();
}

/*
Purpose: ??__Fg_RADIOBUTTON_SPRITE_2@@YAXXZ - tear down the global at 0x9b8f28 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 2 of the flag byte
         at 0x9b8ef0.
// ORIGINAL: 0x0060D0C0 ??__Fg_RADIOBUTTON_SPRITE_2@@YAXXZ 0x0060D0C0-0x0060D0DF
// symbol    ?teardown_g_radiobutton_sprite_2@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_radiobutton_sprite_2() {
    const uint8_t flags = *TeardownFlags009B8EF0;
    if ((2 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B8EF0 = static_cast<uint8_t>(flags | 2);
    g_RADIOBUTTON_SPRITE_2.close();
}

/*
Purpose: ??__Fg_RADIOBUTTON_SPRITE_1@@YAXXZ - tear down the global at 0x9b8ef8 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 1 of the flag byte
         at 0x9b8ef0.
// ORIGINAL: 0x0060D080 ??__Fg_RADIOBUTTON_SPRITE_1@@YAXXZ 0x0060D080-0x0060D09F
// symbol    ?teardown_g_radiobutton_sprite_1@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_radiobutton_sprite_1() {
    const uint8_t flags = *TeardownFlags009B8EF0;
    if ((1 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B8EF0 = static_cast<uint8_t>(flags | 1);
    g_RADIOBUTTON_SPRITE_1.close();
}

/*
Purpose: ??__Fg_WIN_BUFFER@@YAXXZ - tear down the global at 0x9b6f08 through
         ??1Buffer@@QAE@XZ, at most once, gated on bit 1 of the flag byte
         at 0x9b6e30.
// ORIGINAL: 0x005EB370 ??__Fg_WIN_BUFFER@@YAXXZ 0x005EB370-0x005EB38F
// symbol    ?teardown_g_win_buffer@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_win_buffer() {
    const uint8_t flags = *TeardownFlags009B6E30;
    if ((1 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B6E30 = static_cast<uint8_t>(flags | 1);
    g_WIN_BUFFER.Buffer::~Buffer();
}

/*
Purpose: ??__Fg_BUFFER_SPRITE@@YAXXZ - tear down the global at 0x9b3a50 through
         ?close@Sprite@@QAEXXZ, at most once, gated on bit 2 of the flag byte
         at 0x9b37cc.
// ORIGINAL: 0x005D71F0 ??__Fg_BUFFER_SPRITE@@YAXXZ 0x005D71F0-0x005D720F
// TRIED     THE WHOLE ??__F FAMILY SHARES THIS WALL - 0x005D71F0, 0x0060D080 and 0x0063BB00 all measure 0 of 9 identically, so one spelling would move thirteen bodies and none does. The image keeps BOTH values in byte registers - `mov cl, [flags]; mov al, 2; test al, cl` ... `or cl, al; mov [flags], cl` - materialising the mask once and reusing it, where VC6 folds it into two immediates and emits one instruction FEWER (8 against the image's 9). Measured: a named `const uint8_t mask` (folded, 0/9), `flags & 2` operand order (0/9), `volatile uint8_t mask` (0/9), re-reading the flag byte at both sites (0/9), and widening both to `int` - the only one that moves, to 2/9, and only because it matches the instruction COUNT by adding `xor eax, eax` for zero-extension, which the image does not emit either. The flag search already picked /c /O2 /Gy /GR- /Oy- /GX as best. This is the allocator choosing to hold the mask, not a source form.
// symbol    ?teardown_g_buffer_sprite@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_g_buffer_sprite() {
    const uint8_t flags = *TeardownFlags009B37CC;
    if ((2 & flags) != 0) {
        return;
    }
    // Set BEFORE the teardown, not after: the original stores at the
    // instruction preceding its tail jump.
    *TeardownFlags009B37CC = static_cast<uint8_t>(flags | 2);
    g_BUFFER_SPRITE.close();
}

/*
Purpose: Atexit teardown thunk for g_CONSOLE_TIMER.
// ORIGINAL: 0x0050E9A0 ??__Fg_CONSOLE_TIMER@@YAXXZ 0x0050E9A0-0x0050E9AA BYTE_EXACT
// symbol    ?destroy_console_timer@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_console_timer() {
    reinterpret_cast<Time *>(0x00939E88)->Time::~Time();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_DEVICE.
// ORIGINAL: 0x004C5C70 ??__Fg_WAVE_DEVICE@@YAXXZ 0x004C5C70-0x004C5C7A BYTE_EXACT
// symbol    ?destroy_wave_device@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_device() {
    reinterpret_cast<Wave_Device *>(0x0090D978)->Wave_Device::~Wave_Device();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_CAVIAR.
// ORIGINAL: 0x004BF700 ??__Fg_VEHDRAW_CAVIAR@@YAXXZ 0x004BF700-0x004BF70A BYTE_EXACT
// symbol    ?destroy_vehdraw_caviar@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_caviar() {
    // A NAMED METHOD, not the seam. `Caviar::close` is in the tree now, at
    // 0x00617020, and the image tail-jumps straight to it.
    reinterpret_cast<Caviar *>(0x008CC828)->Caviar::close();
}


/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "filebox.h"
#include <cstring>

/*
Purpose: Set the default extension passed to the common dialog.
// ORIGINAL: 0x00634E80 ?set_def_ext@FileBox@@QAEHPAD@Z 0x00634E80-0x00634ECC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The source member is the four-byte inline extension buffer at +0x414, not an executable pointer or a dword value.","evidence":["Original 0x00634E80-0x00634ECC clears byte +0x414 on null and otherwise checks strlen against three before appending to that byte address.","The save/load dialog bodies in src/leaf_recoveries.cpp pass +0x414 to the common-dialog API; git:fee2514f:src/recovered/units/00634e80.cpp retains the historical candidate."]}
// symbol    ?set_def_ext@FileBox@@QAEHPBD@Z
*/
int FileBox::set_def_ext(const char *extension) {
    if (extension == 0) {
        def_ext_[0] = 0;
        return 0;
    }
    if (std::strlen(extension) > 3) {
        return 3;
    }
    def_ext_[0] = 0;
    strcat(def_ext_, extension);
    return 0;
}

/*
Purpose: Set the filename shown when the common dialog opens.
// ORIGINAL: 0x00634ED0 ?set_def_name@FileBox@@QAEHPAD@Z 0x00634ED0-0x00634F1E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The body writes the inline default-name buffer at +0x104, preserving the image's length comparison and null handling.","evidence":["Original 0x00634ED0-0x00634F1E checks strlen against 0x104, clears byte +0x104 and appends the argument there.","src/leaf_recoveries.cpp passes that buffer as OPENFILENAMEA.lpstrFileTitle; git:fee2514f:src/recovered/units/00634ed0.cpp retains the historical exact seed."]}
// symbol    ?set_def_name@FileBox@@QAEHPBD@Z
*/
int FileBox::set_def_name(const char *name) {
    if (name == 0) {
        def_name_[0] = 0;
        return 0;
    }
    // The original rejects > 0x104, not >=; its boundary case remains an
    // overflow obligation, not a silently changed behavior.
    if (std::strlen(name) > 0x104) {
        return 3;
    }
    def_name_[0] = 0;
    strcat(def_name_, name);
    return 0;
}

/*
Purpose: Set the initial directory for FileBox's common dialogs.
// ORIGINAL: 0x00634F20 ?UNK1@FileBox@@QAEHPAD@Z 0x00634F20-0x00634F6E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The unnamed member writes the inline initial-directory buffer at +0x418; the dialog consumers use that buffer as lpstrInitialDir.","evidence":["Original 0x00634F20-0x00634F6E clears +0x418 on null and otherwise checks strlen against 0x104 before appending to +0x418.","src/leaf_recoveries.cpp uses that address as OPENFILENAMEA.lpstrInitialDir; git:fee2514f:src/recovered/00634f20.cpp preserves the historical exact seed."]}
// symbol    ?set_initial_dir@FileBox@@QAEHPBD@Z
*/
int FileBox::set_initial_dir(const char *directory) {
    if (directory == 0) {
        initial_dir_[0] = 0;
        return 0;
    }
    if (std::strlen(directory) > 0x104) {
        return 3;
    }
    initial_dir_[0] = 0;
    strcat(initial_dir_, directory);
    return 0;
}

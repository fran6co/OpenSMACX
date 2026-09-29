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
#include <commdlg.h>
#include <cstring>

/*
Purpose: Construct the dialog's editable buffers and filter cursor.
// ORIGINAL: 0x00634BE0 ??0FileBox@@QAE@XZ 0x00634BE0-0x00634C13 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The constructor initializes the same inline buffers and filter cursor observed in FileBox's dialog consumers, replacing the unused raw-offset redirect.","evidence":["Original 0x00634BE0-0x00634C13 stores zero at +0x30d, +0, +0x30c, +0x104, +0x208, +0x414 and +0x418 and points +0x410 at +0x30c.","The source-owned fields and offsets are declared in src/filebox.h; historical exact redirect: git:a4e385ab:src/field_accessors.cpp."]}
// symbol    ??0FileBox@@QAE@XZ
*/
FileBox::FileBox() {
    filter_[1] = 0;
    filter_cursor_ = filter_;
    file_[0] = 0;
    filter_[0] = 0;
    def_name_[0] = 0;
    field_208_[0] = 0;
    def_ext_[0] = 0;
    initial_dir_[0] = 0;
}

/*
Purpose: Destruct the dialog's inline storage; there are no owned allocations.
// ORIGINAL: 0x00634C20 ??1FileBox@@QAE@XZ 0x00634C20-0x00634C21 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The original destructor is an empty return; FileBox's buffers are inline and the filter cursor is non-owning.","evidence":["Original 0x00634C20-0x00634C21 is a single ret instruction.","Historical exact redirect: git:a4e385ab:src/field_accessors.cpp."]}
// symbol    ??1FileBox@@QAE@XZ
*/
FileBox::~FileBox() {}

/*
Purpose: Reset the editable dialog buffers without reconstructing the object.
// ORIGINAL: 0x00634F70 ?UNK2@FileBox@@QAEXXZ 0x00634F70-0x00634FA1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The unnamed member resets the inline filter cursor and all seven leading characters, so it belongs to FileBox rather than a raw-offset helper.","evidence":["Original 0x00634F70-0x00634FA1 writes +0x410, +0x30d, +0x30c, +0, +0x104, +0x208, +0x414 and +0x418 in that order.","The source-owned fields and offsets are declared in src/filebox.h; historical exact redirect: git:a4e385ab:src/field_accessors.cpp."]}
// symbol    ?reset@FileBox@@QAEXXZ
*/
void FileBox::reset() {
    filter_cursor_ = filter_;
    filter_[1] = 0;
    filter_[0] = 0;
    file_[0] = 0;
    def_name_[0] = 0;
    field_208_[0] = 0;
    def_ext_[0] = 0;
    initial_dir_[0] = 0;
}

/*
Purpose: Show the common save dialog over FileBox's own editable buffers.
// ORIGINAL: 0x00634C30 ?save@FileBox@@QAEXXZ 0x00634C30-0x00634CEE FILE
// RECOVERY_BINDING: v1 {"reason":"The dialog fields are FileBox's inline filename, title, filter, extension and initial-directory buffers, not an independent leaf redirect.","evidence":["Original 0x00634C30-0x00634CEE calls strcat and GetSaveFileNameA using offsets +0, +0x104, +0x30c, +0x414 and +0x418.","The prior compiling redirect and its observations are preserved at git:06248782:src/leaf_recoveries.cpp."]}
// symbol    ?save@FileBox@@QAEXXZ
*/
void FileBox::save() {
    char *const file = file_;
    char *const title = def_name_;
    if (*title != 0) {
        *file = 0;
        strcat(file, title);
    }
    OPENFILENAMEA ofn;
    ofn.lStructSize = 0x4C;
    ofn.hwndOwner = 0;
    ofn.hInstance = 0;
    ofn.lpstrFilter = filter_;
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = file;
    ofn.nMaxFile = 0x104;
    ofn.lpstrFileTitle = title;
    ofn.nMaxFileTitle = 0x104;
    ofn.lpstrInitialDir = initial_dir_[0] ? initial_dir_ : 0;
    ofn.lpstrTitle = def_ext_[0] ? def_ext_ : 0;
    ofn.Flags = 0;
    ofn.nFileOffset = 0xE;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = 0;
    ofn.lCustData = 0;
    ofn.lpfnHook = 0;
    ofn.lpTemplateName = 0;
    GetSaveFileNameA(&ofn);
}

/*
Purpose: Show the common open dialog using FileBox's saved filter and defaults.
// ORIGINAL: 0x00634CF0 ?load@FileBox@@QAEXXZ 0x00634CF0-0x00634DAE FILE
// RECOVERY_BINDING: v1 {"reason":"The open dialog reads and writes the same inline FileBox buffers as the save dialog, with open-specific flags and default extension.","evidence":["Original 0x00634CF0-0x00634DAE calls strcat and GetOpenFileNameA with FileBox offsets +0, +0x104, +0x30c, +0x414 and +0x418.","The prior compiling redirect and its observations are preserved at git:06248782:src/leaf_recoveries.cpp."]}
// symbol    ?load@FileBox@@QAEXXZ
*/
void FileBox::load() {
    char *const file = file_;
    char *const title = def_name_;
    if (*title != 0) {
        *file = 0;
        strcat(file, title);
    }
    OPENFILENAMEA ofn;
    ofn.lStructSize = 0x4C;
    ofn.hwndOwner = 0;
    ofn.hInstance = 0;
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter = 0;
    ofn.lpstrFilter = filter_;
    ofn.nMaxFile = 0x104;
    ofn.nMaxFileTitle = 0x104;
    ofn.lpstrFile = file;
    ofn.lpstrFileTitle = title;
    ofn.lpstrInitialDir = initial_dir_[0] ? initial_dir_ : 0;
    ofn.lpstrTitle = 0;
    ofn.Flags = 0x180E;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = def_ext_[0] ? def_ext_ : 0;
    ofn.lCustData = 0;
    ofn.lpfnHook = 0;
    ofn.lpTemplateName = 0;
    GetOpenFileNameA(&ofn);
}

/*
Purpose: Append one label/pattern pair to the double-NUL filter buffer.
// ORIGINAL: 0x00634DB0 ?add_filter@FileBox@@QAEHPADPAD@Z 0x00634DB0-0x00634E73 FILE
// RECOVERY_BINDING: v1 {"reason":"The cursor at +0x410 advances within FileBox's inline filter buffer at +0x30c; this operation belongs to that class.","evidence":["Original 0x00634DB0-0x00634E73 checks two strings and available filter bytes, appends both pieces and updates +0x410.","The prior compiling redirect and its observations are preserved at git:06248782:src/leaf_recoveries.cpp."]}
// symbol    ?add_filter@FileBox@@QAEHPAD0@Z
*/
int FileBox::add_filter(char *pattern, char *label) {
    if (pattern != 0 && label != 0) {
        char *position = filter_cursor_;
        const unsigned int pattern_length = strlen(pattern);
        const unsigned int label_length = strlen(label);
        const int remaining = (0x104 - static_cast<int>(pattern_length))
                              - static_cast<int>(label_length);
        const int filter_used = static_cast<int>(position - filter_);
        if (remaining < filter_used) {
            return 4;
        }
        strcat(position, label);
        position += strlen(position) + 1;
        *position = 0;
        strcat(position, pattern);
        position += strlen(position) + 1;
        *position = 0;
        filter_cursor_ = position;
        return 0;
    }
    return 3;
}

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

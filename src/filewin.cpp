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
#include "filewin.h"
#include "sprite.h"
#include "text.h"
#include "dialog.h"
#include "basebutton.h"
#include "basepop.h"
#include "buffer.h"
#include "general.h"
#include "popup.h"
#include "temp.h"  // HandleMain

// RECOVERY_OBJECT: v2 0x009B90AC 0x009B90B0 ?ActiveFileWin@@3PAVFileWin@@A 0x00613D50
FileWin *ActiveFileWin = 0;

// The native 0x6139B0 init snapshots exactly 0x44 bytes here; the directory
// body consumes its first DWORD as ListBox attach style flags.
// RECOVERY_OBJECT: v1 0x009B8FD0 ?FileWinInitSharedParameters@@3UFileWinConstructPrefix@@A
FileWinConstructPrefix FileWinInitSharedParameters;

// Native init passes the empty string at 0x9B90B0 to CreateWindowExA. The
// loader-zero byte and its first-byte use are established; no larger capacity
// or neighboring extent is claimed.
// RECOVERY_OBJECT: v1 0x009B90B0 ?FileWinListWindowTitle@@3PADA
char FileWinListWindowTitle[1] = {0};

/*
Purpose: Append one default extension to FileWin's four-slot extension
         table at 0x518: five bytes per slot, the count at 0x514.

// ORIGINAL: 0x00614140 ?add_def_ext@FileWin@@QAEHPAD@Z 0x00614140-0x006141BC
// symbol   ?add_def_ext@FileWin@@QAEHPAD@Z
// size      124 bytes
// prototype int (__thiscall ?add_def_ext@FileWin@@QAEHPAD@Z)(FileWin* this, int8*)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645470
Return Value: 0 on success, 1 when the table is full, 0x10 for a null
              extension, 3 when it is longer than four characters
Status: Complete
*/
int FileWin::add_def_ext(char *extension) {
    if (extension_count_ == 4) {
        return 1;
    }
    if (extension == 0) {
        return 0x10;
    }
    char *const slot = extension_filters_[extension_count_];
    slot[0] = '\0';
    if (strlen(extension) > 4) {
        return 3;
    }
    strcat(slot, extension);
    ++extension_count_;
    return 0;
}

// ORIGINAL: 0x006141C0 ?set_def_name@FileWin@@QAEHPAD@Z 0x006141C0-0x006142C9
// RECOVERY_BINDING: v1 {"evidence": ["src/leaf_recoveries.cpp", "recovery/tasks/game-save-load-owner-coverage.json", "pinned image sha256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_filewin_default_name_behavior.py"], "reason": "Home the default-name split to the actual FileWin buffers and preserve null, overlength, root, drive, and original boundary behavior."}
// TRIED: plain strlen/strcat declarations get inlined by /O2, which the original does not do at this call site; the function-pointer cast idiom forces a real call and got the prologue and the backslash-search loop byte-exact. Still MISMATCH on the drive-letter append block's fall-through layout.
int FileWin::set_def_name(char *name) {
    if (name == 0) {
        file_name_[0] = '\0';
        return 0;
    }
    if (strlen(name) > 0x104) return 3;

    char working[0x104];
    working[0] = '\0';
    strcat(working, name);
    char *scan = working + strlen(working);
    char *tail;
    char *head;
    while (scan != working) {
        if (*scan == '\\') {
            if (scan == working) break;
            tail = scan + 1;
            *scan = '\0';
            head = working;
            goto have_name;
        }
        --scan;
    }
    tail = scan;
    head = 0;

have_name:
    file_name_[0] = '\0';
    strcat(file_name_, tail);
    if (head != 0) {
        if (head[1] == ':') strcat(head, "\\");
        if (strlen(head) <= 0x104) {
            directory_[0] = '\0';
            strcat(directory_, head);
        }
        return 0;
    }
    directory_[0] = '\0';
    return 0;
}

// The modal Win's focus-list nodes carry a window pointer at +4 and a next
// pointer at +0xC. WinNodeList owns the sequence and cursor; this view only
// describes a node encountered by the file-dialog callback.
struct FileWinFocusNode {
    void *tag;
    Win *window;
    void *payload;
    FileWinFocusNode *next;
};

// ORIGINAL: 0x00614B90 ?on_button_clicked@FileWin@@QAEXH@Z 0x00614B90-0x00614C40
// Historical seed evidence: a nested -2/-1/3 decision, with the -1 arm
// rewinding the modal WinNodeList before UNK4. Compiler branch ordering in
// the old standalone scaffold was inconclusive and is not a structural rule.
void FileWin::on_button_clicked(int button_id) {
    if (button_id == -2) {
        field_638_ = 0x12C;
        modal_window_->release_modal();
        return;
    }
    if (button_id == -1) {
        if (WinPendingFocus == static_cast<Win *>(&edit_box_)) {
            WinNodeList &focus = modal_window_->list_;
            if (focus.head_ != 0) {
                focus.tail_ = 0;
                focus.current_ = focus.head_;
                for (int index = 0; index < focus.count_; ++index) {
                    FileWinFocusNode *node =
                        static_cast<FileWinFocusNode *>(focus.current_);
                    if (node->window == static_cast<Win *>(&edit_box_)) {
                        break;
                    }
                    ++focus.tail_;
                    focus.current_ = node->next;
                }
            }
        }
        UNK4();
        return;
    }
    if (button_id == 3) {
        SetCurrentDirectoryA("..");
        populate_directory(".");
    }
}

// Confirmation builds the full selected path and its basename in FileWin
// storage, then releases or refocuses the modal owner through its real controls.
// Focus is captured before changing the editor selection.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x006146A0 ?UNK4@FileWin@@QAEXXZ 0x006146A0-0x00614B59
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/006146a0.cpp", "recovery/tasks/game-save-load-owner-coverage.json", "pinned image sha256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_filewin_unk4_full_branches.py", "recovery/tests/test_filewin_load_contract_behavior.py"], "reason": "Recover the complete FileWin confirmation path through its real selected-path buffers, EditBox/ListBox, modal owner, and focus-list lifecycle."}
void FileWin::UNK4() {
    Win *entry_focus = 0;
    if (modal_window_->list_.count_ != 0) {
        FileWinFocusNode *const node =
            static_cast<FileWinFocusNode *>(modal_window_->list_.current_);
        entry_focus = node->window;
    }

    edit_box_.select_all_text();
    edit_box_.redraw();
    purge_spaces(edit_box_.text_);

    char name_buffer[260];
    WIN32_FIND_DATAA selected_file;
    HANDLE selected_handle = INVALID_HANDLE_VALUE;
    bool selected_exists = false;
    name_buffer[0] = 0;
    strcat(name_buffer, edit_box_.text_);

    int status;
    if (strcmp(edit_box_.text_, "..") == 0) {
        status = 2;
    } else {
        char *const suffix = strrchr(name_buffer, '.');
        status = 0;
        if (suffix != name_buffer && extension_count_ > 0) {
            for (int extension_index = 0;
                 extension_index < extension_count_; ++extension_index) {
                if (suffix == 0) {
                    char *const appended_suffix = strrchr(name_buffer, '.');
                    if (appended_suffix != 0) *appended_suffix = 0;
                    strcat(name_buffer, extension_filters_[extension_index]);
                }

                WIN32_FIND_DATAA found;
                const HANDLE handle = FindFirstFileA(name_buffer, &found);
                if (handle != INVALID_HANDLE_VALUE) {
                    status = 1;
                    FindClose(handle);
                    break;
                }
                FindClose(handle);
            }
        }
    }

    if (status == 0) {
        if (populate_directory(edit_box_.text_) != 0) goto restore_dialog_focus;
    } else if (status != 1) {
        if (status == 2) {
            name_buffer[0] = 0;
            strcat(name_buffer, "..");
            populate_directory(name_buffer);
        }
        goto restore_dialog_focus;
    }

    selected_path_[0] = 0;
    selected_basename_[0] = 0;
    if (strchr(name_buffer, '\\') == 0) {
        GetCurrentDirectoryA(0x104, selected_path_);
        const size_t directory_length = strlen(selected_path_);
        if (name_buffer[0] != '\\' &&
            selected_path_[directory_length - 1] != '\\') {
            strcat(selected_path_, "\\");
        }
        strcat(selected_path_, name_buffer);
        strcat(selected_basename_, name_buffer);
    } else {
        if (name_buffer[1] != ':') {
            GetCurrentDirectoryA(0x104, selected_path_);
            if (name_buffer[0] != '\\') strcat(selected_path_, "\\");
        }
        strcat(selected_path_, name_buffer);
        char *const last_separator = strrchr(selected_path_, '\\');
        strcat(selected_basename_, last_separator + 1);
    }

    selected_handle = FindFirstFileA(name_buffer, &selected_file);
    selected_exists = selected_handle != INVALID_HANDLE_VALUE;
    if (selected_exists) FindClose(selected_handle);

    if (field_638_ == 0xC8) {
        if (selected_exists) {
            modal_window_->release_modal();
        } else {
            parse_says(0, name_buffer, -1, -1);
            char message[260];
            message[0] = 0;
            strcat(message, current_directory_);
            strcat(message, "\\");
            strcat(message, "jackal");
            char load_label[] = "FILEWIN_LOAD";
            pop_caption_title(message, load_label, 0x200000, 0);
        }
    } else if (field_638_ == 0x64) {
        if (selected_exists) {
            parse_says(0, name_buffer, -1, -1);
            char message[260];
            message[0] = 0;
            strcat(message, current_directory_);
            strcat(message, "\\");
            strcat(message, "jackal");
            char save_exists_label[] = "FILEWIN_SAVEEXISTS";
            if (pop_caption_title(message, save_exists_label, 0x200040, 0) != 0)
                goto restore_dialog_focus;
        } else {
            const char invalid_filename_chars[] = "\\\"/[]:;=";
            const size_t valid_prefix =
                strcspn(selected_basename_, invalid_filename_chars);
            if (valid_prefix != strlen(selected_basename_)) {
                char message[260];
                message[0] = 0;
                strcat(message, current_directory_);
                strcat(message, "\\");
                strcat(message, "jackal");
                char illegal_char_label[] = "FILEWIN_ILLEGALCHAR";
                pop_caption_title(message, illegal_char_label, 0x200040, 0);
                goto restore_dialog_focus;
            }
        }
        modal_window_->release_modal();
    }

restore_dialog_focus:
    modal_window_->set_dialog_focus(
        entry_focus == static_cast<Win *>(&edit_box_)
            ? static_cast<Win *>(&edit_box_)
            : static_cast<Win *>(&list_box_));
}

// ORIGINAL: 0x00613D50 ?save@FileWin@@QAEXXZ 0x00613D50-0x00613F5C
// symbol    ?save@FileWin@@QAEPADXZ
// The sibling load path differs in its action code and filename fallback.
// Both use FileWin's real directory, suffix and control subobjects instead
// of the old standalone unit's self-offset and IAT-pointer scaffolding.
// The retired seed's VC6 frame diverged at instruction zero: the original
// reserves 0x144 bytes without EBP, whereas the standalone spelling used
// an EBP frame for WIN32_FIND_DATA. Both image bodies also leave EAX as 0
// or this+0x208 despite their void-decorated catalogue symbols. load_map
// consumes that result through AlphaSave::load (5AB76A..5AB778).
char *FileWin::save() {
    return run_file_dialog(true);
}

// ORIGINAL: 0x00613F60 ?load@FileWin@@QAEXXZ 0x00613F60-0x00614133
// symbol    ?load@FileWin@@QAEPADXZ
char *FileWin::load() {
    return run_file_dialog(false);
}

char *FileWin::run_file_dialog(bool saving) {
    if (modal_window_ == 0) {
        return 0;
    }

    field_638_ = saving ? 0x64 : 0xC8;
    GetCurrentDirectoryA(260, current_directory_);
    previous_directory_[0] = 0;
    strcat(previous_directory_, current_directory_);
    populate_directory(directory_);

    if (file_name_[0] != 0) {
        char *suffix = strrchr(file_name_, '.');
        if (suffix == 0) {
            strcat(file_name_, extension_filters_[0]);
        } else {
            bool matched = false;
            for (int index = 0; index < extension_count_; ++index) {
                if (_strcmpi(suffix, extension_filters_[index]) == 0) {
                    matched = true;
                    break;
                }
                // The original load path clears a nonmatching name inside
                // the scan; save waits until every suffix has been tried.
                if (!saving) {
                    file_name_[0] = 0;
                }
            }
            if (saving && !matched) {
                file_name_[0] = 0;
            }
        }

        WIN32_FIND_DATAA found;
        HANDLE handle = FindFirstFileA(file_name_, &found);
        if (handle != INVALID_HANDLE_VALUE) {
            const int position = list_box_.UNK2(found.cFileName);
            if (position >= 0) {
                list_box_.set_selected_pos(position, 1);
                list_box_.redraw();
            }
            FindClose(handle);
        } else if (saving) {
            edit_box_.set_text(file_name_);
            edit_box_.select_all_text();
            edit_box_.redraw();
        }
    }

    ActiveFileWin = this;
    modal_window_->set_modal(1, 0, 0);
    modal_window_->hide();

    if (field_638_ != 0x12C) {
        GetCurrentDirectoryA(260, previous_directory_);
        if (strlen(previous_directory_) < 261) {
            directory_[0] = 0;
            strcat(directory_, previous_directory_);
        }
    }
    SetCurrentDirectoryA(current_directory_);
    // Native61410F..614132 tests the post-modal action after restoring CWD.
    // Cancellation does not clear a pre-existing filename or selected path.
    return field_638_ == 0x12C ? 0 : selected_path_;
}

// ORIGINAL: 0x00614360 ?UNK3@FileWin@@QAEHPAD@Z 0x00614360-0x00614694
// TRIED: MISMATCH #0 'sub' vs 'push' - original is FPO'd (no ebp frame, ebp free as a GP register); the rebuilt body gets a standard ebp frame instead. Not chased (820B, heaviest Win32-API surface in this batch: GetCurrentDirectoryA/ SetCurrentDirectoryA/DlgDirListA/FindFirstFileA/FindClose/ SendMessageA/GetLastError all called indirectly through the IAT-slot globals, matching the disasm's `call dword ptr [addr]` sites). `edi` = self+0x286c (the embedded ListBox) is constant for the whole function body past its one `lea`; the two vbtable-adjusted calls (remove_all, final slot001) go through a `StringStructThunk`/`VCall` shim built from the raw `*(int*)(*(int*)lb+N)` displacement, not from a modelled virtual-base layout.
// symbol    ?populate_directory@FileWin@@QAEHPAD@Z
// The two DlgDirListA passes fill the temporary Win32 list control, while
// ListBox owns the displayed entries. The suffix table is FileWin storage.
// Native finalization attaches on a missing DIB and refreshes an existing one;
// successful finalization clears markup state and hides the list.
int FileWin::populate_directory(char *directory) {
    if (directory == 0) {
        return 0;
    }

    GetCurrentDirectoryA(260, previous_directory_);
    if (!SetCurrentDirectoryA(directory) &&
        !SetCurrentDirectoryA(previous_directory_)) {
        return 0;
    }

    Dialog &dialog = list_box_;
    dialog.item_list_.remove_all();
    dialog.selected_position_ = 0;
    list_box_.field_18_ = 0;
    list_box_.field_20_ = 0;

    char entry[260];
    char pattern[260] = ".";
    // The original consumes this record even if FindFirstFileA fails. Keep
    // that error-path behavior pending a separately authorized bug fix.
    WIN32_FIND_DATAA found;
    const HWND list_handle = directory_list_handle_;
    if (!DlgDirListA(HandleMain, pattern, 1, 0, 0x8010)) {
        GetLastError();
        return 1;
    }
    const int directory_count = static_cast<int>(
        SendMessageA(list_handle, LB_GETCOUNT, 0, 0));
    for (int i = 0; i < directory_count; ++i) {
        SendMessageA(list_handle, LB_GETTEXT, i,
                     reinterpret_cast<LPARAM>(entry));
        if (strcmp(entry, "[..]") == 0) {
            list_box_.item("..", 2);
        } else {
            const size_t length = strlen(entry);
            entry[length - 1] = 0;
            const HANDLE handle = FindFirstFileA(entry + 1, &found);
            FindClose(handle);
            list_box_.item(found.cFileName, 0);
        }
    }

    strcpy(pattern, ".");
    if (!DlgDirListA(HandleMain, pattern, 1, 0, 0x20)) {
        GetLastError();
        return 1;
    }
    const int file_count = static_cast<int>(
        SendMessageA(list_handle, LB_GETCOUNT, 0, 0));
    for (int file_index = 0; file_index < file_count; ++file_index) {
        SendMessageA(list_handle, LB_GETTEXT, file_index,
                     reinterpret_cast<LPARAM>(entry));
        const char *suffix = strrchr(entry, '.');
        if (suffix == 0) {
            continue;
        }
        for (int filter = 0; filter < extension_count_; ++filter) {
            if (_strcmpi(suffix, extension_filters_[filter]) == 0) {
                const HANDLE handle = FindFirstFileA(entry, &found);
                FindClose(handle);
                list_box_.item(found.cFileName, 1);
                break;
            }
        }
    }

    const int attach_or_refresh_result = list_box_.has_dib_bits()
        ? list_box_.update_changes()
        : dialog.attach(
              modal_window_,
              static_cast<int>(dialog.field_24_),
              static_cast<int>(dialog.field_28_),
              FileWinInitSharedParameters.field_00_ != 0
                  ? FileWinInitSharedParameters.field_00_
                  : 0x80060);
    if (attach_or_refresh_result != 0) {
        return 1;
    }

    list_box_.set_markup_enabled(0);
    list_box_.show(0);
    return directory_count;
}

// The default file-name suffix appended to field_531_ at construction - REAL
// STORAGE seeded from the image's own .data bytes (image_data.py, 0x00697118:
// 2e 00 00 00 - a "." entry in the dword-slotted text table that continues
// with the second ".", "LISTBOX" and "jackal.pcx" further on). It was
// `static char *const FileWinTextTableDefault = (char *)0x00697118`, pointing
// at the image's mapped-in data; the array carries the bytes instead. strcat
// reads up to the first NUL either way, and the `push offset` folds to the
// same instruction, the displacement relocated.
static const char FileWinTextTableDefault[4] = {'.', 0, 0, 0};  // 0x00697118

// The table's NEXT entry (0x0069711C), also "." in the shipped image:
// close() resets the name buffer to it, exactly as the constructor resets
// the buffer to the first. Same real-storage argument as above.
static const char FileWinTextTableDefaultNext[4] = {'.', 0, 0, 0};  // 0x0069711C

// Original .data at 0x00697128 contains "jackal.pcx"; the filename is an
// ordinary source literal, not a pointer into the shipped executable.
static const char FileWinSpriteSheet[] = "jackal.pcx";

// Three independently owned dialog sprites. The image places them at
// 0x009B9018, 0x009B9048 and 0x009B9078, but those are original storage
// identities, not addresses for this build to dereference. Their particular
// UI roles have not been established, so retain neutral names.
static Sprite FileWinSprite0;
static Sprite FileWinSprite1;
static Sprite FileWinSprite2;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x006139B0 ?init@FileWin@@QAEHPADPAUGraphicWin@@PAUFileWinConstruct@@@Z 0x006139B0-0x00613B97
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/006139b0.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover FileWin control initialization through genuine embedded controls, typed caller geometry, shared 17-DWORD parameter prefix and Windows list lifecycle. Retain the historical seed and obligations; no matching or game acceptance claim."}
// symbol    ?init@FileWin@@QAEHPADPAVGraphicWin@@PAUFileWinConstruct@@@Z
int FileWin::init(char *, GraphicWin *parent, FileWinConstruct *parameters) {
  if (parameters == 0 || parent == 0) {
    return 0x10;
  }

  memcpy(&FileWinInitSharedParameters, parameters,
         sizeof(FileWinConstructPrefix));

  modal_window_ = parent;
  SpriteScaleNumerator = 1;
  SpriteScaleDenominator = 1;

  int result = prepare_sprites();
  if (result != 0) {
    return result;
  }

  edit_box_bounds_ = parameters->edit_box_bounds_;
  list_box_bounds_ = parameters->list_box_bounds_;

  result = edit_box_.init(
      edit_box_bounds_.left, edit_box_bounds_.top,
      edit_box_bounds_.right - edit_box_bounds_.left,
      edit_box_bounds_.bottom - edit_box_bounds_.top,
      static_cast<Win *>(parent), 0);
  if (result != 0) {
    return result;
  }
  edit_box_.field_B14_ = 0x104;
  edit_box_.show(0);

  const RECT &button2 = parameters->button2_bounds_;
  result = flat_button2_.init(
      BasePopDefaultOkText, -1, button2.left, button2.top,
      button2.right - button2.left, button2.bottom - button2.top,
      static_cast<Win *>(parent), 0);
  if (result != 0) {
    return result;
  }

  const RECT &button1 = parameters->button1_bounds_;
  result = flat_button1_.init(
      BasePopDefaultCancelText, -2, button1.left, button1.top,
      button1.right - button1.left, button1.bottom - button1.top,
      static_cast<Win *>(parent), 0);
  if (result != 0) {
    return result;
  }

  list_box_.close();
  result = list_box_.Dialog::init(&list_box_bounds_, 0);
  if (result != 0) {
    return result;
  }

  list_box_.field_8_ = 1;
  static_cast<Dialog &>(list_box_).field_34_ =
      (FileWinSprite0.width() * 3) / 2;
  list_box_.set_callbacks(&filebox_draw, 0, 0);

  directory_list_handle_ = CreateWindowExA(
      0, "LISTBOX", FileWinListWindowTitle, 0x40000040,
      0, 0, CW_USEDEFAULT, CW_USEDEFAULT, HandleMain,
      reinterpret_cast<HMENU>(1), WinInstance, 0);
  return directory_list_handle_ == 0 ? 1 : 0;
}

// ORIGINAL: 0x00614C50 ?filebox_draw@FileWin@@QAAXPAUGraphicWin@@HHHHH@Z 0x00614C50-0x00614D84 BYTE_EXACT
// symbol    ?filebox_draw@@YAXPAVGraphicWin@@HHHHH@Z
// The six-argument callback has no hidden FileWin receiver. The three
// process sprites belong to this TU; their width, height and transparent
// index are Sprite members rather than separate address-bound integers.
void __cdecl filebox_draw(GraphicWin *window, int item_kind, int,
                         int x, int y, int height) {
    if (CurrentListBoxDraw == 0) {
        return;
    }
    switch (item_kind) {
    case 0: {
        Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
        FileWinSprite1.draw(canvas, FileWinSprite1.transparent_index(),
                            x + FileWinSprite1.width() / 4,
                            y + (height - FileWinSprite1.height()) / 2, 1, 1);
        break;
    }
    case 1: {
        Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
        FileWinSprite0.draw(canvas, FileWinSprite0.transparent_index(),
                            x + FileWinSprite1.width() / 4,
                            y + (height - FileWinSprite0.height()) / 2, 1, 1);
        break;
    }
    case 2: {
        Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
        FileWinSprite2.draw(canvas, FileWinSprite2.transparent_index(),
                            x + FileWinSprite1.width() / 4,
                            y + (height - FileWinSprite2.height()) / 2, 1, 1);
        break;
    }
    }
}

/*
Purpose: Compose a FileWin from four member subobjects (two FlatButtons, an
         EditBox, a most-derived ListBox), then clear the window's own
         name-buffer/selection fields and append one text-table entry.
// ORIGINAL: 0x00613850 ??0FileWin@@QAE@XZ 0x00613850-0x006138F9;0x006630E0-0x00663114
// size      221 bytes
// prototype void (__thiscall ??0FileWin@@QAE@XZ)(FileWin* this)
// callers   1   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00607CF0 0x00609DB0 0x00614E50 0x00645470
// TRIED: MEASURED 8/42 agreeing - the SEH prologue itself matches
//            (`push -1/handler; mov eax,fs:[0]; push eax; mov fs:[0],esp`,
//            instructions 0-6 all agree), so this is NOT the CheckBox/
//            EditGroup family's esp-vs-ebp divergence. The first mismatch is
//            register allocation for the four member pointers (`push
//            esi/mov esi,ecx` vs this body's extra `push ebp`), which
//            plain member declarations (relying on FlatButton's own real
//            ctor for implicit construction) did not converge on.
Return Value: n/a
Status: Complete
*/
FileWin::FileWin() {
    // The declared members construct once, in declaration order. ListBox's
    // most-derived flag is a compiler ABI argument, not a second construction.
    selected_path_[0] = 0;
    selected_basename_[0] = 0;
    file_name_[0] = 0;
    extension_count_ = 0;
    directory_[0] = 0;
    strcat(directory_, FileWinTextTableDefault);
    modal_window_ = 0;
    directory_list_handle_ = 0;
}

/*
Purpose: Tear down a FileWin: close it, then destroy the ListBox, EditBox and
         two FlatButton members in reverse declaration order. C++ owns the
         member/base destruction and partial-construction unwind.
// ORIGINAL: 0x00407F00 ??1FileWin@@QAE@XZ 0x00407F00-0x00408003;0x00650C80-0x00650CE8
// size      363 bytes
// prototype void (__thiscall ??1FileWin@@QAE@XZ)(FileWin* this)
// callers   2   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x00613900 0x00614F30 0x00616200
// TRIED: MEASURED 14/62 agreeing with named members - better than the
//            preserved artifact's raw-offset form (src/recovered/units/
//            00407f00.cpp, since deleted, reported divergence 1 against a
//            DIFFERENT flag set/scaffold). First mismatch is again register
//            allocation (`sub esp,8` vs `,0xc`; `push esi` vs `push ebx`)
//            starting right after the SEH prologue, which matches through
//            instruction 6.
Return Value: n/a
Status: Complete
*/
FileWin::~FileWin() {
    close();
}

/*
Purpose: Close the file dialog: close the two button strips, the edit box
         and the list (each a virtual or member close on its subobject),
         reset the window's own name/selection fields and append the text
         table's second '.' entry, close the dialog's three fixed sprites,
         drop the selection word, and destroy the dialog's own window
         handle if one was created.

// ORIGINAL: 0x00613900 ?close@FileWin@@QAEXXZ 0x00613900-0x006139AC FILE
// size      172 bytes
// prototype void (__thiscall ?close@FileWin@@QAEXXZ)(FileWin* this)
// callers   5   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3820 0x00609F20 0x00614F30 0x00645470
// indirect  0x00613910 0x00613922 0x0061399D
// TRIED (seed): DestroyWindowFn via *g_0066930c (established lever from
//         00635750.cpp/00635450.cpp) compiles clean; mnemonic sim 0.95,
//         one pop/pop vs add-esp reorder right after the strcat call (#23)
//         not chased further. The two indirect sites are the FlatButton
//         slot-90 closes (BaseButton's virtual close, basebutton.h); the
//         third is user32!DestroyWindow through its import slot.
Status: Coverage transcription - unclaimed body
*/
void FileWin::close() {
    flat_button2_.close();
    flat_button1_.close();
    edit_box_.close();
    list_box_.close();
    selected_path_[0] = 0;
    selected_basename_[0] = 0;
    file_name_[0] = 0;
    extension_count_ = 0;
    directory_[0] = 0;
    strcat(directory_, FileWinTextTableDefaultNext);
    FileWinSprite0.close();
    FileWinSprite1.close();
    FileWinSprite2.close();
    modal_window_ = 0;
    if (directory_list_handle_ != 0) {
        DestroyWindow(directory_list_handle_);
        directory_list_handle_ = 0;
    }
}

// ORIGINAL: 0x00613BA0 ?prepare_sprites@FileWin@@QAEHXZ 0x00613BA0-0x00613D47;0x00663114-0x00663129
// FileWin::prepare_sprites builds the three independent dialog sprites from
// the PCX sheet, or creates blank substitutes when the sheet cannot load.
// The original has a separate cold unwind span; the source-owned Buffer
// lifetime lets C++ generate that cleanup without encoding its old address.
int FileWin::prepare_sprites() {
    Buffer sheet;
    int result;
    if (sheet.load_pcx(FileWinSpriteSheet, 0, 10, 0xEC) == 0) {
        result = FileWinSprite1.extract(&sheet, 9, 1, 1, 0x10, 0x10, 0);
        if (result != 0) return result;
        result = FileWinSprite0.extract(&sheet, 9, 0x12, 1, 0x10, 0x10, 0);
        if (result != 0) return result;
        result = FileWinSprite2.extract(&sheet, 9, 0x23, 1, 0x10, 0x10, 0);
    } else {
        result = FileWinSprite1.create_blank(0x10, 0x10, 9);
        if (result != 0) return result;
        result = FileWinSprite0.create_blank(0x10, 0x10, 9);
        if (result != 0) return result;
        result = FileWinSprite2.create_blank(0x10, 0x10, 9);
    }
    return result;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00614B80 ?UNK6@FileWin@@QAEXXZ 0x00614B80-0x00614B81 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@FileWin@@QAEXXZ)(FileWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK6() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00614350 ?UNK2@FileWin@@QAEXH@Z 0x00614350-0x00614353 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK2@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK2(int) {
}


/*
Purpose: Clear the file window's three name buffers, its selection index and
         one trailing flag.
// ORIGINAL: 0x00614320 ?UNK1@FileWin@@QAEXXZ 0x00614320-0x00614341 BYTE_EXACT
// size      33 bytes
// prototype void (__thiscall ?UNK1@FileWin@@QAEXXZ)(FileWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK1() {
    selected_path_[0] = 0;
    selected_basename_[0] = 0;
    file_name_[0] = 0;
    directory_[0] = 0;
    extension_count_ = 0;
}

// Copy a caller-supplied path into FileWin's own directory storage. A null
// path clears it; an overlong path leaves the previous value intact.
// ORIGINAL: 0x006142D0 ?set_directory@FileWin@@QAEHPAD@Z 0x006142D0-0x0061431E BYTE_EXACT
int FileWin::set_directory(char *directory) {
    if (directory == 0) {
        directory_[0] = 0;
        return 0;
    }
    if (strlen(directory) > 0x104) {
        return 3;
    }
    directory_[0] = 0;
    strcat(directory_, directory);
    return 0;
}

// This was a fabricated slot-58 shim in the exact seed. Win establishes
// slot 58 as release_modal, and field +0x63C holds that Win receiver.
// ORIGINAL: 0x00614B60 ?UNK5@FileWin@@QAEXXZ 0x00614B60-0x00614B78 BYTE_EXACT
void FileWin::UNK5() {
    field_638_ = 0x12c;
    modal_window_->release_modal();
}


/*
// ORIGINAL: 0x00614C40 ?on_double_clicked@FileWin@@QAEXH@Z 0x00614C40-0x00614C48 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?on_double_clicked@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006146A0
Status: Complete
*/
void FileWin::on_double_clicked(int a1) {
    UNK4();
}



/*
// ORIGINAL: 0x00614D90 ?init_class@FileWin@@QAAHXZ 0x00614D90-0x00614E2E BYTE_EXACT
// symbol    ?init_class@FileWin@@SAHXZ
// size      158 bytes
// prototype
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005FD550 0x005FD570 0x00644EF2 0x006453E0 0x00645470
//
// Promoted 2026-08-15 from src/unrecovered/00614d90.cpp, where it measured
// BYTE_EXACT as a self-contained FILE unit, to retire its pending_bodies
// forwarder. The FILE unit forced the strlen/strcat calls through `_strlen`/
// `_strcat` externs - one underscore too deep to LINK, a debt FILE-mode
// measurement never sees. The original plainly called the CRT, so this does
// too; the measurement scaffold supplies the `#pragma function` that keeps
// the calls real where the image has real calls (string_routine_pragma).
Status: Complete
*/
// 0x006971CC and 0x006971D4 - the image stores both names INLINE in .rdata
// ("FILEWIN", then "jackal"), so the tree stores them inline too. The raw
// fixed-address bindings read unmapped memory in the standalone build, which
// is where the boot's `return 6` out of jackal_init_real came from.
static const char kFileWinSection[] = "FILEWIN";  // 0x006971CC
static const char kFileWinTextName[] = "jackal";  // 0x006971D4
// 0x009B90A8 - the loaded file's text buffer: init_class stages text_get()'s
// string into it through mem_get, and close_class frees it once. THE OBJECT,
// not the `static int *const s_filewin_text_ptr = (int *)0x009B90A8` binding:
// the slot itself is real storage now, a pointer-sized object seeded zero
// like the image's, and every `*s_filewin_text_ptr` read/write through the
// binding becomes a read/write of the object at the same folded addresses.
static char *s_filewin_text_ptr = nullptr;  // 0x009B90A8

int __cdecl FileWin::init_class() {
    if (s_filewin_text_ptr != nullptr) {
        free(s_filewin_text_ptr);
        s_filewin_text_ptr = nullptr;
    }
    if (text_open(kFileWinTextName, kFileWinSection) != 0) {
        return 6;
    }
    char *str = text_get();
    if (str == 0) {
        return 1;
    }
    if (s_filewin_text_ptr != nullptr) {
        free(s_filewin_text_ptr);
        s_filewin_text_ptr = nullptr;
    }
    s_filewin_text_ptr = static_cast<char *>(mem_get(strlen(str) + 1));
    if (s_filewin_text_ptr == nullptr) {
        return 4;
    }
    *s_filewin_text_ptr = 0;
    strcat(s_filewin_text_ptr, str);
    return 0;
}

/*
Purpose: Free the file text the init_class staged, once. Reads no state of
         its own - the object is irrelevant to the body - and is called by
         jackal_close with no receiver set up at all.
// ORIGINAL: 0x00614E30 ?close_class@FileWin@@QAAXXZ 0x00614E30-0x00614E4D BYTE_EXACT
// symbol    ?close_class@FileWin@@QAAXXZ
// size      29 bytes
// callers   1   call targets   1
// kind      game
// calls     0x00644EF2
//
// PROMOTED out of the archived verification unit for this address. The free is the same
// CRT callee init_class above reaches with the same `free` spelling; the
// freed pointer is read once and the null test jumps to the plain return.
// jackal_close still reaches this through the free forwarder
// filewin_close_class (src/pending_bodies.cpp), which now forwards to the
// member - the image's call there sets up no receiver, and a member call
// cannot spell that.
Return Value: n/a
Status: Complete
*/
void FileWin::close_class() {
    char *value = s_filewin_text_ptr;
    if (value) {
        free(value);
        s_filewin_text_ptr = nullptr;
    }
}

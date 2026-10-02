#include <cassert>
#include <cstdint>
#include <cstring>

struct OPENFILENAMEA {
    uint32_t lStructSize;
    void *hwndOwner, *hInstance;
    const char *lpstrFilter;
    char *lpstrCustomFilter;
    uint32_t nMaxCustFilter, nFilterIndex;
    char *lpstrFile;
    uint32_t nMaxFile;
    char *lpstrFileTitle;
    uint32_t nMaxFileTitle;
    const char *lpstrInitialDir, *lpstrTitle;
    uint32_t Flags;
    uint16_t nFileOffset, nFileExtension;
    const char *lpstrDefExt;
    intptr_t lCustData;
    void *lpfnHook;
    const char *lpTemplateName;

    // Instrumentation only: observe that the native-unwritten field is not
    // deliberately changed. This is not a target stack-initialization claim.
    OPENFILENAMEA() : nFilterIndex(0xC0FFEEu) {}
};

/* ACTUAL_FILEBOX_CLASS */

static FileBox *expected_box;
static int dialog_result;
static int dialog_calls;
static bool publish_filename;

int GetSaveFileNameA(OPENFILENAMEA *ofn) {
    ++dialog_calls;
    assert(ofn->lStructSize == 0x4C);
    assert(ofn->hwndOwner == 0 && ofn->hInstance == 0);
    assert(ofn->lpstrFilter == expected_box->filter_);
    assert(ofn->lpstrCustomFilter == 0 && ofn->nMaxCustFilter == 0);
    assert(ofn->nFilterIndex == 0xC0FFEEu);
    assert(ofn->lpstrFile == expected_box->file_ && ofn->nMaxFile == 0x104);
    assert(ofn->lpstrFileTitle == expected_box->def_name_);
    assert(ofn->nMaxFileTitle == 0x104);
    assert(ofn->lpstrInitialDir ==
           (expected_box->initial_dir_[0] ? expected_box->initial_dir_ : 0));
    assert(ofn->lpstrTitle == 0 && ofn->Flags == 0xE);
    assert(ofn->nFileOffset == 0 && ofn->nFileExtension == 0);
    assert(ofn->lpstrDefExt ==
           (expected_box->def_ext_[0] ? expected_box->def_ext_ : 0));
    assert(ofn->lCustData == 0 && ofn->lpfnHook == 0 && ofn->lpTemplateName == 0);
    if (publish_filename) std::strcpy(ofn->lpstrFile, "chosen.SC");
    return dialog_result;
}

/* ACTUAL_FILEBOX_BODIES */

int main() {
    FileBox box;
    expected_box = &box;
    dialog_result = 0;
    publish_filename = false;
    std::strcpy(box.file_, "prior.SC");
    std::strcpy(box.def_name_, "default.SC");
    std::strcpy(box.initial_dir_, "maps");
    std::strcpy(box.def_ext_, "SC");
    assert(box.save() == 0);
    assert(std::strcmp(box.file_, "default.SC") == 0);

    dialog_result = 1;
    publish_filename = true;
    assert(box.save() == box.file_);
    assert(std::strcmp(box.file_, "chosen.SC") == 0);

    // Empty defaults and a noncanonical nonzero BOOL still preserve the API.
    box.def_name_[0] = 0;
    box.initial_dir_[0] = 0;
    box.def_ext_[0] = 0;
    dialog_result = -1;
    publish_filename = false;
    assert(box.save() == box.file_);
    assert(std::strcmp(box.file_, "chosen.SC") == 0);
    assert(dialog_calls == 3);
}

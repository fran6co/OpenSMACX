// ORIGINAL: 0x0062BF20 ??0PushButton@@QAE@XZ 0x0062BF20-0x0062C00E FILE
// TRIED: the scaffold's `class PushButton` has no base class (unlike src/pushbutton.h), so genuine base-subobject construction is unreachable; `new (this) T()` on a locally-declared `struct : BaseButton {}` compiles but adds an operator-new call the original does not make. Landed without the BaseButton() call - every other mnemonic/operand (vtable stores + all 18 field copies) matches.
// TRIED: SEH frame, MEASURED 2026-08-21 - the image here (flags carry no `frame`) is the same shape as FlatButton::FlatButton()/PullDown::PullDown() (see flatbutton.cpp for the full measurement) - a real derived constructor whose ONLY call is to a `construct()`-style method (`0x00606F30` = `?construct@BaseButton@@QAEXXZ`). Modelling `PushButton : public BaseButton` for real and calling `BaseButton::construct()` from a real `PushButton::PushButton()` reproduces an SEH frame the image does not pay for - confirmed on FlatButton's identical shape by swapping every by-value member up the GraphicWin/Win/BaseButton chain for raw storage (frame stays) and by a direct `/GX-` recompile (frame drops, but regresses the already-BYTE_EXACT `Buffer::~Buffer()` at 0x005D7410, which needs `/GX` for its own, real, image-matching frame). Not reachable from `src/` alone; this is why the scaffold's no-base-class workaround exists.
// size      238 bytes
// prototype void (__thiscall ??0PushButton@@QAE@XZ)(PushButton* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606F30
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x0062BF20

PushButton::PushButton() {
    volatile uint32_t *const object = reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const defaults =
        reinterpret_cast<volatile const uint32_t *>(0x009BC118);
    object[0x000 / 4] = 0x00670C60;
    object[0x444 / 4] = 0x00670C58;
    object[0xAB8 / 4] = defaults[0];
    object[0xABC / 4] = defaults[1];
    object[0xAC0 / 4] = defaults[2];
    object[0xAC4 / 4] = defaults[3];
    object[0xAC8 / 4] = defaults[4];
    object[0xACC / 4] = defaults[5];
    object[0xAD0 / 4] = defaults[6];
    object[0xAD4 / 4] = defaults[7];
    object[0xAD8 / 4] = defaults[8];
    object[0xADC / 4] = defaults[9];
    object[0xAE0 / 4] = defaults[10];
    object[0xAE4 / 4] = defaults[11];
    object[0xAE8 / 4] = defaults[12];
    object[0xAEC / 4] = defaults[13];
    object[0xAF0 / 4] = defaults[14];
    object[0xAF4 / 4] = defaults[15];
    object[0xAF8 / 4] = defaults[16];
    object[0xAFC / 4] = defaults[17];
}

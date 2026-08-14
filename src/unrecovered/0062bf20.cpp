// ORIGINAL: 0x0062BF20 FILE
// RULED-OUT: the scaffold's `class PushButton` has no base class (unlike
//            src/pushbutton.h), so genuine base-subobject construction is
//            unreachable; `new (this) T()` on a locally-declared
//            `struct : BaseButton {}` compiles but adds an operator-new call
//            the original does not make. Landed without the BaseButton()
//            call - every other mnemonic/operand (vtable stores + all 18
//            field copies) matches.
// name      ??0PushButton@@QAE@XZ
// size      238 bytes
// spans     0x0062BF20-0x0062C00E
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

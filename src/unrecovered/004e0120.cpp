// ORIGINAL: 0x004E0120 ?editor_energy@Console@@QAEXXZ 0x004E0120-0x004E020E
// TRIED: statement order for the two loop-body stores
// size      238 bytes
// prototype void (__thiscall ?editor_energy@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041E790 0x004D9AD0 0x00514880 0x005ABF20 0x005BFD10 0x00625E30 0x00625EC0
// (`DAT_009bbfec`/`DAT_009bbff0`) ahead of `parse_says`, and reordering
// them relative to each other - every spelling tried lands the same 76
// mnemonics, but the original interleaves `push edi` BETWEEN the two
// stores (push -1,-1,esi; store bbfec; push edi; store bbff0; call) where
// this body's natural statement order schedules both stores before the
// 4th push. Two-instruction swap at #34, nothing else diverges.

void Console::editor_energy() {
    auto_undo();
    int iVar1 = editor_who(*g_00939284, 1);
    if (iVar1 >= 0) {
        parse_says(0, reinterpret_cast<char *>(g_00946d54) + iVar1 * 0x59c, -1, -1);
        int iVar4 = 1;
        char *puVar3 = reinterpret_cast<char *>(g_00947020);
        char *puVar2 = reinterpret_cast<char *>(g_0096eccc);
        do {
            *g_009bbfec = *reinterpret_cast<int *>(puVar3 - 0x34);
            *g_009bbff0 = 0;
            parse_says(iVar4, puVar3, -1, -1);
            parse_num(iVar4, *reinterpret_cast<int *>(puVar2));
            puVar3 = puVar3 + 0x59c;
            iVar4 = iVar4 + 1;
            puVar2 = puVar2 + 0x20cc;
        } while (reinterpret_cast<int>(puVar3) < reinterpret_cast<int>(g_00949764));

        int idx = iVar1 * 0x833;
        int *table = reinterpret_cast<int *>(g_0096cc00);
        int iVar5 = X_pop_ask_number(reinterpret_cast<const char *>(g_006890e0), table[idx], 0);
        if (iVar5 == 0) {
            table[idx] = *g_009bb598;
            reinterpret_cast<BaseWin *>(g_006a7628)->on_redraw();
            reinterpret_cast<Console *>(g_009156b0)->update_data(0);
        }
    }
}

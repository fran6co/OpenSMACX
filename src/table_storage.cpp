#include "stdafx.h"
#include "table_storage.h"

#include <stdlib.h>

/*
Purpose: Free a table of records holding three pointers each, then the
         table itself. The original-owner, producer, and full-allocation
         extent remain unresolved; the typed view represents only the
         observed receiver and row prefixes.

         The original reloads the table before each row field and reloads the
         signed count at loop end. Keep these reads across external free calls.
         The initial null-table gate, row order, table-free/clear order, and
         unconditional count/word-8 clears are all load-bearing behavior.

// RECOVERY_ROLE: SCAFFOLD Bounded pointer-table cleanup method; original class, producer and complete storage lifetime remain unresolved.
// ORIGINAL: 0x00616B40 sub_616b40 0x00616B40-0x00616BB6 FILE
// RECOVERY_BINDING: v1 {"reason":"Express the observed table/count/word receiver as an ordinary member cleanup in the table-storage component, preserving callback-visible rereads without asserting VoxInstance, CvrPart or an original class name.","evidence":["pinned:0x00616B40-0x00616BB6 reads receiver+0 table and signed +4 count, frees three pointer positions per twelve-byte row, reloads table per field and count per iteration, then frees the final table and clears +0/+4/+8","source:PointerTripletTable models these accessed receiver words only; producer and complete original allocation remain unknown"]}
// symbol   ?release_contents@PointerTripletTable@@QAEXXZ
// size      118 bytes
// prototype unresolved; original ECX receiver with unused EDX
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void PointerTripletTable::release_contents() {
    if (table_ != 0) {
        for (int index = 0; index < count_; ++index) {
            if (table_[index].first_ != 0) {
                free(table_[index].first_);
            }
            if (table_[index].second_ != 0) {
                free(table_[index].second_);
            }
            if (table_[index].third_ != 0) {
                free(table_[index].third_);
            }
        }
        if (table_ != 0) {
            free(table_);
        }
        table_ = 0;
    }
    count_ = 0;
    field_08_ = 0;
}

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
#include "playerlock.h"

/*
Purpose: Reset both lock entries to their unset sentinels and mark the lock
         inactive.
Original Offset: 0058FF70
Return Value: n/a
Status: Complete
*/
void PlayerLock::clear() {
    for (size_t index = 0; index < 2; ++index) {
        entries_[index].first = -1;
        entries_[index].second = -1;
        entries_[index].flag = 0;
    }
    active_ = 0;
}

void __fastcall player_lock_clear_redirect(PlayerLock *self, void *) {
    self->clear();
}

/*
Purpose: Report whether either lock entry is engaged - the low bit of its flag
         set. The original is typed as returning void but computes this boolean
         in EAX, so the value is returned here to match its behaviour rather
         than its declared type.
Original Offset: 0058FFA0
Return Value: 1 when an entry is engaged, 0 when neither is
Status: Complete
*/
int PlayerLock::active() {
    for (int index = 0; index < 2; ++index) {
        if (entries_[index].flag & 1) {
            return 1;
        }
    }
    return 0;
}

int __fastcall player_lock_active_redirect(PlayerLock *self, void *) {
    return self->active();
}

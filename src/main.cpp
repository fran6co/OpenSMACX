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

/*
 * THE LINK CHECK. This entry point exists so `cmake --build` still answers a
 * real question about the recovered tree: does all of it compile, and does
 * every symbol it names resolve?
 *
 * It replaces `DllMain`. Until 2026-08-12 the buildable artifact was a DLL that
 * injected into the shipped executable - 2,049 fixed-address redirects
 * installed at process attach, each validating a byte signature before writing
 * an `E9 rel32` jump. That machinery is retired (docs/RETIRED_ROUTES.md) while
 * the recovery is finished by byte-matching, which runs nothing. It can come
 * back if the recovery turns out not to reach far enough to stand alone.
 *
 * WHY AN EXECUTABLE AND NOT A LIBRARY. Every object listed on the target is
 * linked directly rather than pulled from an archive on demand, so EVERY
 * undefined symbol in any recovered translation unit has to resolve. That is
 * the property worth keeping: a body that calls a function nobody has written
 * yet, or that names a member no header declares, fails here. A DLL gave the
 * same guarantee only for the 488 names its `.def` happened to export.
 *
 * WHY MAIN DOES NOTHING. Running the recovered code needs the game's data, its
 * globals initialised, and the original process image - none of which exists
 * without the injection route this replaced. Pretending otherwise by calling
 * into it would produce a binary that links and then faults, which is a worse
 * signal than one that links and exits. The question this build answers is
 * "does it compile and link", and it answers exactly that.
 */
int main() {
    return 0;
}

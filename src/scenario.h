/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

class Console;

int __cdecl scenario_top_menu(int mode);
void __cdecl scenario_opening_movie(char *movie_name);
void scenario_human_turn_trampoline();
void scenario_turn_advance_trampoline();

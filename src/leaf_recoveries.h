/*
 * Hand-recovered leaf functions. See src/leaf_recoveries.cpp for why these are
 * written out rather than generated, and for the disassembly each came from.
 */

#pragma once

class Win;
class Buffer;
class Dialog;
class GraphicWin;

void *__fastcall leaf_004c8070_redirect(void *, void *);
void __cdecl leaf_0057dee0_redirect(void *);
int __cdecl leaf_00408470_redirect(const void *, int);
int __stdcall leaf_005d7a10_redirect(const void *, int, int, int);
int __fastcall leaf_005d5470_redirect(void *, void *, const void *, int, int, int);
int __fastcall leaf_004041c0_redirect(void *, void *);

typedef Buffer *(__fastcall func_leaf_buffer_ctor)(Buffer *, void *);
typedef void(__fastcall func_leaf_buffer_dtor)(Buffer *, void *);
extern func_leaf_buffer_ctor *LeafBufferConstruct;
extern func_leaf_buffer_dtor *LeafBufferDestruct;

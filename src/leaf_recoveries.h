/*
 * Hand-recovered leaf functions. See src/leaf_recoveries.cpp for why these are
 * written out rather than generated, and for the disassembly each came from.
 */

#pragma once

class Win;
class Buffer;

void __cdecl leaf_00628180_redirect(const float *, const float *, float *);
void __cdecl leaf_006281b0_redirect(const float *, float, float *);
float __fastcall leaf_00634650_redirect(void *, void *, const float *);
int __cdecl leaf_00559210_redirect(int, int);
uint32_t __fastcall leaf_005e3630_redirect(void *, void *);
uint32_t __fastcall leaf_005e3650_redirect(void *, void *);
void __fastcall leaf_005e3660_redirect(void *, void *, int);
uint32_t __fastcall leaf_004482f0_redirect(void *, void *, int);
void *__fastcall leaf_004c8070_redirect(void *, void *);
void __fastcall leaf_006347c0_redirect(void *, void *, const float *);
void __fastcall leaf_006348f0_redirect(void *, void *, const float *);
void __fastcall leaf_006348c0_redirect(void *, void *, float);
bool __fastcall leaf_006344e0_redirect(void *, void *, const float *);
float __cdecl leaf_006281e0_redirect(const float *);
int32_t __fastcall leaf_0063beb0_redirect(void *, void *, int);
int32_t __fastcall leaf_0063bee0_redirect(void *, void *, int);
void __cdecl leaf_0057dee0_redirect(void *);
void *__fastcall leaf_006252c0_redirect(void *, void *);
int __cdecl leaf_00408470_redirect(const void *, int);
void __fastcall leaf_005ad450_redirect(void *, void *);
int __fastcall leaf_005cc430_redirect(uint32_t);
int __stdcall leaf_005d7a10_redirect(const void *, int, int, int);
int __fastcall leaf_005d5470_redirect(void *, void *, const void *, int, int, int);
uint32_t __cdecl leaf_00642940_redirect(uint32_t, int);
uint32_t __fastcall leaf_0063e7f0_redirect(void *, void *, uint32_t *, uint32_t *);
int __stdcall leaf_00532a50_redirect(int, int *, int *, int);
int __cdecl leaf_00592db0_redirect(int, int, int, int, int, int);
int __fastcall leaf_006161a0_redirect(void *, void *);
int __fastcall leaf_005ad4c0_redirect(void *, void *, int, int, int);
void __fastcall leaf_005cbbc0_redirect(void *, void *);
void *__fastcall leaf_0052dca0_redirect(void *, void *);
int __fastcall leaf_004041c0_redirect(void *, void *);
void *__fastcall leaf_004bea30_redirect(void *, void *);
void __fastcall leaf_00432970_redirect(void *, void *);
void __cdecl leaf_0063d420_redirect(void *, uint32_t);
extern const uint32_t ImageButtonPrimaryVtable;
extern const uint32_t ImageButtonBufferVtable;
void *__fastcall image_button_construct_redirect(void *, void *);
void __fastcall image_button_close_redirect(void *, void *);
int __fastcall image_button_init_redirect(void *, void *, int, LPCSTR, int,
                                          int, int, int, int, Win *, int);

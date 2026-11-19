#include "probe.h"
// CouncWin::set_vote tail: image `mov eax,[ebx+K]; lea ecx,[ebx+K]; push 0; call [eax+4]`, ours `add ebx,K; push 0; mov ecx,ebx`
struct Btn { char pad[0xb4c]; virtual void show(int); virtual void hide(); };
struct CW { char pad[0xd94]; Btn b[6]; int f; void a(int); void b2(int); void c(int); void d(int); void e(int); };
void CW::a(int x) { if (x == ga) { for (int i = 0; i < 6; ++i) b[i].hide(); b[5].show(0); } }
void CW::b2(int x) { if (x == ga) { for (int i = 0; i < 6; ++i) b[i].hide(); Btn *p = &b[5]; p->show(0); } }
void CW::c(int x) { if (x == ga) { for (int i = 0; i < 6; ++i) b[i].hide(); } b[5].show(0); }
void CW::d(int x) { if (x == ga) { for (int i = 0; i < 6; ++i) b[i].hide(); b[5].show(0); } f0(); }
void CW::e(int x) { f0(); if (x == ga) { for (int i = 0; i < 6; ++i) b[i].hide(); b[5].show(0); } }

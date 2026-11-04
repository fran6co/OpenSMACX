#include "stdafx.h"
#include "movie_media.h"

#include <string.h>

/*
Purpose: Create the codec's draw/audio surface through COM: QueryInterface
         slot 6 on the provider with a 0x6c-byte format block (0x1007 tag,
         0x4040 tail, 0x20/4 pair and the caller's fifth argument at its
         end), then QueryInterface slot 0 of the created object for the
         interface at 0x0066FC28 - success stores the interface through the
         out pointer and returns 0, any failure Releases (slot 2) the created
         object and returns 1.

// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CDAD0 sub_5cdad0 0x005CDAD0-0x005CDB83 BYTE_EXACT
// symbol    ?sub_5cdad0@@YIHPAUIDirectDraw2@@PAPAUIDirectDrawSurface2@@HHH@Z
// size      179 bytes
// prototype int (__fastcall sub_5cdad0)(int *, int *, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CDB3E 0x005CDB56 0x005CDB73
*/
int __fastcall sub_5cdad0(IDirectDraw2 *provider,
                          IDirectDrawSurface2 **out_slot,
                          int width, int height, int format_tail) {
  DDSURFACEDESC description;
  memset(&description, 0, 0x6c);
  description.dwSize = 0x6c;
  description.dwFlags = 0x1007;
  description.dwWidth = static_cast<DWORD>(width);
  description.dwHeight = static_cast<DWORD>(height);
  description.ddsCaps.dwCaps = 0x4040;
  description.ddpfPixelFormat.dwSize = 0x20;
  description.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
  description.ddpfPixelFormat.dwFourCC = static_cast<DWORD>(format_tail);

  IDirectDrawSurface2 *queried;
  IDirectDrawSurface *created;
  if (provider->CreateSurface(&description, &created, 0) == DD_OK) {
    if (created->QueryInterface(IID_IDirectDrawSurface2,
                                reinterpret_cast<void **>(&queried)) == DD_OK) {
      *out_slot = queried;
      return 0; // Preserve the original success-path reference/leak behavior.
    }
    created->Release();
  }
  return 1;
}

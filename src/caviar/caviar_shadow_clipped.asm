; CAVIAR_SHADOW_CLIPPED.ASM - Caviar's self-modifying voxel renderer: shadow, clipped to a rect.
;
; One original _SELFMOD contribution, 0x00666A80-0x00666FC1: the renderer
; and the patcher that stores the current draw's buffers, steps and tables
; into the renderer's own immediates and displacements before it runs.
; The image pads between contributions with int3 and no reference crosses
; one, so each is its own module and segment. Assembled by Microsoft ML
; 6.11d; the linker marks _SELFMOD writable (/SECTION:_SELFMOD,ERW).

        .386
        .model flat
        option casemap:none
        include caviar_selfmod.inc

_SELFMOD SEGMENT PARA PUBLIC 'CODE'

; SEED HISTORY (prior excluded record): 0x00666A80 sub_666a80 0x00666A80-0x00666F68 EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00666A80 sub_666a80 0x00666A80-0x00666F68 BYTE_EXACT
; symbol    ?caviar_original_666a80@@YAXHHPBHIPBEH@Z
; kind      game
?caviar_original_666a80@@YAXHHPBHIPBEH@Z PROC PUBLIC
          push ebp
          push esi
          push edi
          push ebx
          mov dword ptr [caviar_kernel_projection_window + 1D4h], esp
          mov edi, dword ptr [esp + 1Ch]
          mov eax, dword ptr [edi]
          mov dword ptr [caviar_kernel_projection_window + 1C8h], eax
          dec eax
          mov dword ptr [vox_clip_left_minus_one], eax
          mov eax, dword ptr [edi + 4]
          mov dword ptr [caviar_kernel_projection_window + 1E4h], eax
          dec eax
          mov dword ptr [caviar_kernel_projection_window + 1CCh], eax
          mov eax, dword ptr [edi + 8]
          mov dword ptr [caviar_kernel_projection_window + 1D0h], eax
          mov eax, dword ptr [edi + 0Ch]
          mov dword ptr [vox_clip_bottom], eax
          mov esi, dword ptr [esp + 14h]
          mov ebp, dword ptr [esp + 18h]
          mov edi, dword ptr [esp + 24h]
          mov eax, dword ptr [esp + 28h]
          mov dl, byte ptr [caviar_kernel_projection_window + 1DCh]
          test eax, 10h
          je L666E31
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          and eax, 1
          jne L666CC1
L666AEF:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L666B10
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L666B30
          shl ebx, 3
          jmp L666BD0
        ALIGN 16
L666B10:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L666C43
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L666B10
L666B30:  movsx eax, byte ptr [edi + 5]
          mov dword ptr [caviar_kernel_projection_window + 1E0h], eax
          fild dword ptr [caviar_kernel_projection_window + 1E0h]
          movsx eax, byte ptr [edi + 6]
          mov dword ptr [caviar_kernel_projection_window + 1E0h], eax
          fild dword ptr [caviar_kernel_projection_window + 1E0h]
          movsx eax, byte ptr [edi + 7]
          mov dword ptr [caviar_kernel_projection_window + 1E0h], eax
          fild dword ptr [caviar_kernel_projection_window + 1E0h]
          fld st(0)
          fmul dword ptr [caviar_kernel_projection_window + 1ACh]
          add edi, 9
          fld st(2)
          fmul dword ptr [caviar_kernel_projection_window + 1B0h]
          xor ebx, ebx
          faddp st(1), st
          fld st(3)
          fmul dword ptr [caviar_kernel_projection_window + 1B4h]
          faddp st(1), st
          fistp dword ptr [caviar_kernel_projection_window + 1E0h]
          fld st(0)
          fmul dword ptr [caviar_kernel_projection_window + 1A0h]
          mov esi, dword ptr [caviar_output_message_window + 1C4h]
          fld st(2)
          fmul dword ptr [caviar_kernel_projection_window + 1A4h]
          add esi, dword ptr [caviar_kernel_projection_window + 1E0h]
          faddp st(1), st
          fld st(3)
          fmul dword ptr [caviar_kernel_projection_window + 1A8h]
          sub esi, dword ptr [caviar_kernel_projection_window]
          faddp st(1), st
          fistp dword ptr [caviar_kernel_projection_window + 1E0h]
          mov ebp, dword ptr [caviar_output_message_window + 1C8h]
          add ebp, dword ptr [caviar_kernel_projection_window + 1E0h]
          fstp st(0)
          fstp st(0)
          fstp st(0)
          jmp L666BD3
        ALIGN 16
L666BD0:  add edi, 9
L666BD3:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ecx, esi
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          sar ecx, 8
          mov eax, ebp
          sar eax, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge L666C4E
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L666C4E
          cmp eax, dword ptr [vox_clip_bottom]
          jge L666C4E
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L666C4E
L666C0B:: add ecx, dword ptr [eax*4 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L666C1D:: mov byte ptr [ecx + 1020304h], dl
L666C23:: mov byte ptr [ecx + 1020304h], dl
L666C29:: mov byte ptr [ecx + 1020304h], dl
L666C2F:: mov byte ptr [ecx + 1020304h], dl
          cmp ebx, 0D0h
          jl L666BD3
          jne L666AEF
L666C43:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L666C4E:  cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg L666CA2
          cmp ecx, dword ptr [vox_clip_left_minus_one]
          jl L666CA2
          cmp eax, dword ptr [vox_clip_bottom]
          jg L666CA2
          cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl L666CA2
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jl L666C79
          dec ecx
          jmp L666C82
L666C79:  cmp ecx, dword ptr [vox_clip_left_minus_one]
          jg L666C82
          inc ecx
L666C82:  cmp eax, dword ptr [vox_clip_bottom]
          jl L666C90
          dec eax
          jmp L666C0B
L666C90:  cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg L666C0B
          inc eax
          jmp L666C0B
L666CA2:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp ebx, 0D0h
          jl L666BD3
          jne L666AEF
          jmp L666C43
L666CC1:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L666CF0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L666D10
          shl ebx, 3
          add edi, 9
          jmp L666DB0
        ALIGN 16
L666CF0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L666C43
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L666CF0
L666D10:  movsx eax, byte ptr [edi + 5]
          mov dword ptr [caviar_kernel_projection_window + 1E0h], eax
          fild dword ptr [caviar_kernel_projection_window + 1E0h]
          movsx eax, byte ptr [edi + 6]
          mov dword ptr [caviar_kernel_projection_window + 1E0h], eax
          fild dword ptr [caviar_kernel_projection_window + 1E0h]
          movsx eax, byte ptr [edi + 7]
          mov dword ptr [caviar_kernel_projection_window + 1E0h], eax
          fild dword ptr [caviar_kernel_projection_window + 1E0h]
          fld st(0)
          fmul dword ptr [caviar_kernel_projection_window + 1ACh]
          add edi, 9
          fld st(2)
          fmul dword ptr [caviar_kernel_projection_window + 1B0h]
          xor ebx, ebx
          faddp st(1), st
          fld st(3)
          fmul dword ptr [caviar_kernel_projection_window + 1B4h]
          faddp st(1), st
          fistp dword ptr [caviar_kernel_projection_window + 1E0h]
          fld st(0)
          fmul dword ptr [caviar_kernel_projection_window + 1A0h]
          mov esi, dword ptr [caviar_output_message_window + 1C4h]
          fld st(2)
          fmul dword ptr [caviar_kernel_projection_window + 1A4h]
          add esi, dword ptr [caviar_kernel_projection_window + 1E0h]
          faddp st(1), st
          fld st(3)
          fmul dword ptr [caviar_kernel_projection_window + 1A8h]
          sub esi, dword ptr [caviar_kernel_projection_window]
          faddp st(1), st
          fistp dword ptr [caviar_kernel_projection_window + 1E0h]
          mov ebp, dword ptr [caviar_output_message_window + 1C8h]
          add ebp, dword ptr [caviar_kernel_projection_window + 1E0h]
          fstp st(0)
          fstp st(0)
          fstp st(0)
          jmp L666DB0
        ALIGN 16
L666DB0:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ecx, esi
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          sar ecx, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L666E13
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg L666E13
          mov eax, ebp
          sar eax, 8
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L666E13
          cmp eax, dword ptr [vox_clip_bottom]
          jg L666E13
L666DE8:: add ecx, dword ptr [eax*4 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L666DFA:: mov byte ptr [ecx + 1020304h], dl
          cmp ebx, 0D0h
          jl L666DB0
          jne L666CC1
          jmp L666C43
L666E13:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp ebx, 0D0h
          jl L666DB0
          jne L666CC1
          jmp L666C43
L666E31:  mov esp, dword ptr [esp + 20h]
          test eax, 20h
          je L666E3D
          dec esp
L666E3D:  and eax, 1
          jne L666F05
          xor eax, eax
          xor ebx, ebx
L666E4A:  mov al, byte ptr [edi]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 8
          mov ebx, ebp
          sar ebx, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge L666EBB
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L666EBB
          cmp ebx, dword ptr [vox_clip_bottom]
          jge L666EBB
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L666EBB
L666E8F:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L666E98:: mov byte ptr [ecx + 1020304h], dl
L666E9E:: mov byte ptr [ecx + 1020304h], dl
L666EA4:: mov byte ptr [ecx + 1020304h], dl
L666EAA:: mov byte ptr [ecx + 1020304h], dl
L666EB0:  add edi, 3
          dec esp
          jne L666E4A
          jmp L666C43
L666EBB:  cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg L666EB0
          cmp ecx, dword ptr [vox_clip_left_minus_one]
          jl L666EB0
          cmp ebx, dword ptr [vox_clip_bottom]
          jg L666EB0
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl L666EB0
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jl L666EE6
          dec ecx
          jmp L666EEF
L666EE6:  cmp ecx, dword ptr [vox_clip_left_minus_one]
          jg L666EEF
          inc ecx
L666EEF:  cmp ebx, dword ptr [vox_clip_bottom]
          jl L666EFA
          dec ebx
          jmp L666E8F
L666EFA:  cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg L666E8F
          inc ebx
          jmp L666E8F
L666F05:  xor eax, eax
          xor ebx, ebx
L666F09:  mov al, byte ptr [edi]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          and ebx, 7FFh
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          mov ebx, ebp
          sar ebx, 8
          cmp ebx, dword ptr [vox_clip_bottom]
          jge L666F5D
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L666F5D
          mov ecx, esi
          sar ecx, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge L666F5D
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L666F5D
L666F4E:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L666F57:: mov byte ptr [ecx + 1020304h], dl
L666F5D:  add edi, 3
          dec esp
          jne L666F09
          jmp L666C43
?caviar_original_666a80@@YAXHHPBHIPBEH@Z ENDP

; SEED HISTORY: former 0x00666F68 record sub_666f68 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00666F68..0x00666FC1; 89-byte seed; catalogued direct callers=4, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x00666F68 sub_666f68 0x00666F68-0x00666FC1 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_666f68@@YAXXZ
?caviar_original_666f68@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_ramp_b]
          mov dword ptr [L666DE8 + 3], eax
          mov dword ptr [L666C0B + 3], eax
          mov dword ptr [L666F4E + 3], eax
          mov dword ptr [L666E8F + 3], eax
          mov eax, dword ptr [vox_setup_copy + 8h]
          mov dword ptr [L666DFA + 2], eax
          mov dword ptr [L666C1D + 2], eax
          mov dword ptr [L666F57 + 2], eax
          mov dword ptr [L666E98 + 2], eax
          inc eax
          mov dword ptr [L666C23 + 2], eax
          mov dword ptr [L666E9E + 2], eax
          add eax, dword ptr [vox_setup_copy + 14h]
          mov dword ptr [L666C2F + 2], eax
          mov dword ptr [L666EAA + 2], eax
          dec eax
          mov dword ptr [L666C29 + 2], eax
          mov dword ptr [L666EA4 + 2], eax
          ret
?caviar_original_666f68@@YAXXZ ENDP

_SELFMOD ENDS

        END

; CAVIAR_PART_S2O_CLIPPED.ASM - Caviar's self-modifying voxel renderer: two-pixel, odd parts, clipped to a rect.
;
; One original _SELFMOD contribution, 0x00667380-0x00667B17: the renderer
; and the patcher that stores the current draw's buffers, steps and tables
; into the renderer's own immediates and displacements before it runs.
; The image pads between contributions with int3 and no reference crosses
; one, so each is its own module and segment. Assembled by UASM (MASM 6
; syntax); the linker marks _SELFMOD writable (/SECTION:_SELFMOD,ERW).

        .386
        .model flat
        option casemap:none
        include caviar_selfmod.inc

_SELFMOD SEGMENT PARA PUBLIC 'CODE'
selfmod_segment_base LABEL BYTE

; SEED HISTORY (prior excluded record): 0x00667380 sub_667380 0x00667380-0x00667A6F EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00667380 sub_667380 0x00667380-0x00667A6F BYTE_EXACT
; symbol    ?caviar_original_667380@@YAXHHHPBHIPBEH@Z
; kind      game
?caviar_original_667380@@YAXHHHPBHIPBEH@Z PROC PUBLIC
          push ebp
          push esi
          push edi
          push ebx
          mov dword ptr [caviar_kernel_projection_window + 1D4h], esp
          mov edi, dword ptr [esp + 20h]
          mov eax, dword ptr [edi]
          shl eax, 1
          mov dword ptr [caviar_output_message_window + 1D0h], eax
          sub eax, 2
          mov dword ptr [caviar_output_message_window + 1C0h], eax
          mov eax, dword ptr [edi + 4]
          mov dword ptr [caviar_kernel_projection_window + 1E4h], eax
          dec eax
          mov dword ptr [caviar_kernel_projection_window + 1CCh], eax
          mov eax, dword ptr [edi + 8]
          shl eax, 1
          mov dword ptr [vox_clip_right_doubled], eax
          mov eax, dword ptr [edi + 0Ch]
          mov dword ptr [vox_clip_bottom], eax
          mov esi, dword ptr [esp + 14h]
          mov ebp, dword ptr [esp + 18h]
          mov edx, dword ptr [esp + 1Ch]
          mov edi, dword ptr [esp + 28h]
          mov eax, dword ptr [esp + 2Ch]
          shl edx, 8
          test eax, 10h
          je near ptr L66788A
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne near ptr L6676A0
L667402:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L667430
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L667450
          shl ebx, 3
          jmp near ptr L667530
        MASM6_ALIGN 16
L667430:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L667618
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L667430
L667450:  movsx eax, byte ptr [edi + 5]
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
          fld st(0)
          fmul dword ptr [caviar_kernel_projection_window + 1BCh]
          mov ebp, dword ptr [caviar_output_message_window + 1C8h]
          fld st(2)
          fmul dword ptr [caviar_kernel_projection_window + 1C0h]
          mov edx, dword ptr [caviar_output_message_window + 1CCh]
          add ebp, dword ptr [caviar_kernel_projection_window + 1E0h]
          faddp st(1), st
          fld st(3)
          fmul dword ptr [caviar_kernel_projection_window + 1C4h]
          sub ebp, dword ptr [caviar_kernel_projection_window + 4h]
          faddp st(1), st
          fistp dword ptr [caviar_kernel_projection_window + 1E0h]
          sub edx, dword ptr [caviar_kernel_projection_window + 8h]
          fstp st(0)
          fstp st(0)
          fstp st(0)
          add edx, dword ptr [caviar_kernel_projection_window + 1E0h]
          jmp short L667533
        MASM6_ALIGN 16
L667530:  add edi, 9
L667533:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          and eax, 7FFh
          and ecx, 0FFFFFF00h
          sar ecx, 7
          mov bl, byte ptr [edi + 2]
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jge near ptr L667623
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl near ptr L667623
          cmp eax, dword ptr [vox_clip_bottom]
          jge near ptr L667623
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl near ptr L667623
L66759B:: add ecx, dword ptr [eax*4 + 1020304h]
L6675A2:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp dx, word ptr [ecx]
          jbe short L6675C4
          mov word ptr [ecx], dx
L6675BD:: mov word ptr [ecx + 1020304h], ax
L6675C4:  cmp dx, word ptr [ecx + 2]
          jbe short L6675D5
          mov word ptr [ecx + 2], dx
L6675CE:: mov word ptr [ecx + 1020304h], ax
L6675D5:: cmp dx, word ptr [ecx + 1020304h]
          jbe short L6675EC
L6675DE:: mov word ptr [ecx + 1020304h], dx
L6675E5:: mov word ptr [ecx + 1020304h], ax
L6675EC:: cmp dx, word ptr [ecx + 1020304h]
          jbe short L667603
L6675F5:: mov word ptr [ecx + 1020304h], dx
L6675FC:: mov word ptr [ecx + 1020304h], ax
L667603:  rol edx, 10h
          cmp ebx, 0D0h
          jl near ptr L667533
          jne near ptr L667402
L667618:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L667623:  cmp ecx, dword ptr [vox_clip_right_doubled]
          jg short L66767B
          cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jl short L66767B
          cmp eax, dword ptr [vox_clip_bottom]
          jg short L66767B
          cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl short L66767B
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jl short L667650
          sub ecx, 2
          jmp short L66765B
L667650:  cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jg short L66765B
          add ecx, 2
L66765B:  cmp eax, dword ptr [vox_clip_bottom]
          jl short L667669
          dec eax
          jmp near ptr L66759B
L667669:  cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg near ptr L66759B
          inc eax
          jmp near ptr L66759B
L66767B:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          rol edx, 10h
          cmp ebx, 0D0h
          jl near ptr L667533
          jne near ptr L667402
          jmp near ptr L667618
L6676A0:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L6676D0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L6676F0
          shl ebx, 3
          add edi, 9
          jmp near ptr L6677D0
        MASM6_ALIGN 16
L6676D0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L667618
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L6676D0
L6676F0:  movsx eax, byte ptr [edi + 5]
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
          fld st(0)
          fmul dword ptr [caviar_kernel_projection_window + 1BCh]
          mov ebp, dword ptr [caviar_output_message_window + 1C8h]
          fld st(2)
          fmul dword ptr [caviar_kernel_projection_window + 1C0h]
          mov edx, dword ptr [caviar_output_message_window + 1CCh]
          add ebp, dword ptr [caviar_kernel_projection_window + 1E0h]
          faddp st(1), st
          fld st(3)
          fmul dword ptr [caviar_kernel_projection_window + 1C4h]
          sub ebp, dword ptr [caviar_kernel_projection_window + 4h]
          faddp st(1), st
          fistp dword ptr [caviar_kernel_projection_window + 1E0h]
          sub edx, dword ptr [caviar_kernel_projection_window + 8h]
          fstp st(0)
          fstp st(0)
          fstp st(0)
          add edx, dword ptr [caviar_kernel_projection_window + 1E0h]
          jmp short L6677D0
        MASM6_ALIGN 16
L6677D0:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          sar ecx, 7
          mov bl, byte ptr [edi + 2]
          and eax, 7FFh
          and ecx, 0FFFFFFFEh
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl short L667868
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jg short L667868
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl short L667868
          cmp eax, dword ptr [vox_clip_bottom]
          jg short L667868
L667825:: add ecx, dword ptr [eax*4 + 1020304h]
L66782C:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp dx, word ptr [ecx]
          jbe short L66784E
          mov word ptr [ecx], dx
L667847:: mov word ptr [ecx + 1020304h], ax
L66784E:  rol edx, 10h
          cmp ebx, 0D0h
          jl near ptr L6677D0
          jne near ptr L6676A0
          jmp near ptr L667618
L667868:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp ebx, 0D0h
          jl near ptr L6677D0
          jne near ptr L6676A0
          jmp near ptr L667618
L66788A:  mov esp, dword ptr [esp + 24h]
          test eax, 20h
          je short L667896
          dec esp
L667896:  and eax, 1
          jne near ptr L6679D6
          xor eax, eax
          xor ebx, ebx
L6678A3:  mov al, byte ptr [edi]
          mov bh, al
          mov bl, byte ptr [edi + 1]
          mov ecx, dword ptr [vox_projection_selected_angle]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          and ebx, 7FFh
          mov bh, byte ptr [ebx + ecx]
          mov bl, byte ptr [edi + 2]
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          add edx, dword ptr [eax*2 + caviar_kernel_projection_window + 8h]
L6678D6:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 7
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jge near ptr L66797E
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl short L66797E
          cmp ebx, dword ptr [vox_clip_bottom]
          jge short L66797E
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl short L66797E
L667915:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jbe short L66792D
          mov word ptr [ecx], dx
L667926:: mov word ptr [ecx + 1020304h], ax
L66792D:  cmp dx, word ptr [ecx + 2]
          jbe short L66793E
          mov word ptr [ecx + 2], dx
L667937:: mov word ptr [ecx + 1020304h], ax
L66793E:: cmp dx, word ptr [ecx + 1020304h]
          jbe short L667955
L667947:: mov word ptr [ecx + 1020304h], dx
L66794E:: mov word ptr [ecx + 1020304h], ax
L667955:: cmp dx, word ptr [ecx + 1020304h]
          jbe short L66796C
L66795E:: mov word ptr [ecx + 1020304h], dx
L667965:: mov word ptr [ecx + 1020304h], ax
L66796C:  rol edx, 10h
          add edi, 3
          dec esp
          jne near ptr L6678A3
          jmp near ptr L667618
L66797E:  cmp ecx, dword ptr [vox_clip_right_doubled]
          jg short L66796C
          cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jl short L66796C
          cmp ebx, dword ptr [vox_clip_bottom]
          jg short L66796C
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl short L66796C
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jl short L6679AB
          sub ecx, 2
          jmp short L6679B6
L6679AB:  cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jg short L6679B6
          add ecx, 2
L6679B6:  cmp ebx, dword ptr [vox_clip_bottom]
          jl short L6679C4
          dec ebx
          jmp near ptr L667915
L6679C4:  cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg near ptr L667915
          inc ebx
          jmp near ptr L667915
L6679D6:  xor eax, eax
          xor ebx, ebx
L6679DA:  mov al, byte ptr [edi]
          mov bh, al
          mov bl, byte ptr [edi + 1]
          mov ecx, dword ptr [vox_projection_selected_angle]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          and ebx, 7FFh
          mov bh, byte ptr [ebx + ecx]
          mov bl, byte ptr [edi + 2]
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          add edx, dword ptr [eax*2 + caviar_kernel_projection_window + 8h]
L667A0D:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          cmp ebx, dword ptr [vox_clip_bottom]
          jge short L667A60
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl short L667A60
          mov ecx, esi
          sar ecx, 7
          and ecx, 0FFFFFFFEh
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jge short L667A60
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl short L667A60
          ror edx, 10h
L667A45:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jbe short L667A5D
          mov word ptr [ecx], dx
L667A56:: mov word ptr [ecx + 1020304h], ax
L667A5D:  rol edx, 10h
L667A60:  add edi, 3
          dec esp
          jne near ptr L6679DA
          jmp near ptr L667618
?caviar_original_667380@@YAXHHHPBHIPBEH@Z ENDP

; SEED HISTORY: former 0x00667A6F record sub_667a6f was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00667A6F..0x00667B17; 168-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x00667A6F sub_667a6f 0x00667A6F-0x00667B17 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_667a6f@@YAXXZ
?caviar_original_667a6f@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L6675A2 + 4], eax
          mov dword ptr [L66782C + 4], eax
          mov dword ptr [L6678D6 + 4], eax
          mov dword ptr [L667A0D + 4], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1D8h]
          mov dword ptr [L66759B + 3], eax
          mov dword ptr [L667825 + 3], eax
          mov dword ptr [L667915 + 3], eax
          mov dword ptr [L667A45 + 3], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L6675BD + 3], eax
          mov dword ptr [L667847 + 3], eax
          mov dword ptr [L667926 + 3], eax
          mov dword ptr [L667A56 + 3], eax
          add eax, 2
          mov dword ptr [L6675CE + 3], eax
          mov dword ptr [L667937 + 3], eax
          mov eax, dword ptr [vox_setup_copy + 0Ch]
          shl eax, 1
          mov dword ptr [L6675D5 + 3], eax
          mov dword ptr [L6675DE + 3], eax
          mov dword ptr [L66793E + 3], eax
          mov dword ptr [L667947 + 3], eax
          add eax, 2
          mov dword ptr [L6675EC + 3], eax
          mov dword ptr [L6675F5 + 3], eax
          mov dword ptr [L667955 + 3], eax
          mov dword ptr [L66795E + 3], eax
          add eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L6675FC + 3], eax
          mov dword ptr [L667965 + 3], eax
          sub eax, 2
          mov dword ptr [L6675E5 + 3], eax
          mov dword ptr [L66794E + 3], eax
          ret
?caviar_original_667a6f@@YAXXZ ENDP

_SELFMOD ENDS

        END

; CAVIAR_PART_S1O_CLIPPED.ASM - Caviar's self-modifying voxel renderer: one-pixel, odd parts, clipped to a rect.
;
; One original _SELFMOD contribution, 0x00664E50-0x00665633: the renderer
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

; SEED HISTORY (prior excluded record): 0x00664E50 sub_664e50 0x00664E50-0x0066554B EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00664E50 sub_664e50 0x00664E50-0x0066554B BYTE_EXACT
; symbol    ?caviar_original_664e50@@YAXHHHPBHIPBEH@Z
; kind      game
?caviar_original_664e50@@YAXHHHPBHIPBEH@Z PROC PUBLIC
          push ebp
          push esi
          push edi
          push ebx
          mov dword ptr [caviar_kernel_projection_window + 1D4h], esp
          mov edi, dword ptr [esp + 20h]
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
          mov edx, dword ptr [esp + 1Ch]
          mov edi, dword ptr [esp + 28h]
          mov eax, dword ptr [esp + 2Ch]
          shl edx, 8
          test eax, 10h
          je near ptr L66534E
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne near ptr L665166
L664ECC:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L664EF0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L664F10
          shl ebx, 3
          jmp near ptr L664FF0
        MASM6_ALIGN 16
L664EF0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L6650E2
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L664EF0
L664F10:  movsx eax, byte ptr [edi + 5]
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
          jmp short L664FF3
        MASM6_ALIGN 16
L664FF0:  add edi, 9
L664FF3:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          and eax, 7FFh
          sar ecx, 8
          mov bl, byte ptr [edi + 2]
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge near ptr L6650ED
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl near ptr L6650ED
          cmp eax, dword ptr [vox_clip_bottom]
          jge near ptr L6650ED
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl near ptr L6650ED
L665055:: add ecx, dword ptr [eax*4 + 1020304h]
L66505C:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L66506D:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665085
L665077:: mov word ptr [ecx*2 + 1020304h], dx
L66507F:: mov byte ptr [ecx + 1020304h], al
L665085:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L66509D
L66508F:: mov word ptr [ecx*2 + 1020304h], dx
L665097:: mov byte ptr [ecx + 1020304h], al
L66509D:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L6650B5
L6650A7:: mov word ptr [ecx*2 + 1020304h], dx
L6650AF:: mov byte ptr [ecx + 1020304h], al
L6650B5:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L6650CD
L6650BF:: mov word ptr [ecx*2 + 1020304h], dx
L6650C7:: mov byte ptr [ecx + 1020304h], al
L6650CD:  rol edx, 10h
          cmp ebx, 0D0h
          jl near ptr L664FF3
          jne near ptr L664ECC
L6650E2:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L6650ED:  cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg short L665141
          cmp ecx, dword ptr [vox_clip_left_minus_one]
          jl short L665141
          cmp eax, dword ptr [vox_clip_bottom]
          jg short L665141
          cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl short L665141
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jl short L665118
          dec ecx
          jmp short L665121
L665118:  cmp ecx, dword ptr [vox_clip_left_minus_one]
          jg short L665121
          inc ecx
L665121:  cmp eax, dword ptr [vox_clip_bottom]
          jl short L66512F
          dec eax
          jmp near ptr L665055
L66512F:  cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg near ptr L665055
          inc eax
          jmp near ptr L665055
L665141:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          rol edx, 10h
          cmp ebx, 0D0h
          jl near ptr L664FF3
          jne near ptr L664ECC
          jmp near ptr L6650E2
L665166:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L665190
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L6651B0
          shl ebx, 3
          add edi, 9
          jmp near ptr L665290
        MASM6_ALIGN 16
L665190:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L6650E2
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L665190
L6651B0:  movsx eax, byte ptr [edi + 5]
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
          jmp short L665290
        MASM6_ALIGN 16
L665290:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          sar ecx, 8
          mov bl, byte ptr [edi + 2]
          and eax, 7FFh
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl short L66532C
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg short L66532C
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl short L66532C
          cmp eax, dword ptr [vox_clip_bottom]
          jg short L66532C
          add ecx, dword ptr [eax*4 + 1020304h]
L6652E9:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L6652FA:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665312
L665304:: mov word ptr [ecx*2 + 1020304h], dx
L66530C:: mov byte ptr [ecx + 1020304h], al
L665312:  rol edx, 10h
          cmp ebx, 0D0h
          jl near ptr L665290
          jne near ptr L665166
          jmp near ptr L6650E2
L66532C:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp ebx, 0D0h
          jl near ptr L665290
          jne near ptr L665166
          jmp near ptr L6650E2
L66534E:  mov esp, dword ptr [esp + 24h]
          test eax, 20h
          je short L66535A
          dec esp
L66535A:  and eax, 1
          jne near ptr L6654AE
          xor eax, eax
          xor ebx, ebx
L665367:  mov al, byte ptr [edi]
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
L66539A:: mov al, byte ptr [ebx + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 8
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge near ptr L66545A
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl near ptr L66545A
          cmp ebx, dword ptr [vox_clip_bottom]
          jge near ptr L66545A
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl short L66545A
L6653DF:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L6653E8:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665400
L6653F2:: mov word ptr [ecx*2 + 1020304h], dx
L6653FA:: mov byte ptr [ecx + 1020304h], al
L665400:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665418
L66540A:: mov word ptr [ecx*2 + 1020304h], dx
L665412:: mov byte ptr [ecx + 1020304h], al
L665418:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665430
L665422:: mov word ptr [ecx*2 + 1020304h], dx
L66542A:: mov byte ptr [ecx + 1020304h], al
L665430:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665448
L66543A:: mov word ptr [ecx*2 + 1020304h], dx
L665442:: mov byte ptr [ecx + 1020304h], al
L665448:  rol edx, 10h
          add edi, 3
          dec esp
          jne near ptr L665367
          jmp near ptr L6650E2
L66545A:  cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg short L665448
          cmp ecx, dword ptr [vox_clip_left_minus_one]
          jl short L665448
          cmp ebx, dword ptr [vox_clip_bottom]
          jg short L665448
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl short L665448
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jl short L665485
          dec ecx
          jmp short L66548E
L665485:  cmp ecx, dword ptr [vox_clip_left_minus_one]
          jg short L66548E
          inc ecx
L66548E:  cmp ebx, dword ptr [vox_clip_bottom]
          jl short L66549C
          dec ebx
          jmp near ptr L6653DF
L66549C:  cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg near ptr L6653DF
          inc ebx
          jmp near ptr L6653DF
L6654AE:  xor eax, eax
          xor ebx, ebx
L6654B2:  mov al, byte ptr [edi]
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
L6654E5:: mov al, byte ptr [ebx + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          cmp ebx, dword ptr [vox_clip_bottom]
          jge short L66553C
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl short L66553C
          mov ecx, esi
          sar ecx, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge short L66553C
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl short L66553C
          ror edx, 10h
L665518:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L665521:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665539
L66552B:: mov word ptr [ecx*2 + 1020304h], dx
L665533:: mov byte ptr [ecx + 1020304h], al
L665539:  rol edx, 10h
L66553C:  add edi, 3
          dec esp
          jne near ptr L6654B2
          jmp near ptr L6650E2
?caviar_original_664e50@@YAXHHHPBHIPBEH@Z ENDP

; SEED HISTORY: former 0x0066554B record sub_66554b was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x0066554B..0x00665633; 232-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x0066554B sub_66554b 0x0066554B-0x00665633 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_66554b@@YAXXZ
?caviar_original_66554b@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L66505C + 2], eax
          mov dword ptr [L6652E9 + 2], eax
          mov dword ptr [L66539A + 2], eax
          mov dword ptr [L6654E5 + 2], eax
          mov eax, dword ptr [vox_active_ramp_b]
          mov dword ptr [L665055 + 3], eax
          mov dword ptr [L6653DF + 3], eax
          mov dword ptr [L665518 + 3], eax
          mov eax, dword ptr [vox_setup_copy]
          mov dword ptr [L66506D + 4], eax
          mov dword ptr [L665077 + 4], eax
          mov dword ptr [L6652FA + 4], eax
          mov dword ptr [L665304 + 4], eax
          mov dword ptr [L6653E8 + 4], eax
          mov dword ptr [L6653F2 + 4], eax
          mov dword ptr [L665521 + 4], eax
          mov dword ptr [L66552B + 4], eax
          add eax, 2
          mov dword ptr [L665085 + 4], eax
          mov dword ptr [L66508F + 4], eax
          mov dword ptr [L665400 + 4], eax
          mov dword ptr [L66540A + 4], eax
          add eax, dword ptr [vox_setup_copy + 10h]
          add eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L6650B5 + 4], eax
          mov dword ptr [L6650BF + 4], eax
          mov dword ptr [L665430 + 4], eax
          mov dword ptr [L66543A + 4], eax
          sub eax, 2
          mov dword ptr [L66509D + 4], eax
          mov dword ptr [L6650A7 + 4], eax
          mov dword ptr [L665418 + 4], eax
          mov dword ptr [L665422 + 4], eax
          mov eax, dword ptr [vox_setup_copy + 4h]
          mov dword ptr [L66507F + 2], eax
          mov dword ptr [L66530C + 2], eax
          mov dword ptr [L6653FA + 2], eax
          mov dword ptr [L665533 + 2], eax
          inc eax
          mov dword ptr [L665097 + 2], eax
          mov dword ptr [L665412 + 2], eax
          add eax, dword ptr [vox_setup_copy + 0Ch]
          mov dword ptr [L6650C7 + 2], eax
          mov dword ptr [L665442 + 2], eax
          dec eax
          mov dword ptr [L6650AF + 2], eax
          mov dword ptr [L66542A + 2], eax
          ret
?caviar_original_66554b@@YAXXZ ENDP

_SELFMOD ENDS

        END

; CAVIAR_PART_S2A_CLIPPED.ASM - Caviar's self-modifying voxel renderer: two-pixel, aligned parts, clipped to a rect.
;
; One original _SELFMOD contribution, 0x00665CA0-0x00666437: the renderer
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

; SEED HISTORY (prior excluded record): 0x00665CA0 sub_665ca0 0x00665CA0-0x0066638F EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00665CA0 sub_665ca0 0x00665CA0-0x0066638F BYTE_EXACT
; symbol    ?caviar_original_665ca0@@YAXHHHPBHIPBEH@Z
; kind      game
?caviar_original_665ca0@@YAXHHHPBHIPBEH@Z PROC PUBLIC
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
          je L6661AA
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne L665FC0
L665D22:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L665D50
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L665D70
          shl ebx, 3
          jmp L665E50
        ALIGN 16
L665D50:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L665F38
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L665D50
L665D70:  movsx eax, byte ptr [edi + 5]
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
          jmp L665E53
        ALIGN 16
L665E50:  add edi, 9
L665E53:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
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
          jge L665F43
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl L665F43
          cmp eax, dword ptr [vox_clip_bottom]
          jge L665F43
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L665F43
L665EBB:: add ecx, dword ptr [eax*4 + 1020304h]
L665EC2:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp dx, word ptr [ecx]
          jae L665EE4
          mov word ptr [ecx], dx
L665EDD:: mov word ptr [ecx + 1020304h], ax
L665EE4:  cmp dx, word ptr [ecx + 2]
          jae L665EF5
          mov word ptr [ecx + 2], dx
L665EEE:: mov word ptr [ecx + 1020304h], ax
L665EF5:: cmp dx, word ptr [ecx + 1020304h]
          jae L665F0C
L665EFE:: mov word ptr [ecx + 1020304h], dx
L665F05:: mov word ptr [ecx + 1020304h], ax
L665F0C:: cmp dx, word ptr [ecx + 1020304h]
          jae L665F23
L665F15:: mov word ptr [ecx + 1020304h], dx
L665F1C:: mov word ptr [ecx + 1020304h], ax
L665F23:  rol edx, 10h
          cmp ebx, 0D0h
          jl L665E53
          jne L665D22
L665F38:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L665F43:  cmp ecx, dword ptr [vox_clip_right_doubled]
          jg L665F9B
          cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jl L665F9B
          cmp eax, dword ptr [vox_clip_bottom]
          jg L665F9B
          cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl L665F9B
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jl L665F70
          sub ecx, 2
          jmp L665F7B
L665F70:  cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jg L665F7B
          add ecx, 2
L665F7B:  cmp eax, dword ptr [vox_clip_bottom]
          jl L665F89
          dec eax
          jmp L665EBB
L665F89:  cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg L665EBB
          inc eax
          jmp L665EBB
L665F9B:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          rol edx, 10h
          cmp ebx, 0D0h
          jl L665E53
          jne L665D22
          jmp L665F38
L665FC0:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L665FF0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L666010
          shl ebx, 3
          add edi, 9
          jmp L6660F0
        ALIGN 16
L665FF0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L665F38
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L665FF0
L666010:  movsx eax, byte ptr [edi + 5]
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
          jmp L6660F0
        ALIGN 16
L6660F0:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
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
          jl L666188
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jg L666188
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L666188
          cmp eax, dword ptr [vox_clip_bottom]
          jg L666188
L666145:: add ecx, dword ptr [eax*4 + 1020304h]
L66614C:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp dx, word ptr [ecx]
          jae L66616E
          mov word ptr [ecx], dx
L666167:: mov word ptr [ecx + 1020304h], ax
L66616E:  rol edx, 10h
          cmp ebx, 0D0h
          jl L6660F0
          jne L665FC0
          jmp L665F38
L666188:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp ebx, 0D0h
          jl L6660F0
          jne L665FC0
          jmp L665F38
L6661AA:  mov esp, dword ptr [esp + 24h]
          test eax, 20h
          je L6661B6
          dec esp
L6661B6:  and eax, 1
          jne L6662F6
          xor eax, eax
          xor ebx, ebx
L6661C3:  mov al, byte ptr [edi]
          mov bh, al
          mov bl, byte ptr [edi + 1]
          mov ecx, dword ptr [vox_projection_selected_angle]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          and ebx, 7FFh
          mov bh, byte ptr [ecx + ebx]
          mov bl, byte ptr [edi + 2]
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          add edx, dword ptr [eax*2 + caviar_kernel_projection_window + 8h]
L6661F6:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 7
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jge L66629E
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl L66629E
          cmp ebx, dword ptr [vox_clip_bottom]
          jge L66629E
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L66629E
L666235:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jae L66624D
          mov word ptr [ecx], dx
L666246:: mov word ptr [ecx + 1020304h], ax
L66624D:  cmp dx, word ptr [ecx + 2]
          jae L66625E
          mov word ptr [ecx + 2], dx
L666257:: mov word ptr [ecx + 1020304h], ax
L66625E:: cmp dx, word ptr [ecx + 1020304h]
          jae L666275
L666267:: mov word ptr [ecx + 1020304h], dx
L66626E:: mov word ptr [ecx + 1020304h], ax
L666275:: cmp dx, word ptr [ecx + 1020304h]
          jae L66628C
L66627E:: mov word ptr [ecx + 1020304h], dx
L666285:: mov word ptr [ecx + 1020304h], ax
L66628C:  rol edx, 10h
          add edi, 3
          dec esp
          jne L6661C3
          jmp L665F38
L66629E:  cmp ecx, dword ptr [vox_clip_right_doubled]
          jg L66628C
          cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jl L66628C
          cmp ebx, dword ptr [vox_clip_bottom]
          jg L66628C
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl L66628C
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jl L6662CB
          sub ecx, 2
          jmp L6662D6
L6662CB:  cmp ecx, dword ptr [caviar_output_message_window + 1C0h]
          jg L6662D6
          add ecx, 2
L6662D6:  cmp ebx, dword ptr [vox_clip_bottom]
          jl L6662E4
          dec ebx
          jmp L666235
L6662E4:  cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg L666235
          inc ebx
          jmp L666235
L6662F6:  xor eax, eax
          xor ebx, ebx
L6662FA:  mov al, byte ptr [edi]
          mov bh, al
          mov bl, byte ptr [edi + 1]
          mov ecx, dword ptr [vox_projection_selected_angle]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          and ebx, 7FFh
          mov bh, byte ptr [ecx + ebx]
          mov bl, byte ptr [edi + 2]
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          add edx, dword ptr [eax*2 + caviar_kernel_projection_window + 8h]
L66632D:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          cmp ebx, dword ptr [vox_clip_bottom]
          jge L666380
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L666380
          mov ecx, esi
          sar ecx, 7
          and ecx, 0FFFFFFFEh
          cmp ecx, dword ptr [vox_clip_right_doubled]
          jge L666380
          cmp ecx, dword ptr [caviar_output_message_window + 1D0h]
          jl L666380
          ror edx, 10h
L666365:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jae L66637D
          mov word ptr [ecx], dx
L666376:: mov word ptr [ecx + 1020304h], ax
L66637D:  rol edx, 10h
L666380:  add edi, 3
          dec esp
          jne L6662FA
          jmp L665F38
?caviar_original_665ca0@@YAXHHHPBHIPBEH@Z ENDP

; SEED HISTORY: former 0x0066638F record sub_66638f was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x0066638F..0x00666437; 168-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x0066638F sub_66638f 0x0066638F-0x00666437 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_66638f@@YAXXZ
?caviar_original_66638f@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L665EC2 + 4], eax
          mov dword ptr [L66614C + 4], eax
          mov dword ptr [L6661F6 + 4], eax
          mov dword ptr [L66632D + 4], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1D8h]
          mov dword ptr [L665EBB + 3], eax
          mov dword ptr [L666145 + 3], eax
          mov dword ptr [L666235 + 3], eax
          mov dword ptr [L666365 + 3], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L665EDD + 3], eax
          mov dword ptr [L666167 + 3], eax
          mov dword ptr [L666246 + 3], eax
          mov dword ptr [L666376 + 3], eax
          add eax, 2
          mov dword ptr [L665EEE + 3], eax
          mov dword ptr [L666257 + 3], eax
          mov eax, dword ptr [vox_setup_copy + 0Ch]
          shl eax, 1
          mov dword ptr [L665EF5 + 3], eax
          mov dword ptr [L665EFE + 3], eax
          mov dword ptr [L66625E + 3], eax
          mov dword ptr [L666267 + 3], eax
          add eax, 2
          mov dword ptr [L665F0C + 3], eax
          mov dword ptr [L665F15 + 3], eax
          mov dword ptr [L666275 + 3], eax
          mov dword ptr [L66627E + 3], eax
          add eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L665F1C + 3], eax
          mov dword ptr [L666285 + 3], eax
          sub eax, 2
          mov dword ptr [L665F05 + 3], eax
          mov dword ptr [L66626E + 3], eax
          ret
?caviar_original_66638f@@YAXXZ ENDP

_SELFMOD ENDS

        END

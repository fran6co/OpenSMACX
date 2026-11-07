; CAVIAR_PART_S1O_CONTAINED.ASM - Caviar's self-modifying voxel renderer: one-pixel, odd parts, fully on screen.
;
; One original _SELFMOD contribution, 0x00665640-0x00665C93: the renderer
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

; SEED HISTORY (prior excluded record): 0x00665640 sub_665640 0x00665640-0x00665BA1 EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00665640 sub_665640 0x00665640-0x00665BA1 BYTE_EXACT
; symbol    ?caviar_original_665640@@YAXHHHIPBEH@Z
; kind      game
?caviar_original_665640@@YAXHHHIPBEH@Z PROC PUBLIC
          push ebp
          push esi
          push edi
          push ebx
          mov dword ptr [caviar_kernel_projection_window + 1D4h], esp
          mov esi, dword ptr [esp + 14h]
          mov ebp, dword ptr [esp + 18h]
          mov edx, dword ptr [esp + 1Ch]
          mov edi, dword ptr [esp + 24h]
          mov eax, dword ptr [esp + 28h]
          shl edx, 8
          test eax, 10h
          je near ptr L665A48
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne near ptr L6658A2
L66568D:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L6656B0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L6656D0
          shl ebx, 3
          jmp near ptr L6657D0
        MASM6_ALIGN 16
L6656B0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L665897
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L6656B0
L6656D0:  movsx eax, byte ptr [edi + 5]
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
          mov ecx, esi
          shl ecx, 18h
          mov eax, ebp
          mov cl, al
          sar ebp, 8
          sar esi, 8
          imul ebp, dword ptr [vox_setup_copy + 10h]
          add ebp, esi
          rol edx, 10h
          and edx, 0FF00FFFFh
          jmp short L6657D3
        MASM6_ALIGN 16
L6657D0:  add edi, 9
L6657D3:  add ecx, dword ptr [ebx*2 + caviar_output_message_window + 20h]
          mov ah, byte ptr [edi]
          adc ebp, dword ptr [ebx*2 + caviar_output_message_window + 24h]
          mov al, byte ptr [edi + 1]
          test ch, ch
          je short L6657F2
          xor ch, ch
L6657EC:: add ebp, 1020304h
L6657F2:  and eax, 7FFh
          add edx, dword ptr [ebx*2 + caviar_output_message_window + 28h]
          adc edx, 0
          mov bl, byte ptr [edi + 2]
          mov bh, byte ptr [esp + eax]
          add edi, 3
L66580A:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L665828
L665814:: mov word ptr [ebp*2 + 1020304h], dx
L66581C:: mov al, byte ptr [ebx + 1020304h]
L665822:: mov byte ptr [ebp + 1020304h], al
L665828:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi - 3]
L665831:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L665849
L66583B:: mov word ptr [ebp*2 + 1020304h], dx
L665843:: mov byte ptr [ebp + 1020304h], al
L665849:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L665861
L665853:: mov word ptr [ebp*2 + 1020304h], dx
L66585B:: mov byte ptr [ebp + 1020304h], al
L665861:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L665879
L66586B:: mov word ptr [ebp*2 + 1020304h], dx
L665873:: mov byte ptr [ebp + 1020304h], al
L665879:  and ebx, 0F8h
          and edx, 0FF00FFFFh
          cmp ebx, 0D0h
          jl near ptr L6657D3
          jne near ptr L66568D
L665897:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L6658A2:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L6658D0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L6658EC
          shl ebx, 3
          add edi, 9
          jmp near ptr L6659D0
        MASM6_ALIGN 16
L6658D0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne short L665897
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L6658D0
L6658EC:  movsx eax, byte ptr [edi + 5]
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
          jmp short L6659D0
        MASM6_ALIGN 16
L6659D0:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          sar ecx, 8
          mov bl, byte ptr [edi + 2]
          and eax, 7FFh
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          add ecx, dword ptr [eax*4 + 1020304h]
L665A09:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L665A1A:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665A32
L665A24:: mov word ptr [ecx*2 + 1020304h], dx
L665A2C:: mov byte ptr [ecx + 1020304h], al
L665A32:  rol edx, 10h
          cmp ebx, 0D0h
          jl short L6659D0
          jne near ptr L6658A2
          jmp near ptr L665897
L665A48:  mov esp, dword ptr [esp + 20h]
          test eax, 20h
          je short L665A54
          dec esp
L665A54:  and eax, 1
          jne near ptr L665B28
          xor eax, eax
          xor ebx, ebx
L665A61:  mov al, byte ptr [edi]
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
L665A94:: mov al, byte ptr [ebx + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 8
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
L665AAD:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L665AB6:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665ACE
L665AC0:: mov word ptr [ecx*2 + 1020304h], dx
L665AC8:: mov byte ptr [ecx + 1020304h], al
L665ACE:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665AE6
L665AD8:: mov word ptr [ecx*2 + 1020304h], dx
L665AE0:: mov byte ptr [ecx + 1020304h], al
L665AE6:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665AFE
L665AF0:: mov word ptr [ecx*2 + 1020304h], dx
L665AF8:: mov byte ptr [ecx + 1020304h], al
L665AFE:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665B16
L665B08:: mov word ptr [ecx*2 + 1020304h], dx
L665B10:: mov byte ptr [ecx + 1020304h], al
L665B16:  rol edx, 10h
          add edi, 3
          dec esp
          jne near ptr L665A61
          jmp near ptr L665897
L665B28:  xor eax, eax
          xor ebx, ebx
L665B2C:  mov al, byte ptr [edi]
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
L665B5F:: mov al, byte ptr [ebx + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          mov ecx, esi
          sar ecx, 8
          ror edx, 10h
L665B72:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L665B7B:: cmp dx, word ptr [ecx*2 + 1020304h]
          jbe short L665B93
L665B85:: mov word ptr [ecx*2 + 1020304h], dx
L665B8D:: mov byte ptr [ecx + 1020304h], al
L665B93:  rol edx, 10h
          add edi, 3
          dec esp
          jne short L665B2C
          jmp near ptr L665897
?caviar_original_665640@@YAXHHHIPBEH@Z ENDP

; SEED HISTORY: former 0x00665BA1 record sub_665ba1 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00665BA1..0x00665C93; 242-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x00665BA1 sub_665ba1 0x00665BA1-0x00665C93 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_665ba1@@YAXXZ
; TRIED: was `EXCLUDED S2a`; same shape as 0x00664D51's sibling (see that file's note in full) - the disassembly is a normal `ret`, no tail-jmp, self-contained store sequence, not an EH funclet by docs/EXCLUSIONS.md 2a's own test. Un-excluded.
; TRIED: combining `value + g_9be6b4 + g_9be6b4` into one expression vs. splitting into two statements - identical rebuilt bytes either way (VC6 folds both into one `lea`/`mov` pair; original is two separate `add`). Kept split as the more faithful source form. First divergence #18 add/mov, mnemonic similarity 0.92, 4 edit blocks, unrelated to this expression's semantics.
?caviar_original_665ba1@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L66581C + 2], eax
          mov dword ptr [L665828 + 2], eax
          mov dword ptr [L665A09 + 2], eax
          mov dword ptr [L665A94 + 2], eax
          mov dword ptr [L665B5F + 2], eax
          mov eax, dword ptr [vox_active_ramp_b]
          mov dword ptr [L665AAD + 3], eax
          mov dword ptr [L665B72 + 3], eax
          mov eax, dword ptr [vox_setup_copy]
          mov dword ptr [L66580A + 4], eax
          mov dword ptr [L665814 + 4], eax
          mov dword ptr [L665A1A + 4], eax
          mov dword ptr [L665A24 + 4], eax
          mov dword ptr [L665AB6 + 4], eax
          mov dword ptr [L665AC0 + 4], eax
          mov dword ptr [L665B7B + 4], eax
          mov dword ptr [L665B85 + 4], eax
          add eax, 2
          mov dword ptr [L665831 + 4], eax
          mov dword ptr [L66583B + 4], eax
          mov dword ptr [L665ACE + 4], eax
          mov dword ptr [L665AD8 + 4], eax
          add eax, dword ptr [vox_setup_copy + 10h]
          add eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L665861 + 4], eax
          mov dword ptr [L66586B + 4], eax
          mov dword ptr [L665AFE + 4], eax
          mov dword ptr [L665B08 + 4], eax
          sub eax, 2
          mov dword ptr [L665849 + 4], eax
          mov dword ptr [L665853 + 4], eax
          mov dword ptr [L665AE6 + 4], eax
          mov dword ptr [L665AF0 + 4], eax
          mov eax, dword ptr [vox_setup_copy + 4h]
          mov dword ptr [L665822 + 2], eax
          mov dword ptr [L665A2C + 2], eax
          mov dword ptr [L665AC8 + 2], eax
          mov dword ptr [L665B8D + 2], eax
          inc eax
          mov dword ptr [L665843 + 2], eax
          mov dword ptr [L665AE0 + 2], eax
          add eax, dword ptr [vox_setup_copy + 0Ch]
          mov dword ptr [L665873 + 2], eax
          mov dword ptr [L665B10 + 2], eax
          dec eax
          mov dword ptr [L66585B + 2], eax
          mov dword ptr [L665AF8 + 2], eax
          mov eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L6657EC + 2], eax
          ret
?caviar_original_665ba1@@YAXXZ ENDP

_SELFMOD ENDS

        END

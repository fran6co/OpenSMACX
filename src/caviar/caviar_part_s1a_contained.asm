; CAVIAR_PART_S1A_CONTAINED.ASM - Caviar's self-modifying voxel renderer: one-pixel, aligned parts, fully on screen.
;
; One original _SELFMOD contribution, 0x006647F0-0x00664E43: the renderer
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

; The prior seed classifications were EXCLUDED S2d. These now have typed,
; compiled callback implementations, but no BYTE_EXACT claim is made. Each
; following SEED HISTORY line retains that former classification and flags.
; SEED HISTORY (prior excluded record): 0x006647F0 sub_6647f0 0x006647F0-0x00664D51 EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x006647F0 sub_6647f0 0x006647F0-0x00664D51 BYTE_EXACT
; symbol    ?caviar_original_6647f0@@YAXHHHIPBEH@Z
; kind      game
?caviar_original_6647f0@@YAXHHHIPBEH@Z PROC PUBLIC
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
          je L664BF8
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne L664A52
L66483D:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L664860
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L664880
          shl ebx, 3
          jmp L664980
        ALIGN 16
L664860:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L664A47
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L664860
L664880:  movsx eax, byte ptr [edi + 5]
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
          jmp L664983
        ALIGN 16
L664980:  add edi, 9
L664983:  add ecx, dword ptr [ebx*2 + caviar_output_message_window + 20h]
          mov ah, byte ptr [edi]
          adc ebp, dword ptr [ebx*2 + caviar_output_message_window + 24h]
          mov al, byte ptr [edi + 1]
          test ch, ch
          je L6649A2
          xor ch, ch
L66499C:: add ebp, 1020304h
L6649A2:  and eax, 7FFh
          add edx, dword ptr [ebx*2 + caviar_output_message_window + 28h]
          adc edx, 0
          mov bl, byte ptr [edi + 2]
          mov bh, byte ptr [esp + eax]
          add edi, 3
L6649BA:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae L6649D8
L6649C4:: mov word ptr [ebp*2 + 1020304h], dx
L6649CC:: mov al, byte ptr [ebx + 1020304h]
L6649D2:: mov byte ptr [ebp + 1020304h], al
L6649D8:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi - 3]
L6649E1:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae L6649F9
L6649EB:: mov word ptr [ebp*2 + 1020304h], dx
L6649F3:: mov byte ptr [ebp + 1020304h], al
L6649F9:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae L664A11
L664A03:: mov word ptr [ebp*2 + 1020304h], dx
L664A0B:: mov byte ptr [ebp + 1020304h], al
L664A11:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae L664A29
L664A1B:: mov word ptr [ebp*2 + 1020304h], dx
L664A23:: mov byte ptr [ebp + 1020304h], al
L664A29:  and ebx, 0F8h
          and edx, 0FF00FFFFh
          cmp ebx, 0D0h
          jl L664983
          jne L66483D
L664A47:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L664A52:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L664A80
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L664A9C
          shl ebx, 3
          add edi, 9
          jmp L664B80
        ALIGN 16
L664A80:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L664A47
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L664A80
L664A9C:  movsx eax, byte ptr [edi + 5]
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
          jmp L664B80
        ALIGN 16
L664B80:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
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
L664BB9:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L664BCA:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664BE2
L664BD4:: mov word ptr [ecx*2 + 1020304h], dx
L664BDC:: mov byte ptr [ecx + 1020304h], al
L664BE2:  rol edx, 10h
          cmp ebx, 0D0h
          jl L664B80
          jne L664A52
          jmp L664A47
L664BF8:  mov esp, dword ptr [esp + 20h]
          test eax, 20h
          je L664C04
          dec esp
L664C04:  and eax, 1
          jne L664CD8
          xor eax, eax
          xor ebx, ebx
L664C11:  mov al, byte ptr [edi]
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
L664C44:: mov al, byte ptr [ebx + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 8
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
L664C5D:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L664C66:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664C7E
L664C70:: mov word ptr [ecx*2 + 1020304h], dx
L664C78:: mov byte ptr [ecx + 1020304h], al
L664C7E:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664C96
L664C88:: mov word ptr [ecx*2 + 1020304h], dx
L664C90:: mov byte ptr [ecx + 1020304h], al
L664C96:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664CAE
L664CA0:: mov word ptr [ecx*2 + 1020304h], dx
L664CA8:: mov byte ptr [ecx + 1020304h], al
L664CAE:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664CC6
L664CB8:: mov word ptr [ecx*2 + 1020304h], dx
L664CC0:: mov byte ptr [ecx + 1020304h], al
L664CC6:  rol edx, 10h
          add edi, 3
          dec esp
          jne L664C11
          jmp L664A47
L664CD8:  xor eax, eax
          xor ebx, ebx
L664CDC:  mov al, byte ptr [edi]
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
L664D0F:: mov al, byte ptr [ebx + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          mov ecx, esi
          sar ecx, 8
          ror edx, 10h
L664D22:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L664D2B:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664D43
L664D35:: mov word ptr [ecx*2 + 1020304h], dx
L664D3D:: mov byte ptr [ecx + 1020304h], al
L664D43:  rol edx, 10h
          add edi, 3
          dec esp
          jne L664CDC
          jmp L664A47
?caviar_original_6647f0@@YAXHHHIPBEH@Z ENDP

; SEED HISTORY: former 0x00664D51 record sub_664d51 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00664D51..0x00664E43; 242-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x00664D51 sub_664d51 0x00664D51-0x00664E43 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_664d51@@YAXXZ
; TRIED: this was carried as `EXCLUDED S2a` (C++ EH unwind funclet) by a prior
;   bulk pass, but the disassembly contradicts that ground:
;   docs/EXCLUSIONS.md 2a's own definition is "no frame of its own ... no
;   prologue, no epilogue, no ret - a tail jmp into another symbol's
;   entry", and this function has a normal `ret`, no tail-jmp into anyone
;   else's entry point, and is fully self-contained (reads six globals,
;   republishes them across table records, returns). It was swept into the
;   funclet population by proximity - it sits in the byte gap between two
;   much larger EH-funclet-bearing functions (0x006647F0 and 0x00664E50) -
;   not by matching the population's own test. Un-excluded here so the
;   body below can be measured.
; TRIED: combining the two `+= g_9be6b4` adds into one expression (`value + g_9be6b4 + g_9be6b4`) vs. splitting them into two statements - VC6 lowers both to the same `lea`/`mov` pair either way (original is two separate `add`); tried both, identical rebuilt bytes. Kept split as the more faithful source form. First divergence #18 add/mov (mnemonic similarity 0.92, 4 edit blocks, none of them affecting this expression's semantics).
?caviar_original_664d51@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L6649CC + 2], eax
          mov dword ptr [L6649D8 + 2], eax
          mov dword ptr [L664BB9 + 2], eax
          mov dword ptr [L664C44 + 2], eax
          mov dword ptr [L664D0F + 2], eax
          mov eax, dword ptr [vox_active_ramp_b]
          mov dword ptr [L664C5D + 3], eax
          mov dword ptr [L664D22 + 3], eax
          mov eax, dword ptr [vox_setup_copy]
          mov dword ptr [L6649BA + 4], eax
          mov dword ptr [L6649C4 + 4], eax
          mov dword ptr [L664BCA + 4], eax
          mov dword ptr [L664BD4 + 4], eax
          mov dword ptr [L664C66 + 4], eax
          mov dword ptr [L664C70 + 4], eax
          mov dword ptr [L664D2B + 4], eax
          mov dword ptr [L664D35 + 4], eax
          add eax, 2
          mov dword ptr [L6649E1 + 4], eax
          mov dword ptr [L6649EB + 4], eax
          mov dword ptr [L664C7E + 4], eax
          mov dword ptr [L664C88 + 4], eax
          add eax, dword ptr [vox_setup_copy + 10h]
          add eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L664A11 + 4], eax
          mov dword ptr [L664A1B + 4], eax
          mov dword ptr [L664CAE + 4], eax
          mov dword ptr [L664CB8 + 4], eax
          sub eax, 2
          mov dword ptr [L6649F9 + 4], eax
          mov dword ptr [L664A03 + 4], eax
          mov dword ptr [L664C96 + 4], eax
          mov dword ptr [L664CA0 + 4], eax
          mov eax, dword ptr [vox_setup_copy + 4h]
          mov dword ptr [L6649D2 + 2], eax
          mov dword ptr [L664BDC + 2], eax
          mov dword ptr [L664C78 + 2], eax
          mov dword ptr [L664D3D + 2], eax
          inc eax
          mov dword ptr [L6649F3 + 2], eax
          mov dword ptr [L664C90 + 2], eax
          add eax, dword ptr [vox_setup_copy + 0Ch]
          mov dword ptr [L664A23 + 2], eax
          mov dword ptr [L664CC0 + 2], eax
          dec eax
          mov dword ptr [L664A0B + 2], eax
          mov dword ptr [L664CA8 + 2], eax
          mov eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L66499C + 2], eax
          ret
?caviar_original_664d51@@YAXXZ ENDP

_SELFMOD ENDS

        END

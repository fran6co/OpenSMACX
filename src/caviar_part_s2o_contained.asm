; CAVIAR_PART_S2O_CONTAINED.ASM - Caviar's self-modifying voxel renderer: two-pixel, odd parts, fully on screen.
;
; One original _SELFMOD contribution, 0x00667B20-0x0066815D: the renderer
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

; SEED HISTORY (prior excluded record): 0x00667B20 sub_667b20 0x00667B20-0x00668069 EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00667B20 sub_667b20 0x00667B20-0x00668069 BYTE_EXACT
; symbol    ?caviar_original_667b20@@YAXHHHIPBEH@Z
; kind      game
?caviar_original_667b20@@YAXHHHIPBEH@Z PROC PUBLIC
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
          je near ptr L667F24
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne near ptr L667D8E
L667B6D:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L667B90
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L667BB0
          shl ebx, 3
          jmp near ptr L667CB0
        MASM6_ALIGN 16
L667B90:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L667D83
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L667B90
L667BB0:  movsx eax, byte ptr [edi + 5]
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
          jmp short L667CB3
        MASM6_ALIGN 16
L667CB0:  add edi, 9
L667CB3:  add ecx, dword ptr [ebx*2 + caviar_output_message_window + 20h]
          mov ah, byte ptr [edi]
          adc ebp, dword ptr [ebx*2 + caviar_output_message_window + 24h]
          mov al, byte ptr [edi + 1]
          test ch, ch
          je short L667CD2
          xor ch, ch
L667CCC:: add ebp, 1020304h
L667CD2:  and eax, 7FFh
          add edx, dword ptr [ebx*2 + caviar_output_message_window + 28h]
          adc edx, 0
          mov bl, byte ptr [edi + 2]
          mov bh, byte ptr [esp + eax]
          add edi, 3
L667CEA:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L667D0C
L667CF4:: mov word ptr [ebp*2 + 1020304h], dx
L667CFC:: mov ax, word ptr [ebx*2 + 1020304h]
L667D04:: mov word ptr [ebp*2 + 1020304h], ax
L667D0C:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi - 3]
L667D17:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L667D31
L667D21:: mov word ptr [ebp*2 + 1020304h], dx
L667D29:: mov word ptr [ebp*2 + 1020304h], ax
L667D31:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L667D4B
L667D3B:: mov word ptr [ebp*2 + 1020304h], dx
L667D43:: mov word ptr [ebp*2 + 1020304h], ax
L667D4B:: cmp dx, word ptr [ebp*2 + 1020304h]
          jbe short L667D65
L667D55:: mov word ptr [ebp*2 + 1020304h], dx
L667D5D:: mov word ptr [ebp*2 + 1020304h], ax
L667D65:  and ebx, 0F8h
          and edx, 0FF00FFFFh
          cmp ebx, 0D0h
          jl near ptr L667CB3
          jne near ptr L667B6D
L667D83:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L667D8E:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L667DB0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L667DCC
          shl ebx, 3
          add edi, 9
          jmp near ptr L667EB0
L667DB0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne short L667D83
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L667DB0
L667DCC:  movsx eax, byte ptr [edi + 5]
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
          jmp short L667EB0
        MASM6_ALIGN 16
L667EB0:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          sar ecx, 7
          mov bl, byte ptr [edi + 2]
          and eax, 7FFh
          and ecx, 0FFFFFFFEh
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
L667EE5:: add ecx, dword ptr [eax*4 + 1020304h]
L667EEC:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp dx, word ptr [ecx]
          jbe short L667F0E
          mov word ptr [ecx], dx
L667F07:: mov word ptr [ecx + 1020304h], ax
L667F0E:  rol edx, 10h
          cmp ebx, 0D0h
          jl short L667EB0
          jne near ptr L667D8E
          jmp near ptr L667D83
L667F24:  mov esp, dword ptr [esp + 20h]
          test eax, 20h
          je short L667F30
          dec esp
L667F30:  and eax, 1
          jne near ptr L667FF4
          xor eax, eax
          xor ebx, ebx
L667F3D:  mov al, byte ptr [edi]
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
L667F70:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 7
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
L667F8B:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jbe short L667FA3
          mov word ptr [ecx], dx
L667F9C:: mov word ptr [ecx + 1020304h], ax
L667FA3:  cmp dx, word ptr [ecx + 2]
          jbe short L667FB4
          mov word ptr [ecx + 2], dx
L667FAD:: mov word ptr [ecx + 1020304h], ax
L667FB4:: cmp dx, word ptr [ecx + 1020304h]
          jbe short L667FCB
L667FBD:: mov word ptr [ecx + 1020304h], dx
L667FC4:: mov word ptr [ecx + 1020304h], ax
L667FCB:: cmp dx, word ptr [ecx + 1020304h]
          jbe short L667FE2
L667FD4:: mov word ptr [ecx + 1020304h], dx
L667FDB:: mov word ptr [ecx + 1020304h], ax
L667FE2:  rol edx, 10h
          add edi, 3
          dec esp
          jne near ptr L667F3D
          jmp near ptr L667D83
L667FF4:  xor eax, eax
          xor ebx, ebx
L667FF8:  mov al, byte ptr [edi]
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
L66802B:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          mov ecx, esi
          sar ecx, 7
          and ecx, 0FFFFFFFEh
          ror edx, 10h
L668043:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jbe short L66805B
          mov word ptr [ecx], dx
L668054:: mov word ptr [ecx + 1020304h], ax
L66805B:  rol edx, 10h
          add edi, 3
          dec esp
          jne short L667FF8
          jmp near ptr L667D83
?caviar_original_667b20@@YAXHHHIPBEH@Z ENDP

; SEED HISTORY: former 0x00668069 record sub_668069 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00668069..0x0066815D; 244-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x00668069 sub_668069 0x00668069-0x0066815D BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_668069@@YAXXZ
; TRIED: was `EXCLUDED S2a`; disassembly is a normal `ret`, no tail-jmp, self-contained store sequence - not an EH funclet by docs/EXCLUSIONS.md 2a's own test (see 0x00664D51's note in full). Un-excluded.
?caviar_original_668069@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L667CFC + 4], eax
          mov dword ptr [L667D0C + 4], eax
          mov dword ptr [L667EEC + 4], eax
          mov dword ptr [L667F70 + 4], eax
          mov dword ptr [L66802B + 4], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1D8h]
          mov dword ptr [L667EE5 + 3], eax
          mov dword ptr [L667F8B + 3], eax
          mov dword ptr [L668043 + 3], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L667F07 + 3], eax
          mov dword ptr [L667F9C + 3], eax
          mov dword ptr [L668054 + 3], eax
          add eax, 2
          mov dword ptr [L667FAD + 3], eax
          mov eax, dword ptr [vox_setup_copy + 0Ch]
          shl eax, 1
          mov dword ptr [L667FB4 + 3], eax
          mov dword ptr [L667FBD + 3], eax
          add eax, 2
          mov dword ptr [L667FCB + 3], eax
          mov dword ptr [L667FD4 + 3], eax
          add eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L667FDB + 3], eax
          sub eax, 2
          mov dword ptr [L667FC4 + 3], eax
          mov eax, dword ptr [vox_setup_copy]
          mov dword ptr [L667CEA + 4], eax
          mov dword ptr [L667CF4 + 4], eax
          add eax, 2
          mov dword ptr [L667D17 + 4], eax
          mov dword ptr [L667D21 + 4], eax
          add eax, dword ptr [vox_setup_copy + 10h]
          add eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L667D4B + 4], eax
          mov dword ptr [L667D55 + 4], eax
          sub eax, 2
          mov dword ptr [L667D31 + 4], eax
          mov dword ptr [L667D3B + 4], eax
          mov eax, dword ptr [vox_setup_copy + 4h]
          mov dword ptr [L667D04 + 4], eax
          add eax, 2
          mov dword ptr [L667D29 + 4], eax
          add eax, dword ptr [vox_setup_copy + 0Ch]
          add eax, dword ptr [vox_setup_copy + 0Ch]
          mov dword ptr [L667D5D + 4], eax
          sub eax, 2
          mov dword ptr [L667D43 + 4], eax
          mov eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L667CCC + 2], eax
          ret
?caviar_original_668069@@YAXXZ ENDP

_SELFMOD ENDS

        END

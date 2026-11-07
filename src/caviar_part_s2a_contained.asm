; CAVIAR_PART_S2A_CONTAINED.ASM - Caviar's self-modifying voxel renderer: two-pixel, aligned parts, fully on screen.
;
; One original _SELFMOD contribution, 0x00666440-0x00666A7D: the renderer
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

; SEED HISTORY (prior excluded record): 0x00666440 sub_666440 0x00666440-0x00666989 EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00666440 sub_666440 0x00666440-0x00666989 BYTE_EXACT
; symbol    ?caviar_original_666440@@YAXHHHIPBEH@Z
; kind      game
?caviar_original_666440@@YAXHHHIPBEH@Z PROC PUBLIC
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
          je near ptr L666844
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne near ptr L6666AE
L66648D:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L6664B0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L6664D0
          shl ebx, 3
          jmp near ptr L6665D0
        MASM6_ALIGN 16
L6664B0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L6666A3
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L6664B0
L6664D0:  movsx eax, byte ptr [edi + 5]
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
          jmp short L6665D3
        MASM6_ALIGN 16
L6665D0:  add edi, 9
L6665D3:  add ecx, dword ptr [ebx*2 + caviar_output_message_window + 20h]
          mov ah, byte ptr [edi]
          adc ebp, dword ptr [ebx*2 + caviar_output_message_window + 24h]
          mov al, byte ptr [edi + 1]
          test ch, ch
          je short L6665F2
          xor ch, ch
L6665EC:: add ebp, 1020304h
L6665F2:  and eax, 7FFh
          add edx, dword ptr [ebx*2 + caviar_output_message_window + 28h]
          adc edx, 0
          mov bl, byte ptr [edi + 2]
          mov bh, byte ptr [esp + eax]
          add edi, 3
L66660A:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae short L66662C
L666614:: mov word ptr [ebp*2 + 1020304h], dx
L66661C:: mov ax, word ptr [ebx*2 + 1020304h]
L666624:: mov word ptr [ebp*2 + 1020304h], ax
L66662C:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi - 3]
L666637:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae short L666651
L666641:: mov word ptr [ebp*2 + 1020304h], dx
L666649:: mov word ptr [ebp*2 + 1020304h], ax
L666651:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae short L66666B
L66665B:: mov word ptr [ebp*2 + 1020304h], dx
L666663:: mov word ptr [ebp*2 + 1020304h], ax
L66666B:: cmp dx, word ptr [ebp*2 + 1020304h]
          jae short L666685
L666675:: mov word ptr [ebp*2 + 1020304h], dx
L66667D:: mov word ptr [ebp*2 + 1020304h], ax
L666685:  and ebx, 0F8h
          and edx, 0FF00FFFFh
          cmp ebx, 0D0h
          jl near ptr L6665D3
          jne near ptr L66648D
L6666A3:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L6666AE:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L6666D0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L6666EC
          shl ebx, 3
          add edi, 9
          jmp near ptr L6667D0
L6666D0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne short L6666A3
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L6666D0
L6666EC:  movsx eax, byte ptr [edi + 5]
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
          jmp short L6667D0
        MASM6_ALIGN 16
L6667D0:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
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
L666805:: add ecx, dword ptr [eax*4 + 1020304h]
L66680C:: mov ax, word ptr [ebx*2 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp dx, word ptr [ecx]
          jae short L66682E
          mov word ptr [ecx], dx
L666827:: mov word ptr [ecx + 1020304h], ax
L66682E:  rol edx, 10h
          cmp ebx, 0D0h
          jl short L6667D0
          jne near ptr L6666AE
          jmp near ptr L6666A3
L666844:  mov esp, dword ptr [esp + 20h]
          test eax, 20h
          je short L666850
          dec esp
L666850:  and eax, 1
          jne near ptr L666914
          xor eax, eax
          xor ebx, ebx
L66685D:  mov al, byte ptr [edi]
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
L666890:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 7
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
L6668AB:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jae short L6668C3
          mov word ptr [ecx], dx
L6668BC:: mov word ptr [ecx + 1020304h], ax
L6668C3:  cmp dx, word ptr [ecx + 2]
          jae short L6668D4
          mov word ptr [ecx + 2], dx
L6668CD:: mov word ptr [ecx + 1020304h], ax
L6668D4:: cmp dx, word ptr [ecx + 1020304h]
          jae short L6668EB
L6668DD:: mov word ptr [ecx + 1020304h], dx
L6668E4:: mov word ptr [ecx + 1020304h], ax
L6668EB:: cmp dx, word ptr [ecx + 1020304h]
          jae short L666902
L6668F4:: mov word ptr [ecx + 1020304h], dx
L6668FB:: mov word ptr [ecx + 1020304h], ax
L666902:  rol edx, 10h
          add edi, 3
          dec esp
          jne near ptr L66685D
          jmp near ptr L6666A3
L666914:  xor eax, eax
          xor ebx, ebx
L666918:  mov al, byte ptr [edi]
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
L66694B:: mov ax, word ptr [ebx*2 + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          mov ecx, esi
          sar ecx, 7
          and ecx, 0FFFFFFFEh
          ror edx, 10h
L666963:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
          cmp dx, word ptr [ecx]
          jae short L66697B
          mov word ptr [ecx], dx
L666974:: mov word ptr [ecx + 1020304h], ax
L66697B:  rol edx, 10h
          add edi, 3
          dec esp
          jne short L666918
          jmp near ptr L6666A3
?caviar_original_666440@@YAXHHHIPBEH@Z ENDP

; SEED HISTORY: former 0x00666989 record sub_666989 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00666989..0x00666A7D; 244-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x00666989 sub_666989 0x00666989-0x00666A7D BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_666989@@YAXXZ
; TRIED: was `EXCLUDED S2a`; disassembly is a normal `ret`, no tail-jmp, self-contained store sequence - not an EH funclet by docs/EXCLUSIONS.md 2a's own test (see 0x00664D51's note in full). Un-excluded.
?caviar_original_666989@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L66661C + 4], eax
          mov dword ptr [L66662C + 4], eax
          mov dword ptr [L66680C + 4], eax
          mov dword ptr [L666890 + 4], eax
          mov dword ptr [L66694B + 4], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1D8h]
          mov dword ptr [L666805 + 3], eax
          mov dword ptr [L6668AB + 3], eax
          mov dword ptr [L666963 + 3], eax
          mov eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L666827 + 3], eax
          mov dword ptr [L6668BC + 3], eax
          mov dword ptr [L666974 + 3], eax
          add eax, 2
          mov dword ptr [L6668CD + 3], eax
          mov eax, dword ptr [vox_setup_copy + 0Ch]
          shl eax, 1
          mov dword ptr [L6668D4 + 3], eax
          mov dword ptr [L6668DD + 3], eax
          add eax, 2
          mov dword ptr [L6668EB + 3], eax
          mov dword ptr [L6668F4 + 3], eax
          add eax, dword ptr [caviar_kernel_projection_window + 1B8h]
          mov dword ptr [L6668FB + 3], eax
          sub eax, 2
          mov dword ptr [L6668E4 + 3], eax
          mov eax, dword ptr [vox_setup_copy]
          mov dword ptr [L66660A + 4], eax
          mov dword ptr [L666614 + 4], eax
          add eax, 2
          mov dword ptr [L666637 + 4], eax
          mov dword ptr [L666641 + 4], eax
          add eax, dword ptr [vox_setup_copy + 10h]
          add eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L66666B + 4], eax
          mov dword ptr [L666675 + 4], eax
          sub eax, 2
          mov dword ptr [L666651 + 4], eax
          mov dword ptr [L66665B + 4], eax
          mov eax, dword ptr [vox_setup_copy + 4h]
          mov dword ptr [L666624 + 4], eax
          add eax, 2
          mov dword ptr [L666649 + 4], eax
          add eax, dword ptr [vox_setup_copy + 0Ch]
          add eax, dword ptr [vox_setup_copy + 0Ch]
          mov dword ptr [L66667D + 4], eax
          sub eax, 2
          mov dword ptr [L666663 + 4], eax
          mov eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L6665EC + 2], eax
          ret
?caviar_original_666989@@YAXXZ ENDP

_SELFMOD ENDS

        END

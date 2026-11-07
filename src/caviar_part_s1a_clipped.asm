; CAVIAR_PART_S1A_CLIPPED.ASM - Caviar's self-modifying voxel renderer: one-pixel, aligned parts, clipped to a rect.
;
; One original _SELFMOD contribution, 0x00664000-0x006647E3: the renderer
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

; SEED HISTORY (prior excluded record): 0x00664000 sub_664000 0x00664000-0x006646FB EXCLUDED S2d; hidden;sp_ready;purged_ok
; ORIGINAL: 0x00664000 sub_664000 0x00664000-0x006646FB BYTE_EXACT
; symbol    ?caviar_original_664000@@YAXHHHPBHIPBEH@Z
; kind      game
?caviar_original_664000@@YAXHHHPBHIPBEH@Z PROC PUBLIC
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
          je L6644FE
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          mov dword ptr [caviar_output_message_window + 1CCh], edx
          mov esp, dword ptr [vox_projection_selected_angle]
          and eax, 1
          jne L664316
L66407C:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L6640A0
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L6640C0
          shl ebx, 3
          jmp L6641A0
        ALIGN 16
L6640A0:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L664292
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L6640A0
L6640C0:  movsx eax, byte ptr [edi + 5]
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
          jmp L6641A3
        ALIGN 16
L6641A0:  add edi, 9
L6641A3:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
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
          jge L66429D
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L66429D
          cmp eax, dword ptr [vox_clip_bottom]
          jge L66429D
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L66429D
L664205:: add ecx, dword ptr [eax*4 + 1020304h]
L66420C:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L66421D:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664235
L664227:: mov word ptr [ecx*2 + 1020304h], dx
L66422F:: mov byte ptr [ecx + 1020304h], al
L664235:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L66424D
L66423F:: mov word ptr [ecx*2 + 1020304h], dx
L664247:: mov byte ptr [ecx + 1020304h], al
L66424D:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L664265
L664257:: mov word ptr [ecx*2 + 1020304h], dx
L66425F:: mov byte ptr [ecx + 1020304h], al
L664265:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L66427D
L66426F:: mov word ptr [ecx*2 + 1020304h], dx
L664277:: mov byte ptr [ecx + 1020304h], al
L66427D:  rol edx, 10h
          cmp ebx, 0D0h
          jl L6641A3
          jne L66407C
L664292:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L66429D:  cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg L6642F1
          cmp ecx, dword ptr [vox_clip_left_minus_one]
          jl L6642F1
          cmp eax, dword ptr [vox_clip_bottom]
          jg L6642F1
          cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl L6642F1
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jl L6642C8
          dec ecx
          jmp L6642D1
L6642C8:  cmp ecx, dword ptr [vox_clip_left_minus_one]
          jg L6642D1
          inc ecx
L6642D1:  cmp eax, dword ptr [vox_clip_bottom]
          jl L6642DF
          dec eax
          jmp L664205
L6642DF:  cmp eax, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg L664205
          inc eax
          jmp L664205
L6642F1:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          rol edx, 10h
          cmp ebx, 0D0h
          jl L6641A3
          jne L66407C
          jmp L664292
L664316:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je L664340
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge L664360
          shl ebx, 3
          add edi, 9
          jmp L664440
        ALIGN 16
L664340:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne L664292
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je L664340
L664360:  movsx eax, byte ptr [edi + 5]
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
          jmp L664440
        ALIGN 16
L664440:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ah, byte ptr [edi]
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          mov al, byte ptr [edi + 1]
          add edx, dword ptr [ebx*2 + caviar_kernel_projection_window + 8h]
          mov ecx, esi
          sar ecx, 8
          mov bl, byte ptr [edi + 2]
          and eax, 7FFh
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L6644DC
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg L6644DC
          ror edx, 10h
          mov bh, byte ptr [esp + eax]
          mov eax, ebp
          sar eax, 8
          cmp eax, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L6644DC
          cmp eax, dword ptr [vox_clip_bottom]
          jg L6644DC
          add ecx, dword ptr [eax*4 + 1020304h]
L664499:: mov al, byte ptr [ebx + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L6644AA:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L6644C2
L6644B4:: mov word ptr [ecx*2 + 1020304h], dx
L6644BC:: mov byte ptr [ecx + 1020304h], al
L6644C2:  rol edx, 10h
          cmp ebx, 0D0h
          jl L664440
          jne L664316
          jmp L664292
L6644DC:  mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
          cmp ebx, 0D0h
          jl L664440
          jne L664316
          jmp L664292
L6644FE:  mov esp, dword ptr [esp + 24h]
          test eax, 20h
          je L66450A
          dec esp
L66450A:  and eax, 1
          jne L66465E
          xor eax, eax
          xor ebx, ebx
L664517:  mov al, byte ptr [edi]
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
L66454A:: mov al, byte ptr [ebx + 1020304h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 8
          mov ebx, ebp
          sar ebx, 8
          ror edx, 10h
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge L66460A
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L66460A
          cmp ebx, dword ptr [vox_clip_bottom]
          jge L66460A
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L66460A
L66458F:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L664598:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L6645B0
L6645A2:: mov word ptr [ecx*2 + 1020304h], dx
L6645AA:: mov byte ptr [ecx + 1020304h], al
L6645B0:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L6645C8
L6645BA:: mov word ptr [ecx*2 + 1020304h], dx
L6645C2:: mov byte ptr [ecx + 1020304h], al
L6645C8:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L6645E0
L6645D2:: mov word ptr [ecx*2 + 1020304h], dx
L6645DA:: mov byte ptr [ecx + 1020304h], al
L6645E0:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L6645F8
L6645EA:: mov word ptr [ecx*2 + 1020304h], dx
L6645F2:: mov byte ptr [ecx + 1020304h], al
L6645F8:  rol edx, 10h
          add edi, 3
          dec esp
          jne L664517
          jmp L664292
L66460A:  cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jg L6645F8
          cmp ecx, dword ptr [vox_clip_left_minus_one]
          jl L6645F8
          cmp ebx, dword ptr [vox_clip_bottom]
          jg L6645F8
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jl L6645F8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jl L664635
          dec ecx
          jmp L66463E
L664635:  cmp ecx, dword ptr [vox_clip_left_minus_one]
          jg L66463E
          inc ecx
L66463E:  cmp ebx, dword ptr [vox_clip_bottom]
          jl L66464C
          dec ebx
          jmp L66458F
L66464C:  cmp ebx, dword ptr [caviar_kernel_projection_window + 1CCh]
          jg L66458F
          inc ebx
          jmp L66458F
L66465E:  xor eax, eax
          xor ebx, ebx
L664662:  mov al, byte ptr [edi]
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
L664695:: mov al, byte ptr [ebx + 1020304h]
          mov ebx, ebp
          sar ebx, 8
          cmp ebx, dword ptr [vox_clip_bottom]
          jge L6646EC
          cmp ebx, dword ptr [caviar_kernel_projection_window + 1E4h]
          jl L6646EC
          mov ecx, esi
          sar ecx, 8
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1D0h]
          jge L6646EC
          cmp ecx, dword ptr [caviar_kernel_projection_window + 1C8h]
          jl L6646EC
          ror edx, 10h
L6646C8:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L6646D1:: cmp dx, word ptr [ecx*2 + 1020304h]
          jae L6646E9
L6646DB:: mov word ptr [ecx*2 + 1020304h], dx
L6646E3:: mov byte ptr [ecx + 1020304h], al
L6646E9:  rol edx, 10h
L6646EC:  add edi, 3
          dec esp
          jne L664662
          jmp L664292
?caviar_original_664000@@YAXHHHPBHIPBEH@Z ENDP

; SEED HISTORY: former 0x006646FB record sub_6646fb was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x006646FB..0x006647E3; 232-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
; kind      game
; ORIGINAL: 0x006646FB sub_6646fb 0x006646FB-0x006647E3 BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
; symbol    ?caviar_original_6646fb@@YAXXZ
?caviar_original_6646fb@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_colour_table]
          mov dword ptr [L66420C + 2], eax
          mov dword ptr [L664499 + 2], eax
          mov dword ptr [L66454A + 2], eax
          mov dword ptr [L664695 + 2], eax
          mov eax, dword ptr [vox_active_ramp_b]
          mov dword ptr [L664205 + 3], eax
          mov dword ptr [L66458F + 3], eax
          mov dword ptr [L6646C8 + 3], eax
          mov eax, dword ptr [vox_setup_copy]
          mov dword ptr [L66421D + 4], eax
          mov dword ptr [L664227 + 4], eax
          mov dword ptr [L6644AA + 4], eax
          mov dword ptr [L6644B4 + 4], eax
          mov dword ptr [L664598 + 4], eax
          mov dword ptr [L6645A2 + 4], eax
          mov dword ptr [L6646D1 + 4], eax
          mov dword ptr [L6646DB + 4], eax
          add eax, 2
          mov dword ptr [L664235 + 4], eax
          mov dword ptr [L66423F + 4], eax
          mov dword ptr [L6645B0 + 4], eax
          mov dword ptr [L6645BA + 4], eax
          add eax, dword ptr [vox_setup_copy + 10h]
          add eax, dword ptr [vox_setup_copy + 10h]
          mov dword ptr [L664265 + 4], eax
          mov dword ptr [L66426F + 4], eax
          mov dword ptr [L6645E0 + 4], eax
          mov dword ptr [L6645EA + 4], eax
          sub eax, 2
          mov dword ptr [L66424D + 4], eax
          mov dword ptr [L664257 + 4], eax
          mov dword ptr [L6645C8 + 4], eax
          mov dword ptr [L6645D2 + 4], eax
          mov eax, dword ptr [vox_setup_copy + 4h]
          mov dword ptr [L66422F + 2], eax
          mov dword ptr [L6644BC + 2], eax
          mov dword ptr [L6645AA + 2], eax
          mov dword ptr [L6646E3 + 2], eax
          inc eax
          mov dword ptr [L664247 + 2], eax
          mov dword ptr [L6645C2 + 2], eax
          add eax, dword ptr [vox_setup_copy + 0Ch]
          mov dword ptr [L664277 + 2], eax
          mov dword ptr [L6645F2 + 2], eax
          dec eax
          mov dword ptr [L66425F + 2], eax
          mov dword ptr [L6645DA + 2], eax
          ret
?caviar_original_6646fb@@YAXXZ ENDP

_SELFMOD ENDS

        END

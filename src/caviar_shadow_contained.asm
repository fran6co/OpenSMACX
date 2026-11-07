; CAVIAR_SHADOW_CONTAINED.ASM - Caviar's self-modifying voxel renderer: shadow, fully on screen.
;
; One original _SELFMOD contribution, 0x00666FD0-0x00667375: the renderer
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

; Contained-shadow callback: the pinned CFG reaches its common return at
; 0x0066714D; the terminal jump ends at exclusive 0x0066731C, where the
; separately catalogued shared self-modifying setup helper begins.
; ORIGINAL: 0x00666FD0 sub_666fd0 0x00666FD0-0x0066731C BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "build/recovery/caviar-owner-refresh-oct6/shadow-666fd0-boundary-evidence.md:217-reachable-instructions-831-bytes-13-padding-bytes", "source:src/caviar_raster_kernels.inc:caviar_shadow_kernel_candidate", "docs/recovery/ida9-functions.csv:neighbor-shared-helper-0x0066731C-0x00667375"], "reason": "Recover the contained-shadow callback on the genuine Caviar render record; local wrapper name is descriptive, not an original decorated symbol. Pinned reachable CFG and eight relocated dispatch pointers establish the entry; terminal branch ends exactly where the independently catalogued setup helper begins. Shared source kernel implements the callback, with x87 exceptional and allocation/lifetime fidelity still unresolved."}
; symbol    ?caviar_original_666fd0@@YAXHHIPBEH@Z
?caviar_original_666fd0@@YAXHHIPBEH@Z PROC PUBLIC
          push ebp
          push esi
          push edi
          push ebx
          mov dword ptr [caviar_kernel_projection_window + 1D4h], esp
          mov esi, dword ptr [esp + 14h]
          mov ebp, dword ptr [esp + 18h]
          mov edi, dword ptr [esp + 20h]
          mov eax, dword ptr [esp + 24h]
          mov dl, byte ptr [caviar_kernel_projection_window + 1DCh]
          test eax, 10h
          je near ptr L667273
          mov dword ptr [caviar_output_message_window + 1C4h], esi
          mov dword ptr [caviar_output_message_window + 1C8h], ebp
          and eax, 1
          jne near ptr L66714E
L667010:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L667030
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L667050
          shl ebx, 3
          jmp near ptr L6670F0
        MASM6_ALIGN 16
L667030:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne near ptr L667143
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L667030
L667050:  movsx eax, byte ptr [edi + 5]
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
          jmp short L6670F3
        MASM6_ALIGN 16
L6670F0:  add edi, 9
L6670F3:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ecx, esi
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          sar ecx, 8
          mov eax, ebp
          sar eax, 8
L66710B:: add ecx, dword ptr [eax*4 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L66711D:: mov byte ptr [ecx + 1020304h], dl
L667123:: mov byte ptr [ecx + 1020304h], dl
L667129:: mov byte ptr [ecx + 1020304h], dl
L66712F:: mov byte ptr [ecx + 1020304h], dl
          cmp ebx, 0D0h
          jl short L6670F3
          jne near ptr L667010
L667143:  mov esp, dword ptr [caviar_kernel_projection_window + 1D4h]
          pop ebx
          pop edi
          pop esi
          pop ebp
          ret
L66714E:  mov eax, dword ptr [edi]
          and eax, dword ptr [vox_projection_octant_mask]
          je short L667170
          xor ebx, ebx
          mov bl, byte ptr [edi + 8]
          and bl, 1Fh
          cmp bl, 1Ah
          jge short L66718C
          shl ebx, 3
          add edi, 9
          jmp near ptr L667230
L667170:  mov al, byte ptr [edi + 8]
          and al, 80h
          jne short L667143
          xor eax, eax
          mov al, byte ptr [edi + 4]
          lea eax, [eax + eax*2 + 0Ch]
          add edi, eax
          mov ebx, dword ptr [vox_projection_octant_mask]
          and ebx, dword ptr [edi]
          je short L667170
L66718C:  movsx eax, byte ptr [edi + 5]
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
          jmp short L667230
        MASM6_ALIGN 16
L667230:  add esi, dword ptr [ebx*2 + caviar_kernel_projection_window]
          mov ecx, esi
          add ebp, dword ptr [ebx*2 + caviar_kernel_projection_window + 4h]
          sar ecx, 8
          mov eax, ebp
          sar eax, 8
L667248:: add ecx, dword ptr [eax*4 + 1020304h]
          mov bl, byte ptr [edi]
          add edi, 3
          and ebx, 0F8h
L66725A:: mov byte ptr [ecx + 1020304h], dl
          cmp ebx, 0D0h
          jl short L667230
          jne near ptr L66714E
          jmp near ptr L667143
L667273:  mov esp, dword ptr [esp + 1Ch]
          test eax, 20h
          je short L66727F
          dec esp
L66727F:  and eax, 1
          jne short L6672D9
          xor eax, eax
          xor ebx, ebx
L667288:  mov al, byte ptr [edi]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          mov ecx, esi
          and ecx, 0FFFFFF00h
          sar ecx, 8
          mov ebx, ebp
          sar ebx, 8
L6672AD:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L6672B6:: mov byte ptr [ecx + 1020304h], dl
L6672BC:: mov byte ptr [ecx + 1020304h], dl
L6672C2:: mov byte ptr [ecx + 1020304h], dl
L6672C8:: mov byte ptr [ecx + 1020304h], dl
          add edi, 3
          dec esp
          jne short L667288
          jmp near ptr L667143
L6672D9:  xor eax, eax
          xor ebx, ebx
L6672DD:  mov al, byte ptr [edi]
          and eax, 0F8h
          add esi, dword ptr [eax*2 + caviar_kernel_projection_window]
          and ebx, 7FFh
          add ebp, dword ptr [eax*2 + caviar_kernel_projection_window + 4h]
          mov ebx, ebp
          sar ebx, 8
          mov ecx, esi
          sar ecx, 8
L667302:: mov ebx, dword ptr [ebx*4 + 1020304h]
          add ecx, ebx
L66730B:: mov byte ptr [ecx + 1020304h], dl
          add edi, 3
          dec esp
          jne short L6672DD
          jmp near ptr L667143
?caviar_original_666fd0@@YAXHHIPBEH@Z ENDP

; 0x0066731C-0x00667375: the contained-shadow patcher. Its bytes are the
; second span of the four setup stubs (sub_63aee0/63af00/63af20/63af40),
; which tail-jump here after their own three patcher calls.
?caviar_shadow_contained_patch@@YAXXZ PROC PUBLIC
          mov eax, dword ptr [vox_active_ramp_b]
          mov dword ptr [L667248 + 3], eax
          mov dword ptr [L66710B + 3], eax
          mov dword ptr [L667302 + 3], eax
          mov dword ptr [L6672AD + 3], eax
          mov eax, dword ptr [vox_setup_copy + 8h]
          mov dword ptr [L66725A + 2], eax
          mov dword ptr [L66711D + 2], eax
          mov dword ptr [L66730B + 2], eax
          mov dword ptr [L6672B6 + 2], eax
          inc eax
          mov dword ptr [L667123 + 2], eax
          mov dword ptr [L6672BC + 2], eax
          add eax, dword ptr [vox_setup_copy + 14h]
          mov dword ptr [L66712F + 2], eax
          mov dword ptr [L6672C8 + 2], eax
          dec eax
          mov dword ptr [L667129 + 2], eax
          mov dword ptr [L6672C2 + 2], eax
          ret
?caviar_shadow_contained_patch@@YAXXZ ENDP

_SELFMOD ENDS

        END

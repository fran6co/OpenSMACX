; CAVIAR_SELFMOD.INC - shared declarations of the Caviar _SELFMOD modules.
;
; Every module includes this after `.model flat`. It names the C++ data the
; renderers read and their patchers copy, and supplies MASM6_ALIGN.

; ---------------------------------------------------------------- MASM6_ALIGN
;
; MASM 6's padding convention, not program logic. The original modules were
; assembled by MASM 6, whose code-segment ALIGN fills the gap with `2E 8B C0`
; (mov eax,eax with a CS prefix) repeated, then `8B C0` or `90` for a two- or
; one-byte remainder. These fills sit after an unconditional jmp/ret inside
; the renderers and never execute. UASM's own ALIGN pads with different no-op
; encodings and has no option to change them, and MASM syntax cannot spell a
; segment prefix on a register-only instruction, so this macro emits the
; MASM 6 fill bytes for the modules' `ALIGN n` (user decision, 2026-11-07).
; It assumes the module defines `selfmod_segment_base` at the start of its
; paragraph-aligned segment, so offsets from it are segment offsets.
; Use it only in the Caviar _SELFMOD modules, in place of ALIGN.
; UASM settles a macro's expansion in its first pass, before forward jumps
; are sized, so the modules spell every jump's size (`short` / `near ptr`,
; exactly the original encodings) and the offsets this macro reads are final.
MASM6_ALIGN MACRO n:REQ
    LOCAL pad
    pad = (n - (($ - selfmod_segment_base) MOD n)) MOD n
    WHILE pad GE 3
        db 2Eh, 8Bh, 0C0h
        pad = pad - 3
    ENDM
    IF pad EQ 2
        mov eax, eax
    ELSEIF pad EQ 1
        nop
    ENDIF
ENDM

; ----------------------------------------------------------- the shared data
;
; Defined in src/caviar.cpp, each beside its RECOVERY_OBJECT binding. The
; aliases are the C++ names; the modules address members by byte offset.

; 0x009BE6A0: pointer into the selected projection-angle row.
EXTERNDEF ?vox_projection_selected_angle_@@3PBEB:DWORD
vox_projection_selected_angle TEXTEQU <?vox_projection_selected_angle_@@3PBEB>
; 0x009BE6A4: depth/colour/shadow buffers (+0/+4/+8) and their row steps
; (+0Ch colour, +10h depth, +14h shadow).
EXTERNDEF ?vox_setup_copy_@@3UCaviarDrawSetupPointerStorageCandidate@@A:BYTE
vox_setup_copy TEXTEQU <?vox_setup_copy_@@3UCaviarDrawSetupPointerStorageCandidate@@A>
; 0x009BE6BC: the render record's colour table.
EXTERNDEF ?vox_active_colour_table_@@3PAXA:DWORD
vox_active_colour_table TEXTEQU <?vox_active_colour_table_@@3PAXA>
; 0x009C0828: the two-pixel renderers' right clip bound, rect.right * 2.
EXTERNDEF ?vox_clip_right_doubled_@@3HA:DWORD
vox_clip_right_doubled TEXTEQU <?vox_clip_right_doubled_@@3HA>
; 0x009C082C: the render record's ramp B table.
EXTERNDEF ?vox_active_ramp_b_@@3PBHB:DWORD
vox_active_ramp_b TEXTEQU <?vox_active_ramp_b_@@3PBHB>
; 0x009C0840-0x009C0A40: projection cells, then the kernel scratch tail at
; +1A0h (0x009C09E0); the renderers keep their clip/edge state there too.
EXTERNDEF ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A:BYTE
caviar_kernel_projection_window TEXTEQU <?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A>
; 0x009C0B74: the projection octant mask.
EXTERNDEF ?vox_projection_octant_mask_@@3HA:DWORD
vox_projection_octant_mask TEXTEQU <?vox_projection_octant_mask_@@3HA>
; 0x009C0B80-0x009C0E60: I/O callbacks, packed projection rows from +20h.
EXTERNDEF ?CaviarOutputMessageWindow@@3UCaviarOutputMessageWindowCandidate@@A:BYTE
caviar_output_message_window TEXTEQU <?CaviarOutputMessageWindow@@3UCaviarOutputMessageWindowCandidate@@A>
; 0x009C0E60: rect.bottom; 0x009C0E64: rect.left - 1.
EXTERNDEF ?vox_clip_bottom_@@3HA:DWORD
vox_clip_bottom TEXTEQU <?vox_clip_bottom_@@3HA>
EXTERNDEF ?vox_clip_left_minus_one_@@3HA:DWORD
vox_clip_left_minus_one TEXTEQU <?vox_clip_left_minus_one_@@3HA>

; MOVIE_MEDIA_DECODER_UV.ASM - the EA uV decoder's hand-written MASM kernels.
;
; These routines were written in assembly in the original (AGENTS.md, "Inline
; assembly"): private esi/edi/ebp register conventions, no prologue, and
; encodings or data VC6 inline asm cannot reproduce. Assembled by UASM in MASM
; 6 mode (-coff); see cmake/toolchains/vc6.cmake and recovery/environment/wine.py.
;
; The C++ declarations that callers use name these procedures by their
; mangled spellings, so the procedures are declared with exactly those names.

        .386
        .model flat
        option casemap:none

EXTERNDEF ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A:BYTE
EXTERNDEF ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A:BYTE
EXTERNDEF ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A:BYTE

; UVA_DATA is the uV module's own data section in the original image.
UVA_DATA SEGMENT DWORD PUBLIC 'DATA'
; 0x009C9AA8: the low three accumulator bits select one of eight run arms.
movie_uv_coefficient_run_dispatch \
        DD m5cf150_5cf1ce, m5cf150_5cf239, m5cf150_5cf1ac, m5cf150_5cf26a, m5cf150_5cf1db, m5cf150_5cf226, m5cf150_5cf1bc, m5cf150_5cf26a
UVA_DATA ENDS

_TEXT SEGMENT DWORD PUBLIC 'CODE'

; ORIGINAL: 0x005CF150 sub_5cf150 0x005CF150-0x005CF2F0 FILE BYTE_EXACT
; RECOVERY_BINDING: v1 {"evidence": ["pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; mapped span 005CF150-005CF2F0", "pinned dispatch table at 009C9AA8 targets 005CF1CE/239/1AC/26A/1DB/226/1BC/26A; run arms read distinct A8/A4/A0 windows", "both native exits write ESI/EBP/ECX to 009C5920/28/24 and return EDI without restoring it; 5CF2F0 consumes those outputs after five calls", "existing movie_media_decoder.cpp shared workspace, scale and transform-bit-state owners; stack workspace pushes are cleaned by callers and not read in the 416-byte body", "historical leaf source comments preserved verbatim immediately before this mapping; new C++ wrapper/helper does not claim a measured symbol or ABI"], "reason": "Source-owned full implementation of the mapped 5CF150 transform; its normalized return carries the original post-call register state to the six-step 5CF2F0 decoder. This is not native custom-register ABI evidence."}
; symbol    ?sub_5cf150@@YA?AUMediaTransformRegisterContinuation@@XZ
; TRIED: The old leaf source transcript was not register-faithful. The mapped function now owns the full eight-way implementation; its C++ result normalizes the native ESI/EBP/ECX/EDI continuation and does not claim the unknown external custom-register ABI.
; Hand-written asm (EA uV decoder module): no prologue, clobbers esi/ebp unsaved, register-convention outputs, imul/shl/shr/adc rounding.
; Hand-written asm: no prologue, clobbers esi/ebp unsaved, register-convention
; outputs, imul/shl/shr/adc rounding, and a code-pointer jump table.
?sub_5cf150@@YA?AUMediaTransformRegisterContinuation@@XZ PROC PUBLIC
        mov eax, dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 108h]
        mov esi, dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 100h]
        shl eax, 18h
        mov ebp, dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 108h]
        sar eax, 8
        mov ecx, dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 104h]
        imul dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A]
        shl edx, 10h
        xor ebx, ebx
        shr eax, 10h
        mov edi, 3fh
        adc eax, edx
        mov dword ptr [?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], eax
        xor eax, eax
        shr ebp, 8
        mov al, byte ptr [esi]
        inc esi
        sub ecx, 8
        shl eax, cl
        add ecx, 8
        or ebp, eax
m5cf150_5cf199::
        mov eax, ebp
        mov ebx, dword ptr [edi*4 + ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A + 14h]
        and eax, 7
        jmp dword ptr [eax*4 + movie_uv_coefficient_run_dispatch]
m5cf150_5cf1ac::
        mov eax, dword ptr [ebx*4 + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A]
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], eax
        jmp m5cf150_5cf1f9
m5cf150_5cf1bc::
        mov eax, dword ptr [ebx*4 + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A]
        neg eax
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], eax
        jmp m5cf150_5cf1f9
m5cf150_5cf1ce::
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], 0
        jmp m5cf150_5cf1f9
m5cf150_5cf1db::
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], 0
        mov ebx, dword ptr [edi*4 + ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A + 10h]
        dec edi
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], 0
m5cf150_5cf1f9::
        shr ebp, 3
        sub ecx, 3
        cmp ecx, 8
        jge m5cf150_5cf210
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        shl eax, cl
        add ecx, 8
        or ebp, eax
m5cf150_5cf210::
        dec edi
        jg m5cf150_5cf199
        mov dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 100h], esi
        mov dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 108h], ebp
        mov dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 104h], ecx
        ret
m5cf150_5cf226::
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], 0
        mov ebx, dword ptr [edi*4 + ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A + 10h]
        dec edi
m5cf150_5cf239::
        mov eax, ebp
        xor edx, edx
        and eax, 0ffh
        shr eax, 3
m5cf150_5cf245::
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], edx
        mov ebx, dword ptr [edi*4 + ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A + 10h]
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], edx
        mov ebx, dword ptr [edi*4 + ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A + 0ch]
        sub edi, 2
        dec eax
        jne m5cf150_5cf245
        inc edi
        jmp m5cf150_5cf2c4
m5cf150_5cf26a::
        mov eax, ebp
        and eax, 0fch
        cmp eax, 0fch
        je m5cf150_5cf296
        shl eax, 18h
        sar eax, 0ah
        imul dword ptr [ebx*4 + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A]
        shl edx, 10h
        shr eax, 10h
        adc eax, edx
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], eax
        jmp m5cf150_5cf2c4
m5cf150_5cf296::
        shr ebp, 8
        mov al, byte ptr [esi]
        inc esi
        sub ecx, 8
        shl eax, cl
        add ecx, 8
        or ebp, eax
        mov eax, ebp
        shl eax, 18h
        sar eax, 8
        imul dword ptr [ebx*4 + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A]
        shl edx, 10h
        shr eax, 10h
        adc eax, edx
        mov dword ptr [ebx*4 + ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A + 600h], eax
m5cf150_5cf2c4::
        shr ebp, 8
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        sub ecx, 8
        shl eax, cl
        add ecx, 8
        or ebp, eax
        dec edi
        jg m5cf150_5cf199
        mov dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 100h], esi
        mov dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 108h], ebp
        mov dword ptr [?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 104h], ecx
        ret
?sub_5cf150@@YA?AUMediaTransformRegisterContinuation@@XZ ENDP

; ORIGINAL: 0x005CEB12 sub_5ceb12 0x005CEB12-0x005CED1D FILE BYTE_EXACT
; symbol    ?sub_5ceb12@@YAHPBE0PAI@Z
; TRIED: no C-callable convention passes 3 pointers in esi/edi/ebp with zero prologue and no stack args - the original has no `push ebp`/frame at all and even leaves ecx/edx's high 24 bits as whatever the caller left them (Ghidra's CONCAT31 "uVar1/uVar2" tracking), which only a hand-written leaf/tail-continuation does. Landed as a real 3-pointer function reproducing the LUT-pack arithmetic; first divergence is the (unavoidable) prologue itself.
; working copy - scaffold materialised by --work
; size      523 bytes
; prototype
; callers   1   call targets   0
; kind      game
; flags     hidden;sp_ready;purged_ok
; calls     (none)


; Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebp register inputs.
; Hand-written asm: no prologue, esi/edi/ebp register inputs, and eight stores
; spelled [ebp*1+disp32] (89 04 2D), the form the original assembler emitted.
?sub_5ceb12@@YAHPBE0PAI@Z PROC PUBLIC
        xor ebx, ebx
        xor eax, eax
        mov al, byte ptr [esi + 2]
        mov bl, byte ptr [edi + 2]
        mov cl, byte ptr [esi + 6]
        mov dl, byte ptr [edi + 102h]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 0], eax
        xor eax, eax
        mov al, byte ptr [esi + 0ah]
        mov bl, byte ptr [edi + 6]
        mov cl, byte ptr [esi + 0eh]
        mov dl, byte ptr [edi + 106h]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 4], eax
        xor eax, eax
        mov al, byte ptr [esi + 12h]
        mov bl, byte ptr [edi + 0ah]
        mov cl, byte ptr [esi + 16h]
        mov dl, byte ptr [edi + 10ah]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 8], eax
        xor eax, eax
        mov al, byte ptr [esi + 1ah]
        mov bl, byte ptr [edi + 0eh]
        mov cl, byte ptr [esi + 1eh]
        mov dl, byte ptr [edi + 10eh]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 0ch], eax
        xor eax, eax
        mov al, byte ptr [esi + 22h]
        mov bl, byte ptr [edi + 12h]
        mov cl, byte ptr [esi + 26h]
        mov dl, byte ptr [edi + 112h]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 10h], eax
        xor eax, eax
        mov al, byte ptr [esi + 2ah]
        mov bl, byte ptr [edi + 16h]
        mov cl, byte ptr [esi + 2eh]
        mov dl, byte ptr [edi + 116h]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 14h], eax
        xor eax, eax
        mov al, byte ptr [esi + 32h]
        mov bl, byte ptr [edi + 1ah]
        mov cl, byte ptr [esi + 36h]
        mov dl, byte ptr [edi + 11ah]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 18h], eax
        xor eax, eax
        mov al, byte ptr [esi + 3ah]
        mov bl, byte ptr [edi + 1eh]
        mov cl, byte ptr [esi + 3eh]
        mov dl, byte ptr [edi + 11eh]
        mov al, byte ptr [eax + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        mov bl, byte ptr [ebx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl eax, 8
        mov cl, byte ptr [ecx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        shl ebx, 8
        mov dl, byte ptr [edx + ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A + 70ch]
        or eax, ecx
        or ebx, edx
        shl eax, 10h
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp*1 + 1ch], eax
        ret
?sub_5ceb12@@YAHPBE0PAI@Z ENDP

_TEXT ENDS

        END

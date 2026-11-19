# 4-byte aggregates in registers (MATCHING_GUIDE 3.3)

sub_5cd560 0x005CD560 builds a dword with `mov bx,dx; shl ebx,16; mov bx,dx` each iteration. These
probes look for a C spelling that keeps a union/struct in a register.

| script | spellings |
| --- | --- |
| run.py | union DWORD/WORD, struct of two shorts, masks, bitfields, `*(WORD *)&p` |
| more.py | `WORD half[2]`, member order, int/short, named union, byte member, use in the loop |
| asmfn.py | the same inside a function with an `__asm` helper, game and uv (/Oi /Oy /Ob1) flags |
| partial.py | per-iteration 16-bit value from memory (nothing hoistable) |
| word16.py | `WORD h = (WORD)fill; DWORD p = h; p <<= 16; p |= h;` and variants |

Answer: aggregates always stay in a frame slot; scalar spellings fold to `and/shl/or` and hoist. No
spelling produces the image's partial-register merge.

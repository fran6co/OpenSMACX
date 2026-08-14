// ORIGINAL: 0x00432060 FILE
// RULED-OUT: statement order, not declaration order - `this - 0xA14` (the
//        GraphicWin-adjusted receiver used for slide_show/slide_hide,
//        draw_entry, show, hide) is recomputed fresh inside each branch
//        rather than hoisted once; and in the a1==0 branch the
//        `*(self+0x2020)=0` store sits between computing the 0x126A0
//        vtable-slot address and dereferencing it, matching the
//        lea/mov/mov/call order byte for byte. Vtable dispatch and the
//        recursive `esi-0xa14` receiver both use the OriginalObject/
//        original_method/original_slot pointer-to-member pattern from
//        src/original_seam.h, inlined here since the scaffold TU cannot
//        #include project headers and `__thiscall` on a function-pointer
//        typedef is C4234 in this cl.
// name      ?on_status@Datalink@@QAEXH@Z
// size      242 bytes
// spans     0x00432060-0x00432152
// prototype void (__thiscall ?on_status@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042BF10 0x0045D440 0x0045D970 0x005ED9D0 0x005EDCD0
// indirect  0x0043207D 0x004320AC 0x004320D1 0x004320ED 0x004320FC 0x0043212E 0x00432140
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00432060

class __single_inheritance OriginalObject;

template <class Method>
Method original_method(unsigned long address) {
    union {
        unsigned long address;
        Method method;
    } cast;
    cast.address = address;
    return cast.method;
}

template <class Method>
Method original_slot(const void *slot) {
    return original_method<Method>(*reinterpret_cast<const unsigned long *>(slot));
}

#define ORIGINAL(pointer) (reinterpret_cast<OriginalObject *>(pointer))

typedef void (OriginalObject::*VCall0)();
typedef void (OriginalObject::*VCall1)(int);

void Datalink::on_status(int a1) {
    char *const self = (char *)this;

    if (a1 != 0) {
        char *adjusted = self - 0xA14;
        void *vt0 = *(void **)adjusted;
        (ORIGINAL(adjusted)->*original_slot<VCall0>((char *)vt0 + 0x30))();
        slide_show((GraphicWin *)adjusted, 2);
        ((Datalink *)adjusted)->draw_entry();
        ((Win *)adjusted)->show(3);

        char *entry = self + 0x2028;
        for (int i = 0; i < 0x12; ++i) {
            void *vt = *(void **)entry;
            (ORIGINAL(entry)->*original_slot<VCall1>((char *)vt + 4))(0);
            entry += 0xB4C;
        }

        void *p1 = *(void **)(self + 0xEC14);
        int off = *(int *)((char *)p1 + 4);
        char *receiver = self + 0xEC14 + off;
        void *vt2 = *(void **)receiver;
        (ORIGINAL(receiver)->*original_slot<VCall1>((char *)vt2 + 4))(0);
        return;
    }

    {
        void *slot_addr = (void *)(self + 0x126A0);
        *(int *)(self + 0x2020) = 0;
        void *vt = *(void **)slot_addr;
        (ORIGINAL(slot_addr)->*original_slot<VCall0>((char *)vt + 8))();
    }
    {
        void *vt = *(void **)(self + 0xFB00);
        (ORIGINAL((void *)(self + 0xFB00))->*original_slot<VCall0>((char *)vt + 8))();
    }
    {
        char *adjusted = self - 0xA14;
        slide_hide((GraphicWin *)adjusted, 2);
        ((Win *)adjusted)->hide();
    }

    {
        void *p1 = *(void **)(self + 0xEC14);
        int off = *(int *)((char *)p1 + 4);
        char *receiver = self + 0xEC14 + off;
        void *vt2 = *(void **)receiver;
        (ORIGINAL(receiver)->*original_slot<VCall0>((char *)vt2 + 8))();
    }

    char *entry = self + 0x2028;
    for (int i = 0; i < 0x12; ++i) {
        void *vt = *(void **)entry;
        (ORIGINAL(entry)->*original_slot<VCall0>((char *)vt + 8))();
        entry += 0xB4C;
    }
}

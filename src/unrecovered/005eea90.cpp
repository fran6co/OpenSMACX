// ORIGINAL: 0x005EEA90 FILE
// RULED-OUT: nothing yet - MISMATCH #1 push/mov (73% mnemonic sim). Vtable
//            calls on offsets 0xB0/0x64/0x5C/0x1C/0x100 and one recursive
//            self-call whose receiver comes from a field, not `this`.
//            RULED-OUT: `__thiscall` on a raw function-pointer typedef is
//            C4234 in this cl (reserved keyword) - used the pointer-to-
//            member OriginalObject/original_method/original_slot pattern
//            documented in src/original_seam.h instead (inlined here since
//            the scaffold TU cannot #include project headers).
// name      ?key_click_event@Win@@QAEHHH@Z
// size      241 bytes
// spans     0x005EEA90-0x005EEB81
// prototype int (__thiscall ?key_click_event@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EEA90
// indirect  0x005EEAD3 0x005EEAE1 0x005EEB13 0x005EEB2B 0x005EEB38 0x005EEB48 0x005EEB53
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x005EEA90

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

typedef int (OriginalObject::*VCall1)(int);
typedef int (OriginalObject::*VCall2)(int, int);
typedef int (OriginalObject::*VCall0)();
typedef int (__cdecl *Callback2)(int, int);

int Win::key_click_event(int a1, int a2) {
    void *const self = this;

    if ((*(unsigned int *)((char *)self + 0x98) & 0x200000) != 0) {
        return 0;
    }
    if ((*(unsigned char *)((char *)self + 0x9c) & 8) != 0) {
        return 0;
    }

    {
        void *vtbl = *(void **)self;
        if (a2 == 0x1b) {
            (ORIGINAL(self)->*original_slot<VCall1>((char *)vtbl + 0xb0))(-2);
        } else if (a2 == 0xd || a2 == 0x1000d) {
            (ORIGINAL(self)->*original_slot<VCall1>((char *)vtbl + 0xb0))(-1);
        }
    }

    if (*(int *)((char *)self + 0xd4) != 0) {
        Win *target = 0;
        if (*(int *)((char *)self + 0xcc) != 0) {
            int *field_d0 = *(int **)((char *)self + 0xd0);
            target = *(Win **)((char *)field_d0 + 4);
        }
        if (a2 == 9) {
            void *tvtbl = *(void **)target;
            (ORIGINAL(target)->*original_slot<VCall0>((char *)tvtbl + 0x100))();
        } else {
            if (target->key_click_event(a1, a2) != 0) {
                return 1;
            }
        }
    }

    int result = 0;
    *(void **)0x009B7AB8 = self;
    Callback2 callback = *(Callback2 *)((char *)self + 0x42c);
    if (callback != 0) {
        result = callback(a1, a2);
    }
    void *vtbl2 = *(void **)self;
    int r2 = (ORIGINAL(self)->*original_slot<VCall2>((char *)vtbl2 + 0x64))(a1, a2);
    Win *child = *(Win **)((char *)self + 0x38);
    if (child != 0) {
        void *cvtbl = *(void **)child;
        if ((ORIGINAL(child)->*original_slot<VCall0>((char *)cvtbl + 0x5c))() == 0) {
            void *cvtbl2 = *(void **)child;
            (ORIGINAL(child)->*original_slot<VCall0>((char *)cvtbl2 + 0x1c))();
        }
    }
    return result + r2;
}

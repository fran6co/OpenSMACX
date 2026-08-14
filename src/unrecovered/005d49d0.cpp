// ORIGINAL: 0x005D49D0
// name      sub_5d49d0
// size      471 bytes
// spans     0x005D49D0-0x005D4BA7
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004015B0 0x005D4BB0 0x00644F3A
// PROPOSAL: extern "C" int __cdecl sub_5d49d0() -> a __thiscall member, no
//           free params - evidence: `mov esi, ecx` at entry, every access
//           reached from esi, and no explicit `mov eax,` before `ret`
//           (void return). Expressed via an introduced `Sub5d49d0Shim`
//           class, as 0x005CC710 does.
// RULED-OUT: the object's fields at +8/+0xc/+0x10/+0x14 line up exactly
//            with StringStruct's head_/current_/entry_count_/
//            current_position_ (src/stringstruct.h), and the one named
//            callee (0x004015B0) is StringStruct::seek_pos - so `this` is
//            reinterpret_cast to StringStruct* for that one call, while
//            every other access stays raw pointer arithmetic since the
//            rest of this function is not itself a StringStruct method.
//            Landing the closest control-flow-faithful form (divergence
//            starts at instruction #4, on the prologue's register choice).

class Sub5d49d0Shim {
 public:
  void run();
};

void Sub5d49d0Shim::run() {
  char *base = reinterpret_cast<char *>(this);
  int *count10 = reinterpret_cast<int *>(base + 0x10);
  int *field8 = reinterpret_cast<int *>(base + 8);
  int *cur = reinterpret_cast<int *>(base + 0xc);
  int *pos14 = reinterpret_cast<int *>(base + 0x14);

  int iVar3 = *count10;
  int localC = 0;
  int iVar2 = iVar3 - 1;
  if (iVar2 <= 0) {
    return;
  }

  int iVar4;
  int iVar5;
  int iVar6;
  int localVar8;
  int iVar2b;

  do {
    if (localC <= iVar2) {
      *cur = *field8;
      if (localC < 0) {
        iVar3 = *count10;
        {
          int a = abs(localC);
          if (iVar3 < a) goto LAB_join;
          a = abs(localC);
          if (a > 0) {
            do {
              --a;
              *cur = *reinterpret_cast<int *>(*cur + 0x10);
            } while (a != 0);
          }
        }
        iVar3 = *count10;
        iVar4 = iVar3 + localC;
      } else {
        iVar4 = localC;
        if (localC > 0) {
          int a = localC;
          do {
            --a;
            *cur = *reinterpret_cast<int *>(*cur + 0xc);
          } while (a != 0);
        }
      }
      *pos14 = iVar4;
    }
  LAB_join:
    if (*field8 == 0) {
      iVar2b = 0;
    } else {
      iVar2b = *reinterpret_cast<int *>(*cur + 8) + 0xc;
    }
    iVar4 = localC + 1;
    iVar6 = localC;
    localVar8 = iVar4;
    if (iVar4 < iVar3) {
      do {
        if (*field8 != 0) {
          iVar5 = *pos14 + 1;
          *cur = *reinterpret_cast<int *>(*cur + 0xc);
          *pos14 = iVar5;
          if (iVar5 == iVar3) {
            *pos14 = 0;
          }
        }
        int iVar3b;
        if (*cur == 0) {
          iVar3b = 0;
        } else if (*field8 == 0) {
          iVar3b = 0xc;
        } else {
          iVar3b = *reinterpret_cast<int *>(*cur + 8) + 0xc;
        }
        int cmp = sub_5d4bb0(iVar3b, iVar2b);
        if (cmp < 0) {
          if (*field8 == 0) {
            iVar2b = 0;
          } else {
            iVar2b = *reinterpret_cast<int *>(*cur + 8) + 0xc;
          }
          iVar6 = *pos14;
        }
        iVar3 = *count10;
        ++localVar8;
      } while (localVar8 < iVar3);

      if (iVar6 != localC && iVar6 <= iVar3 - 1) {
        *cur = *field8;
        if (iVar6 < 0) {
          int a = abs(iVar6);
          if (*count10 < a) goto LAB_tail;
          a = abs(iVar6);
          if (a > 0) {
            do {
              --a;
              *cur = *reinterpret_cast<int *>(*cur + 0x10);
            } while (a != 0);
          }
          iVar6 = iVar6 + *count10;
        } else {
          if (iVar6 > 0) {
            int a = iVar6;
            do {
              --a;
              *cur = *reinterpret_cast<int *>(*cur + 0xc);
            } while (a != 0);
          }
        }
        int node1 = *cur;
        *pos14 = iVar6;
        int seekOk = reinterpret_cast<StringStruct *>(base)->seek_pos(localC);
        if (seekOk != 0) {
          int node2 = *cur;
          unsigned int t = *reinterpret_cast<unsigned int *>(node1 + 4) ^
                            *reinterpret_cast<unsigned int *>(node2 + 4);
          *reinterpret_cast<unsigned int *>(node1 + 4) = t;
          t = *reinterpret_cast<unsigned int *>(node2 + 4) ^ t;
          *reinterpret_cast<unsigned int *>(node2 + 4) = t;
          int saved8 = *reinterpret_cast<int *>(node1 + 8);
          *reinterpret_cast<unsigned int *>(node1 + 4) =
              *reinterpret_cast<unsigned int *>(node1 + 4) ^ t;
          *reinterpret_cast<int *>(node1 + 8) =
              *reinterpret_cast<int *>(node2 + 8);
          *reinterpret_cast<int *>(node2 + 8) = saved8;
        }
      }
    }
  LAB_tail:
    iVar3 = *count10;
    iVar2 = iVar3 - 1;
    localC = iVar4;
  } while (iVar4 < iVar2);
}

// ORIGINAL: 0x006176E0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@PAUVOX_Vect@@H@Z 0x006176E0-0x006177FE
// RULED-OUT: calling sub_628150 through a cast function pointer (its scaffold prototype is nullary) turns the direct `call rel32` into a loaded/indirect call; the loop body and field offsets (record_->0x24 loop count, record_->0x20 divisor, the index*3+0x1d4 table read, the 0x10ac/0x10c/0x110 tri-tables) match the disassembly, but the prologue register allocation (push/sub order) diverges early. Landing the closest MISMATCH.
// size      286 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@PAUVOX_Vect@@H@Z)(Caviar* this, CaviarData*, CaviarData*, VOX_Vect*, int)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00628150

int Caviar::add_object(CaviarData * a1, CaviarData * a2, VOX_Vect * a3, int a4) {
  typedef int(__cdecl * Fn3)(int, VOX_Vect *, int);
  Fn3 emit = (Fn3)&sub_628150;

  if (a1 == 0 || a2 == 0) {
    return 0x10;
  }
  if (a1->record_ == 0 || a2->record_ == 0) {
    return 7;
  }

  int *rec1 = (int *)a1->record_;
  int *rec2 = (int *)a2->record_;
  int loop_count = rec1[0x24 / 4];
  int divisor = rec2[0x20 / 4];
  char *self = (char *)this;

  if (loop_count > 0) {
    int byte_off = 0;
    int rem3 = (a4 % divisor) * 0xc;
    for (int i = 0; i < loop_count; ++i) {
      uint32_t index = field_13CC_;
      if (index == 200) {
        return 1;
      }
      int *arr_base = *(int **)(rec2 + (0x30 / 4));
      int arg1 = *(int *)((char *)arr_base + byte_off + 4) + rem3;
      int32_t table_val = *(int32_t *)(self + (index * 3 + 0x1d4) * 4);
      emit(arg1, a3, table_val);
      byte_off += 0xc;

      *(int32_t *)(self + 0x10ac + field_13CC_ * 4) = rec1[0x14 / 4] + 0x302;
      uint32_t idx = field_13CC_;
      int *rec1_2c = *(int **)(rec1 + (0x2c / 4));
      *(int32_t *)(self + 0x10c + idx * 8) = rec1_2c[i * 2];
      *(int32_t *)(self + 0x110 + idx * 8) = rec1_2c[i * 2 + 1];
      field_13CC_ = field_13CC_ + 1;
    }
  }
  return 0;
}

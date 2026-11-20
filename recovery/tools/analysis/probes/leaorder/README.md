# lea base/index order of `this + field` (MATCHING_GUIDE 3.4)

| script | question | answer |
| --- | --- | --- |
| run.py | what decides `[idx+esi+K]` against `[esi+idx+K]` for `&text_[field]` | the number of pointer memory references (this->f, q->f, folded `f = f` included) before the field's first use; period 4 for loads, 2 for stores; globals, parameters, dead code and later code do not count |
| forms.py | which natural source forms count, and by how much (phase mod 4) | MATCHING_GUIDE 3.4 "What counts": loads +1, stores +2, compound RMW 0, struct copy +2, `&member` argument +3, parameter arithmetic +3; dead reads, CSE'd reloads and spelling order 0 |

Applied: EditBox::on_key_down 0x00615840 (`field_B38_ |= 1`, 7 -> 3 rows).

# lea base/index order of `this + field` (MATCHING_GUIDE 3.4)

| script | question | answer |
| --- | --- | --- |
| run.py | what decides `[idx+esi+K]` against `[esi+idx+K]` for `&text_[field]` | the number of pointer memory references (this->f, q->f, folded `f = f` included) before the field's first use; period 4 for loads, 2 for stores; globals, parameters, dead code and later code do not count |

Applied: EditBox::on_key_down 0x00615840 (`field_B38_ |= 1`, 7 -> 3 rows).

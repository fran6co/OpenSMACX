"""Current emitted symbols, not replacement original identities or claims."""
from dataclasses import fields
from pathlib import Path
import json
import subprocess

import pytest
from match_recovery.source import read_text
from match_recovery.source.writer import write_symbol

ROOT = Path(__file__).resolve().parents[2]
BASE = '094185e602c671759be086f15b53a6c3e7411961'
BINDINGS = {
    'basebutton': (0x607A20,'?on_left_click@BaseButton@@UAEXHH@Z'),
    'basepop': (0x604410,'?on_key_down@BasePop@@UAEHH@Z'),
    'councwin': (0x425050,'?on_left_click@CouncWin@@UAEXHH@Z'),
    'designwin': (0x439960,'?on_left_click@DesignWin@@UAEXHH@Z'),
    'diplowin': (0x442510,'?on_left_click@DiploWin@@UAEXHH@Z'),
    'famewin': (0x44A500,'?on_left_click@FameWin@@UAEXHH@Z'),
    'net_class': (0x62EAA0,'?create_session@Net@@QAEPAVNetAllocationNode@@PADH0@Z'),
    'prodpicker': (0x493EA0,'?on_right_click@ProdPicker@@UAEXHH@Z'),
    'socialwin': (0x4B1BC0,'?on_right_click@SocialWin@@UAEXHH@Z'),
}


def migration(text, relative, *, reverse=False):
    fixture = Path(__file__).with_name('emitted-symbol-facts-oct11.patch').read_text()
    section = next(p for p in fixture.split('diff --git ')[1:] if p.startswith('a/'+relative+' b/'+relative+'\n'))
    for hunk in section.split('\n@@ ')[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = ''.join(l[1:] for l in lines if l.startswith((' ','-')))
        after = ''.join(l[1:] for l in lines if l.startswith((' ','+')))
        if reverse:
            before, after = after, before
        assert before and text.count(before)==1
        text = text.replace(before,after,1)
    return text


def restore_emitted_symbol_facts(text, relative):
    symbol = BINDINGS[Path(relative).stem][1]
    return migration(text,relative,reverse=True) if '// symbol    '+symbol in text else text


@pytest.mark.parametrize('stem',BINDINGS)
def test_writer_only_adds_current_symbol_and_preserves_originals(stem):
    relative = 'src/'+stem+'.cpp'
    path = ROOT / relative
    old = subprocess.check_output(['git','show',BASE+':'+relative],cwd=ROOT)
    before = read_text(old.decode(),path)
    address,symbol = BINDINGS[stem]
    selected = next(r for r in before if r.address==address)
    assert not selected.symbol and selected.name != symbol
    expected = write_symbol(old,selected,symbol)
    assert expected == path.read_bytes()
    assert migration(old.decode(),relative)==expected.decode()
    assert restore_emitted_symbol_facts(expected.decode(),relative)==old.decode()
    after = read_text(expected.decode(),path)
    assert len(before)==len(after)
    for a,b in zip(before,after):
        for field in fields(a):
            if field.name not in {'line','region','origin','symbol'}:
                assert getattr(a,field.name)==getattr(b,field.name)
        assert b.symbol==(symbol if a.address==address else a.symbol)
        lessons = lambda r: [(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in r.origin]
        assert lessons(a)==lessons(b)


def test_task_keeps_original_identity_and_exact_scope():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT/'recovery/tasks/emitted-symbol-facts-oct11.json').read_text()))
    assert task.base_revision==BASE
    assert {s.address for s in task.subjects}=={a for a,_ in BINDINGS.values()}
    assert set(p for p in task.editable_files if p.startswith('src/'))=={'src/'+s+'.cpp' for s in BINDINGS}

"""Physical selected COFF/PE/PDB controls for associated recovery evidence."""

from hashlib import sha256
import struct

import pytest

from match_recovery.pe_coff import PeImage,Coff
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.linked_objects import SelectedObjects
from match_recovery.linked_code_region import verify_code_region
from match_recovery.linked_code_graph import observe_code_graph,compare_code_graphs
from match_recovery.msvc_link_map import parse_link_map
from test_multispan import object_fixture
from test_pdb_fixups import parts
from test_msf import small_msf
from test_link_input_coverage import inputs
from test_link_file_evidence import stamp
from test_linked_objects import inventory


def fixture(root, *, target=0x1020, operand=0x1B, child=b"\xc3", record=True,
            addend=0, extra=False, map_child=None):
    raw_obj = bytearray(object_fixture(tail=child,
        extra=(b".text$y",b"\xc3",(),1,5,0x60501020) if extra else None))
    code_at, = struct.unpack_from("<I",raw_obj,20+20)
    struct.pack_into("<i",raw_obj,code_at+1,addend)
    evidence,log=inputs(root)
    path=root/'app.obj';path.write_bytes(raw_obj)
    row=next(row for row in evidence['files'] if row['path']==str(path))
    row['before']=row['after']=stamp(raw_obj)
    objects=SelectedObjects(*inventory(evidence,log))
    raw,streams=parts(((20,0,0x1001,target),) if record else ())
    raw=bytearray(raw)
    struct.pack_into('<I',raw,0x98+16,0x1000)
    struct.pack_into('<I',raw,0x178+36,0x60000020)
    streams[5]=bytes(raw[0x178:0x1A0])
    raw[512:528]=b'\xe8'+struct.pack('<i',operand)+b'\xc3'+b'\xcc'*10
    raw[544:560]=child+b'\xcc'*(16-len(child))
    image=PeImage(bytes(raw),sha256(raw).hexdigest())
    fixups=bound_fixups(image,small_msf(streams))
    mapping=('synthetic\n Preferred load address is 00400000\n'
             ' Address         Publics by Value              Rva+Base     Lib:Object\n'
             ' 0001:00000000 _f 00401000 f app.obj\n'
             ' entry point at 0001:00000000\n')
    if map_child is not None:
        mapping+=f'\n Static symbols\n 0001:{map_child-0x1000:08x} $tail2 {0x400000+map_child:08x} app.obj\n'
    return Coff(bytes(raw_obj)),objects,image,fixups,parse_link_map(mapping.encode())


def graph(values):
    _,objects,image,fixups,mapping=values
    return observe_code_graph(objects,mapping,image,[{'owner':'app.obj','symbol':'_f'}],fixups=fixups)


def test_complete_owned_child_is_independently_placed_and_verified(tmp_path):
    values=fixture(tmp_path)
    result=graph(values)
    assert result['result']=='pass',result['gaps']
    proof=result['nodes'][0]['proof']
    assert [row['address'] for row in proof['regions']]==[0x401000,0x401020]
    assert all(row['code']['result']=='pass' for row in proof['regions'])
    assert result['remaining_linked_effects']=='unknown' and not result['ready_for_collection']


@pytest.mark.parametrize('options',[{'record':False},{'extra':True},{'map_child':0x1040}])
def test_missing_child_or_conflicting_map_cannot_be_hidden(tmp_path,options):
    result=graph(fixture(tmp_path,**options))
    assert result['result']=='unknown' and result['graph_fingerprint'] is None


@pytest.mark.parametrize('options',[{'operand':0x1F},{'target':0x1040}])
def test_pointer_or_linker_target_damage_does_not_become_its_own_binding(tmp_path,options):
    result=graph(fixture(tmp_path,**options))
    assert result['result']!='pass'


def test_child_body_change_is_part_of_cross_build_graph_fingerprint(tmp_path):
    left=tmp_path/'left';right=tmp_path/'right';left.mkdir();right.mkdir()
    baseline=graph(fixture(left))
    current=graph(fixture(right,child=b'\x90\xc3'))
    assert baseline['result']==current['result']=='pass'
    assert compare_code_graphs(baseline,current)['result']=='fail'


def test_nonzero_addend_is_applied_after_fixup_symbol_base(tmp_path):
    result=graph(fixture(tmp_path,addend=1,operand=0x1C,child=b'\x90\xc3'))
    assert result['result']=='pass',result['gaps']


def test_effective_child_target_outside_extent_refuses(tmp_path):
    result=graph(fixture(tmp_path,addend=16,operand=0x2B))
    assert result['result']=='unknown'


def test_fixups_from_a_different_image_are_not_reused(tmp_path):
    obj,objects,image,fixups,mapping=fixture(tmp_path)
    raw=bytearray(image.data);raw[560]=1
    changed=PeImage(bytes(raw),sha256(raw).hexdigest())
    with pytest.raises(ValueError,match='exact image'):
        verify_code_region(obj,obj.function('_f'),'app.obj',mapping,changed,objects=objects,fixups=fixups)


def test_missing_selected_object_provenance_refuses(tmp_path):
    obj,_,image,fixups,mapping=fixture(tmp_path)
    with pytest.raises(ValueError,match='selected physical'):
        verify_code_region(obj,obj.function('_f'),'app.obj',mapping,image,fixups=fixups)

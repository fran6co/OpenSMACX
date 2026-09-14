"""Physical export protection and native output composition, no PASS doubles."""
from dataclasses import FrozenInstanceError
from hashlib import sha256
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery import linked_pe_metadata as metadata
from match_recovery.pe_exports import export_metadata, exports
from match_recovery.pe_coff import PeImage
from match_recovery.msf import SmallMsf
from test_linked_initializers import update_artifact
from test_linked_output_preservation import fixture as output_fixture, compare
from test_pe_exports import export_image
from test_msf import small_msf


def fixture(root, *, stamp=1, data=False, mutate=None, body_change=False):
    context = output_fixture(root, policy='/OPT:ICF', body_change=body_change)
    raw = bytearray((context.directory / 'app.exe').read_bytes())
    struct.pack_into('<I', raw, 0x88, stamp)
    struct.pack_into('<I', raw, 0x4C4, stamp)
    struct.pack_into('<II', raw, 0x98 + 96, 0x2020, 0x80)
    struct.pack_into('<IIHHIIIIIII', raw, 0x420,
                     0, stamp, 0, 0, 0x2060, 1, 2, 2, 0x2048, 0x2050, 0x2058)
    struct.pack_into('<II', raw, 0x448, 0x1000, 0x11E0)
    struct.pack_into('<II', raw, 0x450, 0x2070, 0x2078)
    struct.pack_into('<HH', raw, 0x458, 0, 1)
    raw[0x460:0x469] = b'DEMO.exe\0'
    raw[0x470:0x476] = b'Alpha\0'
    raw[0x478:0x47D] = b'Beta\0'
    original = SmallMsf((context.directory / 'app.pdb').read_bytes())
    streams = [original.stream(index) for index in range(original.stream_count)]
    if data:
        payload = bytes(raw[0x600:])
        raw[0x600:] = bytes(0x200) + payload
        struct.pack_into('<I', raw, 0x4D8, 0x800)
        struct.pack_into('<H', raw, 0x86, 3)
        struct.pack_into('<I', raw, 0x98 + 56, 0x4000)
        struct.pack_into('<8sIIIIIIHHI', raw, 0x1C8,
                         b'.edata2\0', 0x200, 0x3000, 0x200, 0x600, 0, 0, 0, 0, 0xC0000040)
        streams[5] = bytes(raw[0x178:0x1F0])
        struct.pack_into('<I', raw, 0x44C, 0x3004)
        raw[0x600:0x608] = b'DATAHERE'
    if mutate:
        mutate(raw, streams)
    update_artifact(context, 'app.exe', bytes(raw))
    update_artifact(context, 'app.pdb', small_msf(streams))
    return context


def observe(context, *, runtime_regions=()):
    with linked_build(context) as native:
        return metadata.qualify_metadata(native, [], project_locator=True,
                                         runtime_regions=runtime_regions)


def test_single_parser_preserves_inventory_and_immutable_exact_ranges():
    raw = export_image()
    image = PeImage(raw, sha256(raw).hexdigest())
    report = export_metadata(image)
    assert report.entries == exports(image)
    assert report.timestamp_rva == 0x1204
    assert {row.kind for row in report.ranges} >= {
        'export_directory', 'export_header', 'export_address_table',
        'export_name_pointers', 'export_ordinals', 'export_module_name', 'export_name'}
    with pytest.raises(FrozenInstanceError):
        report.timestamp = 3
    with pytest.raises(FrozenInstanceError):
        report.ranges[0].size = 1


@pytest.mark.parametrize('data', [False, True])
def test_native_exported_paired_images_only_project_recognized_identification(tmp_path, data):
    old = fixture(tmp_path / 'old', data=data)
    new = fixture(tmp_path / 'new', stamp=321, data=data)
    a, b = observe(old), observe(new)
    assert a['projected_bytes'] == b['projected_bytes']
    assert {'offset': 0x424, 'size': 4, 'kind': 'export_creation_timestamp'} in a['fields']
    report = compare(old, new)
    assert report['result'] == 'pass', report
    assert report['proof_basis'] == 'whole_projected_image_equality'
    assert report['outside_bytes']['scoped_file_bytes'] == 0
    assert report['metadata']['baseline']['export_target_section_protection']['count'] == (2 if data else 1)


@pytest.mark.parametrize('offset,value', [
    (0x428, 1),  # export major/minor version is not creation identity
    (0x430, 3),  # ordinal base
    (0x448, 0x08),  # direct target RVA 0x1000 -> 0x1008
    (0x458, 1),  # alias/name ordinal
    (0x460, ord('G')),  # module name
    (0x471, ord('m')),  # exported name, still lexically before Beta
])
def test_all_nonidentification_export_fields_remain_literal(tmp_path, offset, value):
    old = observe(fixture(tmp_path / 'old'))
    new = observe(fixture(tmp_path / 'new', mutate=lambda raw, _: raw.__setitem__(offset, value)))
    assert old['projected_bytes'] != new['projected_bytes']


def test_forwarder_payload_retained_and_protected_outside_arrays(tmp_path):
    def forward(raw, _):
        struct.pack_into('<I', raw, 0x44C, 0x2088)
        raw[0x488:0x491] = b'DLL.Name\0'
    old = observe(fixture(tmp_path / 'old', mutate=forward))
    def changed(raw, streams):
        forward(raw, streams)
        raw[0x48C] = ord('G')
    new = observe(fixture(tmp_path / 'new', mutate=changed))
    assert old['projected_bytes'] != new['projected_bytes']
    assert any(row == {'offset': 0x488, 'size': 9, 'kind': 'export_forwarder'}
               for row in old['protected_ranges'])


def test_unexplained_export_timestamp_never_gains_projection(tmp_path):
    context = fixture(tmp_path / 'project', mutate=lambda raw, _: struct.pack_into('<I', raw, 0x424, 9))
    with pytest.raises(ValueError, match='export creation timestamp differs'):
        observe(context)


def test_indirect_export_arrays_and_strings_outside_directory_are_protected(tmp_path):
    context = fixture(tmp_path / 'project', mutate=lambda raw, _: struct.pack_into(
        '<I', raw, 0x98 + 100, 40))
    report = observe(context)
    for kind, offset in [('export_address_table', 0x448), ('export_name_pointers', 0x450),
                         ('export_ordinals', 0x458), ('export_module_name', 0x460),
                         ('export_name', 0x470)]:
        assert any(row['kind'] == kind and row['offset'] == offset for row in report['protected_ranges'])


@pytest.mark.parametrize('kind,stamp', [('module', 0x41414141), ('names', 0x2070), ('ordinals', 0x10000)])
def test_creation_timestamp_aliases_other_export_metadata_cannot_be_projected(tmp_path, kind, stamp):
    def alias(raw, _):
        if kind == 'module':
            struct.pack_into('<I', raw, 0x42C, 0x2024)
        else:
            struct.pack_into('<I', raw, 0x438, 1)
            struct.pack_into('<I', raw, 0x440 if kind == 'names' else 0x444, 0x2024)
    context = fixture(tmp_path / 'project', stamp=stamp, mutate=alias)
    raw = (context.directory / 'app.exe').read_bytes()
    assert len(exports(PeImage(raw, sha256(raw).hexdigest()))) == 2
    with pytest.raises(ValueError, match='another metadata use'):
        observe(context)


@pytest.mark.parametrize('pointer', [0x2024, 0x2020, 0x20C4, 0x20E0])
def test_data_export_in_same_metadata_section_refuses_even_interior_or_unobserved_label(tmp_path, pointer):
    context = fixture(tmp_path / 'project', mutate=lambda raw, _: struct.pack_into('<I', raw, 0x44C, pointer))
    # A target in the export directory is a forwarder, not an address. That
    # malformed form also refuses; it must not bypass the parser's namespace.
    with pytest.raises(ValueError):
        observe(context)


def test_export_timestamp_cannot_also_be_an_eat_entry(tmp_path):
    def alias(raw, _):
        struct.pack_into('<I', raw, 0x434, 1)
        struct.pack_into('<I', raw, 0x438, 0)
        struct.pack_into('<III', raw, 0x43C, 0x2024, 0, 0)
    context = fixture(tmp_path / 'project', stamp=0x1000, mutate=alias)
    raw = (context.directory / 'app.exe').read_bytes()
    assert exports(PeImage(raw, sha256(raw).hexdigest()))[0].rva == 0x1000
    with pytest.raises(ValueError, match='another metadata use'):
        observe(context)


def test_export_timestamp_cannot_also_be_a_valid_forwarder(tmp_path):
    context = fixture(tmp_path / 'project', stamp=int.from_bytes(b'A.B\0', 'little'),
                      mutate=lambda raw, _: struct.pack_into('<I', raw, 0x44C, 0x2024))
    raw = (context.directory / 'app.exe').read_bytes()
    assert exports(PeImage(raw, sha256(raw).hexdigest()))[1].forwarder == 'A.B'
    with pytest.raises(ValueError, match='another metadata use'):
        observe(context)


def test_cached_string_preserves_each_metadata_role():
    raw = bytearray(export_image())
    # Reuse Alpha's bytes for the module name, read before the export names.
    struct.pack_into('<I', raw, 0x40C, 0x1300)
    report = export_metadata(PeImage(bytes(raw), sha256(raw).hexdigest()))
    uses = {row.kind for row in report.ranges if row.rva == 0x1300}
    assert uses == {'export_module_name', 'export_name'}


@pytest.mark.parametrize('kind', ['array', 'ordinal', 'name', 'relocation', 'runtime'])
def test_malformed_or_aliased_export_evidence_refuses(tmp_path, kind):
    def damage(raw, _):
        if kind == 'array':
            struct.pack_into('<I', raw, 0x43C, 0xFFFFFFF0)
        elif kind == 'ordinal':
            struct.pack_into('<H', raw, 0x458, 2)
        elif kind == 'name':
            struct.pack_into('<I', raw, 0x450, 0x20C4)
        elif kind == 'relocation':
            struct.pack_into('<I', raw, 0x380, 0x2000)
            struct.pack_into('<H', raw, 0x388, 0x3024)
    context = fixture(tmp_path / 'project', mutate=damage)
    with pytest.raises(ValueError):
        observe(context, runtime_regions=([{'address': 0x402024, 'size': 4}] if kind == 'runtime' else ()))


def test_exports_do_not_make_changed_icf_output_acceptable(tmp_path):
    report = compare(fixture(tmp_path / 'old'), fixture(tmp_path / 'new', body_change=True))
    assert report['result'] == 'unknown', report
    assert 'NOICF' in report['reason']


def test_export_projection_preserves_native_final_guard(tmp_path, monkeypatch):
    context = fixture(tmp_path / 'project')
    original = metadata.export_metadata
    def expired(image):
        value = original(image)
        context.alive = False
        return value
    with pytest.raises(ValueError, match='expired'):
        with linked_build(context) as native:
            monkeypatch.setattr(metadata, 'export_metadata', expired)
            metadata.qualify_metadata(native, [])

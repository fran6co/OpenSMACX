"""Target32 vehicle-alias test support; no production changes or acceptance grant.

The caller supplies a freshly qualified application build. One control TU uses
its actual ordinary storage object, real headers and extracted current bodies.
"""
from contextlib import nullcontext
from dataclasses import asdict
from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import sys
import time

from match_recovery.includes import COMMENTS_OR_STRINGS


def definition(text, signature):
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)
    hits = list(re.finditer(re.escape(signature), masked))
    assert len(hits) == 1, signature
    start = hits[0].start()
    end = masked.index('{', start) + 1
    depth = 1
    while depth:
        depth += (masked[end] == '{') - (masked[end] == '}')
        end += 1
    return text[start:end]


def digest(data):
    return hashlib.sha256(data.encode() if isinstance(data, str) else data).hexdigest()


def render_program(root, pointer_oracle, signed_oracle):
    root = Path(root)
    veh = (root / 'src/veh.cpp').read_text()
    alpha = (root / 'src/alpha.h').read_text()
    original = {name: definition(veh, 'int __cdecl ' + name + '(')
                for name in ('offense_proto', 'armor_proto')}
    variants, proof = [], []

    def emit(base, suffix, replacements):
        text = original[base]
        for before, after in replacements:
            assert text.count(before) == 1, (base, suffix, before)
            text = text.replace(before, after, 1)
        before = 'int __cdecl ' + base + '('
        assert text.count(before) == 1
        text = text.replace(before, 'int __cdecl ' + base + '_' + suffix + '(', 1)
        variants.append(text)
        proof.append(dict(original=base, variant=base + '_' + suffix,
                          original_sha256=digest(original[base]),
                          variant_sha256=digest(text), replacements=replacements,
                          only_additional_change='definition name'))

    for base, argument in (('offense_proto', 'veh_id_def'),
                           ('armor_proto', 'veh_id_atk')):
        read = 'Vehs[' + argument + '].proto_id'
        anchor = read + ' != BSC_SPORE_LAUNCHER'
        assert original[base].count(anchor) == 1
        guarded = '(' + argument + ' == -1 ? static_cast<int16_t>('
        guarded += 'reinterpret_cast<uintptr_t>(Reactor[0].name_short) >> 16) : '
        guarded += read + ') != BSC_SPORE_LAUNCHER'
        emit(base, 'product', [])
        emit(base, 'guarded', [(anchor, guarded)])
        emit(base, 'wrong_member', [(anchor, anchor.replace('.proto_id', '.flags'))])
        emit(base, 'wrong_owner', [(anchor, guarded.replace('Reactor[0]', 'Reactor[1]'))])
        if base == 'offense_proto':
            emit(base, 'old_magnitude', [(
                'is_proto_missile(proto_id) && off_rating_byte < 99',
                'is_proto_missile(proto_id) && off_rating < 99')])

    rules = '\n'.join(definition(alpha, 'struct ' + name + ' {') + ';'
                      for name in ('RulesBasic', 'RulesStorageWindow'))
    pointer = pointer_oracle
    host_only = ('    if (sizeof(uintptr_t) > 4)\n'
                 '        check_pointer_word(static_cast<uintptr_t>(UINT64_C(0xabcd000f0040)),true);\n')
    assert pointer.count(host_only) == 1
    pointer = pointer.replace(host_only, '')
    pointer = pointer.replace('int main() {', 'static void pointer_control() {', 1)
    signed = signed_oracle.replace('int main() {', 'static void signed_control() {', 1)
    assert signed.count('        prototype_offense_value = rating;') == 1
    signed = signed.replace('        prototype_offense_value = rating;', '')
    assert signed.count('            prototype_missile = missile;') == 1
    signed = signed.replace('            prototype_missile = missile;',
                            '            Chassis[0].missile = static_cast<uint8_t>(missile);')
    assert signed.count('    assert(cases == 3072);') == 1
    signed = signed.replace('    assert(cases == 3072);',
                            '    assert(cases == 3072);\n    signed_cases_total += cases;')
    for before, after in (('offense_proto(', 'offense_under_test('),
                          ('armor_proto(', 'armor_under_test('),
                          ('assert(', 'CONTROL_CHECK('),
                          ('std::memcpy', 'memcpy'), ('std::fprintf', 'fprintf')):
        pointer, signed = pointer.replace(before, after), signed.replace(before, after)
    source = HEADERS + rules + STATE + '\n\n'.join(variants) + pointer + signed + MAIN
    return source, dict(body_variants=proof, rules_sha256=digest(rules),
                        pointer_oracle_sha256=digest(pointer_oracle),
                        signed_oracle_sha256=digest(signed_oracle),
                        generated_source_sha256=digest(source),
                        high64_case='separate host representation oracle only',
                        data_setup='actual typed tables; pointer cases defense-2, signed cases+1')

HEADERS = r"""#include "stdafx.h"
#include "general.h"
#include "veh.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
"""

STATE = r"""
#define Rules (RulesStorage.rules_)
RulesStorageWindow RulesStorage;
VehPrototype VehPrototypes[MaxVehProtoNum];
RulesWeapon Weapon[MaxWeaponNum];
RulesArmor Armor[MaxArmorNum];
RulesChassis Chassis[ChassisStorageCount];
int GenderDefault;
BOOL PluralityDefault;
int TimerHoldFlag;
BOOL VehBattleUnkTgl;
uint32_t VehBattleModCount[2];
int __cdecl speed(int, BOOL) { fputs("UNEXERCISED_LINK_DOUBLE speed\n",stderr); ExitProcess(90); return 0; }
int __cdecl veh_drop(int,int,int) { fputs("UNEXERCISED_LINK_DOUBLE veh_drop\n",stderr); ExitProcess(91); return 0; }
int __cdecl veh_lift(int) { fputs("UNEXERCISED_LINK_DOUBLE veh_lift\n",stderr); ExitProcess(92); return 0; }
static const char *mode_name;
static int failure_code;
static int signed_cases_total;
static void fail_check(const char *expr, int code, int line) {
 fprintf(stderr,"CONTROL_FAILURE mode=%s code=%d line=%d expression=%s\n",mode_name,code,line,expr);
 fflush(stderr); ExitProcess(code);
}
#define CHECK_CODE(expr,code) do { if (!(expr)) fail_check(#expr,code,__LINE__); } while (0)
#define CONTROL_CHECK(expr) CHECK_CODE(expr,failure_code)
typedef int (__cdecl *RatingFn)(int,int,BOOL);
static RatingFn offense_under_test, armor_under_test;
static void initial_layout() {
 CHECK_CODE(sizeof(void*)==4,10);CHECK_CODE(sizeof(uintptr_t)==4,11);
 CHECK_CODE(sizeof(BOOL)==4,12);CHECK_CODE(sizeof(int16_t)==2,13);
 CHECK_CODE(sizeof(RulesReactor)==12,14);CHECK_CODE(sizeof(Veh)==52,15);
 CHECK_CODE(offsetof(Veh,proto_id)==10,16);CHECK_CODE(offsetof(RulesReactor,name_short)==4,17);
 CHECK_CODE(reinterpret_cast<uintptr_t>(Vehs)-reinterpret_cast<uintptr_t>(Reactor)==48,18);
 CHECK_CODE(reinterpret_cast<uintptr_t>(Vehs)-52+10==reinterpret_cast<uintptr_t>(&Reactor[0].name_short)+2,19);
 size_t i; const unsigned char *reactor_bytes=reinterpret_cast<const unsigned char*>(Reactor);
 const unsigned char *vehicle_bytes=reinterpret_cast<const unsigned char*>(Vehs);
 for(i=0;i<sizeof(Reactor);++i) CHECK_CODE(reactor_bytes[i]==0,20);
 for(i=0;i<sizeof(Vehs);++i) CHECK_CODE(vehicle_bytes[i]==0,21);
 fprintf(stdout,"LAYOUT pointers=%u reactor=%u veh=%u proto=%u short=%u delta=%u zero=%u\n",
  (unsigned)sizeof(void*),(unsigned)sizeof(RulesReactor),(unsigned)sizeof(Veh),
  (unsigned)offsetof(Veh,proto_id),(unsigned)offsetof(RulesReactor,name_short),
  (unsigned)(reinterpret_cast<uintptr_t>(Vehs)-reinterpret_cast<uintptr_t>(Reactor)),
  (unsigned)(sizeof(Reactor)+sizeof(Vehs)));
 fflush(stdout);
}
static void reset_tables() {
 memset(VehPrototypes,0,sizeof(VehPrototypes));memset(Weapon,0,sizeof(Weapon));
 memset(Armor,0,sizeof(Armor));memset(Chassis,0,sizeof(Chassis));
 Rules.psi_combat_ratio_atk[0]=3;Rules.psi_combat_ratio_atk[1]=11;Rules.psi_combat_ratio_atk[2]=13;
 Rules.psi_combat_ratio_def[0]=7;Rules.psi_combat_ratio_def[1]=16;Rules.psi_combat_ratio_def[2]=17;
 Vehs[0].proto_id=0;Vehs[2].proto_id=0;
}
static void prepare_pointer() {
 reset_tables(); Weapon[0].offense_rating=4;Armor[0].defense_rating=-2;
 Chassis[0].triad=TRIAD_AIR;Chassis[1].triad=TRIAD_SEA;
 VehPrototypes[BSC_SPORE_LAUNCHER].chassis_id=1;
}
static void prepare_signed() {
 reset_tables();Armor[0].defense_rating=1;Chassis[0].triad=TRIAD_SEA;
}
"""

MAIN = r"""
int main(int argc,char **argv) {
 mode_name=argc==2?argv[1]:"missing";failure_code=60;initial_layout();
 if(argc!=2) return 2;
 if(strcmp(mode_name,"actual")==0) {
  offense_under_test=offense_proto_guarded;armor_under_test=armor_proto_guarded;
  prepare_pointer();pointer_control();prepare_signed();signed_control();
  offense_under_test=offense_proto_product;armor_under_test=armor_proto_product;
  prepare_pointer();pointer_control();prepare_signed();signed_control();
  CONTROL_CHECK(signed_cases_total==6144);
  puts("CONTROL_PASS guarded+product pointer/spore/infowar/positive; signed_cases=3072_each,total6144; high64_host_only");return 0;
 }
 if(strcmp(mode_name,"wrong-member")==0) {
  failure_code=41;offense_under_test=offense_proto_wrong_member;armor_under_test=armor_proto_wrong_member;
  prepare_pointer();pointer_control();return 3;
 }
 if(strcmp(mode_name,"wrong-owner")==0) {
  failure_code=42;offense_under_test=offense_proto_wrong_owner;armor_under_test=armor_proto_wrong_owner;
  prepare_pointer();pointer_control();return 4;
 }
 if(strcmp(mode_name,"old-magnitude")==0) {
  failure_code=51;offense_under_test=offense_proto_old_magnitude;armor_under_test=armor_proto_product;
  prepare_signed();signed_control();return 5;
 }
 return 6;
}
"""


def _dump(directory, name, value):
    (directory / name).write_text(json.dumps(value, indent=2, default=str) + '\n')


def _file_hash(path):
    return digest(Path(path).read_bytes())


def _linked_layout(executable, map_path, pdb_path, storage, objects, *, zero_tail):
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.msvc_link_map import parse_link_map
    from match_recovery.pdb_contributions import bound_contributions

    image = PeImage(executable.read_bytes(), _file_hash(executable))
    link_map = parse_link_map(map_path.read_bytes())
    link_map.validate(image)
    names = ('?Reactor@@3PAURulesReactor@@A', '?Vehs@@3PAUVeh@@A')
    publics = []
    for name in names:
        matches = [row for row in link_map.publics if row.symbol == name]
        assert len(matches) == 1, name
        publics.append(matches[0])
    assert publics[0].segment == publics[1].segment
    assert publics[1].address - publics[0].address == 48
    assert all(row.owner == storage.name for row in publics)
    definitions = {name: [] for name in names}
    for path in objects:
        coff = Coff(path.read_bytes())
        for symbol in coff.symbols.values():
            if symbol.name not in definitions or symbol.section <= 0:
                continue
            section = coff.sections[symbol.section - 1]
            definitions[symbol.name].append(dict(
                path=str(path), offset=symbol.value, flags=section.flags,
                extent=coff.section_sizes[symbol.section - 1],
                raw_bytes=len(section.data), relocations=len(section.relocations)))
    for name, offset in zip(names, (0, 48)):
        matches = definitions[name]
        assert len(matches) == 1 and matches[0]['path'] == str(storage)
        row = matches[0]
        assert row['offset'] == offset and row['extent'] == 106596
        assert not row['flags'] & 0x1000  # ordinary contribution, not COMDAT
        assert row['raw_bytes'] == 0 and row['relocations'] == 0
    region, _, file_size = image.sections[publics[0].segment - 1]
    begin, end = publics[0].address, publics[1].address + 2049 * 52
    assert region.start <= begin < end <= region.end
    assert image.section_flags[publics[0].segment - 1] & 0x80000000
    if zero_tail:
        assert region.start + file_size <= begin
    pdb = bound_contributions(image, pdb_path.read_bytes())
    modules = [(index, module) for index, module in enumerate(pdb.modules)
               if module.object_name.replace('\\', '/').split('/')[-1] == storage.name]
    assert len(modules) == 1
    module_index, module = modules[0]
    cover = [row for row in pdb.contributions
             if row.module == module_index and row.section == publics[0].segment
             and region.start + row.offset == begin and row.size >= end - begin]
    assert len(cover) == 1
    overlap = [row for row in pdb.contributions if row.module != module_index
               and row.size and row.section == publics[0].segment
               and max(region.start + row.offset, begin)
               < min(region.start + row.offset + row.size, end)]
    assert not overlap
    return dict(publics=[asdict(row) for row in publics], definitions=definitions,
                delta=48, pdb_module=asdict(module), contribution=asdict(cover[0]),
                other_module_overlap=0, pe_map_validated=True,
                allocation=[begin, end], pe_zero_tail_required=zero_tail,
                limits='Bounded storage observation; not general linked preservation.')


def run_native_control(root, build_report_path, directory, pointer_oracle, signed_oracle,
                       resource_file=None):
    """One compile/link and four runs against a caller-supplied current build."""
    from match_recovery.resources import resource_environment
    from match_recovery.processes import run_bounded
    from match_recovery.build_compile_evidence import live_build_compilations
    from match_recovery.compile_file_evidence import verify_compile_files
    from match_recovery.link_file_evidence import verify_link_files
    from match_recovery.source_inventory import source_snapshot, assert_snapshot_current
    from match_recovery.measurement import Measurement

    root, directory = Path(root).resolve(), Path(directory).resolve()
    build_report_path = Path(build_report_path).resolve()
    assert directory.is_dir() and directory.is_relative_to(root)
    source, obj = directory / 'ratings.cpp', directory / 'ratings.obj'
    executable = directory / 'ratings.exe'
    assert not any(path.exists() for path in (source, obj, executable))
    snapshot = source_snapshot(root, root / '.opensmacx/game/terranx_original.exe')
    settings = Measurement.read(root)
    policy = settings.snapshot(root, {})
    own_inputs = [root / 'recovery/tests/test_vehicle_negative_prototype_alias.py',
                  Path(__file__).resolve(), build_report_path]
    own_hashes = {str(path): _file_hash(path) for path in own_inputs}
    built = json.loads(build_report_path.read_text())
    assert built['result'] == 'pass'
    build_directory = Path(built['directory'])
    if not build_directory.is_absolute():
        build_directory = root / build_directory
    build_directory = build_directory.resolve()
    source_text, generation = render_program(root, pointer_oracle, signed_oracle)
    assert_snapshot_current(root, snapshot)
    assert Measurement.read(root).snapshot(root, {}) == policy
    assert {str(path): _file_hash(path) for path in own_inputs} == own_hashes
    source.write_text(source_text)
    _dump(directory, 'generation-proof.json', generation)
    report = dict(status='running', compiler_calls=0, linker_calls=0, runtime_calls=0,
                  build_report=str(build_report_path), build_report_sha256=_file_hash(build_report_path),
                  artifact_directory=str(directory))
    _dump(directory, 'progress.json', report)
    compile_link = json.loads((root / 'recovery/build.json').read_text())['compile_link']
    with live_build_compilations(root, build_directory, compile_link,
                                 outputs=built['outputs']) as (live, current):
        _dump(directory, 'application-live-before.json', live)
        assert live['result'] == 'pass'
        rows = {Path(row['source']).relative_to(root).as_posix(): row
                for row in live['compilations']}
        assert 'src/vehicle_storage.cpp' in rows and 'src/veh.cpp' in rows
        storage = Path(rows['src/vehicle_storage.cpp']['output'])
        receipt = Path(str(storage) + '.compile.json')
        storage_hash = _file_hash(storage)
        receipt_hash = _file_hash(receipt)
        observed = verify_compile_files(receipt.read_bytes(), object_path=storage)
        _dump(directory, 'storage-before.json', observed)
        assert observed['result'] == 'pass'
        layout = _linked_layout(build_directory / 'OpenSMACX.exe',
                                build_directory / 'OpenSMACX.map',
                                build_directory / 'OpenSMACX.pdb', storage,
                                [Path(row['output']) for row in live['compilations']],
                                zero_tail=True)
        _dump(directory, 'application-linked-layout.json', layout)
        production_path = Path(str(rows['src/veh.cpp']['output']) + '.compile.json')
        production = json.loads(production_path.read_text())
        actual_source = root / 'src/veh.cpp'
        assert production['source'] == str(actual_source)
        source_observation = next(row for row in production['inputs']
                                  if row['path'] == str(actual_source))
        assert source_observation['after']['sha256'] == _file_hash(actual_source)
        flags = [word for word in production['arguments']
                 if word not in ('src/veh.cpp', str(actual_source), '/Brepro')
                 and not word.startswith(('/Fo', '/Fd'))]
        assert '/O2' in flags and '/Ob2' in flags and '/DNDEBUG' in flags
        assert any(word in ('-c', '/c') for word in flags)
        inherited = bool(os.environ.get('MATCH_RECOVERY_RESOURCE_LEASE'))
        if inherited:
            lease = nullcontext('inherited')
        else:
            resource_file = Path(resource_file or root / '.opensmacx/recovery/worker-resource.json')
            assert resource_file.is_file(), 'selected native lane requires an assigned compiler resource'
            lease = resource_environment(json.loads(resource_file.read_text()), [root])
        with lease as slot:
            # Import after acquiring the lease; wrappers inherit VC6_PREFIX.
            from recovery.environment.wine import environment
            assert os.environ.get('MATCH_RECOVERY_RESOURCE_LEASE')
            assert os.environ.get('VC6_PREFIX'), 'selected resource must provide VC6_PREFIX'
            runtime_environment = environment()
            runtime_environment['WINEPREFIX'] = os.environ['VC6_PREFIX']
            report.update(resource=str(slot), VC6_PREFIX=os.environ['VC6_PREFIX'])
            wrapper = root / 'recovery/environment/wine.py'
            command = [sys.executable, str(wrapper), '--tool=cl', '--repro-root=' + str(root),
                       '--compile-diagnostics=' + str(directory / 'ratings.compile.json'),
                       *flags, '/Fo' + str(obj), '/Fd' + str(directory / 'compile.pdb'), str(source)]
            _dump(directory, 'compile-command.json', command)
            report['compiler_calls'] += 1
            _dump(directory, 'progress.json', report)
            tick = time.perf_counter()
            compiled = run_bounded(command, cwd=root, timeout=300)
            _dump(directory, 'compile-result.json', dict(returncode=compiled.returncode,
                  elapsed_seconds=time.perf_counter() - tick,
                  stdout=compiled.stdout.decode(errors='replace'),
                  stderr=compiled.stderr.decode(errors='replace')))
            assert compiled.returncode == 0, (compiled.stdout + compiled.stderr).decode(errors='replace')
            checked = verify_compile_files((directory / 'ratings.compile.json').read_bytes(), object_path=obj)
            _dump(directory, 'compile-check.json', checked)
            assert checked['result'] == 'pass'
            link = [sys.executable, str(wrapper), '--tool=link',
                    '--link-diagnostics=' + str(directory / 'link.log'), '/nologo', '/VERBOSE',
                    '/SUBSYSTEM:CONSOLE', '/INCREMENTAL:NO', '/OPT:REF', '/OPT:ICF', '/DEBUG',
                    '/DEBUGTYPE:CV,FIXUP', '/PDBTYPE:SEPT', '/MAP:' + str(directory / 'ratings.map'),
                    '/PDB:' + str(directory / 'ratings.pdb'), '/OUT:' + str(executable),
                    str(obj), str(storage), 'kernel32.lib']
            _dump(directory, 'link-command.json', link)
            report['linker_calls'] += 1
            _dump(directory, 'progress.json', report)
            linked = run_bounded(link, cwd=directory, timeout=120)
            _dump(directory, 'link-result.json', dict(returncode=linked.returncode,
                  stdout=linked.stdout.decode(errors='replace'), stderr=linked.stderr.decode(errors='replace')))
            assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors='replace')
            checked = verify_link_files((directory / 'link.inputs.json').read_bytes(),
                                        (directory / 'link.log').read_bytes())
            _dump(directory, 'link-check.json', checked)
            assert checked['result'] == 'pass'
            _dump(directory, 'control-linked-layout.json',
                  _linked_layout(executable, directory / 'ratings.map', directory / 'ratings.pdb',
                                 storage, [storage, obj], zero_tail=False))
            outcomes = {}
            for mode in ('actual', 'wrong-member', 'wrong-owner', 'old-magnitude'):
                report['runtime_calls'] += 1
                _dump(directory, 'progress.json', report)
                ran = subprocess.run(['wine32', str(executable), mode], cwd=directory,
                                     env=runtime_environment, capture_output=True, timeout=30)
                outcome = dict(returncode=ran.returncode, stdout=ran.stdout.decode(errors='replace'),
                               stderr=ran.stderr.decode(errors='replace'))
                _dump(directory, 'runtime-' + mode + '.json', outcome)
                outcomes[mode] = outcome
        current()
        assert_snapshot_current(root, snapshot)
        assert Measurement.read(root).snapshot(root, {}) == policy
        assert {str(path): _file_hash(path) for path in own_inputs} == own_hashes
        assert _file_hash(storage) == storage_hash and _file_hash(receipt) == receipt_hash
        checked = verify_compile_files(receipt.read_bytes(), object_path=storage)
        _dump(directory, 'storage-after.json', checked)
        assert checked['result'] == 'pass'
        _dump(directory, 'application-live-after.json', dict(result='pass', current_guard_called=True,
                                                           compilations=len(live['compilations'])))
        report.update(status='complete', outcomes=outcomes, source_settings_unchanged=True,
                      storage_object=str(storage), storage_object_sha256=storage_hash,
                      storage_receipt_sha256=receipt_hash, generation=generation,
                      signed_cases_each=3072, signed_calls_guarded_product=6144,
                      limits=['Native bounded ABI/behavior control, not full-game acceptance.',
                              'Wrong-owner/member modes stop at first offense failure; armor-only mutant not claimed.',
                              'High64 pointer case remains host representation only.'])
        _dump(directory, 'report.json', report)
    _dump(directory, 'manifest.json', {path.name: _file_hash(path) for path in directory.iterdir()
                                     if path.is_file() and path.name != 'manifest.json'})
    return report

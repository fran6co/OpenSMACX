"""Whole-body host controls for the recovered Probe owner.

The compiled implementation is extracted only from src/probe.cpp, beginning
at its first recovered helper. The adjacent fixture supplies host-only typed
stubs; no ignored packet fragments or copied candidate function bodies are
loaded by these tests.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve().parent


def repository_root():
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/probe.cpp").is_file():
            return parent
    raise AssertionError("OpenSMACX source tree not found")


def probe_source_region():
    source = (repository_root() / "src/probe.cpp").read_text()
    begin = source.index(
        "static ProbeTechnologyScan probe_scan_technology_flags(")
    signature = "int __cdecl probe("
    function_start = source.index(signature, begin)
    opening = source.index("{", function_start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            # The host doubles deliberately replace target-specific headers;
            # preserve every actual implementation statement in the region.
            return re.sub(r'^\s*#include[^\n]*\n', '',
                          source[begin:end + 1], flags=re.MULTILINE)
    raise AssertionError("unterminated product probe body")


def compilers():
    found = []
    for name in ("g++", "clang++", "c++"):
        path = shutil.which(name)
        if path and path not in found:
            found.append(path)
    if not found:
        pytest.skip("GCC/Clang C++ compiler unavailable")
    return found


def stubs():
    return (HERE / "fixtures/probe_host_stubs.inc").read_text()


def compile_and_run(tmp_path, compiler, source, main, stem):
    unit = tmp_path / (stem + ".cpp")
    executable = tmp_path / stem
    unit.write_text(stubs() + "\n" + source + "\n" + main)
    subprocess.run([compiler, "-std=c++98", "-w", str(unit), "-o",
                    str(executable)], check=True, capture_output=True,
                   text=True, timeout=60)
    return subprocess.run([str(executable)], capture_output=True, timeout=30)


def instrument_roll_threshold_observer(source):
    """Observe the real helper argument; only inject a test-side log write."""
    start = source.index("static BOOL probe_native_success_roll_passes(")
    opening = source.index("{", start)
    observation = (
        "\n    if (ProbeHostRollThresholdCount < 8)\n"
        "        ProbeHostRollThresholds[ProbeHostRollThresholdCount] = "
        "roll_threshold;\n"
        "    ++ProbeHostRollThresholdCount;\n"
    )
    source = source[:opening + 1] + observation + source[opening + 1:]
    # The native slot has no eager initialization. A sentinel is injected only
    # in this test copy to make a no-store edge observable without depending on
    # host stack garbage or changing the product's initialization semantics.
    declaration = "int native_probe_roll_threshold;"
    assert declaration in source
    return source.replace(declaration,
                          "int native_probe_roll_threshold = 77;", 1)


def test_complete_product_helper_and_probe_region_compiles_cpp98(tmp_path):
    source = probe_source_region()
    assert "RadiusOffsetX[coordinate_index]" in source
    assert "RadiusOffsetY[coordinate_index]" in source
    assert "int __cdecl probe(" in source
    assert "return 1;" in source and "return 0;" in source
    labels = set(re.findall(r"^\s*label_([0-9a-fA-F]+):", source,
                            flags=re.MULTILINE))
    gotos = set(re.findall(r"\bgoto\s+label_([0-9a-fA-F]+)\s*;", source))
    assert gotos <= labels, "unresolved labels: %r" % sorted(gotos - labels)
    main = "int main() { return 0; }\n"
    for compiler in compilers():
        result = compile_and_run(tmp_path, compiler, source, main,
                                "probe-source-syntax")
        assert result.returncode == 0, result.stderr


def test_product_entry_warning_zero_exit_and_popup_scope_cpp98(tmp_path):
    source = probe_source_region()
    mutant = source.replace(
        "if (entry_route == 1) {\n"
        "        probe_native_entry_local_warning(actor_faction_id, mode);\n"
        "        goto label_5a2cf4;",
        "if (entry_route == 1) {\n"
        "        probe_native_entry_local_warning(actor_faction_id, mode);\n"
        "        goto label_5a5816;", 1)
    assert mutant != source
    main = r'''
        #include <cassert>
        int main() {
            Vehs[0].faction_id=1;
            Vehs[0].proto_id=0;
            Vehs[0].x=1; Vehs[0].y=1;
            Vehs[1].faction_id=1;
            Vehs[1].proto_id=0;
            Bases[0].faction_id_current=2;
            Bases[0].x=2; Bases[0].y=2;
            Chassis[0].triad=0;
            ProbeHostMaps[2*64+2].climate=0x60;
            const int constructs=ProbeHostPopupConstructs;
            const int destructs=ProbeHostPopupDestructs;
            assert(probe(0,0,1,0)==0);
            assert(ProbeTargetFactionID==2);
            assert(ProbeHostPopupConstructs==constructs+1);
            assert(ProbeHostPopupDestructs==destructs+1);
            const int constructs_after_warning=ProbeHostPopupConstructs;
            const int destructs_after_warning=ProbeHostPopupDestructs;
            assert(probe(0,-1,1,0)==1);
            assert(ProbeHostPopupConstructs==constructs_after_warning+1);
            assert(ProbeHostPopupDestructs==destructs_after_warning+1);
        }
    '''
    for compiler in compilers():
        baseline = compile_and_run(tmp_path, compiler, source, main,
                                  "probe-entry-baseline")
        mutated = compile_and_run(tmp_path, compiler, mutant, main,
                                  "probe-entry-mutant")
        assert baseline.returncode == 0, baseline.stderr
        assert mutated.returncode != 0


def test_product_selector_six_popup_and_accounting_cpp98(tmp_path):
    source = probe_source_region()
    main = r'''
        #include <cassert>
        int main() {
            Vehs[0].faction_id=1; Vehs[0].proto_id=0;
            Vehs[0].x=1; Vehs[0].y=1;
            Vehs[1].faction_id=2; Vehs[1].proto_id=0;
            Bases[0].faction_id_current=2;
            Bases[0].x=2; Bases[0].y=2;
            ProbeHostMaps[1*64+1].climate=0x60;
            ProbeHostMaps[2*64+2].climate=0x60;
            ConsoleGlobal.faction_id_=1;
            LockEnableMask=1u<<1;
            NetDaemonNet.channels_[0].leading_words_[3]=3;
            PlayersData[1].energy_reserves=100;
            PlayersData[2].energy_reserves=100;
            ProbeHostPopupExecResult=6;
            const int constructs=ProbeHostPopupConstructs;
            const int destructs=ProbeHostPopupDestructs;
            assert(probe(0,0,1,1)==1);
            assert(Vehs[0].probe_action==46);
            assert(PlayersData[1].energy_reserves==100);
            assert(PlayersData[1].mind_control_total==4);
            assert(PlayersData[2].diplo_mind_control[1]==4);
            assert(ProbeHostPopupExecCalls==2);
            assert(ProbeHostPopupConstructs==constructs+1);
            assert(ProbeHostPopupDestructs==destructs+1);
        }
    '''
    for compiler in compilers():
        result = compile_and_run(tmp_path, compiler, source, main,
                                "probe-selector-six")
        assert result.returncode == 0, result.stderr


@pytest.mark.parametrize(("path", "selector", "thought", "expected"), (
    ("local", 6, 1, 77),   # raw result 1 leaves prior EBP-0x30 value alone
    ("local", 6, 2, 1),    # raw result 2 writes one
    ("remote", 6, 0, 77),  # saved bit 3 clear leaves prior value alone
    ("remote", 14, 0, 1),  # saved bit 3 set writes one
))
def test_product_action_six_roll_threshold_rejoin_cpp98(
        tmp_path, path, selector, thought, expected):
    source = instrument_roll_threshold_observer(probe_source_region())
    if path == "local":
        console_faction = 1
        network_flags = 3
        setup_execs = (
            "ProbeHostPopupExecSequence[0] = %d;\n"
            "            ProbeHostPopupExecSequence[1] = %d;\n"
            "            ProbeHostPopupExecSequenceCount = 2;\n"
        ) % (selector, thought)
        vehicle_byte = 0
    else:
        console_faction = 2
        network_flags = 2
        setup_execs = ""
        vehicle_byte = selector
    main = r'''
        #include <cassert>
        int main() {
            Vehs[0].faction_id=1; Vehs[0].proto_id=0;
            Vehs[0].x=1; Vehs[0].y=1;
            Vehs[0].probe_action=%d;
            Vehs[1].faction_id=1; Vehs[1].proto_id=0;
            Bases[0].faction_id_current=2;
            Bases[0].x=2; Bases[0].y=2;
            ProbeHostMaps[1*64+1].climate=0x60;
            ProbeHostMaps[2*64+2].climate=0x60;
            ConsoleGlobal.faction_id_=%d;
            LockEnableMask=1u<<1;
            NetDaemonNet.channels_[0].leading_words_[3]=%d;
            PlayersData[1].energy_reserves=100;
            PlayersData[2].energy_reserves=100;
            %s
            probe(0,0,1,1);
            assert(ProbeHostRollThresholdCount > 0);
            assert(ProbeHostRollThresholds[0] == %d);
        }
    ''' % (vehicle_byte, console_faction, network_flags, setup_execs, expected)
    for compiler in compilers():
        result = compile_and_run(tmp_path, compiler, source, main,
                                "probe-action6-threshold-%s-%d-%d" %
                                (path, selector, thought))
        assert result.returncode == 0, result.stderr

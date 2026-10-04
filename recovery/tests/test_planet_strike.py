"""Actual-source callback/damage controls, not native/runtime acceptance."""
import shutil
import subprocess
import tempfile
from pathlib import Path

from recovery.tests.test_pact_ascent import definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/veh.cpp").is_file())
FIXTURE = Path(__file__).with_name("planet_strike_control.cpp").read_text()


def actual_bodies():
    source = (ROOT / "src/veh.cpp").read_text()
    helpers = "\n\n".join(definition(source, signature) for signature in (
        r"static\s+int\s+shoot_signed_bits\s*\(",
        r"static\s+int\s+shoot_wrapped_subtract\s*\(",
    ))
    return {
        "shoot": helpers + "\n\n" + definition(source, r"int\s+__cdecl\s+shoot\s*\("),
        "planet": definition(source, r"int\s+__cdecl\s+planet_busting\s*\("),
    }


def build(directory, bodies, compiler):
    source = FIXTURE.replace("// @@SHOOT_HELPERS_AND_BODY@@", bodies["shoot"], 1)
    source = source.replace("// @@PLANET_BUSTING_BODY@@", bodies["planet"], 1)
    assert "@@" not in source
    cpp, exe = directory / "actual.cpp", directory / "control"
    cpp.write_text(source)
    result = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                             "-pedantic", str(cpp), "-o", str(exe)],
                            capture_output=True, text=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


def run(exe, success):
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)
    if success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "compiled mutation escaped the source control"


def test_actual_strike_source_and_damage_controls():
    bodies = actual_bodies()
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers, "GCC or Clang is required for these host controls"
    mutations = (
        ("shoot", "int shots_left = shoot_wrapped_subtract(\n"
         "        shoot_signed_bits(PlayersData[def_factionID].satellites_[3]),",
         "int shots_left = shoot_wrapped_subtract(\n        deployed_at_start,"),
        ("shoot", "if (PlayersData[def_factionID].satellites_[3] == 0)",
         "if (deployed_at_start == 0)"),
        ("shoot", "if (bonus_hit)\n        goto report_hit;",
         "if (bonus_hit) {\n"
         "        --PlayersData[def_factionID].satellites_[3];\n"
         "        goto report_hit;\n    }"),
        ("planet", "PlayersData[faction].diplo_treaties[defender]",
         "PlayersData[defender].diplo_treaties[faction]"),
        ("planet", "const int territory_owner = whose_territory",
         "if (intercepted != 0) return intercepted;\n"
         "    const int territory_owner = whose_territory"),
        ("planet", "kill(veh_id);\n    draw_radius(x, y, 0, 2);",
         "draw_radius(x, y, 0, 2);\n    kill(veh_id);"),
    )
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="planet-strike-source-control-") as temp:
            directory = Path(temp)
            run(build(directory, bodies, compiler), True)
            for key, before, after in mutations:
                assert bodies[key].count(before) == 1, (key, before)
                damaged = dict(bodies)
                damaged[key] = damaged[key].replace(before, after, 1)
                run(build(directory, damaged, compiler), False)

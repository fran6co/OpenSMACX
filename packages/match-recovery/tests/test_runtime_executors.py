"""Native measurement executors retain explicitly selected runtime authority."""

from contextvars import ContextVar
import json
import multiprocessing
from pathlib import Path
import subprocess
import threading

import pytest

from match_recovery import framework_runtime as runtime
from test_claim_integrity import configuration
from test_framework_runtime import installed, write_binding
from test_recording import FIRST, SECOND, project, run
from test_variants import Compiler, measure, project as variant_project


@pytest.mark.parametrize("operation", ["record", "match", "measure"])
@pytest.mark.parametrize("selected", [False, True])
def test_measurement_pools_explicitly_supply_runtime_initializer(project, monkeypatch, operation, selected):
    from match_recovery import match_check, recording, source_measure

    (project / "engine/family.cc").write_text(FIRST)
    (project / "seeds/other.cc").write_text(SECOND)
    (project / "build").mkdir(exist_ok=True)
    (project / "build/commands.json").write_text(json.dumps([
        {"directory": str(project), "file": "engine/family.cc", "arguments": ["compiler", "engine/family.cc"]},
        {"directory": str(project), "file": "seeds/other.cc", "arguments": ["compiler", "seeds/other.cc"]},
    ]))
    configuration(project, lambda value: value.update(mappings={
        "compile_commands": "build/commands.json", "forwarder_macros": []}))
    binding = str(project / "build/runtime.json") if selected else None
    exported = []

    def export():
        exported.append(binding)
        return binding

    class PoolObserved(Exception):
        pass

    class Pool:
        def __init__(self, *, max_workers, initializer, initargs):
            assert max_workers == 2
            assert initializer is runtime.initialize_worker
            assert initargs == (binding,)

        def __enter__(self):
            return self

        def __exit__(self, *args):
            return False

        def map(self, function, work):
            assert len(work) == 2
            raise PoolObserved("Runtime was explicitly supplied before dispatch")

    module = {"record": recording, "match": match_check, "measure": source_measure}[operation]
    monkeypatch.setattr(runtime, "export_binding", export)
    monkeypatch.setattr(module, "ProcessPoolExecutor", Pool)
    with pytest.raises(PoolObserved):
        if operation == "record":
            run(project, jobs=2)
        elif operation == "match":
            match_check.check_matches(project, Path("original/input.exe"), jobs=2)
        else:
            source_measure.measure_source(project, ["_f", "_h"], image=Path("original/input.exe"), jobs=2)
    assert exported == [binding]


@pytest.mark.parametrize("method", multiprocessing.get_all_start_methods())
@pytest.mark.parametrize("selected", [False, True])
def test_real_installed_process_workers_activate_explicit_binding(tmp_path, installed, method, selected):
    path = write_binding(tmp_path, installed)
    python, _, _ = installed
    script = """
from concurrent.futures import ProcessPoolExecutor
from contextlib import nullcontext
from multiprocessing import get_context
from pathlib import Path
import json
import sys
from match_recovery import framework_runtime as runtime
with runtime.binding(Path(sys.argv[1])) if sys.argv[3] == 'yes' else nullcontext():
    expected = runtime.fingerprint()
    with ProcessPoolExecutor(max_workers=2, mp_context=get_context(sys.argv[2]),
                             initializer=runtime.initialize_worker,
                             initargs=(runtime.export_binding(),)) as pool:
        results = [future.result(timeout=30) for future in
                   [pool.submit(runtime.fingerprint), pool.submit(runtime.fingerprint)]]
    assert results == [expected, expected]
    print(json.dumps(results))
"""
    result = subprocess.run([str(python), "-I", "-B", "-c", script, str(path), method,
                             "yes" if selected else "no"],
                            capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    values = json.loads(result.stdout)
    assert values[0] == values[1]
    if selected:
        assert isinstance(values[0], str) and len(values[0]) == 64
    else:
        assert values == [None, None]


def test_variant_threads_use_distinct_context_copies(variant_project):
    marker = ContextVar("variant_control", default=None)
    barrier = threading.Barrier(2, timeout=10)
    seen = []

    class ContextCompiler(Compiler):
        def variant(self, source, contents, *args):
            if b"proposed" in contents or b"different" in contents:
                assert marker.get() == "coordinator context"
                marker.set(contents)
                barrier.wait()
                assert marker.get() == contents  # Simultaneous jobs cannot share mutable context.
                seen.append(threading.get_ident())
            return super().variant(source, contents, *args)

    token = marker.set("coordinator context")
    try:
        report = measure(variant_project, compiler=ContextCompiler(), jobs=2)
        assert marker.get() == "coordinator context"
    finally:
        marker.reset(token)
    assert report["status"] == "scored" and len(set(seen)) == 2


def test_variant_threads_preserve_active_runtime_binding(variant_project, tmp_path, installed):
    path = write_binding(tmp_path, installed)
    barrier = threading.Barrier(2, timeout=10)
    seen = []

    class RuntimeCompiler(Compiler):
        def variant(self, source, contents, *args):
            if b"proposed" in contents or b"different" in contents:
                barrier.wait()
                seen.append(runtime.fingerprint())
            return super().variant(source, contents, *args)

    with runtime.binding(path):
        expected = runtime.fingerprint()
        report = measure(variant_project, compiler=RuntimeCompiler(), jobs=2)
    assert report["status"] == "scored" and seen == [expected, expected]
    assert expected is not None and runtime.fingerprint() is None

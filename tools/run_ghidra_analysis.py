#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import shutil
import subprocess

from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "recovery" / "ghidra-functions.csv"
DEFAULT_REFERENCES_OUTPUT = (
    REPO_ROOT / "docs" / "recovery" / "ghidra-interior-references.csv")
DEFAULT_PROJECT_DIR = REPO_ROOT / "build" / "ghidra-projects"


SNAP_APP = "ghidra.analyzeHeadless"


def snap_shim_app(path):
    """Return the snap app name if `path` is a /snap/bin shim, else None.

    /snap/bin/ghidra.analyzeHeadless is a symlink to /usr/bin/snap.  Resolving
    it and executing the result runs snapd itself with Ghidra's arguments --
    silently, with no exception and no warning -- so a shim must never be
    resolved.  It is only meaningful as the three-word `snap run <app>`.
    """
    path = Path(path).expanduser()
    if not path.is_symlink() or path.name == "snap":
        return None
    if Path(os.path.realpath(path)).name != "snap":
        return None
    return path.name


def locate_snap_analyze_headless():
    """Return the snap argv prefix when the Ghidra snap is installed."""
    shim = Path("/snap/bin") / SNAP_APP
    if snap_shim_app(shim) and shutil.which("snap"):
        return ["snap", "run", SNAP_APP]
    return None


def locate_analyze_headless(value):
    """Return the argv PREFIX that launches analyzeHeadless, as a list.

    A list rather than a single Path because the working launcher is not
    always one file: a snap-installed Ghidra is only reachable through
    `snap run ghidra.analyzeHeadless`.  Callers must splice the result into
    their command list (`[*prefix, ...]`), never `str()` it.
    """
    if value:
        given = Path(value).expanduser()
        app = snap_shim_app(given)
        if app:
            return ["snap", "run", app]
        path = given.resolve()
        if path.is_file():
            return [str(path)]
        for candidate in (
                path / "support" / "analyzeHeadless",
                path / "support" / "analyzeHeadless.bat",
                path / "libexec" / "support" / "analyzeHeadless",
                path / "libexec" / "support" / "analyzeHeadless.bat"):
            if candidate.is_file():
                return [str(candidate)]
        raise RuntimeError(f"analyzeHeadless not found under {path}")

    executable = shutil.which("analyzeHeadless") or shutil.which("analyzeHeadless.bat")
    if executable:
        app = snap_shim_app(executable)
        return ["snap", "run", app] if app else [str(Path(executable))]
    snap = locate_snap_analyze_headless()
    if snap:
        return snap
    for candidate in (
            Path("/opt/homebrew/opt/ghidra/libexec/support/analyzeHeadless"),
            Path("/usr/local/opt/ghidra/libexec/support/analyzeHeadless")):
        if candidate.is_file():
            return [str(candidate)]
    raise RuntimeError("Ghidra analyzeHeadless was not found; pass --ghidra-home")


def configure_java(environment):
    """Point JAVA_HOME at a JDK for launchers that need one on the host.

    Deliberately knows nothing about the snap: measured on the snap Ghidra
    12.1, `snap run ghidra.analyzeHeadless` reports "openjdk version 21.0.11"
    and runs with JAVA_HOME unset AND with JAVA_HOME=/nonexistent/jdk, because
    the snap brings its own JVM and ignores the host variable.  A snap entry
    here would be code that can never change an outcome.
    """
    if environment.get("JAVA_HOME"):
        return
    for candidate in (
            Path("/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home"),
            Path("/usr/local/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home")):
        if candidate.is_dir():
            environment["JAVA_HOME"] = str(candidate)
            return


def main():
    parser = argparse.ArgumentParser(
        description="Run Ghidra headlessly and export function metadata")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--references-output", type=Path,
                        default=DEFAULT_REFERENCES_OUTPUT)
    parser.add_argument("--project-dir", type=Path, default=DEFAULT_PROJECT_DIR)
    parser.add_argument("--ghidra-home",
                        help="Ghidra root or path to analyzeHeadless")
    parser.add_argument("--max-cpu", type=int, default=max(1, os.cpu_count() or 1))
    args = parser.parse_args()

    exe = args.exe.expanduser().resolve()
    output = args.output.expanduser().resolve()
    references_output = args.references_output.expanduser().resolve()
    try:
        project_dir = require_local_artifact_path(
            args.project_dir, "Ghidra project directory")
    except RuntimeError as error:
        parser.error(str(error))
    if not exe.is_file():
        parser.error(f"executable not found: {exe}")
    project_dir.mkdir(parents=True, exist_ok=True)
    output.parent.mkdir(parents=True, exist_ok=True)
    references_output.parent.mkdir(parents=True, exist_ok=True)

    analyze_headless = locate_analyze_headless(args.ghidra_home)
    script_dir = Path(__file__).resolve().parent / "ghidra"
    environment = os.environ.copy()
    configure_java(environment)
    command = [
        *analyze_headless,
        str(project_dir),
        "opensmacx-recovery",
        "-import", str(exe),
        "-overwrite",
        "-max-cpu", str(args.max_cpu),
        "-scriptPath", str(script_dir),
        "-postScript", "ExportFunctions.java", str(output),
        "-postScript", "ExportInteriorReferences.java", str(references_output),
        "-deleteProject",
    ]
    subprocess.run(command, check=True, env=environment)


if __name__ == "__main__":
    main()

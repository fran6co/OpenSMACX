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


def locate_analyze_headless(value):
    if value:
        path = Path(value).expanduser().resolve()
        if path.is_file():
            return path
        for candidate in (
                path / "support" / "analyzeHeadless",
                path / "support" / "analyzeHeadless.bat",
                path / "libexec" / "support" / "analyzeHeadless",
                path / "libexec" / "support" / "analyzeHeadless.bat"):
            if candidate.is_file():
                return candidate
        raise RuntimeError(f"analyzeHeadless not found under {path}")

    executable = shutil.which("analyzeHeadless") or shutil.which("analyzeHeadless.bat")
    if executable:
        return Path(executable)
    for candidate in (
            Path("/opt/homebrew/opt/ghidra/libexec/support/analyzeHeadless"),
            Path("/usr/local/opt/ghidra/libexec/support/analyzeHeadless")):
        if candidate.is_file():
            return candidate
    raise RuntimeError("Ghidra analyzeHeadless was not found; pass --ghidra-home")


def configure_java(environment):
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
        str(analyze_headless),
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

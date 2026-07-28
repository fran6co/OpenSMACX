#!/usr/bin/env python3
"""Check whether this machine can run the recovery harness, and how well.

Run this FIRST on a new host, before building anything:

    python3 tools/host_doctor.py

It deliberately imports nothing outside the standard library. A fresh box has
no capstone and no pefile - detecting that is half the job, so importing them
would turn the diagnosis into a traceback.

THE HEADLINE CHECK is whether 32-bit x86 runs natively or through a translator.
Everything else here is a package you can install; that one is a property of the
machine, and it decides how much of this project's harness can be trusted:

  * Undefined flags. IDIV/DIV/MUL/IMUL leave some flags architecturally
    undefined. Real silicon is at least self-consistent. Under translation this
    project measured ONE instruction class giving THREE different answers
    (?fixed_div returns the entry flags, ?speed_proto clears SF even for a
    negative product, ?guard_check sets CF where a standalone probe leaves it
    clear), which is why tools/lifted_oracle_plan.py has to compute a per
    function `undef=<hex>` mask and exclude those flags from comparison. On a
    native host that mask should be droppable with no new FAILs, and that is
    the second acceptance check below.
  * The top 64 KiB. On the macOS/Rosetta host, guest 0xFFFF0000..0xFFFFFFFF is
    ordinary committed memory that VirtualQuery refuses, so it can be neither
    walled nor sealed, and small negative pointers land there. That forced a
    three-fill arbitration: re-run the original under three page fills and
    discard the case if its answer moves. On a 32-bit Linux address space the
    address is above TASK_SIZE and simply faults, and the arbitration becomes
    dead weight.

So `x86: native` is not a nice-to-have. It is the difference between excluding
flags from the comparison and comparing them.
"""
from __future__ import annotations

import hashlib
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"

# The build this project's numbers were measured against. A different image is
# not an error - it is a different game build - but every recorded address and
# every byte total belongs to this one, so it must be said out loud.
KNOWN_IMAGE_SHA256 = (
    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")

OK, WARN, BAD = "ok", "warn", "bad"


class Report:
    def __init__(self) -> None:
        self.rows: list[tuple[str, str, str, str]] = []

    def add(self, state: str, name: str, detail: str, fix: str = "") -> None:
        self.rows.append((state, name, detail, fix))

    def worst(self) -> str:
        states = {state for state, _, _, _ in self.rows}
        return BAD if BAD in states else (WARN if WARN in states else OK)

    def render(self) -> str:
        mark = {OK: "  ok  ", WARN: " warn ", BAD: " MISS "}
        width = max((len(name) for _, name, _, _ in self.rows), default=0)
        lines = []
        for state, name, detail, fix in self.rows:
            lines.append(f"[{mark[state]}] {name:<{width}}  {detail}")
            if fix and state != OK:
                lines.append(f"{'':<{width + 11}}  -> {fix}")
        return "\n".join(lines)


def run(argv: list[str], timeout: float = 20.0) -> tuple[int, str]:
    """Exit status and combined output, or (-1, reason) if it could not run."""
    try:
        done = subprocess.run(argv, capture_output=True, text=True,
                              timeout=timeout)
    except (OSError, subprocess.SubprocessError) as error:
        return -1, str(error)
    return done.returncode, (done.stdout or "") + (done.stderr or "")


def check_cpu(report: Report) -> bool:
    """Is 32-bit x86 native here, or is something translating it?

    Returns True only for a genuinely native x86 host. Being wrong in the
    optimistic direction would let someone believe a flag comparison that is
    actually measuring a translator, so every uncertain case reports WARN.
    """
    machine = platform.machine().lower()
    system = platform.system()
    native = machine in ("x86_64", "amd64", "i386", "i686")

    if system == "Darwin":
        # Rosetta 2 also lies about the machine when Python itself is
        # translated, so ask the kernel rather than the interpreter.
        status, out = run(["sysctl", "-n", "sysctl.proc_translated"])
        translated = status == 0 and out.strip() == "1"
        status, brand = run(["sysctl", "-n", "machdep.cpu.brand_string"])
        brand = brand.strip() or "unknown CPU"
        if machine == "arm64" or translated:
            report.add(
                WARN, "x86 execution", f"TRANSLATED - {brand}, arm64 host",
                "x86 here runs under Rosetta 2. Undefined flags are a "
                "translator's answer, so the oracle must keep its per-function "
                "undef= masks, and the top-64-KiB arbitration stays load "
                "bearing. Migrating to a native x86-64 host removes both.")
            return False
        report.add(OK, "x86 execution", f"native - {brand}")
        return True

    if system == "Linux":
        if not native:
            report.add(
                BAD, "x86 execution", f"{machine} - not an x86 host",
                "The harness is 32-bit x86 code. On a non-x86 Linux box it can "
                "only run under emulation, which is the thing this check "
                "exists to detect.")
            return False
        # A linux/amd64 container on an arm64 machine is the trap: uname says
        # x86_64 while binfmt_misc quietly routes every exec through QEMU or
        # Rosetta. That is translation wearing a native machine name.
        emulated = []
        binfmt = Path("/proc/sys/fs/binfmt_misc")
        for entry in ("qemu-x86_64", "qemu-i386", "rosetta"):
            handler = binfmt / entry
            try:
                if handler.exists() and "enabled" in handler.read_text():
                    emulated.append(entry)
            except OSError:
                pass
        if emulated:
            report.add(
                WARN, "x86 execution",
                f"{machine}, but binfmt_misc has {', '.join(emulated)} enabled",
                "This may be an x86 container on a non-x86 machine. Confirm "
                "with `lscpu` that the CPU is really x86 before trusting a "
                "flag comparison.")
            return False
        report.add(OK, "x86 execution", f"native - {machine} Linux")
        return True

    report.add(WARN, "x86 execution", f"{system}/{machine} - unrecognised host")
    return False


def check_tool(report: Report, name: str, candidates: list[str],
               version_argv: list[str], fix: str, required: bool = True) -> str:
    for candidate in candidates:
        found = shutil.which(candidate) or (
            candidate if Path(candidate).is_file() else None)
        if not found:
            continue
        status, out = run([found] + version_argv)
        first = out.strip().splitlines()[0] if out.strip() else "(no version)"
        report.add(OK, name, f"{found} - {first[:70]}")
        return found
    report.add(BAD if required else WARN, name, "not found", fix)
    return ""


def check_wine(report: Report) -> None:
    # wine_runtime.find_wine already knows every place wine hides - $WINE, the
    # PATH, and the macOS .app bundles - so use it rather than a second,
    # worse copy of that search. It is the only repo import here, and it is
    # stdlib-only itself, so a fresh box can still run this script.
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    try:
        import wine_runtime
        wine = wine_runtime.find_wine(None)
    except Exception:
        wine = shutil.which("wine") or shutil.which("wine64")
    if not wine:
        report.add(
            BAD, "wine", "not found",
            "Debian/Ubuntu: dpkg --add-architecture i386 && apt update && "
            "apt install wine wine32:i386. The harness is a 32-bit PE, so the "
            "i386 packages are the ones that matter.")
        return
    status, out = run([wine, "--version"])
    version = out.strip().splitlines()[0] if out.strip() else "unknown"
    report.add(OK, "wine", f"{wine} - {version}")

    # 32-bit support is the part that actually breaks: a wine64-only install
    # runs `wine --version` happily and then cannot load an i386 PE at all.
    prefix = os.environ.get("WINEPREFIX", "")
    detail = f"WINEPREFIX={prefix}" if prefix else "WINEPREFIX unset (~/.wine)"
    if prefix:
        report.add(OK, "wine prefix", detail)
    else:
        report.add(
            WARN, "wine prefix", detail,
            "The harness faults hundreds of times per sweep by design. Point "
            "it at an owned prefix (tools/owned_wine_prefix.py) so those "
            "faults never touch your personal ~/.wine.")


def check_python_deps(report: Report) -> None:
    """Report the interpreter that has the tools' dependencies, if any."""
    wanted = ("capstone", "pefile")
    probe = ("import importlib.util as u;"
             "print(','.join(n for n in %r if u.find_spec(n)))" % (wanted,))
    if shutil.which("uv"):
        report.add(
            OK, "uv", shutil.which("uv") or "uv",
            )
        report.add(
            OK, "python deps",
            "run tools via: uv run --with-requirements tools/requirements.txt")
        return
    status, out = run([sys.executable, "-c", probe])
    have = set(out.strip().split(",")) - {""}
    missing = [name for name in wanted if name not in have]
    if missing:
        report.add(
            BAD, "python deps", f"missing {', '.join(missing)}",
            "Install uv (https://astral.sh/uv) and run tools with "
            "`uv run --with-requirements tools/requirements.txt python3 ...`, "
            "or pip install -r tools/requirements.txt into a venv and pass it "
            "to cmake as -DOPENSMACX_PYTHON=<that python>.")
    else:
        report.add(OK, "python deps", f"{sys.executable} has {', '.join(wanted)}")


def check_image(report: Report, path: Path) -> None:
    if not path.is_file():
        report.add(
            BAD, "original image", f"{path} not found",
            "Copy your own terranx_original.exe here. It is deliberately NOT "
            "in git and never should be; the harness loads it at runtime.")
        return
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    size = path.stat().st_size
    if digest == KNOWN_IMAGE_SHA256:
        report.add(OK, "original image", f"{size:,} bytes, sha256 matches")
    else:
        report.add(
            WARN, "original image",
            f"{size:,} bytes, sha256 {digest[:16]}... differs",
            "Every address and byte total recorded in this repo was measured "
            f"against {KNOWN_IMAGE_SHA256[:16]}... A different build is not an "
            "error, but the numbers will not line up.")


def main() -> int:
    exe = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_EXE
    report = Report()

    native = check_cpu(report)
    check_tool(report, "mingw i686 g++",
               ["i686-w64-mingw32-g++",
                "/opt/homebrew/bin/i686-w64-mingw32-g++",
                "/usr/bin/i686-w64-mingw32-g++"],
               ["--version"],
               "Debian/Ubuntu: apt install g++-mingw-w64-i686. "
               "macOS: brew install mingw-w64. Override with $CXX.")
    check_wine(report)
    check_tool(report, "cmake", ["cmake"], ["--version"],
               "apt install cmake / brew install cmake")
    check_tool(report, "host c++", ["c++", "g++", "clang++"], ["--version"],
               "apt install build-essential")
    check_python_deps(report)
    check_image(report, exe)

    print(report.render())
    print()

    worst = report.worst()
    if worst == BAD:
        print("VERDICT: something required is missing - see the -> lines above.")
    elif native:
        print("VERDICT: ready, and x86 is NATIVE here.")
        print()
        print("Three acceptance checks decide whether the migration paid off.")
        print("Run them before trusting any number this host produces:")
        print()
        print("  1. The old PASSes must reproduce. Build and sweep, then")
        print("     compare against the report you brought with you:")
        print("       tools/lifted_oracle_build.sh && tools/lifted_oracle_sweep.sh")
        print("       python3 tools/lifted_oracle_summary.py build/oracle/report.tsv")
        print("     Same PASS set, or the move changed behaviour and you need")
        print("     to know why before going further.")
        print()
        print("  2. The undefined-flag masks should now be DROPPABLE. Rebuild")
        print("     the plan with the undef= token suppressed and re-sweep. On")
        print("     native silicon this should cost ZERO new FAILs. If it does")
        print("     not, the masks were hiding a real lowering bug, which is a")
        print("     finding worth more than the migration.")
        print()
        print("  3. A guest read at 0xFFFF0000 should FAULT rather than return")
        print("     data. If it faults, the three-fill top-page arbitration in")
        print("     lifted_oracle.cpp is now dead weight and can be retired.")
    else:
        print("VERDICT: usable, but x86 is not native here - see the warning.")
    return 1 if worst == BAD else 0


if __name__ == "__main__":
    raise SystemExit(main())

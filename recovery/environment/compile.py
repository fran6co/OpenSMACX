"""VC6 compilation shared by application and dependency verification."""

from pathlib import Path
import subprocess
import tempfile

from match_recovery.processes import run_bounded


FLAG_SETS = (("/c", "/O2", "/Gy", "/GR-", "/GX"),
             ("/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"))


def compile_object(source: Path, command: tuple[str, ...], flags: tuple[str, ...]) -> bytes:
    """Objects are disposable; a nonzero compiler exit never supplies evidence."""
    with tempfile.TemporaryDirectory(prefix="recover-vc6-") as temporary:
        work = Path(temporary)
        try:
            result = run_bounded([*command, *flags, "/Founit.obj", str(source.resolve())],
                                 cwd=work, timeout=300)
        except subprocess.TimeoutExpired as error:
            raise ValueError("VC6 compilation exceeded 300 seconds") from error
        if result.returncode != 0 or not (work / "unit.obj").is_file():
            raise ValueError("VC6 compilation failed: " +
                             (result.stdout + result.stderr).decode(errors="replace").strip())
        return (work / "unit.obj").read_bytes()

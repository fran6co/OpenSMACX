import os
import re


def write_regular(path, content, exclusive=False):
    flags = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    if exclusive:
        flags |= os.O_EXCL
    if hasattr(os, "O_NOFOLLOW"):
        flags |= os.O_NOFOLLOW
    descriptor = os.open(path, flags, 0o600)
    with os.fdopen(descriptor, "wb") as output:
        output.write(content)


def configure_intro_movie_skip(game_dir, token):
    ini_path = game_dir / "Alpha Centauri.ini"
    batch_path = game_dir / f".opensmacx-movie-skip-{token}.bat"
    if ini_path.is_symlink() or (ini_path.exists() and not ini_path.is_file()):
        raise RuntimeError(f"invalid game configuration path: {ini_path}")
    if batch_path.exists() or batch_path.is_symlink():
        raise RuntimeError(f"movie skip path already exists: {batch_path}")

    original = ini_path.read_bytes() if ini_path.exists() else None
    content = original or b""
    command = f"MoviePlayerCommand=cmd.exe /c .\\{batch_path.name}\r\n".encode("ascii")
    section = re.compile(rb"(?im)^\[PRACX\][^\r\n]*(?:\r?\n|$)")
    setting = re.compile(rb"(?im)^MoviePlayerCommand=[^\r\n]*(?:\r?\n|$)")
    section_match = section.search(content)
    if section_match:
        following_section = re.compile(rb"(?m)^\[").search(content, section_match.end())
        end = following_section.start() if following_section else len(content)
        section_body = content[section_match.end():end]
        if setting.search(section_body):
            section_body = setting.sub(command, section_body, count=1)
        else:
            section_body = command + section_body
        content = content[:section_match.end()] + section_body + content[end:]
    else:
        if content and not content.endswith((b"\n", b"\r")):
            content += b"\r\n"
        content += b"[PRACX]\r\n" + command
    try:
        write_regular(batch_path, b"@exit /b 0\r\n", exclusive=True)
        write_regular(ini_path, content)
    except OSError:
        if original is None:
            ini_path.unlink(missing_ok=True)
        else:
            try:
                write_regular(ini_path, original)
            except OSError:
                pass
        batch_path.unlink(missing_ok=True)
        raise
    return ini_path, original, batch_path


def restore_intro_movie_config(ini_path, original, batch_path):
    if original is None:
        ini_path.unlink(missing_ok=True)
    else:
        write_regular(ini_path, original)
    batch_path.unlink(missing_ok=True)

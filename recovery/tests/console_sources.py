"""Read Console definitions from their current compiled source owners."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
CONSOLE_OWNERS = (ROOT / "src/console.cpp", ROOT / "src/console_commands.cpp")


def console_method_source(signature: str) -> str:
    """Return the actual owner text containing one Console definition signature.

    This is a host-fixture lookup only. It does not combine source files or
    represent compiler translation-unit context.
    """
    matches = []
    for path in CONSOLE_OWNERS:
        text = path.read_text(encoding="utf-8")
        if signature in text:
            matches.append(text)
    if len(matches) != 1:
        raise AssertionError(
            f"expected {signature!r} in exactly one current Console owner; "
            f"found {len(matches)}"
        )
    return matches[0]

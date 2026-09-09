"""Read Microsoft COFF library members without extraction or linker selection.

Layout: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#archive-library-file-format
Linker indexes are not trusted to select a member. This reader enumerates the
actual records; an exact member identity must be unique. Thin/BSD archives and
non-Microsoft longname encodings are unsupported, not silently reinterpreted.
"""

from collections import defaultdict
import re


class CoffArchive:
    def __init__(self, data: bytes):
        if not data.startswith(b"!<arch>\n"):
            raise ValueError("expected a COFF archive signature")
        records, at, longnames = [], 8, None
        while at < len(data):
            header = data[at:at + 60]
            if len(header) != 60 or header[58:] != b"`\n":
                raise ValueError("truncated or invalid COFF archive member header")
            try:
                name = header[:16].decode("ascii").rstrip(" ")
                size_text = header[48:58].decode("ascii").rstrip(" ")
            except UnicodeDecodeError as error:
                raise ValueError("non-ASCII COFF archive member header") from error
            if not re.fullmatch(r"[0-9]+", size_text):
                raise ValueError("invalid COFF archive member size")
            size = int(size_text)
            start, end = at + 60, at + 60 + size
            if end > len(data):
                raise ValueError("truncated COFF archive member")
            body = data[start:end]
            if name == "//":
                if longnames is not None:
                    raise ValueError("duplicate COFF archive longnames table")
                longnames = body
            elif name != "/":
                records.append((name, body))
            at = end
            if at % 2:
                if data[at:at + 1] != b"\n":
                    raise ValueError("missing COFF archive alignment padding")
                at += 1
        names = {}
        if longnames is not None:
            start = 0
            while start < len(longnames):
                end = longnames.find(b"\0", start)
                if end < 0:
                    raise ValueError("unterminated COFF archive longname")
                try:
                    names[start] = longnames[start:end].decode("ascii")
                except UnicodeDecodeError as error:
                    raise ValueError("non-ASCII COFF archive longname") from error
                start = end + 1
        self.members = defaultdict(list)
        for name, body in records:
            if re.fullmatch(r"/[0-9]+", name):
                offset = int(name[1:])
                if offset not in names:
                    raise ValueError("COFF archive longname offset is not an entry boundary")
                name = names[offset]
            elif name.startswith("/") or not name.endswith("/") or name.startswith("#1/"):
                raise ValueError("unsupported COFF archive member name encoding")
            else:
                name = name[:-1]
            if not name or any(ord(char) < 32 or ord(char) == 127 for char in name):
                raise ValueError("invalid COFF archive member name")
            # Archive names are identities, never extraction paths. Canonical
            # forward slashes make Windows-produced libraries portable; aliases
            # remain multiple candidates and cannot silently pick a winner.
            name = name.replace("\\", "/")
            self.members[name].append(body)

    def member(self, name: str) -> bytes:
        candidates = self.members.get(name, ())
        if len(candidates) != 1:
            raise ValueError(f"expected one COFF archive member named {name!r}; found {len(candidates)}")
        return candidates[0]

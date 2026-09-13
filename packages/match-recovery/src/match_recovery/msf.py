"""Read the bounded SmallMSF 2.00/JG container, not PDB or build provenance.

Format authority: Microsoft's PDB/msf/msf.cpp (MSF_HDR, rgmsfparms,
internalSerializeSmallMsf, and load):
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/msf/msf.cpp
This is the old 16-bit page format, not the MSF 7.00 superblock format.
"""

import struct


MAGIC = b"Microsoft C/C++ program database 2.00\r\n\x1aJG\0\0"
MAX_BYTES = 128 * 1024 * 1024
MAX_STREAMS = 4096  # The original reader requires the count to be strictly smaller.
MAX_DIRECTORY = 4 + MAX_STREAMS * 8 + 65535 * 2
_PAGES = {1024: (8, 65535), 2048: (4, 65535), 4096: (1, 32767)}


def _unpack(fmt, data, offset):
    if offset < 0 or offset + struct.calcsize(fmt) > len(data):
        raise ValueError("truncated SmallMSF metadata")
    return struct.unpack_from(fmt, data, offset)


class SmallMsf:
    """Expose immutable streams only after validating the complete active map.

    Both FPM banks are reserved; only the header-selected bank supplies live
    allocation bits. Unallocated/free file slack and obsolete transaction bytes
    are not streams. Old persisted pointer fields and directory padding are
    ignored by the format reader, not interpreted as offsets or required zero.
    Absence and an existing empty stream remain distinct.
    """

    def __init__(self, data: bytes):
        if type(data) is not bytes or not 60 <= len(data) <= MAX_BYTES:
            raise ValueError("SmallMSF requires bounded immutable bytes")
        if data[:44] != MAGIC:
            raise ValueError("unsupported SmallMSF signature")
        page_size, fpm, page_count, directory_size, _ = _unpack("<IHHII", data, 44)
        if page_size not in _PAGES:
            raise ValueError("unsupported SmallMSF page size")
        fpm_pages, max_pages = _PAGES[page_size]
        first_data = 1 + 2 * fpm_pages
        if (fpm not in (1, 1 + fpm_pages) or not first_data < page_count <= max_pages
                or len(data) != page_count * page_size):
            raise ValueError("invalid SmallMSF page count, backing extent or free-page map")
        if not 12 <= directory_size <= MAX_DIRECTORY:
            raise ValueError("invalid SmallMSF directory size")
        directory_pages = (directory_size + page_size - 1) // page_size
        if 60 + directory_pages * 2 > page_size:
            raise ValueError("SmallMSF directory page list exceeds the header")
        active_map = data[fpm * page_size:(fpm + fpm_pages) * page_size]

        def free(page):
            return bool(active_map[page // 8] & (1 << (page % 8)))

        if any(free(page) for page in range(first_data)):
            raise ValueError("SmallMSF reserved page is marked free")
        owned = set()

        def claim(pages):
            for page in pages:
                if not first_data <= page < page_count:
                    raise ValueError("SmallMSF active page is reserved or outside its backing")
                if page in owned:
                    raise ValueError("SmallMSF active pages overlap")
                if free(page):
                    raise ValueError("SmallMSF active page is marked free")
                owned.add(page)

        directory_indices = _unpack("<" + "H" * directory_pages, data, 60)
        claim(directory_indices)
        directory = b"".join(data[p * page_size:(p + 1) * page_size]
                             for p in directory_indices)[:directory_size]
        count, _ = _unpack("<HH", directory, 0)
        if not 1 <= count < MAX_STREAMS:
            raise ValueError("unsupported SmallMSF stream count")
        if 4 + count * 8 > len(directory):
            raise ValueError("truncated SmallMSF stream descriptions")
        cursor = 4 + count * 8
        streams = []
        for index in range(count):
            size, _ = _unpack("<II", directory, 4 + index * 8)
            if size == 0xFFFFFFFF:
                streams.append(None)
                continue
            if size > MAX_BYTES:
                raise ValueError("SmallMSF stream exceeds its resource bound")
            length = (size + page_size - 1) // page_size
            if cursor + length * 2 > len(directory):
                raise ValueError("truncated SmallMSF stream page list")
            pages = _unpack("<" + "H" * length, directory, cursor)
            cursor += length * 2
            # Microsoft's load replaces serialized stream zero with the header
            # directory. Its former page list is obsolete transaction metadata;
            # consume its encoding but never read it or claim live ownership.
            if index:
                claim(pages)
            streams.append((size, pages))
        if cursor != len(directory):
            raise ValueError("SmallMSF directory has unconsumed metadata")
        streams[0] = (directory_size, directory_indices)
        self._data = data
        self._streams = tuple(streams)
        self.page_size = page_size
        self.stream_count = count

    def stream(self, index: int) -> bytes | None:
        if type(index) is not int or not 0 <= index < self.stream_count:
            raise ValueError("SmallMSF stream index is outside the directory")
        entry = self._streams[index]
        if entry is None:
            return None
        size, pages = entry
        return b"".join(self._data[p * self.page_size:(p + 1) * self.page_size]
                        for p in pages)[:size]

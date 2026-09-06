"""Conservative source include closures for impact and cache invalidation.

Quoted includes resolve in includer-directory then configured include-path order.
Both conditional branches are included. Macro/missing includes widen to every
known project header, never silently narrow the invalidation set. System headers
remain separately pinned toolchain inputs, not presumed project dependencies.
"""

from pathlib import PurePosixPath
import posixpath
import re
from typing import Mapping

from .components import relative_path
from .graph import DependencyGraph, Edge


INCLUDE = re.compile(r'^\s*#\s*include\b(.*)$', re.MULTILINE)
LITERAL = re.compile(r'^\s*([<"])([^>"\r\n]+)[>"]\s*(?://.*|/\*.*\*/\s*)?$')
COMMENTS_OR_STRINGS = re.compile(r'''"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|/\*.*?\*/|//[^\n]*''', re.DOTALL)


def include_graph(sources: Mapping[str, str], include_dirs: tuple[str, ...]) -> tuple[DependencyGraph, tuple[str, ...]]:
    for name in sources:
        relative_path(name)
    for directory in include_dirs:
        relative_path(directory)
    headers = {name for name in sources if PurePosixPath(name).suffix in (".h", ".hpp", ".hh", ".inc")}
    edges, widened = [], []
    for name, text in sorted(sources.items()):
        # Joining preprocessing continuations avoids missing a continued directive.
        text = text.replace("\\\r\n", "").replace("\\\n", "")
        text = COMMENTS_OR_STRINGS.sub(lambda match: ''.join('\n' if char == '\n' else ' ' for char in match[0])
                                      if match[0].startswith(('/*', '//')) else match[0], text)
        if 'R"' in text or '??=' in text:
            # Raw strings and trigraph directives need a full lexer/preprocessor.
            # This conservative v1 graph must not guess their directive boundaries.
            widened.append(name)
            edges.extend(Edge(name, header, "possibly_includes", f"source:{name}:unsupported_lexing")
                         for header in sorted(headers))
        for match in INCLUDE.finditer(text):
            token = LITERAL.fullmatch(match.group(1))
            target = None
            if token:
                quote, spelling = token.groups()
                search = ((str(PurePosixPath(name).parent),) if quote == '"' else ()) + include_dirs
                for directory in search:
                    option = posixpath.normpath(posixpath.join(directory, spelling.replace("\\", "/")))
                    if option in sources:
                        target = option
                        break
            evidence = f"source:{name}:include:{match.start()}"
            if target is not None:
                edges.append(Edge(name, target, "includes", evidence))
            else:
                widened.append(name)
                edges.extend(Edge(name, header, "possibly_includes", evidence) for header in sorted(headers))
    return DependencyGraph(edges), tuple(sorted(set(widened)))

"""Lossless path operands for this project's VC6 compiler/link command stages.

This describes syntax only. It never compares commands, grants permission,
resolves libraries or infers a path from a file's existence. In particular /Fd
describes the directory convention used by this project's application recipes.
"""

from pathlib import PurePosixPath
import re


def describe_command(arguments, cwd, *, stage):
    if stage not in {'compiler-effective', 'link-wrapper'}:
        raise ValueError('unsupported command observation stage')
    cwd = str(cwd)
    if (not cwd or '\0' in cwd or '\\' in cwd or cwd.startswith('//') or not PurePosixPath(cwd).is_absolute()
            or str(PurePosixPath(cwd)) != cwd or '..' in PurePosixPath(cwd).parts):
        raise ValueError('command working directory must be a normalized absolute host path')
    if (not isinstance(arguments, (list, tuple)) or not arguments
            or any(not isinstance(word, str) or not word or '\0' in word for word in arguments)):
        raise ValueError('command arguments must be a nonempty string sequence')
    codecs = [{'anchor': '/', 'prefix': '/', 'separator': '/'},
              {'anchor': '/', 'prefix': 'Z:\\', 'separator': '\\'},
              {'anchor': cwd, 'prefix': '', 'separator': '/'}]
    if stage == 'link-wrapper':
        # LINK 6's NB10 locator uses doubled backslashes. This separately
        # declared artifact spelling does not relax command-operand parsing.
        codecs.append({'anchor': '/', 'prefix': 'Z:\\\\', 'separator': '\\\\'})
    operands, unsupported = [], []

    def refuse(index, reason):
        unsupported.append(f'argument {index}: {reason}')

    def operand(index, start, kind):
        word = arguments[index]
        spelling = word[start:]
        if not spelling:
            refuse(index, 'missing path operand')
            return
        if spelling.startswith('Z:\\'):
            codec, relative = 1, spelling[3:]
            if '/' in relative:
                refuse(index, 'mixed compiler path separators')
                return
            relative = relative.replace('\\', '/')
        elif re.match(r'^[A-Za-z]:', spelling) or spelling.startswith('\\'):
            refuse(index, 'unsupported compiler drive or path encoding')
            return
        elif spelling.startswith('/'):
            codec, relative = 0, spelling[1:]
        else:
            codec, relative = 2, spelling
        if ('\\' in relative or '"' in relative or '@' in relative
                or relative and any(part in {'', '.', '..'} for part in relative.split('/'))):
            refuse(index, 'path requires unsupported quoting or lexical normalization')
            return
        path = PurePosixPath(codecs[codec]['anchor']) / relative
        operands.append({'argument': index, 'start': start, 'end': len(word),
                         'path': str(path), 'kind': kind, 'codec': codec})

    compiler_options = (('/FI', 'input_file'), ('/Fo', 'output_file'),
                        ('/Fd', 'output_directory'), ('/I', 'input_directory'))
    linker_options = (('/OUT:', 'output_file'), ('/IMPLIB:', 'output_file'),
                      ('/PDB:', 'output_file'), ('/MAP:', 'output_file'),
                      ('/LIBPATH:', 'input_directory'), ('/DEF:', 'input_file'))
    index = 0
    while index < len(arguments):
        word = arguments[index]
        if word.startswith('@'):
            refuse(index, 'unexpanded response file')
            index += 1
            continue
        option = '/' + word[1:] if word.startswith('-') else word
        comparison = option if stage == 'compiler-effective' else option.upper()
        grammar = compiler_options if stage == 'compiler-effective' else linker_options
        found = next(((prefix, kind) for prefix, kind in grammar if comparison.startswith(prefix)), None)
        if found is not None:
            prefix, kind = found
            if len(word) == len(prefix):
                # VC6 permits separated compile path options, not /OUT: <file>.
                if stage != 'compiler-effective' or index + 1 == len(arguments):
                    refuse(index, 'missing path operand')
                else:
                    index += 1
                    operand(index, 0, kind)
            else:
                operand(index, len(prefix), kind)
        elif stage == 'compiler-effective' and option.startswith(('/D', '/U')):
            # Macro literals are never rewritten, even when they contain paths.
            if option in {'/D', '/U'}:
                if index + 1 == len(arguments):
                    refuse(index, 'missing macro operand')
                else:
                    index += 1
        elif stage == 'compiler-effective' and option.startswith(('/Fa', '/Fe', '/Fi', '/Fp', '/Y', '/Tc', '/Tp')):
            refuse(index, 'unsupported compiler path option')
        elif stage == 'link-wrapper' and comparison.startswith(('/DEFAULTLIB:', '/NODEFAULTLIB:')):
            pass  # These are library requests, not host file identities.
        else:
            # Bare source/object paths have explicit roles; library basenames
            # remain ordered requests resolved by the linker search policy.
            suffix = PurePosixPath(word.replace('\\', '/')).suffix.lower()
            source = stage == 'compiler-effective' and suffix in {'.c', '.cc', '.cpp', '.cxx'}
            link_input = stage == 'link-wrapper' and (
                suffix in {'.obj', '.res', '.exp'} or suffix == '.lib' and ('/' in word or '\\' in word))
            is_option = word.startswith('-') or word.startswith('/') and (':' in word or '=' in word)
            if (source or link_input) and not is_option:
                operand(index, 0, 'source' if source else 'input_file')
            elif word.startswith(('/', '-')):
                if '/' in word[1:] or '\\' in word or '@' in word:
                    refuse(index, 'unsupported path-bearing option')
                # All remaining switches are exact opaque tokens.
            elif stage == 'link-wrapper' and suffix == '.lib' and not re.match(r'^[A-Za-z]:', word):
                pass
            else:
                refuse(index, 'unsupported bare input or path grammar')
        index += 1
    return {'schema_version': 1, 'stage': stage, 'codecs': codecs,
            'operands': operands, 'unsupported': unsupported}

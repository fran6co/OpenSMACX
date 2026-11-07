"""Causal attribution via temporary definition renames, never matching bytes.

Retain the original declaration and rename its definition in an atomic compiler
overlay. The exact ordinary symbol must lose its definition and the renamed
counterpart must appear with unchanged ABI decoration. An unchanged overlay is
the control. This avoids mistaking a wrong inline callee for the selected body.
"""

import base64
from dataclasses import replace
import hashlib
from pathlib import Path
import re
import secrets
import shlex

from .coff_preservation import compare_outside_functions
from .pe_coff import Coff
from .source import read_text
from .source.body import resolve_body, has_placeholder_body
from .source.mangled import identifier, qualified_name
from .source.reader import _is_placeholder_region
from .source_inventory import assert_snapshot_current


def compare_unit_with_attribution(records, image, path, arguments, flags, shared, *,
                                  root, compiler, fragment_pairs=(), targets=None):
    """Qualify byte-exact recording candidates without probing measured misses."""
    from .source.asm import compare_unit
    from .source.model import Tier
    from .source_inventory import source_snapshot
    records = tuple(records)
    captured = {}

    def compile_capture(source, command, trial):
        data = compiler(source, command, trial)
        captured[(Path(source).resolve(), tuple(command), shlex.join(trial))] = (data, tuple(trial))
        return data

    result = compare_unit(records, image, path, arguments, flags, shared,
                          compiler=compile_capture, root=root, fragment_pairs=fragment_pairs,
                          strict_command_for=lambda _path: arguments, targets=targets)
    if not any(record.body for record in records):
        return result
    attribution = BodyAttribution(root, source_snapshot(root))
    verified = []
    for record, score in result.results:
        if record.body and not isinstance(score, str) and score.verdict is Tier.BYTE_EXACT:
            try:
                entry = captured.get((Path(path).resolve(), tuple(arguments), score.flags))
                if entry is None:
                    raise ValueError('no unique ordinary compiler trial for source body attribution')
                raw, actual_flags = entry
                attribution.qualify(record, Coff(raw), unit=Path(path).relative_to(root).as_posix(),
                                    arguments=arguments, flags=actual_flags, compiler=compiler)
            except (OSError, ValueError) as error:
                score = 'declared source body attribution refused: ' + str(error)
        verified.append((record, score))
    attribution.assert_current()
    return replace(result, results=tuple(verified),
                   compile_attempts=result.compile_attempts + attribution.compile_attempts)


def _renamed_symbol(symbol, name, probe):
    """Replace a supported ordinary global name; preserve the exact ABI tail."""
    prefix = '?' + name + '@@'
    if symbol.startswith(prefix):
        return '?' + probe + '@@' + symbol[len(prefix):]
    match = re.fullmatch(r'([_@]?)' + re.escape(name) + r'(@\d+)?', symbol)
    if match:
        return match[1] + probe + (match[2] or '')
    raise ValueError('body attribution supports ordinary free-function symbols only')


def _definition_edit(text, candidate, name, probe):
    """Retain the declaration and rename only the actual definition head."""
    first, end, declaration, replacement = _definition_parts(text, candidate, name, probe)
    return first, end, declaration + replacement


def _definition_parts(text, candidate, name, probe):
    from .source_quality import _mask_non_code
    lines = text.splitlines(keepends=True)
    first = sum(map(len, lines[:candidate.line - 1]))
    end = sum(map(len, lines[:candidate.end_line]))
    code = _mask_non_code(text)[first:end]
    if code.lstrip().startswith('#'):
        raise ValueError('macro-contained definition heads are unsupported for attribution')
    matches = list(re.finditer(r'(?<![\w:])' + re.escape(name) + r'\s*\(', code))
    if not matches or '\n' in code[:matches[0].start()]:
        raise ValueError('definition head is not on the resolved starting line')
    match = matches[0]
    at, depth = match.end(), 1
    while at < len(code) and depth:
        if code[at] == '(':
            depth += 1
        elif code[at] == ')':
            depth -= 1
        at += 1
    if depth:
        raise ValueError('definition parameter list is unbounded')
    suffix = re.match(r'\s*(?:const\b\s*)?', code[at:])
    opening = at + suffix.end()
    if code[opening:opening + 1] != '{':
        raise ValueError('unsupported function definition head for attribution')
    declaration = text[first:first + opening].rstrip() + ';\n'
    replacement = (text[first:first + match.start()] + probe
                   + text[first + match.start() + len(name):end])
    return first, end, declaration, replacement



def _in_class_removal_edit(text, candidate, name):
    """Turn one in-class member definition into its own declaration.

    Members cannot be renamed without changing their class (and special
    members cannot be renamed at all), so the causal control removes the
    body: ``~Owner() { ... }`` becomes ``~Owner();``. The class keeps the
    declaration, so the emitted symbol must lose its definition.
    """
    from .source_quality import _mask_non_code
    member = name.partition('::')[2]
    lines = text.splitlines(keepends=True)
    first = sum(map(len, lines[:candidate.line - 1]))
    end = sum(map(len, lines[:candidate.end_line]))
    code = _mask_non_code(text)[first:end]
    if member.startswith('~'):
        head = r'(?<![\w:])~\s*' + re.escape(member[1:]) + r'\s*\('
    else:
        head = r'(?<![\w:~])' + re.escape(member) + r'\s*\('
    matches = list(re.finditer(head, code))
    if len(matches) != 1 or '\n' in code[:matches[0].start()]:
        raise ValueError('in-class definition head is not unique on its resolved line')
    at, depth = matches[0].end(), 1
    while at < len(code) and depth:
        depth += {'(': 1, ')': -1}.get(code[at], 0)
        at += 1
    if depth:
        raise ValueError('definition parameter list is unbounded')
    opening = at + re.match(r'\s*(?:const\b\s*)?', code[at:]).end()
    if code[opening:opening + 1] != '{':
        raise ValueError('unsupported in-class definition head for attribution')
    close, depth = opening + 1, 1
    while close < len(code) and depth:
        depth += {'{': 1, '}': -1}.get(code[close], 0)
        close += 1
    if depth:
        raise ValueError('in-class definition body is unbounded')
    return first, end, text[first:first + at] + code[at:opening].rstrip() + ';' + text[first + close:end]



def _member_control_edit(text, candidate, name):
    """The source edit whose compilation must drop the selected member.

    In-class definitions become their own declaration; an out-of-class
    destructor definition is deleted (its in-class declaration remains); an
    implicit destructor gains a user declaration before its class closes, so
    the compiler stops generating it. Returns the complete edited text.
    """
    from .source_quality import _mask_non_code
    if candidate.form == 'in_class':
        first, end, replacement = _in_class_removal_edit(text, candidate, name)
        return text[:first] + replacement + text[end:]
    lines = text.splitlines(keepends=True)
    first = sum(map(len, lines[:candidate.line - 1]))
    end = sum(map(len, lines[:candidate.end_line]))
    member = name.partition('::')[2]
    if candidate.form == 'implicit':
        closing = sum(map(len, lines[:candidate.end_line - 1]))
        if _mask_non_code(text)[closing:end].strip() != '};':
            raise ValueError('implicit member class does not close on its own line')
        return text[:closing] + ' public:\n  ' + member + '();\n' + text[closing:]
    if candidate.form == 'qualified' and _special_member(name):
        return text[:first] + text[end:]
    raise ValueError('compiler body attribution supports ordinary free functions, '
                     'in-class members, constructors, destructors and implicit destructors only')


def _special_member(name):
    """A constructor or destructor: it cannot be renamed, only removed."""
    owner, _, member = name.rpartition('::')
    owner = owner.rpartition('::')[2]
    return bool(owner) and member in (owner, '~' + owner)


def _inline_definition(text, candidate):
    """Whether the resolved definition head is declared ``inline``.

    VC6 emits an inline definition only on demand or through its retained
    declaration's attributes (for example ``__declspec(dllexport)``). Its
    renamed probe carries neither, so absence of the probe is expected.
    """
    from .source_quality import _mask_non_code
    lines = text.splitlines(keepends=True)
    first = sum(map(len, lines[:candidate.line - 1]))
    end = sum(map(len, lines[:candidate.end_line]))
    head = _mask_non_code(text)[first:end].partition('{')[0]
    return re.search(r'(?<![\w])(?:__forceinline|__inline|inline)(?![\w])', head) is not None


def _free_removal_edit(text, candidate, name):
    """Replace one free-function definition with its own declaration."""
    first, end, declaration, _ = _definition_parts(text, candidate, name, name)
    return text[:first] + declaration + text[end:]


class BodyAttribution:
    """Invocation-local controls; never replay saved attribution reports."""

    def __init__(self, root, snapshot):
        self.root = Path(root).resolve()
        self.snapshot = snapshot
        self.bytes = {name: base64.b64decode(info['content_base64'])
                      for name, info in snapshot['files'].items()}
        self.texts = {name: data.decode('utf-8', errors='replace')
                      for name, data in self.bytes.items()}
        self.symbol_format = snapshot['source_configuration'].get('symbol_format', 'none')
        self.rows, self.records, self.trials = {}, {}, {}
        self.compile_attempts = 0
        for row in snapshot['records']:
            if row.get('body_path'):
                self.rows.setdefault(row['entity_id'], []).append(row)

    def assert_current(self):
        assert_snapshot_current(self.root, self.snapshot)

    def has_reference(self, entity_id):
        return entity_id in self.rows

    def record(self, entity_id):
        rows = self.rows[entity_id]
        if len(rows) != 1:
            raise ValueError('declared source body has multiple mapping owners')
        row, = rows
        path = row['path']
        if path not in self.records:
            self.records[path] = {record.line: record for record in read_text(
                self.texts[path], self.root / path)}
        return self.records[path][row['line']]

    def _candidate(self, record):
        candidate = resolve_body(record, marker_path=record.path.relative_to(self.root).as_posix(),
                                 source_texts=self.texts, symbol_format=self.symbol_format)
        symbol = record.symbol or record.name
        if candidate.status == 'implicit':
            if self.symbol_format != 'msvc' or not symbol.startswith('??1'):
                raise ValueError('implicit declared body requires an MSVC complete destructor symbol')
            return candidate, qualified_name(symbol)
        if candidate.status != 'candidate':
            raise ValueError('declared source body unresolved: ' + candidate.reason)
        if _is_placeholder_region(candidate.body):
            raise ValueError('declared source body is an unfinished placeholder')
        symbol = record.symbol or record.name
        name = qualified_name(symbol) if self.symbol_format == 'msvc' else identifier(symbol)
        if name and '::' in name and (candidate.form == 'in_class' or _special_member(name)):
            return candidate, name
        if not name or not name.isidentifier():
            raise ValueError('compiler body attribution currently supports ordinary free functions '
                             'and in-class member definitions only')
        return candidate, name

    def is_placeholder(self, record):
        """A body pointer cannot erase a still-authored matching body slot."""
        return has_placeholder_body(record, marker_path=record.path.relative_to(self.root).as_posix(),
                                    source_texts=self.texts, symbol_format=self.symbol_format)

    def _trial(self, unit, obj, arguments, flags, compiler):
        overlay = getattr(compiler, 'overlay', None)
        if not callable(overlay):
            raise ValueError('configured compiler client has no source-overlay capability')
        edits, probes, originals = {}, {}, {}
        used_probes = set()
        for entity_id, rows in self.rows.items():
            if len(rows) != 1 or (rows[0].get('context_path') or rows[0]['path']) != unit:
                continue
            record = self.record(entity_id)
            try:
                candidate, name = self._candidate(record)
                if '::' in name:
                    continue
                symbol = record.symbol or record.name
                obj.function(symbol)
                probe = 'match_recovery_body_probe_' + secrets.token_hex(12)
                renamed = _renamed_symbol(symbol, name, probe)
                if (probe in used_probes or any(probe in text for text in self.texts.values())
                        or any(probe in item.name for item in obj.symbols.values())):
                    raise ValueError('probe collides with source or ordinary compiler output')
                text = self.bytes[candidate.path].decode('utf-8')
                edit = _definition_edit(text, candidate, name, probe)
            except (UnicodeError, ValueError):
                continue
            edits.setdefault(candidate.path, []).append(edit)
            used_probes.add(probe)
            originals[candidate.path] = self.bytes[candidate.path]
            probes[(record.path, record.line)] = (symbol, renamed, candidate)
        if not probes:
            raise ValueError('no supported uniquely emitted declared bodies to attribute')
        replacements = {}
        for path, spans in edits.items():
            ordered = sorted(spans)
            if any(left[1] > right[0] for left, right in zip(ordered, ordered[1:])):
                raise ValueError('declared definition probe ranges overlap')
            text = originals[path].decode('utf-8')
            for first, end, replacement in reversed(ordered):
                text = text[:first] + replacement + text[end:]
            replacements[path] = text.encode('utf-8')
        self._unchanged_control(unit, obj, originals, arguments, flags, overlay)
        self.compile_attempts += 1
        changed = Coff(overlay(self.root / unit, replacements, tuple(arguments), tuple(flags)))
        return changed, probes

    def _unchanged_control(self, unit, obj, originals, arguments, flags, overlay):
        self.compile_attempts += 1
        unchanged = Coff(overlay(self.root / unit, originals, tuple(arguments), tuple(flags)))
        if compare_outside_functions(obj, unchanged, {}, {})['result'] != 'pass':
            raise ValueError('unchanged source overlay differs from ordinary compiler output')

    def _removal_trial(self, record, candidate, name, unit, obj, arguments, flags, compiler):
        """Per-member control: removing its in-class body removes its definition.

        Run alone so that one member's removal cannot change whether another
        selected inline definition is emitted.
        """
        overlay = getattr(compiler, 'overlay', None)
        if not callable(overlay):
            raise ValueError('configured compiler client has no source-overlay capability')
        original = self.bytes[candidate.path]
        try:
            text = original.decode('utf-8')
        except UnicodeError as error:
            raise ValueError('declared body is not UTF-8 source') from error
        removed = _member_control_edit(text, candidate, name).encode('utf-8')
        self._unchanged_control(unit, obj, {candidate.path: original}, arguments, flags, overlay)
        self.compile_attempts += 1
        return Coff(overlay(self.root / unit, {candidate.path: removed}, tuple(arguments), tuple(flags)))

    def _free_removal_trial(self, candidate, name, unit, arguments, flags, compiler):
        overlay = getattr(compiler, 'overlay', None)
        if not callable(overlay):
            raise ValueError('configured compiler client has no source-overlay capability')
        original = self.bytes[candidate.path]
        try:
            text = original.decode('utf-8')
        except UnicodeError as error:
            raise ValueError('declared body is not UTF-8 source') from error
        removed = _free_removal_edit(text, candidate, name).encode('utf-8')
        self.compile_attempts += 1
        return Coff(overlay(self.root / unit, {candidate.path: removed}, tuple(arguments), tuple(flags)))

    def qualify(self, record, obj, *, unit, arguments, flags, compiler):
        if self.is_placeholder(record):
            raise ValueError('declared body or marker-owned definition is an unfinished placeholder')
        candidate, name = self._candidate(record)
        symbol = record.symbol or record.name
        obj.function(symbol)
        key = (unit, tuple(arguments), tuple(flags), hashlib.sha256(obj.raw_data).hexdigest())
        if '::' in name:
            changed = self._removal_trial(record, candidate, name, unit, obj, arguments, flags, compiler)
            control = {'in_class': 'in-class body removal', 'implicit': 'user destructor declaration',
                       'qualified': 'out-of-class definition removal'}[candidate.form]
            if any(item.name == symbol and item.section > 0 for item in changed.symbols.values()):
                raise ValueError(control + ' did not remove the selected function definition')
            return {'result': 'pass', 'path': candidate.path, 'line': candidate.line,
                    'end_line': candidate.end_line, 'symbol': symbol, 'unit': unit, 'form': candidate.form,
                    'evidence': 'unchanged-overlay equivalence and ' + control + '/disappearance',
                    'ordinary_object_sha256': key[-1],
                    'limitations': ['current configured compilation only; no original-TU, linked-selection or runtime proof']}
        if key not in self.trials:
            try:
                self.trials[key] = self._trial(unit, obj, arguments, flags, compiler)
            except (OSError, ValueError) as error:
                self.trials[key] = str(error)
        trial = self.trials[key]
        if isinstance(trial, str):
            raise ValueError(trial)
        changed, probes = trial
        probe = probes.get((record.path, record.line))
        if probe is None:
            raise ValueError('declared body has no supported nonoverlapping rename control')
        original, renamed, captured_candidate = probe
        if original != symbol or captured_candidate != candidate:
            raise ValueError('requested body differs from its captured source mapping')
        if any(item.name == original and item.section > 0 for item in changed.symbols.values()):
            raise ValueError('declared-body rename did not remove the selected function definition')
        evidence = 'unchanged-overlay equivalence and exact definition rename/disappearance'
        if not any(item.name == renamed and item.section > 0 for item in changed.symbols.values()):
            text = self.bytes[candidate.path].decode('utf-8')
            if not _inline_definition(text, candidate):
                changed.function(renamed)  # Refuses: the ordinary probe must be emitted.
            # The renamed inline probe lost the retained declaration's emission
            # attributes. Use the member-style control, alone: replacing the
            # definition by its declaration must drop the selected definition.
            removed = self._free_removal_trial(candidate, name, unit, arguments, flags, compiler)
            if any(item.name == original and item.section > 0 for item in removed.symbols.values()):
                raise ValueError('inline definition removal did not remove the selected function definition')
            evidence = 'unchanged-overlay equivalence and inline definition removal/disappearance'
        else:
            changed.function(renamed)
        return {'result': 'pass', 'path': candidate.path, 'line': candidate.line,
                'end_line': candidate.end_line, 'symbol': symbol, 'unit': unit,
                'evidence': evidence,
                'ordinary_object_sha256': key[-1],
                'limitations': ['current configured compilation only; no original-TU, linked-selection or runtime proof']}

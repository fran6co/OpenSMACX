"""Portable lexical controls; these do not establish compiler ownership."""

import pytest

from match_recovery.inline_body import damage_body


SYMBOL = "?current_text@StringList@@QAEPADXZ"


def test_in_class_probe_preserves_every_byte_outside_body():
    prefix = b"#pragma once\r\nclass StringList {\r\n public:\r\n  MEASURED char *current_text() "
    body = b"{ return head_ ? current_->payload->text : 0; }"
    suffix = b"\r\n  int field;\r\n};\r\n"
    assert damage_body(prefix + body + suffix, SYMBOL, "msvc") == prefix + b"{ for (;;) {} }" + suffix


def test_qualified_definition_and_nested_braces():
    raw = b"inline char *StringList::current_text() { if (head_) { return current_->text; } return 0; }\n"
    assert damage_body(raw, SYMBOL, "msvc") == b"inline char *StringList::current_text() { for (;;) {} }\n"


def test_masked_literals_and_comments_preserve_byte_offsets():
    prefix = b'// current_text() { } \xc3\xa9\n/* } */ class StringList { public: char *current_text() '
    body = b'''{ const char *s = "} \\\" {"; char c = '}'; /* { */ return 0; }'''
    suffix = b"; }; // untouched\n"
    assert damage_body(prefix + body + suffix, SYMBOL, "msvc") == prefix + b"{ for (;;) {} }" + suffix


@pytest.mark.parametrize("symbol,format", [("text", "none"), ("_text", "msvc"), ("_text@0", "msvc"), ("@text@0", "msvc"), ("?text@@YAPADXZ", "msvc")])
def test_supported_plain_and_decorated_identifiers(symbol, format):
    assert damage_body(b"char *text() { return 0; }", symbol, format) == b"char *text() { for (;;) {} }"


def test_plain_identifier_does_not_strip_leading_underscore():
    assert damage_body(b"int _text() { return 0; }", "_text", "none") == b"int _text() { for (;;) {} }"


def test_const_member_and_unrelated_function_are_untouched():
    raw = b"class X { int other() { return 1; } int text() const { return 2; } };"
    assert damage_body(raw, "text", "none") == b"class X { int other() { return 1; } int text() const { for (;;) {} } };"


def test_ordinary_calls_do_not_look_like_definitions():
    raw = b"int caller() { return text(); } int text() { return 0; }"
    assert damage_body(raw, "text", "none") == b"int caller() { return text(); } int text() { for (;;) {} }"


@pytest.mark.parametrize("raw", [
    b"", b" \n", b"int other() { return 0; }", b"int text();",
    b"int text() {} int text(int n) {}",
    b"class A { int text() {} }; class B { int text() {} };",
    b"int text(int n); int text() {}",
    b"int text(); int text() {}",  # Conservative: no signature inference.
    b"#if A\nint text() {}\n#else\nint text() {}\n#endif",
    b"#define text() { return 0; }\n",
    b"int text() {\n#if A\nreturn 0;\n#endif\n}",
    b"int text(\n#define X 1\n) {}",
    b"int\n#if A\ntext() {}\n#endif",
    b"int text() { /* unterminated }",
    b'int text() { const char *s = "unterminated; }',
    b"int text() { char c = 'x; }",
    b'int text() { const char *s = "line\nbreak"; }',
    b"int text() { return 0; \\\n}",
    b"int text() { return 0; \\\r\n}",
    b"int text() { return 0; \\\r}",
    b"int text() { return 0; } // ??/\n",
    b'int text() { const char *s = R"(})"; }',
    b"int text() <% return 0; %>",
    b"int text() { return (0; }",
    b"int text() { return 0;",
    b"int text() noexcept { return 0; }",
    b"auto text() -> int { return 0; }",
    b"template<class T> int text() { return 0; }",
    b"template<class T> int text(T v) {} int text() {}",
    b"template<> int text<int>() {} int text() {}",
    b"class text { text() {} };",
    b"int text() { for (;;) {} }",
])
def test_ambiguous_or_unsupported_source_is_refused(raw):
    with pytest.raises(ValueError):
        damage_body(raw, "text", "none")


@pytest.mark.parametrize("symbol,format", [
    ("", "none"), (" text", "none"), ("X::text", "none"),
    ("text@0", "none"), ("text", "itanium"),
    ("??0StringList@@QAE@XZ", "msvc"), ("??HStringList@@QAEHH@Z", "msvc"),
    ("?text@@garbage", "msvc"), ("?text@?$List@H@@QAEPADXZ", "msvc"),
    ("operator", "none"),
])
def test_unsupported_symbols_are_refused(symbol, format):
    with pytest.raises(ValueError):
        damage_body(b"int text() { return 0; }", symbol, format)

"""Pure controls for the source-spelling search: transforms, loop and distance."""

import pytest

from match_recovery import permute
from match_recovery.permute import Body, Score, Unsupported, mutation_sites, search, tokenize


def replacements(text, operator, parameters=""):
    body = Body(text, parameters)
    return [site.apply(text) for site in permute.OPERATORS[operator](body)]


def compact(text):
    return " ".join(token.text for token in tokenize(text) if token.kind not in ("ws", "comment"))


def compacted(text, operator, parameters=""):
    return [compact(item) for item in replacements(text, operator, parameters)]


# ------------------------------------------------------------ tokenizer

def test_tokenizer_keeps_comments_strings_and_offsets_lossless():
    text = 'a = "}{" + \'}\'; // } brace\n/* { */ b <<= 2;\n#define X 1\n'
    tokens = tokenize(text)
    assert "".join(token.text for token in tokens) == text
    assert [t.kind for t in tokens if t.kind not in ("ws",)][:6] == [
        "ident", "punct", "string", "punct", "char", "punct"]
    assert any(t.kind == "pp" and t.text == "#define X 1" for t in tokens)
    assert any(t.text == "<<=" for t in tokens)


def test_unsupported_syntax_is_refused_not_guessed():
    with pytest.raises(Unsupported):
        tokenize('x = R"(raw)";')
    with pytest.raises(Unsupported):
        Body("if (a) { b; ")
    with pytest.raises(Unsupported):
        Body("else b;")


# ------------------------------------------------------------ operators

def test_commute_swaps_only_binary_operands_and_respects_precedence():
    text = "x = a + b * c; y = *p & mask; z = -q;"
    out = compacted(text, "commute")
    assert "x = b * c + a ; y = * p & mask ; z = - q ;" in out
    assert "x = a + c * b ; y = * p & mask ; z = - q ;" in out
    assert "x = a + b * c ; y = mask & * p ; z = - q ;" in out
    assert len(out) == 3            # unary `*`/`-` are not binary sites


def test_commute_parenthesizes_a_left_associative_left_operand():
    assert compacted("x = a - b + c;", "commute") == ["x = c + ( a - b ) ;"]


def test_commute_includes_casts_and_skips_declaration_types():
    out = compacted("Foo *p = q; r = static_cast<int>(s) + t;", "commute")
    assert out == ["Foo * p = q ; r = t + static_cast < int > ( s ) ;"]


def test_commute_ignores_pointer_casts():
    assert compacted("x = (char *)&y;", "commute") == []


def test_flip_comparison():
    assert compacted("if (a < b + 1) { f(); }", "flip_comparison") == ["if ( b + 1 > a ) { f ( ) ; }"]
    assert compacted("while (i >= n) g();", "flip_comparison") == ["while ( n <= i ) g ( ) ;"]


def test_swap_adjacent_requires_independence():
    independent = "int a = p->x; int b = q->y;"
    assert compacted(independent, "swap_statements") == ["int b = q -> y ; int a = p -> x ;"]
    assert compacted("a = 1; b = a;", "swap_statements") == []          # data dependence
    assert compacted("f(); g();", "swap_statements") == []              # two calls
    assert compacted("p->x = 1; y = q->z;", "swap_statements") == []    # aliasing write
    assert compacted("x = 1; return x;", "swap_statements") == []       # jump is a barrier


def test_swap_adjacent_treats_parameters_and_locals_as_unaliased():
    assert compacted("int t = a + 1; f(b);", "swap_statements", "int a, int b") == [
        "f ( b ) ; int t = a + 1 ;"]
    assert compacted("int t = g_value; f(b);", "swap_statements", "int b") == []


def test_swap_preserves_comments_between_statements():
    text = "a = 1; // first\n    b = 2;"
    assert replacements(text, "swap_statements", "int a, int b") == ["b = 2; // first\n    a = 1;"]
    assert replacements(text, "swap_statements") == []        # globals may alias


def test_reorder_declarations_exchanges_non_adjacent_declarations():
    out = compacted("int a; x = 1; int b;", "reorder_declarations")
    assert out == ["int b ; x = 1 ; int a ;"]


def test_toggle_compound_both_directions():
    assert compacted("x += y * 2;", "toggle_compound") == ["x = x + y * 2 ;"]
    assert compacted("x -= a - b;", "toggle_compound") == ["x = x - ( a - b ) ;"]
    assert compacted("s.n = s.n + k;", "toggle_compound") == ["s . n += k ;"]
    assert compacted("x = x - a - b;", "toggle_compound") == []
    assert compacted("a[i++] += 1;", "toggle_compound") == []
    assert compacted("f().n += 1;", "toggle_compound") == []


def test_swap_if_branches_negates_and_unwraps():
    assert compacted("if (c) a(); else b();", "swap_if_branches") == [
        "if ( ! ( c ) ) { b ( ) ; } else { a ( ) ; }"]
    assert compacted("if (!x) { a(); } else { b(); }", "swap_if_branches") == [
        "if ( x ) { b ( ) ; } else { a ( ) ; }"]
    assert compacted("if (c) a();", "swap_if_branches") == []


def test_loop_forms_round_trip_and_refuse_continue():
    out = compacted("for (i = 0; i < n; ++i) { f(i); }", "loop_form")
    assert out == ["i = 0 ; while ( i < n ) { f ( i ) ; ++ i ; }"]
    back = compacted("i = 0; while (i < n) { f(i); ++i; }", "loop_form")
    assert "for ( i = 0 ; i < n ; ++ i ) { f ( i ) ; }" in back
    assert "i = 0 ; for ( ; i < n ; ) { f ( i ) ; ++ i ; }" in back
    assert compacted("for (i = 0; i < n; ++i) { if (a) continue; f(); }", "loop_form") == []
    assert compacted("for (;;) { g(); }", "loop_form") == ["while ( 1 ) { g ( ) ; }"]


def test_int_casts_add_and_remove():
    out = compacted("x = a + (int)b;", "int_cast")
    assert "x = a + b ;" in out
    assert "x = ( int ) a + ( int ) b ;" in out
    assert compacted("x = static_cast<int>(y);", "int_cast") == ["x = ( y ) ;"]
    assert all("( int ) 1" not in item for item in compacted("x = a + 1;", "int_cast"))


def test_ternary_forms_both_directions():
    assert compacted("x = c ? a : b;", "ternary") == ["if ( c ) { x = a ; } else { x = b ; }"]
    assert compacted("if (c) { x = a; } else { x = b; }", "ternary") == ["x = ( c ) ? a : b ;"]
    assert compacted("if (c) return 1; else return 2;", "ternary") == ["return ( c ) ? 1 : 2 ;"]
    assert compacted("return c ? 1 : 2;", "ternary") == [
        "if ( c ) { return 1 ; } else { return 2 ; }"]


def test_shift_multiply_power_of_two_only():
    assert compacted("y = x * 8 + 1;", "shift_multiply") == ["y = ( x << 3 ) + 1 ;"]
    assert compacted("y = x << 2;", "shift_multiply") == ["y = ( x * 4 ) ;"]
    assert compacted("y = x * 6;", "shift_multiply") == []


def test_temporaries_hoist_and_inline():
    hoisted = compacted("x = f(a) + b;", "temporary")
    assert "int tmp_p0 = f ( a ) ; x = tmp_p0 + b ;" in hoisted
    inline = compacted("int t = a * b; g(t);", "temporary")
    assert "g ( ( a * b ) ) ;" in inline
    assert compacted("int t = a; t = 2;", "temporary") == []           # written, not a value use
    assert all("tmp_p0" not in item for item in compacted("x = a && f(b);", "temporary"))


def test_mutations_stay_inside_their_body_text():
    text = "int v = a + b;\n    if (v < 3) { g(v); } else { h(); }\n"
    for site in mutation_sites(text):
        changed = site.apply(text)
        assert changed[:site.start] == text[:site.start]
        assert changed[len(changed) - (len(text) - site.end):] == text[site.end:]
        Body(changed)                   # every variant still parses


# ---------------------------------------------------------------- search

class FakeScorer:
    """Distance = how far `x = b + a;` is from the wanted spelling; exact at zero."""

    def __init__(self, wanted, fail_on=()):
        self.wanted, self.fail_on, self.calls = wanted, fail_on, []

    def __call__(self, text):
        self.calls.append(text)
        if any(word in text for word in self.fail_on):
            return None
        key = compact(text)
        distance = sum(1 for a, b in zip(key.split(), self.wanted.split()) if a != b) + abs(
            len(key.split()) - len(self.wanted.split()))
        return Score(distance, distance == 0, {"key": key})


def test_search_reaches_exact_and_stops():
    scorer = FakeScorer("x = b + a ; y = d * c ;")
    result = search("x = a + b; y = c * d;", scorer, iterations=200, seed=3)
    assert result.best.exact and result.stop_reason == "exact"
    assert compact(result.best_text) == "x = b + a ; y = d * c ;"
    assert result.compiles == len(scorer.calls)
    assert result.improvements and result.improvements[-1]["exact"]


def test_search_is_reproducible_for_a_seed():
    def run(seed):
        scorer = FakeScorer("never matches anything")
        result = search("x = a + b; y = c * d; z = e | f;", scorer, iterations=40, seed=seed)
        return scorer.calls, result.best_text
    assert run(7) == run(7)


def test_search_discards_variants_that_do_not_compile():
    scorer = FakeScorer("x = b + a ;", fail_on=("(int)",))
    result = search("x = a + b;", scorer, iterations=60, seed=1)
    assert "(int)" not in result.best_text
    assert result.best.exact


def test_search_caches_repeated_spellings_and_keeps_the_start_when_nothing_helps():
    scorer = FakeScorer("q ;")
    result = search("x = a + b;", scorer, iterations=50, seed=2, operators=["commute"])
    # Only two spellings exist: the start and its commuted form.
    assert result.compiles == 2 and result.cache_hits == 49
    assert result.best == result.start and compact(result.best_text) == "x = a + b ;"


def test_search_respects_time_limit_with_injected_clock():
    ticks = iter(range(1000))
    scorer = FakeScorer("never")
    result = search("x = a + b; y = c + d;", scorer, iterations=100, seed=0, time_limit=3,
                    clock=lambda: next(ticks))
    assert result.stop_reason == "time_limit" and result.iterations < 5


def test_keyboard_interrupt_keeps_the_best_so_far():
    class Interrupting(FakeScorer):
        def __call__(self, text):
            if len(self.calls) == 3:
                raise KeyboardInterrupt
            return super().__call__(text)
    scorer = Interrupting("x = b + a ; y = c * d ; z = 1 ;")
    improved = []
    result = search("x = a + b; y = c * d; z = 1;", scorer, iterations=100, seed=5,
                    on_improve=lambda text, score: improved.append(score.distance))
    assert result.interrupted and result.stop_reason == "interrupted"
    assert result.best.distance <= result.start.distance
    assert result.best_text  # a usable variant is returned


def test_search_refuses_an_uncompilable_start():
    with pytest.raises(ValueError, match="does not compile"):
        search("x = a + b;", lambda text: None, iterations=1)


def test_already_exact_start_runs_no_trials():
    calls = []
    result = search("x = a;", lambda text: calls.append(text) or Score(0, True), iterations=10)
    assert result.stop_reason == "already_exact" and len(calls) == 1


# -------------------------------------------------------------- distance

capstone = pytest.importorskip("capstone")


def test_listing_distance_counts_registers_order_and_masks_relocations():
    from match_recovery.source.asm import Listing
    from match_recovery.source_permute import listing_distance

    # mov eax,[0x1000]; add eax,ecx; ret
    original = Listing(bytes.fromhex("a100100000" "01c8" "c3"), 0x401000)
    relocated = Listing(bytes.fromhex("a100000000" "01c8" "c3"), 0x401000, mask=frozenset(range(1, 5)))
    assert listing_distance(original, relocated) == (0, 0)
    # add ecx,eax: same mnemonic, different register allocation
    swapped = Listing(bytes.fromhex("a100000000" "01c1" "c3"), 0x401000, mask=frozenset(range(1, 5)))
    assert listing_distance(original, swapped) == (2, 2)
    # reordered: one instruction missing on each side of the alignment
    reordered = Listing(bytes.fromhex("01c8" "a100000000" "c3"), 0x401000, mask=frozenset(range(3, 7)))
    instructions, _bytes = listing_distance(original, reordered)
    assert instructions == 2
    # wrong constant: same shape, bytes differ
    constant = Listing(bytes.fromhex("a100200000" "01c8" "c3"), 0x401000)
    assert listing_distance(original, constant) == (2, 1)


def test_split_and_join_declarations():
    assert compacted("int x = f(a); g(x);", "split_declaration") == ["int x ; x = f ( a ) ; g ( x ) ;"]
    assert compacted("int x; x = f(a); g(x);", "split_declaration") == ["int x = f ( a ) ; g ( x ) ;"]
    assert compacted("const int x = 1; int &r = y;", "split_declaration") == []
    assert compacted("int x; x = x + 1;", "split_declaration") == []


def test_minimize_reverts_hunks_the_score_does_not_need():
    from match_recovery.permute import SearchResult, _minimize

    original = "a = x + y;\nb = p * q;\n"
    drifted = "a = y + x;\nb = q * p;\n"

    def scorer(text):          # only the first line matters
        distance = 0 if text.startswith("a = y + x;") else 1
        return Score(distance, distance == 0)
    result = SearchResult(start=Score(1, False), best=Score(0, True), best_text=drifted)
    _minimize(original, result, scorer, {})
    assert result.best_text == "a = y + x;\nb = p * q;\n"
    assert result.best.exact and result.reverted_hunks == 1


def test_while_to_for_joins_only_the_loop_counter():
    assert all(not item.startswith("for ( k = 0")
               for item in compacted("k = 0; while (i < n) { f(i); ++i; }", "loop_form"))

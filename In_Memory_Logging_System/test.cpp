// Compile: g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

using Op     = vector<string>;
using Ops    = vector<Op>;
using Row    = vector<string>;
using Result = vector<Row>;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

static Result run(Ops ops) { return solution(ops); }

// ── Part 1 Example 1 ──────────────────────────────────────────────────────────
// upper + suffix; search is case-sensitive on the transformed message.
void test_p1_example1() {
    Result got = run({
        {"add_handler", "upper"},
        {"add_handler", "suffix", " DONE"},
        {"add_log", "hello world"},
        {"add_log", "mixed Case"},
        {"get_logs"},
        {"search", "HELLO"},
        {"search", "hello"},   // 'hello' ≠ 'HELLO'; case-sensitive miss
        {"search", "DONE"},
    });
    check("p1_ex1: get_logs",     got[0] == Row{"1:HELLO WORLD DONE","2:MIXED CASE DONE"});
    check("p1_ex1: search HELLO", got[1] == Row{"1:HELLO WORLD DONE"});
    check("p1_ex1: search hello", got[2] == Row{});
    check("p1_ex1: search DONE",  got[3] == Row{"1:HELLO WORLD DONE","2:MIXED CASE DONE"});
}

// ── Part 1 Example 2 ──────────────────────────────────────────────────────────
// prefix then suffix; pipeline order determines the final form.
void test_p1_example2() {
    Result got = run({
        {"add_handler", "prefix", "pre "},
        {"add_handler", "suffix", " post"},
        {"add_log", "core"},
        {"search", "pre"},
        {"search", "post"},
        {"get_logs"},
    });
    check("p1_ex2: search pre",  got[0] == Row{"1:pre core post"});
    check("p1_ex2: search post", got[1] == Row{"1:pre core post"});
    check("p1_ex2: get_logs",    got[2] == Row{"1:pre core post"});
}

// ── Part 2 Example 1 ──────────────────────────────────────────────────────────
// "hello hello world" → "HELLO HELLO WORLD"; HELLO is indexed once despite appearing twice.
void test_p2_example1() {
    Result got = run({
        {"add_handler", "upper"},
        {"add_log", "hello hello world"},
        {"add_log", "world"},
        {"search", "HELLO"},
        {"search", "WORLD"},
        {"search", "hello"},   // case-sensitive; not in index
        {"get_logs"},
    });
    check("p2_ex1: search HELLO", got[0] == Row{"1:HELLO HELLO WORLD"});
    check("p2_ex1: search WORLD", got[1] == Row{"1:HELLO HELLO WORLD","2:WORLD"});
    check("p2_ex1: search hello", got[2] == Row{});
    check("p2_ex1: get_logs",     got[3] == Row{"1:HELLO HELLO WORLD","2:WORLD"});
}

// ── Part 2 Example 2 ──────────────────────────────────────────────────────────
// Empty log stored as "1:"; empty keyword always returns [].
void test_p2_example2() {
    Result got = run({
        {"add_log", ""},
        {"search", "anything"},
        {"search", ""},
        {"get_logs"},
    });
    check("p2_ex2: search anything", got[0] == Row{});
    check("p2_ex2: search empty kw", got[1] == Row{});
    check("p2_ex2: get_logs",        got[2] == Row{"1:"});
}

// ── No handlers: message stored verbatim ──────────────────────────────────────
void test_no_handlers() {
    Result got = run({
        {"add_log", "hello world"},
        {"get_logs"},
        {"search", "hello"},
        {"search", "HELLO"},  // no upper handler → miss
    });
    check("no_handlers: get_logs",    got[0] == Row{"1:hello world"});
    check("no_handlers: lowercase",   got[1] == Row{"1:hello world"});
    check("no_handlers: uppercase",   got[2] == Row{});
}

// ── Handler order matters ─────────────────────────────────────────────────────
// [prefix("pre "), upper]: "core" → "pre core" → "PRE CORE"
// [upper, prefix("pre ")]: "core" → "CORE"     → "pre CORE"
void test_handler_order() {
    Result got1 = run({
        {"add_handler", "prefix", "pre "},
        {"add_handler", "upper"},
        {"add_log", "core"},
        {"get_logs"},
        {"search", "PRE"},
    });
    check("order_prefix_upper: get_logs",  got1[0] == Row{"1:PRE CORE"});
    check("order_prefix_upper: search PRE",got1[1] == Row{"1:PRE CORE"});

    Result got2 = run({
        {"add_handler", "upper"},
        {"add_handler", "prefix", "pre "},
        {"add_log", "core"},
        {"get_logs"},
        {"search", "pre"},
    });
    check("order_upper_prefix: get_logs",  got2[0] == Row{"1:pre CORE"});
    check("order_upper_prefix: search pre",got2[1] == Row{"1:pre CORE"});
}

// ── Empty logger: get_logs and search both return [] ─────────────────────────
void test_empty_logger() {
    Result got = run({
        {"get_logs"},
        {"search", "anything"},
    });
    check("empty: get_logs",  got[0].empty());
    check("empty: search",    got[1].empty());
}

// ── Word-boundary matching: substring must NOT match ──────────────────────────
// "HELL" should not match a log whose only word is "HELLO".
void test_word_boundary() {
    Result got = run({
        {"add_log", "HELLO"},
        {"search", "HELL"},    // substring; no match
        {"search", "HELLO"},   // exact word; matches
        {"search", "HELLOX"},  // longer; no match
    });
    check("boundary: substring",   got[0].empty());
    check("boundary: exact",       got[1] == Row{"1:HELLO"});
    check("boundary: longer",      got[2].empty());
}

// ── Suffix WITHOUT leading space creates no word boundary ─────────────────────
// suffix("END") on "msg" → "msgEND"; word is "msgEND", not "END" alone.
void test_suffix_no_space() {
    Result got = run({
        {"add_handler", "suffix", "END"},   // no space before END
        {"add_log", "msg"},
        {"search", "END"},       // 'END' is not a whitespace token in 'msgEND'
        {"search", "msgEND"},    // 'msgEND' is the exact token
        {"get_logs"},
    });
    check("suffix_no_space: END",    got[0].empty());
    check("suffix_no_space: msgEND", got[1] == Row{"1:msgEND"});
    check("suffix_no_space: get",    got[2] == Row{"1:msgEND"});
}

// ── Suffix WITH leading space creates a word boundary ────────────────────────
void test_suffix_with_space() {
    Result got = run({
        {"add_handler", "suffix", " END"},  // space before END
        {"add_log", "msg"},
        {"search", "END"},
        {"search", "msg"},
    });
    check("suffix_with_space: END", got[0] == Row{"1:msg END"});
    check("suffix_with_space: msg", got[1] == Row{"1:msg END"});
}

// ── Multiple logs; results in insertion order ─────────────────────────────────
void test_insertion_order() {
    Result got = run({
        {"add_log", "alpha beta"},
        {"add_log", "beta gamma"},
        {"add_log", "gamma alpha"},
        {"search", "beta"},
        {"search", "alpha"},
        {"search", "gamma"},
    });
    check("order: beta",  got[0] == Row{"1:alpha beta","2:beta gamma"});
    check("order: alpha", got[1] == Row{"1:alpha beta","3:gamma alpha"});
    check("order: gamma", got[2] == Row{"2:beta gamma","3:gamma alpha"});
}

// ── Repeated word in one log: appears once in search results ──────────────────
void test_dup_word_in_log() {
    Result got = run({
        {"add_log", "abc abc abc abc"},
        {"search", "abc"},
        {"get_logs"},
    });
    // Log should appear exactly once, not four times
    check("dup_word: count",    got[0] == Row{"1:abc abc abc abc"});
    check("dup_word: get_logs", got[1] == Row{"1:abc abc abc abc"});
}

// ── Same handler added twice (e.g., double prefix) ────────────────────────────
void test_double_handler() {
    Result got = run({
        {"add_handler", "prefix", "x"},
        {"add_handler", "prefix", "x"},
        {"add_log", "y"},
        {"get_logs"},         // "x" + "xy" = "xxy"
        {"search", "xxy"},
    });
    check("double_prefix: get_logs",   got[0] == Row{"1:xxy"});
    check("double_prefix: search xxy", got[1] == Row{"1:xxy"});
}

// ── Whitespace-only message has no indexable words ───────────────────────────
void test_whitespace_only_message() {
    Result got = run({
        {"add_log", "   "},    // only spaces; no words
        {"search", ""},        // empty keyword → always []
        {"search", "   "},     // "   " is not a word (stream extraction skips it)
        {"get_logs"},
    });
    check("whitespace: empty kw",     got[0].empty());
    check("whitespace: space kw",     got[1].empty());
    check("whitespace: get_logs",     got[2] == Row{"1:   "});
}

// ── Cross-validate: inverted index and linear scan produce identical output ───
void test_scan_vs_index_agreement() {
    Logger log;
    log.addLog("apple banana cherry");
    log.addLog("banana date");
    log.addLog("cherry elderberry cherry");  // repeated 'cherry': indexed once
    log.addLog("apple date fig");

    const vector<string> keywords = {
        "apple", "banana", "cherry", "date", "fig", "elderberry", "missing", ""
    };
    for (const auto& kw : keywords) {
        bool agree = (log.search(kw) == log.searchLinear(kw));
        check("scan_vs_index: '" + kw + "'", agree);
    }
}

// ── Linear scan also deduplicates per log (break after first match) ───────────
void test_linear_scan_dedup() {
    Logger log;
    log.addLog("foo foo foo");
    log.addLog("foo bar");

    auto r = log.searchLinear("foo");
    check("linear_dedup: count", r.size() == 2);
    check("linear_dedup: log1",  r[0] == "1:foo foo foo");
    check("linear_dedup: log2",  r[1] == "2:foo bar");
}

// ── Upper handler treats prefix text as part of the message ──────────────────
// [prefix("abc "), upper]: "xyz" → "abc xyz" → "ABC XYZ"
void test_upper_uppercases_prefix() {
    Result got = run({
        {"add_handler", "prefix", "abc "},
        {"add_handler", "upper"},
        {"add_log", "xyz"},
        {"get_logs"},
        {"search", "ABC"},
        {"search", "abc"},
    });
    check("upper_prefix: get_logs",  got[0] == Row{"1:ABC XYZ"});
    check("upper_prefix: search ABC",got[1] == Row{"1:ABC XYZ"});
    check("upper_prefix: search abc",got[2] == Row{});
}

// ── Large id numbering: ids stay sequential with many logs ───────────────────
void test_sequential_ids() {
    Ops ops;
    for (int i = 1; i <= 10; ++i)
        ops.push_back({"add_log", "word" + to_string(i)});
    ops.push_back({"get_logs"});
    Result got = run(ops);
    check("seq_ids: count", got[0].size() == 10);
    check("seq_ids: first", got[0][0] == "1:word1");
    check("seq_ids: last",  got[0][9] == "10:word10");
}

// ── Main ─────────────────────────────────────────────────────────────────────

int main() {
    cout << "── Platform examples ───────────────────────────────────\n";
    test_p1_example1();
    test_p1_example2();
    test_p2_example1();
    test_p2_example2();

    cout << "\n── Handler pipeline ────────────────────────────────────\n";
    test_no_handlers();
    test_handler_order();
    test_double_handler();
    test_upper_uppercases_prefix();

    cout << "\n── Search correctness ──────────────────────────────────\n";
    test_empty_logger();
    test_word_boundary();
    test_suffix_no_space();
    test_suffix_with_space();
    test_insertion_order();
    test_dup_word_in_log();
    test_whitespace_only_message();
    test_sequential_ids();

    cout << "\n── Cross-validation: scan vs index ─────────────────────\n";
    test_scan_vs_index_agreement();
    test_linear_scan_dedup();

    cout << "\nAll tests passed.\n";
    return 0;
}

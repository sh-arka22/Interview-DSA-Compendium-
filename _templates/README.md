---
tags:
  - template
  - json-parsing
  - reusable
---

# JSON Parser Template

A minimal recursive-descent JSON parser for coding-platform problems that hand you
a JSON string of records (e.g. `{"op":"put","key":"a","value":1}`).

Used by: [[Canonical_Cache_Key]], [[Persistent_LRU_Cache]].

## Files

| File | Purpose |
|---|---|
| `json_parser.hpp` | The `JsonParser` struct — copy into your `Solution`. |
| `demo.cpp` | 13 usage checks (`g++ -std=c++17 demo.cpp && ./demo`). |

## API

```cpp
JsonParser jp(inputJson);

string        v   = jp.parseValue();   // ANY value -> canonical string (object keys sorted)
map<str,str>  obj = jp.parseObject();  // {..} -> field(quoted) -> canonical value
vector<str>   arr = jp.parseArray();   // [..] -> canonical element strings

JsonParser::serialize(obj);            // map -> "{...}" with keys alphabetical
JsonParser::unquote("\"abc\"");        // -> "abc"
```

## Two rules it enforces

1. **Objects → keys sorted alphabetically** (canonical, so equal content hashes equal).
2. **Arrays → order preserved.**

`{"b":2,"a":1}` and `{"a":1,"b":2}` both become `{"a":1,"b":2}`.

## Gotchas

- **Tokens keep their quotes.** `parseValue` on `"abc"` returns `"abc"` *with* the quote
  chars. Look up object fields as `obj["\"key\""]`, and call `unquote()` for the raw value.
- **Re-parse nested strings.** `parseObject`/`parseArray` store nested values as *strings*.
  To walk into one, wrap it in a new parser: `JsonParser inner(obj["\"operations\""]);`.
- Handles whitespace (pretty-printed OK) and `\`-escapes inside strings.
- Assumes **valid JSON** — no error recovery (fine for these platforms).

## Typical top-level walk

```cpp
JsonParser jp(inputJson);
auto root = jp.parseObject();

int capacity = stoi(root["\"capacity\""]);

JsonParser ops(root["\"operations\""]);      // re-parse the nested array
for (auto& recStr : ops.parseArray()) {
    JsonParser rp(recStr);
    auto rec = rp.parseObject();
    string op  = JsonParser::unquote(rec["\"op\""]);
    string key = rec["\"key\""];             // keep canonical (may be an object!)
    // ... dispatch on op ...
}
```

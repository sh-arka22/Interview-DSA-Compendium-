// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static void checkVec(const string& name, vector<string> got, vector<string> exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: [";
        for (auto& s : got) cerr << s << " ";
        cerr << "]\n  exp: [";
        for (auto& s : exp) cerr << s << " ";
        cerr << "]\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

static void checkStr(const string& name, const string& got, const string& exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << got << "\n  exp: " << exp << "\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

// LeetCode 588 worked example: ls empty root, mkdir builds all middle dirs,
// addContentToFile creates the file, ls sees the new top dir, read returns content.
void test_example1() {
    FileSystem fs;
    checkVec("ex1 ls root empty", fs.ls("/"), {});
    fs.mkdir("/a/b/c");
    fs.addContentToFile("/a/b/c/d", "hello");
    checkVec("ex1 ls root after mkdir", fs.ls("/"), {"a"});
    checkStr("ex1 read file", fs.readContentFromFile("/a/b/c/d"), "hello");
}

// ls on a file path returns just that file's own name, not a directory listing.
void test_ls_on_file() {
    FileSystem fs;
    fs.addContentToFile("/x/y.txt", "abc");
    checkVec("ls on file path", fs.ls("/x/y.txt"), {"y.txt"});
}

// addContentToFile appends on repeat calls rather than overwriting.
void test_append_not_overwrite() {
    FileSystem fs;
    fs.addContentToFile("/f", "foo");
    fs.addContentToFile("/f", "bar");
    checkStr("append accumulates", fs.readContentFromFile("/f"), "foobar");
}

// ls returns files and directories together, sorted lexicographically.
void test_ls_sorted_mixed() {
    FileSystem fs;
    fs.mkdir("/root/zdir");
    fs.mkdir("/root/adir");
    fs.addContentToFile("/root/mfile.txt", "x");
    checkVec("ls sorted mixed dirs+files", fs.ls("/root"),
             {"adir", "mfile.txt", "zdir"});
}

// mkdir is idempotent / safe to call again on an existing prefix; middle dirs
// created once are reused, not duplicated, by later paths sharing that prefix.
void test_mkdir_shared_prefix() {
    FileSystem fs;
    fs.mkdir("/a/b");
    fs.mkdir("/a/c");
    checkVec("shared prefix reused", fs.ls("/a"), {"b", "c"});
}

// Deeply nested path with no prior mkdir: addContentToFile must create every
// missing middle directory itself.
void test_addcontent_creates_middles() {
    FileSystem fs;
    fs.addContentToFile("/p/q/r/s.txt", "deep");
    checkVec("middles auto-created", fs.ls("/p"), {"q"});
    checkVec("middles auto-created (nested)", fs.ls("/p/q"), {"r"});
    checkStr("deep file content", fs.readContentFromFile("/p/q/r/s.txt"), "deep");
}

int main() {
    test_example1();
    test_ls_on_file();
    test_append_not_overwrite();
    test_ls_sorted_mixed();
    test_mkdir_shared_prefix();
    test_addcontent_creates_middles();
    cout << "\nAll tests passed.\n";
}

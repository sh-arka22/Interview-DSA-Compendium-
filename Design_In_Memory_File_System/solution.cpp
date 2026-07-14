#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;

// Design: a TRIE OF HASH MAPS. Every path segment ("a", "b", "c", ...) is one hop
// down a Node whose children are keyed by name in an unordered_map — O(1) average
// lookup/insert per segment instead of the O(fan-out) linear scan a vector<Node*>
// would force. A single Node type doubles as both "directory" and "file" (isFile
// flag) so there is no separate File struct, no NULL branching between the two,
// and no code duplication in ls/mkdir/addContentToFile.
class FileSystem {
private:
    struct Node {
        bool isFile = false;
        string content;                          // meaningful only when isFile
        unordered_map<string, Node*> children;    // name -> child Node (dir or file)
    };

    Node* root;

    // "/a/b/c" -> ["a","b","c"];  "/" -> [].
    vector<string> parse(const string& path) {
        vector<string> parts;
        string cur;
        for (char c : path) {
            if (c == '/') { if (!cur.empty()) { parts.push_back(cur); cur.clear(); } }
            else cur += c;
        }
        if (!cur.empty()) parts.push_back(cur);
        return parts;
    }

    // Walk parts[0..n), creating any missing directory nodes along the way.
    Node* walkOrCreate(const vector<string>& parts, size_t n) {
        Node* cur = root;
        for (size_t i = 0; i < n; i++) {
            auto it = cur->children.find(parts[i]);
            if (it == cur->children.end())
                it = cur->children.emplace(parts[i], new Node()).first;
            cur = it->second;
        }
        return cur;
    }

    // Walk parts[0..n) of a path guaranteed (by problem constraints) to exist.
    Node* walk(const vector<string>& parts, size_t n) {
        Node* cur = root;
        for (size_t i = 0; i < n; i++) cur = cur->children[parts[i]];
        return cur;
    }

public:
    FileSystem() { root = new Node(); }

    vector<string> ls(string path) {
        vector<string> parts = parse(path);
        Node* node = walk(parts, parts.size());

        if (node->isFile) return { parts.back() };   // file path -> just its own name

        vector<string> out;
        out.reserve(node->children.size());
        for (auto& entry : node->children) out.push_back(entry.first);
        sort(out.begin(), out.end());                 // lexicographic order required by spec
        return out;
    }

    void mkdir(string path) {
        vector<string> parts = parse(path);
        walkOrCreate(parts, parts.size());
    }

    void addContentToFile(string filePath, string content) {
        vector<string> parts = parse(filePath);
        Node* dir = walkOrCreate(parts, parts.size() - 1);
        auto it = dir->children.find(parts.back());
        if (it == dir->children.end()) {
            Node* file = new Node();
            file->isFile = true;
            it = dir->children.emplace(parts.back(), file).first;
        }
        it->second->content += content;               // create-or-append in one path
    }

    string readContentFromFile(string filePath) {
        vector<string> parts = parse(filePath);
        Node* dir = walk(parts, parts.size() - 1);
        return dir->children[parts.back()]->content;
    }
};

#include <vector>
#include <list>
#include <unordered_map>
using namespace std;

class Solution {
public:
    vector<int> solution(int capacity, vector<vector<int>> operations) {
        // list: front = MRU, back = LRU; each element is {key, val}
        list<pair<int,int>> lru;
        // maps key → iterator into lru for O(1) access
        unordered_map<int, list<pair<int,int>>::iterator> cache;

        vector<int> results;

        for (auto& op : operations) {
            if (op[0] == 0) {
                // get(key)
                int key = op[1];
                auto it = cache.find(key);
                if (it == cache.end()) {
                    results.push_back(-1);
                } else {
                    // move to front (MRU); iterator remains valid after splice
                    lru.splice(lru.begin(), lru, it->second);
                    results.push_back(it->second->second);
                }
            } else {
                // put(key, val)
                int key = op[1], val = op[2];
                auto it = cache.find(key);
                if (it != cache.end()) {
                    it->second->second = val;
                    lru.splice(lru.begin(), lru, it->second);
                } else {
                    lru.push_front({key, val});
                    cache[key] = lru.begin();
                    if ((int)cache.size() > capacity) {
                        cache.erase(lru.back().first);
                        lru.pop_back();
                    }
                }
            }
        }

        return results;
    }
};

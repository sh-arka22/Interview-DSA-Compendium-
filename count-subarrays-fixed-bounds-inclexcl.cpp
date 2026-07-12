#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

long long countSubarrays(const std::vector<int>& arr, int minK, int maxK) {

    int n = arr.size();
    long long cntSubarrays = 0;

    // Helper: number of subarrays of arr[lo..hi] that contain NO occurrence
    // of forbidden value(s). Sum over maximal clean runs: run of length k
    // contributes k*(k+1)/2, accumulated incrementally as len per index.
    auto countAvoiding = [&](int lo, int hi, int f1, int f2) -> long long {
        long long total = 0, len = 0;
        for (int i = lo; i <= hi; i++) {
            if (arr[i] == f1 || arr[i] == f2) len = 0;
            else len += 1;
            total += len;
        }
        return total;
    };

    int i = 0;
    while (i < n) {
        // Skip "bad" elements — outside [minK, maxK]. No valid subarray crosses them.
        if (arr[i] < minK || arr[i] > maxK) { i++; continue; }

        // Maximal clean segment [i..j]: every element within [minK, maxK]
        int j = i;
        while (j + 1 < n && arr[j + 1] >= minK && arr[j + 1] <= maxK) j++;

        long long m = j - i + 1;
        long long total  = m * (m + 1) / 2;                       // all subarrays in segment
        long long noMin  = countAvoiding(i, j, minK, minK);       // missing minK
        long long noMax  = countAvoiding(i, j, maxK, maxK);       // missing maxK
        long long noBoth = countAvoiding(i, j, minK, maxK);       // missing both

        // Inclusion–exclusion: |has minK AND has maxK| = total - |no minK| - |no maxK| + |no both|
        cntSubarrays += total - noMin - noMax + noBoth;

        i = j + 1;
    }

    return cntSubarrays;
}

int main() {
    std::vector<int> e1 = {1, 3, 5, 2, 7, 5};
    std::vector<int> e2 = {1, 1, 1, 1};
    std::cout << countSubarrays(e1, 1, 5) << "\n";   // expected 2
    std::cout << countSubarrays(e2, 1, 1) << "\n";   // expected 10
    return 0;
}

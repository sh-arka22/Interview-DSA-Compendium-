#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

// ─── Interview-ready 30-min version ──────────────────────────────────────
// Strategy: minimal parser (just enough for Python-style input) + tight sim loop.
// In an interview, explain parsing briefly ("recursive descent for Python dicts")
// and spend your time on the simulation logic.

class Solution {
    // ─── Tiny parser for Python dicts/lists with single quotes ───────────
    string T; int P;
    void ws() { while (P < (int)T.size() && T[P] <= ' ') P++; }
    string str() { P++; string r; while (T[P]!='\'') r+=T[P++]; P++; return r; }
    double num() { int s=P; if(T[P]=='-')P++; while(P<(int)T.size()&&(T[P]>='0'&&T[P]<='9'||T[P]=='.'))P++; return stod(T.substr(s,P-s)); }
    void skip(char c) { ws(); if(P<(int)T.size()&&T[P]==c)P++; ws(); }

    // ─── Structs ─────────────────────────────────────────────────────────
    struct Atk { string name, type; double dmg; };
    struct Mon { string name, type; double hp; vector<Atk> atks; };

    // Parse one {...} into key/value pairs, fill struct fields
    Mon parseMon() {
        Mon m{}; skip('{');
        while (T[P] != '}') {
            ws(); string k = str(); skip(':');
            if      (k=="name")    m.name = str();
            else if (k=="type")    m.type = str();
            else if (k=="health")  m.hp   = num();
            else if (k=="attacks") {
                skip('[');
                while (T[P] != ']') {
                    Atk a{}; skip('{');
                    while (T[P] != '}') {
                        ws(); string ak = str(); skip(':');
                        if      (ak=="name")   a.name = str();
                        else if (ak=="type")   a.type = str();
                        else if (ak=="damage") a.dmg  = num();
                        skip(',');
                    } P++; skip(',');
                    m.atks.push_back(a);
                } P++;
            }
            skip(',');
        } P++; return m;
    }

    // ─── Format float like Python: 20.0, 3.5 ────────────────────────────
    string f(double v) {
        char b[64]; snprintf(b,64,"%g",v);
        string s=b; if(s.find('.')==string::npos) s+=".0"; return s;
    }

public:
    string solution(string teamA, string teamB, string typeChart) {
        // ── Parse teams ──────────────────────────────────────────────────
        auto parseTeam = [&](const string& s) -> pair<string,vector<Mon>> {
            T=s; P=0; string name; vector<Mon> v;
            skip('{');
            while (T[P] != '}') {
                ws(); string k = str(); skip(':');
                if (k=="name") name = str();
                else if (k=="monsters") {
                    skip('[');
                    while (T[P] != ']') { ws(); v.push_back(parseMon()); skip(','); }
                    P++;
                }
                skip(',');
            }
            return {name, v};
        };

        // ── Parse type chart: {('fire','grass'): 2.0, ...} ──────────────
        unordered_map<string,double> chart;
        {
            T=typeChart; P=0; skip('{');
            while (T[P] != '}') {
                skip('('); string t1=str(); skip(','); string t2=str(); skip(')');
                skip(':'); chart[t1+"|"+t2] = num(); skip(',');
            }
        }
        auto mult = [&](const string& a, const string& d) {
            auto it = chart.find(a+"|"+d);
            return it != chart.end() ? it->second : 1.0;
        };

        auto [nA, mA] = parseTeam(teamA); // name of teamA, and monters
        auto [nB, mB] = parseTeam(teamB);

        // ── Skip initially-dead monsters ─────────────────────────────────
        auto nxt = [](vector<Mon>& v, int i) {
            while (i < (int)v.size() && v[i].hp <= 0) i++;
            return i;
        };

        // ══════════════════════════════════════════════════════════════════
        // ── CORE SIMULATION (this is the part to focus on in interview) ──
        // ══════════════════════════════════════════════════════════════════
        int iA = nxt(mA,0), iB = nxt(mB,0);
        bool turnA = true;
        string log;

        while (iA < (int)mA.size() && iB < (int)mB.size()) {
            Mon& att = turnA ? mA[iA] : mB[iB];
            Mon& def = turnA ? mB[iB] : mA[iA];
            const string& aT = turnA ? nA : nB;   // attacker team name
            const string& dT = turnA ? nB : nA;   // defender team name

            // 1. Pick best attack (highest effective, earliest on tie)
            int bi = 0;
            double bd = att.atks[0].dmg * mult(att.atks[0].type, def.type);
            for (int i = 1; i < (int)att.atks.size(); i++) {
                double d = att.atks[i].dmg * mult(att.atks[i].type, def.type);
                if (d > bd) { bd = d; bi = i; }
            }

            // 2. Deal damage
            def.hp = max(0.0, def.hp - bd);

            // 3. Log attack
            if (!log.empty()) log += ", ";
            log += "{'event': 'attack', 'attacker_team': '"+aT+
                   "', 'attacker': '"+att.name+
                   "', 'defender_team': '"+dT+
                   "', 'defender': '"+def.name+
                   "', 'attack': '"+att.atks[bi].name+
                   "', 'damage': "+f(bd)+
                   ", 'defender_health': "+f(def.hp)+"}";

            // 4. Faint?
            if (def.hp <= 0) {
                log += ", {'event': 'faint', 'team': '"+dT+"', 'monster': '"+def.name+"'}";
                if (turnA) iB = nxt(mB, iB+1); else iA = nxt(mA, iA+1);
                turnA = true;     // ← KEY RULE: Team A always first in new matchup
            } else {
                turnA = !turnA;   // alternate
            }
        }

        string w = iA<(int)mA.size() ? "team_a" : iB<(int)mB.size() ? "team_b" : "draw";
        return "{'winner': '"+w+"', 'battle_log': ["+log+"]}";
    }
};

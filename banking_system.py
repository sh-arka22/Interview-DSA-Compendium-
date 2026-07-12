"""
Banking System Simulation (CodeSignal-style, Levels 1-4)
=========================================================
Time-ordered stream of operations -> one string result per query.

Design (first principles)
-------------------------
State:
  balance[acc]   -> int                current balance
  outgoing[acc]  -> int                lifetime outgoing (TRANSFER + PAY)
  payments[pid]  -> {owner, at, cb}    owner = CURRENT owner (survives merges)
  acc_payments[acc] -> list[pid]       payment ids owned by acc (for merge)
  cashback_heap  -> min-heap of (credit_time, pid)   pending cashbacks

Key idea: lazy settlement. Before EVERY query, pop all heap entries with
credit_time <= timestamp and credit floor(2%) to payments[pid]["owner"].
The heap stores payment ids (not account ids), so MERGE only has to
re-point payment ownership — pending heap entries follow automatically.

Complexity: n = #queries <= 1e4.
  Each op O(log n) amortized (heap), TOP_SPENDERS O(A log A),
  MERGE O(#payments of merged account). Total well within limits.
"""

import heapq
from typing import Dict, List

CASHBACK_DELAY_MS = 86_400_000  # 24h
CASHBACK_PCT = 2


class BankingSystem:
    def __init__(self) -> None:
        self.balance: Dict[str, int] = {}
        self.outgoing: Dict[str, int] = {}
        self.payments: Dict[str, dict] = {}          # pid -> {owner, at, cb}
        self.acc_payments: Dict[str, List[str]] = {} # acc -> [pid]
        self.cashback_heap: List[tuple] = []         # (credit_time, pid)
        self.pay_counter = 0

    # ---------- lazy cashback settlement (run before every op) ----------
    def _settle(self, ts: int) -> None:
        while self.cashback_heap and self.cashback_heap[0][0] <= ts:
            _, pid = heapq.heappop(self.cashback_heap)
            p = self.payments[pid]
            self.balance[p["owner"]] += p["cb"]
            p["done"] = True

    # ---------- Level 1 ----------
    def create_account(self, ts: int, acc: str) -> str:
        if acc in self.balance:
            return "false"
        self.balance[acc] = 0
        self.outgoing[acc] = 0
        self.acc_payments[acc] = []
        return "true"

    def deposit(self, ts: int, acc: str, amount: int) -> str:
        if acc not in self.balance:
            return ""
        self.balance[acc] += amount
        return str(self.balance[acc])

    def get_balance(self, ts: int, acc: str) -> str:
        if acc not in self.balance:
            return ""
        return str(self.balance[acc])

    # ---------- Level 2 ----------
    def transfer(self, ts: int, src: str, dst: str, amount: int) -> str:
        if (src not in self.balance or dst not in self.balance
                or src == dst or self.balance[src] < amount):
            return ""
        self.balance[src] -= amount
        self.balance[dst] += amount
        self.outgoing[src] += amount
        return str(self.balance[src])

    def top_spenders(self, ts: int, n: int) -> str:
        ranked = sorted(self.outgoing.items(), key=lambda kv: (-kv[1], kv[0]))
        return ", ".join(f"{acc}({total})" for acc, total in ranked[:n])

    # ---------- Level 3 ----------
    def pay(self, ts: int, acc: str, amount: int) -> str:
        if acc not in self.balance or self.balance[acc] < amount:
            return ""
        self.balance[acc] -= amount
        self.outgoing[acc] += amount
        self.pay_counter += 1
        pid = f"payment{self.pay_counter}"
        self.payments[pid] = {
            "owner": acc,
            "at": ts,
            "cb": amount * CASHBACK_PCT // 100,
            "done": False,
        }
        self.acc_payments[acc].append(pid)
        heapq.heappush(self.cashback_heap, (ts + CASHBACK_DELAY_MS, pid))
        return pid

    def get_payment_status(self, ts: int, acc: str, pid: str) -> str:
        if acc not in self.balance:
            return ""
        p = self.payments.get(pid)
        if p is None or p["owner"] != acc:
            return ""
        return "CASHBACK_RECEIVED" if p["done"] else "IN_PROGRESS"

    # ---------- Level 4 ----------
    def merge_accounts(self, ts: int, a1: str, a2: str) -> str:
        if a1 == a2 or a1 not in self.balance or a2 not in self.balance:
            return "false"
        self.balance[a1] += self.balance[a2]
        self.outgoing[a1] += self.outgoing[a2]
        # Redirect payment ownership -> pending cashbacks & status follow.
        for pid in self.acc_payments[a2]:
            self.payments[pid]["owner"] = a1
        self.acc_payments[a1].extend(self.acc_payments[a2])
        del self.balance[a2], self.outgoing[a2], self.acc_payments[a2]
        return "true"


def solution(queries: List[List[str]]) -> List[str]:
    bank = BankingSystem()
    ops = {
        "CREATE_ACCOUNT":     lambda ts, a: bank.create_account(ts, a),
        "DEPOSIT":            lambda ts, a, amt: bank.deposit(ts, a, int(amt)),
        "GET_BALANCE":        lambda ts, a: bank.get_balance(ts, a),
        "TRANSFER":           lambda ts, s, d, amt: bank.transfer(ts, s, d, int(amt)),
        "TOP_SPENDERS":       lambda ts, n: bank.top_spenders(ts, int(n)),
        "PAY":                lambda ts, a, amt: bank.pay(ts, a, int(amt)),
        "GET_PAYMENT_STATUS": lambda ts, a, p: bank.get_payment_status(ts, a, p),
        "MERGE_ACCOUNTS":     lambda ts, a1, a2: bank.merge_accounts(ts, a1, a2),
    }
    results = []
    for op, ts_str, *args in queries:
        ts = int(ts_str)
        bank._settle(ts)  # apply all cashbacks due at/before ts
        results.append(ops[op](ts, *args))
    return results


# ---------------------------- tests ----------------------------
if __name__ == "__main__":
    # Provided example
    q = [
        ["CREATE_ACCOUNT", "1", "alice"],
        ["CREATE_ACCOUNT", "2", "bob"],
        ["DEPOSIT", "3", "alice", "2000"],
        ["TRANSFER", "4", "alice", "bob", "500"],
        ["TOP_SPENDERS", "5", "2"],
        ["PAY", "6", "alice", "1000"],
        ["GET_PAYMENT_STATUS", "7", "alice", "payment1"],
        ["GET_PAYMENT_STATUS", "86400007", "alice", "payment1"],
        ["GET_BALANCE", "86400008", "alice"],
    ]
    expected = ["true", "true", "2000", "1500", "alice(500), bob(0)",
                "payment1", "IN_PROGRESS", "CASHBACK_RECEIVED", "520"]
    got = solution(q)
    assert got == expected, f"\ngot:      {got}\nexpected: {expected}"

    # Merge: pending cashback redirected, status queryable via new owner
    q2 = [
        ["CREATE_ACCOUNT", "1", "a"],
        ["CREATE_ACCOUNT", "2", "b"],
        ["DEPOSIT", "3", "b", "1000"],
        ["PAY", "4", "b", "500"],                       # payment1, cb 10 at t=86400004
        ["MERGE_ACCOUNTS", "5", "a", "b"],              # b folded into a
        ["GET_BALANCE", "6", "b"],                      # "" (gone)
        ["GET_PAYMENT_STATUS", "7", "a", "payment1"],   # IN_PROGRESS via a
        ["GET_BALANCE", "86400004", "a"],               # 500 + 10 cashback
        ["GET_PAYMENT_STATUS", "86400005", "a", "payment1"],
        ["TOP_SPENDERS", "86400006", "5"],              # a inherits b's outgoing
    ]
    expected2 = ["true", "true", "1000", "payment1", "true", "",
                 "IN_PROGRESS", "510", "CASHBACK_RECEIVED", "a(500)"]
    got2 = solution(q2)
    assert got2 == expected2, f"\ngot:      {got2}\nexpected: {expected2}"

    # Invalid ops: self-transfer, insufficient funds, missing accounts
    q3 = [
        ["CREATE_ACCOUNT", "1", "x"],
        ["CREATE_ACCOUNT", "2", "x"],           # false (duplicate)
        ["DEPOSIT", "3", "ghost", "100"],       # ""
        ["TRANSFER", "4", "x", "x", "0"],       # "" (self)
        ["TRANSFER", "5", "x", "ghost", "10"],  # "" (missing target)
        ["PAY", "6", "x", "1"],                 # "" (insufficient)
        ["MERGE_ACCOUNTS", "7", "x", "x"],      # false
        ["GET_PAYMENT_STATUS", "8", "x", "payment1"],  # "" (no such payment)
    ]
    expected3 = ["true", "false", "", "", "", "", "false", ""]
    got3 = solution(q3)
    assert got3 == expected3, f"\ngot:      {got3}\nexpected: {expected3}"

    print("All tests passed.")

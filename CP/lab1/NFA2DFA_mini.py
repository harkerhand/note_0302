from collections import defaultdict
import itertools
import networkx as nx
import matplotlib.pyplot as plt
import json
import os

from draw_merged_NFA import NFAs

# ======== 合并 NFA ========
merged_states = set()
merged_trans = defaultdict(set)
merged_accept = set()
merged_start = "START"
symbols = set()

for name, nfa in NFAs.items():
    prefix = name + "_"
    # rename states
    states = {prefix + s for s in nfa["states"]}
    start = prefix + nfa["start"]
    accepts = {prefix + s for s in nfa["accept"]}
    merged_states |= states
    merged_accept |= accepts
    merged_trans[merged_start].add(("ε", start))
    for src, sym, dst in nfa["trans"]:
        merged_trans[prefix + src].add((sym, prefix + dst))
        symbols.add(sym)

# ======== ε-闭包 ========
def epsilon_closure(states):
    stack = list(states)
    closure = set(states)
    while stack:
        s = stack.pop()
        for sym, dst in merged_trans[s]:
            if sym == "ε" and dst not in closure:
                closure.add(dst)
                stack.append(dst)
    return closure

# ======== Move 函数 ========
def move(states, symbol):
    result = set()
    for s in states:
        for sym, dst in merged_trans[s]:
            if sym == symbol:
                result.add(dst)
    return result

# ======== NFA → DFA 子集构造法 ========
start_closure = frozenset(epsilon_closure({merged_start}))
unmarked = [start_closure]
dfa_states = [start_closure]
dfa_trans = {}
dfa_accept = set()

while unmarked:
    T = unmarked.pop()
    for sym in symbols:
        if sym == "ε":  # 忽略 ε
            continue
        U = epsilon_closure(move(T, sym))
        if not U:
            continue
        U_frozen = frozenset(U)
        dfa_trans[(T, sym)] = U_frozen
        if U_frozen not in dfa_states:
            dfa_states.append(U_frozen)
            unmarked.append(U_frozen)

for sset in dfa_states:
    if any(x in merged_accept for x in sset):
        dfa_accept.add(sset)

# ======== DFA 最小化（划分算法） ========
P = [dfa_accept, set(dfa_states) - dfa_accept]
W = [dfa_accept.copy()]

while W:
    A = W.pop()
    for sym in symbols:
        X = {s for s in dfa_states if dfa_trans.get((s, sym)) in A}
        new_P = []
        for Y in P:
            inter = X & Y
            diff = Y - X
            if inter and diff:
                new_P.extend([inter, diff])
                if Y in W:
                    W.remove(Y)
                    W.extend([inter, diff])
                else:
                    if len(inter) <= len(diff):
                        W.append(inter)
                    else:
                        W.append(diff)
            else:
                new_P.append(Y)
        P = new_P

# ======== 输出结果 ========
print("Total DFA states:", len(dfa_states))
print("Total minimized DFA states:", len(P))
print("\nDFA Transition Table:")
for (src, sym), dst in dfa_trans.items():
    print(f"{set(x.split('_')[0] for x in src)} --{sym}--> {set(x.split('_')[0] for x in dst)}")

print("\nMinimized Partitions:")
for i, part in enumerate(P):
    print(f"  Group {i}: {[set(x.split('_')[0] for x in s) for s in part]}")

# ======== 输出 JSON 格式的最小化 DFA ========
# 将每个分区编号，并生成在分区层面的最小化 DFA
partition_map = {i: p for i, p in enumerate(P)}

# map each DFA state (frozenset) to its partition id
state_to_partition = {}
for pid, part in partition_map.items():
    for s in part:
        state_to_partition[s] = pid

# 找到包含 merged_start 的起始分区
start_partition = None
for s in dfa_states:
    if merged_start in s:
        start_partition = state_to_partition.get(s)
        break

# 构建最小化转换表（以分区 id 表示）
min_trans = []
for (src, sym), dst in dfa_trans.items():
    src_pid = state_to_partition.get(src)
    dst_pid = state_to_partition.get(dst)
    if src_pid is None or dst_pid is None:
        continue
    min_trans.append({
        "src": f"S{src_pid}",
        "symbol": sym,
        "dst": f"S{dst_pid}"
    })

# 接受分区集合
accept_partitions = [f"S{pid}" for pid, part in partition_map.items() if any(x in merged_accept for s in part for x in s)]

# 每个分区包含的成员（便于阅读）
states_info = {}
for pid, part in partition_map.items():
    members = [sorted(list(s)) for s in part]
    states_info[f"S{pid}"] = {"members": members}

minimized_dfa = {
    "states": list(states_info.keys()),
    "states_info": states_info,
    "start": f"S{start_partition}" if start_partition is not None else None,
    "accepts": accept_partitions,
    "transitions": min_trans
}

out_path = os.path.join(os.path.dirname(__file__), "minimized_DFA.json")
with open(out_path, "w", encoding="utf-8") as f:
    json.dump(minimized_dfa, f, ensure_ascii=False, indent=2)

print(f"Minimized DFA JSON written to: {out_path}")

# ======== 可视化最小化 DFA（选） ========
G = nx.DiGraph()
color_map = itertools.cycle(["red", "blue", "green", "purple", "orange"])
state_map = {frozenset.union(*p) if len(p) > 1 else list(p)[0]: i for i, p in enumerate(P)}

for (src, sym), dst in dfa_trans.items():
    G.add_edge(str(src), str(dst), label=sym)

pos = nx.spring_layout(G, seed=42, k=0.7)
plt.figure(figsize=(12, 8))
for node in G.nodes():
    plt.scatter(*pos[node], s=500, color="lightblue")
    plt.text(*pos[node], f"S{list(G.nodes()).index(node)}", ha="center", va="center")

for u, v, data in G.edges(data=True):
    plt.annotate(
        "",
        xy=pos[v],
        xytext=pos[u],
        arrowprops=dict(arrowstyle="->", color="gray", lw=1.5),
    )
    xm, ym = (pos[u] + pos[v]) / 2
    plt.text(xm, ym, data["label"], fontsize=8, color="gray")

plt.title("Minimized DFA")
plt.axis("off")
plt.tight_layout()
plt.savefig(os.path.join(os.path.dirname(__file__), "minimized_DFA.png"))

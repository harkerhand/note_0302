import networkx as nx
import matplotlib.pyplot as plt
import itertools
import os

# ======== 定义各个 NFA ========
NFAs = {
    "ID": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [
            ("s0", "[A-Z a-z _]", "s1"),
            ("s1", "[A-Z a-z 0-9 _]", "s1"),
        ],
    },
    "NUM": {
        "states": {"s0", "s1", "s2", "s3"},
        "start": "s0",
        "accept": {"s1", "s3"},
        "trans": [
            ("s0", "[0-9]", "s1"),
            ("s1", "[0-9]", "s1"),
            ("s1", "[.]", "s2"),
            ("s2", "[0-9]", "s3"),
            ("s3", "[0-9]", "s3"),
        ],
    },
    "COMMENT": {
        "states": {"s0", "s1", "s2"},
        "start": "s0",
        "accept": {"s2"},
        "trans": [
            ("s0", "[/]", "s1"),
            ("s1", "[/]", "s2"),
            ("s2", "[^\\n]", "s2"),
        ],
    },
    "POW1": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[\\^]", "s1")],
    },
    "POW2": {
        "states": {"s0", "s1", "s2"},
        "start": "s0",
        "accept": {"s2"},
        "trans": [
            ("s0", "[\\^]", "s1"),
            ("s1", "[\\^]", "s2"),
        ],
    },
    "EQ": {
        "states": {"s0", "s1", "s2"},
        "start": "s0",
        "accept": {"s2"},
        "trans": [
            ("s0", "[=]", "s1"),
            ("s1", "[=]", "s2"),
        ],
    },
    "NEQ": {
        "states": {"s0", "s1", "s2"},
        "start": "s0",
        "accept": {"s2"},
        "trans": [
            ("s0", "[!]", "s1"),
            ("s1", "[=]", "s2"),
        ],
    },
    "GE": {
        "states": {"s0", "s1", "s2"},
        "start": "s0",
        "accept": {"s2"},
        "trans": [
            ("s0", "[>]", "s1"),
            ("s1", "[=]", "s2"),
        ],
    },
    "LE": {
        "states": {"s0", "s1", "s2"},
        "start": "s0",
        "accept": {"s2"},
        "trans": [
            ("s0", "[<]", "s1"),
            ("s1", "[=]", "s2"),
        ],
    },
    "ASSIGN": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[=]", "s1")],
    },
    "PLUS": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[+]", "s1")],
    },
    "MINUS": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[-]", "s1")],
    },
    "MUL": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[*]", "s1")],
    },
    "DIV": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[/]", "s1")],
    },
    "GT": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[>]", "s1")],
    },
    "LT": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [("s0", "[<]", "s1")],
    },
    "SEP": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [
            ("s0", "[(]", "s1"),
            ("s0", "[)]", "s1"),
            ("s0", "[{]", "s1"),
            ("s0", "[}]", "s1"),
            ("s0", "[;]", "s1"),
            ("s0", "[,]", "s1"),
        ],
    },
    "WS": {
        "states": {"s0", "s1"},
        "start": "s0",
        "accept": {"s1"},
        "trans": [
            ("s0", "[ \\t\\n\\r]", "s1"),
            ("s1", "[ \\t\\n\\r]", "s1"),
        ],
    },
}

# ======== 构建大 NFA ========
G = nx.MultiDiGraph()

# 超级起点
G.add_node("START", kind="start")

colors = itertools.cycle(["red", "blue", "green", "purple", "orange", "brown"])

for name, nfa in NFAs.items():
    color = next(colors)
    prefix = name + "_"

    # 添加状态节点
    for s in nfa["states"]:
        kind = "accept" if s in nfa["accept"] else "normal"
        G.add_node(prefix + s, kind=kind, color=color, nfa=name)

    # 起点 ε 转移
    G.add_edge("START", prefix + nfa["start"], label="ε", color=color)

    # 添加转换
    for src, sym, dst in nfa["trans"]:
        G.add_edge(prefix + src, prefix + dst, label=sym, color=color)

# ======== 可视化绘制 ========
pos = nx.spring_layout(G, seed=42)  # 自动布局

plt.figure(figsize=(12, 8))
ax = plt.gca()
ax.set_title("Merged NFA", fontsize=16)

# 绘制节点
for node, data in G.nodes(data=True):
    x, y = pos[node]
    color = data.get("color", "black")
    if data["kind"] == "start":
        plt.scatter(x, y, c="black", s=400, marker="*", label="START")
    elif data["kind"] == "accept":
        plt.scatter(x, y, c=color, s=400, edgecolors="black", linewidths=2)
        circle = plt.Circle((x, y), 0.05, color=color, fill=False, lw=2)
        ax.add_patch(circle)
    else:
        plt.scatter(x, y, c=color, s=300)

    plt.text(x, y + 0.03, node, fontsize=9, ha="center")

# 绘制边
for u, v, data in G.edges(data=True):
    x1, y1 = pos[u]
    x2, y2 = pos[v]
    plt.annotate(
        "",
        xy=(x2, y2),
        xytext=(x1, y1),
        arrowprops=dict(
            arrowstyle="->", color=data.get("color", "gray"), lw=1.5
        ),
    )
    # 标签居中
    xm, ym = (x1 + x2) / 2, (y1 + y2) / 2
    plt.text(xm, ym, data["label"], fontsize=8, color=data.get("color", "gray"))

plt.axis("off")
plt.tight_layout()
out_path = "/home/harkerhand/classes/note_0302/CP/lab1/merged_nfa.png"
os.makedirs(os.path.dirname(out_path), exist_ok=True)
plt.savefig(out_path, dpi=300, bbox_inches="tight")
plt.close()

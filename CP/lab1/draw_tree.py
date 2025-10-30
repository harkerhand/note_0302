import json
from graphviz import Digraph

# 1. 读取 JSON AST
with open("ast.json", "r") as f:
    ast_json = json.load(f)

dot = Digraph(comment="AST", format="png")

node_id_counter = 0

def add_node(node_json, parent_id=None):
    global node_id_counter
    node_id = f"node{node_id_counter}"
    node_id_counter += 1

    # 节点标签
    if node_json.get("type") == "NonTerminal":
        label = node_json.get("kind", "NonTerminal")
    elif node_json.get("type") == "Terminal" or node_json.get("kind") == "Terminal":
        token = node_json.get("token", {})
        label = f'{token.get("kind","")}: {token.get("lexeme","")}'
    else:
        label = node_json.get("kind", "Unknown")

    dot.node(node_id, label)

    # 如果有父节点，画边
    if parent_id:
        dot.edge(parent_id, node_id)

    # 递归添加 children
    for child in node_json.get("children", []):
        add_node(child, node_id)

    return node_id

# 2. 构建整个 AST 图
add_node(ast_json)

# 3. 输出 PNG
dot.render("ast_tree", format="png")
print("AST 可视化图生成完成：ast_tree.png")

# REs

- 标识符
`ID   = [A-Za-z_][A-Za-z0-9_]* `
- 数字
`NUM  = [0-9]+(\.[0-9]+)?`
- 注释
`COMMENT = "//" [^\n]* `
- 双字符运算符
`POW2 = "^^" EQ  = "==" NEQ = "!=" GE  = ">=" LE  = "<="`
- 单字符运算符
`POW1 = "^"
ASSIGN = "="
PLUS   = "+"
MINUS  = "-"
MUL    = "*"
DIV    = "/"
GT     = ">"
LT     = "<"
`
- 单字符分隔符
`SEP = "(" | ")" | "{" | "}" | ";" | "," `
- 空格
`WS = [ \t\n\r]+`
- 关键字
`KEY = "let" "print" "read" "if" "else"`
 
# NFA
```
ID_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[A-Z a-z _]--> s1
    s1 --[A-Z a-z 0-9 _]--> s1

NUM_NFA:
  states: {s0, s1, s2, s3}
  start: s0
  accept: {s1, s3}
  transitions:
    s0 --[0-9]--> s1
    s1 --[0-9]--> s1
    s1 --[.]--> s2
    s2 --[0-9]--> s3
    s3 --[0-9]--> s3

COMMENT_NFA:
  states: {s0, s1, s2}
  start: s0
  accept: {s2}
  transitions:
    s0 --[/]--> s1
    s1 --[/]--> s2
    s2 --[^\n]--> s2

POW2_NFA:
  states: {s0, s1, s2}
  start: s0
  accept: {s2}
  transitions:
    s0 --[\^]--> s1
    s1 --[\^]--> s2

POW1_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[\^]--> s1

EQ_NFA:
  states: {s0, s1, s2}
  start: s0
  accept: {s2}
  transitions:
    s0 --[=]--> s1
    s1 --[=]--> s2

NEQ_NFA:
  states: {s0, s1, s2}
  start: s0
  accept: {s2}
  transitions:
    s0 --[!]--> s1
    s1 --[=]--> s2

GE_NFA:
  states: {s0, s1, s2}
  start: s0
  accept: {s2}
  transitions:
    s0 --[>]--> s1
    s1 --[=]--> s2

LE_NFA:
  states: {s0, s1, s2}
  start: s0
  accept: {s2}
  transitions:
    s0 --[<]--> s1
    s1 --[=]--> s2

ASSIGN_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[=]--> s1

PLUS_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[+]--> s1

MINUS_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[-]--> s1

MUL_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[*]--> s1

DIV_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[/]--> s1

GT_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[>]--> s1

LT_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[<]--> s1

SEP_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[(]--> s1
    s0 --[)]--> s1
    s0 --[{]--> s1
    s0 --[}]--> s1
    s0 --[;]--> s1
    s0 --[,]--> s1

WS_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[ \t\n\r]--> s1
    s1 --[ \t\n\r]--> s1

KEY_NFA:
  states: {s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14}
  start: s0
  accept: {s3, ID}
  transitions:
    # "let"
    s0 --[l]--> s1
    s1 --[e]--> s2
    s2 --[t]--> s3
    s3 --[A-Za-z0-9_]--> ID

    # "print"
    s0 --[p]--> s4
    s4 --[r]--> s5
    s5 --[i]--> s6
    s6 --[n]--> s7
    s7 --[t]--> s3

    # "read"
    s0 --[r]--> s9
    s9 --[e]--> s10
    s10 --[a]--> s11
    s11 --[d]--> s3

    # "if"
    s0 --[i]--> s13
    s13 --[f]--> s3

    # "else"
    s0 --[e]--> s15
    s15 --[l]--> s16
    s16 --[s]--> s17
    s17 --[e]--> s3
```

# Merge
```
LEXER_NFA:
  states:
    - S0
    - all states from each sub-NFA (renamed to avoid collision)
  start: S0
  accept:
    - all accept states from each sub-NFA
  transitions:
    # epsilon transitions from global start
    S0 --[ε]--> ID_s0
    S0 --[ε]--> NUM_s0
    S0 --[ε]--> COMMENT_s0
    S0 --[ε]--> POW2_s0
    S0 --[ε]--> POW1_s0
    S0 --[ε]--> EQ_s0
    S0 --[ε]--> NEQ_s0
    S0 --[ε]--> GE_s0
    S0 --[ε]--> LE_s0
    S0 --[ε]--> ASSIGN_s0
    S0 --[ε]--> PLUS_s0
    S0 --[ε]--> MINUS_s0
    S0 --[ε]--> MUL_s0
    S0 --[ε]--> DIV_s0
    S0 --[ε]--> GT_s0
    S0 --[ε]--> LT_s0
    S0 --[ε]--> SEP_s0
    S0 --[ε]--> WS_s0
    S0 --[ε]--> KEY_s0

    ID_s0 --[A-Z a-z _]--> ID_s1
    ID_s1 --[A-Z a-z 0-9 _]--> ID_s1

    NUM_s0 --[0-9]--> NUM_s1
    NUM_s1 --[0-9]--> NUM_s1
    NUM_s1 --[.]--> NUM_s2
    NUM_s2 --[0-9]--> NUM_s3
    NUM_s3 --[0-9]--> NUM_s3

    COMMENT_s0 --[/]--> COMMENT_s1
    COMMENT_s1 --[/]--> COMMENT_s2
    COMMENT_s2 --[^\n]--> COMMENT_s2

    POW2_s0 --[\^]--> POW2_s1
    POW2_s1 --[\^]--> POW2_s2

    POW1_s0 --[\^]--> POW1_s1

    EQ_s0 --[=]--> EQ_s1
    EQ_s1 --[=]--> EQ_s2

    NEQ_s0 --[!]--> NEQ_s1
    NEQ_s1 --[=]--> NEQ_s2

    GE_s0 --[>]--> GE_s1
    GE_s1 --[=]--> GE_s2

    LE_s0 --[<]--> LE_s1
    LE_s1 --[=]--> LE_s2

    ASSIGN_s0 --[=]--> ASSIGN_s1

    PLUS_s0 --[+]--> PLUS_s1
    MINUS_s0 --[-]--> MINUS_s1
    MUL_s0 --[*]--> MUL_s1
    DIV_s0 --[/]--> DIV_s1
    GT_s0 --[>]--> GT_s1
    LT_s0 --[<]--> LT_s1

    SEP_s0 --[(]--> SEP_s1
    SEP_s0 --[)]--> SEP_s1
    SEP_s0 --[{]--> SEP_s1
    SEP_s0 --[}]--> SEP_s1
    SEP_s0 --[;]--> SEP_s1
    SEP_s0 --[,]--> SEP_s1

    WS_s0 --[ \t\n\r]--> WS_s1
    WS_s1 --[ \t\n\r]--> WS_s1

    KEY_s0 --[l]--> KEY_s1
    KEY_s1 --[e]--> KEY_s2
    KEY_s2 --[t]--> KEY_s3
    KEY_s3 --[A-Za-z0-9_]--> KEY_ID

    # "print"
    KEY_s0 --[p]--> KEY_s4
    KEY_s4 --[r]--> KEY_s5
    KEY_s5 --[i]--> KEY_s6
    KEY_s6 --[n]--> KEY_s7
    KEY_s7 --[t]--> KEY_s3

    # "read"
    KEY_s0 --[r]--> KEY_s9
    KEY_s9 --[e]--> KEY_s10
    KEY_s10 --[a]--> KEY_s11
    KEY_s11 --[d]--> KEY_s3

    # "if"
    KEY_s0 --[i]--> KEY_s13
    KEY_s13 --[f]--> KEY_s3

    # "else"
    KEY_s0 --[e]--> KEY_s15
    KEY_s15 --[l]--> KEY_s16
    KEY_s16 --[s]--> KEY_s17
    KEY_s17 --[e]--> KEY_s3
```

# Minimize

```json
{
    "states": [
        "START",
        "ID",
        "NUM_INT",
        "NUM_MIDDLE",
        "NUM_FLOAT",
        "OP:POW",
        "OP:POW2",
        "ASSIGN",
        "OP:EQ",
        "OP:LESS",
        "OP:LESSEQ",
        "OP:DIV",
        "OP:GREATER",
        "OP:GREATEREQ",
        "OP:MINUS",
        "OP:ADD",
        "OP:MUL",
        "WS",
        "OP:NOT",
        "OP:NOTEQ",
        "SEP",
        "COMMENT"
    ],
    "start": "START",
    "accept": [
        "ID",
        "NUM_INT",
        "NUM_FLOAT",
        "OP:POW2",
        "ASSIGN",
        "OP:EQ",
        "OP:LESSEQ",
        "OP:DIV",
        "OP:GREATEREQ",
        "OP:MINUS",
        "OP:ADD",
        "OP:MUL",
        "WS",
        "OP:NOTEQ",
        "SEP",
        "COMMENT"
    ],
    "trans": [
        {
            "from": "START",
            "to": "ID",
            "pattern": "[A-Z a-z _]"
        },
        {
            "from": "ID",
            "to": "ID",
            "pattern": "[A-Z a-z 0-9 _]"
        },
        {
            "from": "START",
            "to": "NUM_INT",
            "pattern": "[0-9]"
        },
        {
            "from": "NUM_INT",
            "to": "NUM_INT",
            "pattern": "[0-9]"
        },
        {
            "from": "NUM_INT",
            "to": "NUM_MIDDLE",
            "pattern": "[.]"
        },
        {
            "from": "NUM_MIDDLE",
            "to": "NUM_FLOAT",
            "pattern": "[0-9]"
        },
        {
            "from": "NUM_FLOAT",
            "to": "NUM_FLOAT",
            "pattern": "[0-9]"
        },
        {
            "from": "START",
            "to": "OP:POW",
            "pattern": "[\\^]"
        },
        {
            "from": "OP:POW",
            "to": "OP:POW2",
            "pattern": "[\\^]"
        },
        {
            "from": "START",
            "to": "ASSIGN",
            "pattern": "[=]"
        },
        {
            "from": "ASSIGN",
            "to": "OP:EQ",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "OP:LESS",
            "pattern": "[<]"
        },
        {
            "from": "START",
            "to": "OP:LESS",
            "pattern": "[<]"
        },
        {
            "from": "OP:LESS",
            "to": "OP:LESSEQ",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "OP:DIV",
            "pattern": "[/]"
        },
        {
            "from": "START",
            "to": "OP:GREATER",
            "pattern": "[>]"
        },
        {
            "from": "OP:GREATER",
            "to": "OP:GREATEREQ",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "OP:MINUS",
            "pattern": "[-]"
        },
        {
            "from": "START",
            "to": "OP:ADD",
            "pattern": "[+]"
        },
        {
            "from": "START",
            "to": "OP:MUL",
            "pattern": "[*]"
        },
        {
            "from": "START",
            "to": "WS",
            "pattern": "[ \\t\\n\\r]"
        },
        {
            "from": "WS",
            "to": "WS",
            "pattern": "[ \\t\\n\\r]"
        },
        {
            "from": "START",
            "to": "OP:NOT",
            "pattern": "[!]"
        },
        {
            "from": "OP:NOT",
            "to": "OP:NOTEQ",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "SEP",
            "pattern": "[(){};,]"
        },
        {
            "from": "OP:DIV",
            "to": "COMMENT",
            "pattern": "[/]"
        },
        {
            "from": "COMMENT",
            "to": "COMMENT",
            "pattern": "[^\\n]"
        }
    ]
}
```
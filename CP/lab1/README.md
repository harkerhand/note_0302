# REs

- 标识符
`ID   = [A-Za-z_][A-Za-z0-9_]* `
- 数字
`NUM  = [0-9]+(\.[0-9]+)?`
- 注释
`COMMENT = "//" [^\n]* `
- 双字符运算符
`POW2 = "^^" EQ = "==" NEQ = "!=" GE = ">=" LE = "<="`
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
`SEP = [(){};,]`
- 空格
`WS = [ \t\n\r]+`
- 关键字
`KEY = (let|print|read|if|else)`

# NFA
```
ID_NFA:
  states: {s0, s1}
  start: s0
  accept: {s1}
  transitions:
    s0 --[A-Zabcdfghjkmnoqstuvwxyz_]--> s1
    s1 --[A-Za-z0-9_]--> s1

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

    s3 --[A-Za-z0-9_]--> ID
    ID --[A-Za-z0-9_]--> ID
    # "let"
    s0 --[l]--> s1
    s1 --[A-Za-df-z0-9_]--> ID
    s1 --[e]--> s2
    s2 --[A-Za-su-z0-9_]--> ID
    s2 --[t]--> s3

    # "print"
    s0 --[p]--> s4
    s4 --[A-Za-qs-z0-9_]--> ID
    s4 --[r]--> s5
    s5 --[A-Za-hj-z0-9_]--> ID
    s5 --[i]--> s6
    s6 --[A-Za-mo-z0-9_]--> ID
    s6 --[n]--> s7
    s7 --[A-Za-su-z0-9_]--> ID
    s7 --[t]--> s3

    # "read"
    s0 --[r]--> s9
    s9 --[A-Za-qst-z0-9_]--> ID
    s9 --[e]--> s10
    s10 --[A-Zb-z0-9_]--> ID
    s10 --[a]--> s11
    s10 --[A-Za-ce-z0-9_]--> ID
    s11 --[d]--> s3

    # "if"
    s0 --[i]--> s13
    s13 --[A-Za-eg-z0-9_]--> ID
    s13 --[f]--> s3

    # "else"
    s0 --[e]--> s15
    s15 --[A-Za-km-z0-9_]--> ID
    s15 --[l]--> s16
    s16 --[A-Za-rt-z0-9_]--> ID
    s16 --[s]--> s17
    s17 --[A-Za-df-z0-9_]--> ID
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

    ID_s0 --[A-Zabcdfghjkmnoqstuvwxyz_]--> ID_s1
    ID_s1 --[A-ZZa-z0-9_]--> ID_s1

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

    KEY_s3 --[A-Za-z0-9_]--> KEY_ID
    KEY_ID --[A-Za-z0-9_]--> KEY_ID
    # "let"
    KEY_s0 --[l]--> KEY_s1
    KEY_s1 --[A-Za-df-z0-9_]--> KEY_ID
    KEY_s1 --[e]--> KEY_s2
    KEY_s2 --[A-Za-su-z0-9_]--> KEY_ID
    KEY_s2 --[t]--> KEY_s3

    # "print"
    KEY_s0 --[p]--> KEY_s4
    KEY_s4 --[A-Za-qs-z0-9_]--> KEY_ID
    KEY_s4 --[r]--> KEY_s5
    KEY_s5 --[A-Za-hj-z0-9_]--> KEY_ID
    KEY_s5 --[i]--> KEY_s6
    KEY_s6 --[A-Za-mo-z0-9_]--> KEY_ID
    KEY_s6 --[n]--> KEY_s7
    KEY_s7 --[A-Za-su-z0-9_]--> KEY_ID
    KEY_s7 --[t]--> KEY_s3

    # "read"
    KEY_s0 --[r]--> KEY_s9
    KEY_s9 --[A-Za-qst-z0-9_]--> KEY_ID
    KEY_s9 --[e]--> KEY_s10
    KEY_s10 --[A-Zb-z0-9_]--> KEY_ID
    KEY_s10 --[a]--> KEY_s11
    KEY_s10 --[A-Za-ce-z0-9_]--> KEY_ID
    KEY_s11 --[d]--> KEY_s3

    # "if"
    KEY_s0 --[i]--> KEY_s13
    KEY_s13 --[A-Za-eg-z0-9_]--> KEY_ID
    KEY_s13 --[f]--> KEY_s3

    # "else"
    KEY_s0 --[e]--> KEY_s15
    KEY_s15 --[A-Za-km-z0-9_]--> KEY_ID
    KEY_s15 --[l]--> KEY_s16
    KEY_s16 --[A-Za-rt-z0-9_]--> KEY_ID
    KEY_s16 --[s]--> KEY_s17
    KEY_s17 --[A-Za-df-z0-9_]--> KEY_ID
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
        "OP1",
        "OP2",
        "REPEAT",
        "ASSIGN",
        "WS",
        "SEP",
        "COMMENT",
        "KEY"
    ],
    "start": "START",
    "accept": [
        "ID",
        "NUM_INT",
        "NUM_FLOAT",
        "OP1",
        "OP2",
        "REPEAT",
        "ASSIGN",
        "WS",
        "SEP",
        "COMMENT",
        "KEY"
    ],
    "trans": [
        {
            "from": "START",
            "to": "ID",
            "pattern": "[A-Zabcdfghjkmnoqstuvwxyz_]"
        },
        {
            "from": "ID",
            "to": "ID",
            "pattern": "[A-Za-z0-9_]"
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
            "to": "OP1",
            "pattern": "[\\^]"
        },
        {
            "from": "OP1",
            "to": "REPEAT",
            "pattern": "[\\^]"
        },
        {
            "from": "START",
            "to": "ASSIGN",
            "pattern": "[=]"
        },
        {
            "from": "ASSIGN",
            "to": "OP2",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "OP1",
            "pattern": "[<]"
        },
        {
            "from": "OP1",
            "to": "OP2",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "OP1",
            "pattern": "[/]"
        },
        {
            "from": "START",
            "to": "OP1",
            "pattern": "[>]"
        },
        {
            "from": "OP1",
            "to": "OP2",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "OP1",
            "pattern": "[-]"
        },
        {
            "from": "START",
            "to": "OP1",
            "pattern": "[+]"
        },
        {
            "from": "START",
            "to": "OP1",
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
            "to": "OP1",
            "pattern": "[!]"
        },
        {
            "from": "OP1",
            "to": "OP2",
            "pattern": "[=]"
        },
        {
            "from": "START",
            "to": "SEP",
            "pattern": "[(){};,]"
        },
        {
            "from": "OP1",
            "to": "COMMENT",
            "pattern": "[/]"
        },
        {
            "from": "COMMENT",
            "to": "COMMENT",
            "pattern": "[^\\n]"
        },
        {
            "from": "KEY",
            "to": "ID",
            "pattern": "[A-Za-z0-9_]"
        },
        {
            "from": "START",
            "to": "LET:L",
            "pattern": "[l]"
        },
        {
            "from": "LET:L",
            "to": "ID",
            "pattern": "[A-Za-df-z0-9_]"
        },
        {
            "from": "LET:L",
            "to": "LET:E",
            "pattern": "[e]"
        },
        {
            "from": "LET:E",
            "to": "ID",
            "pattern": "[A-Za-su-z0-9_]"
        },
        {
            "from": "LET:E",
            "to": "KEY",
            "pattern": "[t]"
        },
        {
            "from": "START",
            "to": "PRINT:P",
            "pattern": "[p]"
        },
        {
            "from": "PRINT:P",
            "to": "ID",
            "pattern": "[A-Za-qs-z0-9_]"
        },
        {
            "from": "PRINT:P",
            "to": "PRINT:R",
            "pattern": "[r]"
        },
        {
            "from": "PRINT:R",
            "to": "ID",
            "pattern": "[A-Za-hj-z0-9_]"
        },
        {
            "from": "PRINT:R",
            "to": "PRINT:I",
            "pattern": "[i]"
        },
        {
            "from": "PRINT:I",
            "to": "ID",
            "pattern": "[A-Za-mo-z0-9_]"
        },
        {
            "from": "PRINT:I",
            "to": "PRINT:N",
            "pattern": "[n]"
        },
        {
            "from": "PRINT:N",
            "to": "ID",
            "pattern": "[A-Za-su-z0-9_]"
        },
        {
            "from": "PRINT:N",
            "to": "KEY",
            "pattern": "[t]"
        },
        {
            "from": "START",
            "to": "READ:R",
            "pattern": "[r]"
        },
        {
            "from": "READ:R",
            "to": "ID",
            "pattern": "[A-Za-df-z0-9_]"
        },
        {
            "from": "READ:R",
            "to": "READ:E",
            "pattern": "[e]"
        },
        {
            "from": "READ:E",
            "to": "ID",
            "pattern": "[A-Zb-z0-9_]"
        },
        {
            "from": "READ:E",
            "to": "READ:A",
            "pattern": "[a]"
        },
        {
            "from": "READ:A",
            "to": "ID",
            "pattern": "[A-Za-ce-z0-9_]"
        },
        {
            "from": "READ:A",
            "to": "KEY",
            "pattern": "[d]"
        },
        {
            "from": "START",
            "to": "IF:I",
            "pattern": "[i]"
        },
        {
            "from": "IF:I",
            "to": "ID",
            "pattern": "[A-Za-eg-z0-9_]"
        },
        {
            "from": "IF:I",
            "to": "KEY",
            "pattern": "[f]"
        },
        {
            "from": "START",
            "to": "ELSE:E",
            "pattern": "[e]"
        },
        {
            "from": "ELSE:E",
            "to": "ID",
            "pattern": "[A-Za-km-z0-9_]"
        },
        {
            "from": "ELSE:E",
            "to": "ELSE:L",
            "pattern": "[l]"
        },
        {
            "from": "ELSE:L",
            "to": "ID",
            "pattern": "[A-Za-rt-z0-9_]"
        },
        {
            "from": "ELSE:L",
            "to": "ELSE:S",
            "pattern": "[s]"
        },
        {
            "from": "ELSE:S",
            "to": "ID",
            "pattern": "[A-Za-df-z0-9_]"
        },
        {
            "from": "ELSE:S",
            "to": "KEY",
            "pattern": "[e]"
        }
    ]
}
```

# CFG

- Program -> StmtList
- StmtList -> Stmt StmtList | ε
- Stmt -> DeclStmt | AssignStmt | BlockMaybeRepeat | FuncCallStmt | IfStmt
- DeclStmt -> KEY(let) ID SEP(;)
- AssignStmt -> ID ASSIGN Expr SEP(;)
- BlockMaybeRepeat -> Block | RepeatTail
- Block -> SEP({) StmtList SEP(})
- FuncCallStmt -> FuncCall SEP(;)
- FuncCall -> KEY(print) SEP(() Expr SEP()) | KEY(read) SEP(() ID SEP())
- IfStmt -> KEY(if) SEP(() Expr SEP()) Block ElsePart
- ElsePart -> KEY(else) Block | ε
- RepeatTail -> REPEAT Expr SEP(;) | ε
- Expr -> EqualityExpr
- EqualityExpr -> RelationExpr EqualityExpr'
- EqualityExpr' -> OP2(==) RelationExpr EqualityExpr' | OP2(!=) RelationExpr EqualityExpr' | ε
- RelationExpr -> AddExpr RelationExpr'
- RelationExpr' -> OP2(>=) AddExpr RelationExpr' | OP2(<=) AddExpr RelationExpr' | OP1(>) AddExpr RelationExpr' | OP1(<) AddExpr RelationExpr' | ε
- AddExpr -> MulExpr AddExpr'
- AddExpr' -> OP1(+) MulExpr AddExpr' | OP1(-) MulExpr AddExpr' | ε
- MulExpr -> UnaryExpr MulExpr'
- MulExpr' -> OP1(*) UnaryExpr MulExpr' | OP1(/) UnaryExpr MulExpr' | ε
- UnaryExpr -> OP1(+) UnaryExpr | OP1(-) UnaryExpr | OP1(!) UnaryExpr | PowerExpr
- PowerExpr -> PrimaryExpr PowerExpr'
- PowerExpr' -> OP1(^) PowerExpr | ε
- PrimaryExpr -> SEP(() Expr SEP()) | NUM_INT | NUM_FLOAT | ID

# First and Follow Sets

- First Sets
  - First(Program) = First(StmtList) = {KEY(let), ID, SEP({), KEY(print), KEY(read), KEY(if), ε}
  - First(Stmt) = {KEY(let), ID, SEP({), KEY(print), KEY(read), KEY(if)}
  - First(DeclStmt) = {KEY(let)}
  - First(AssignStmt) = {ID}
  - First(BlockMaybeRepeat) = First(Block) = {SEP({)}
  - First(FuncCallStmt) = First(FuncCall) = {KEY(print), KEY(read)}
  - First(IfStmt) = {KEY(if)}
  - First(ElsePart) = {KEY(else), ε}
  - First(RepeatTail) = {REPEAT, ε}
  - First(Expr) = First(EqualityExpr) = First(RelationExpr) = First(AddExpr) = First(MulExpr) = {SEP(() , NUM_INT, NUM_FLOAT, ID, OP1(+), OP1(-), OP1(!)}
  - First(UnaryExpr) = {OP1(+), OP1(-), OP1(!), SEP(() , NUM_INT, NUM_FLOAT, ID}
  - First(EqualityExpr') = {OP2(==), OP2(!=), ε}
  - First(RelationExpr') = {OP2(>=), OP2(<=), OP1(>), OP1(<), ε}
  - First(AddExpr') = {OP1(+), OP1(-), ε}
  - First(MulExpr') = {OP1(*), OP1(/), ε}
  - First(PowerExpr) = {SEP(() , NUM_INT, NUM_FLOAT, ID}
  - First(PowerExpr') = {OP1(^), ε}
  - First(PrimaryExpr) = {SEP(() , NUM_INT, NUM_FLOAT, ID}
- Follow Sets
  - Follow(Program) = {#}
  - Follow(StmtList) = {#, SEP(})}
  - Follow(Stmt) = Follow(DeclStmt) = Follow(AssignStmt) = Follow(IfStmt) = Follow(ElsePart) = Follow(Block) = {KEY(let), ID, SEP({), KEY(print), KEY(read), KEY(if), SEP(}), #}
  - Follow(BlockMaybeRepeat) = {KEY(let), ID, SEP({), KEY(print), KEY(read), KEY(if), SEP(}), #, KEY(else), REPEAT}
  - Follow(RepeatTail) = {KEY(let), ID, SEP({), KEY(print), KEY(read), KEY(if), SEP(}), #}
  - Follow(FuncCallStmt) = Follow(FuncCall) = {SEP(;)}
  - Follow(Expr) = Follow(EqualityExpr) = Follow(EqualityExpr') = {SEP()), SEP(;) }
  - Follow(RelationExpr) = Follow(RelationExpr') = {OP2(==), OP2(!=), SEP()), SEP(;) }
  - Follow(AddExpr) = Follow(AddExpr') = {OP2(>=), OP2(<=), OP1(>), OP1(<), OP2(==), OP2(!=), SEP()), SEP(;) }
  - Follow(MulExpr) = Follow(MulExpr') = {OP1(+), OP1(-), OP2(>=), OP2(<=), OP1(>), OP1(<), OP2(==), OP2(!=), SEP()), SEP(;) }
  - Follow(UnaryExpr) = {OP1(*), OP1(/), OP1(+), OP1(-), OP2(>=), OP2(<=), OP1(>), OP1(<), OP2(==), OP2(!=), SEP()), SEP(;) }
  - Follow(PowerExpr) = Follow(PowerExpr') = {OP1(^), OP1(*), OP1(/), OP1(+), OP1(-), OP2(>=), OP2(<=), OP1(>), OP1(<), OP2(==), OP2(!=), SEP()), SEP(;) }
  - Follow(PrimaryExpr) = {OP1(^), OP1(*), OP1(/), OP1(+), OP1(-), OP2(>=), OP2(<=), OP1(>), OP1(<), OP2(==), OP2(!=), SEP()), SEP(;) }

# LL(1) Parsing Table

| Non-Terminal     | KEY(let)           | ID                    | SEP({)                 | KEY(print)                    | KEY(read)                  | KEY(if)                                   | SEP(()                            | NUM_INT                            | NUM_FLOAT                            | OP1(+)                            | OP1(-)                            | OP1(!)                            | SEP(}) | KEY(else)       | REPEAT             | SEP(;) | #        |
| ---------------- | ------------------ | --------------------- | ---------------------- | ----------------------------- | -------------------------- | ----------------------------------------- | --------------------------------- | ---------------------------------- | ------------------------------------ | --------------------------------- | --------------------------------- | --------------------------------- | ------ | --------------- | ------------------ | ------ | -------- |
| Program          | StmtList           | StmtList              | StmtList               | StmtList                      | StmtList                   | StmtList                                  |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        | StmtList |
| StmtList         | Stmt StmtList      | Stmt StmtList         | Stmt StmtList          | Stmt StmtList                 | Stmt StmtList              | Stmt StmtList                             |                                   |                                    |                                      |                                   |                                   |                                   | ε      |                 |                    | ε      | ε        |
| Stmt             | DeclStmt           | AssignStmt            | BlockMaybeRepeat       | FuncCallStmt                  | FuncCallStmt               | IfStmt                                    |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| DeclStmt         | KEY(let) ID SEP(;) |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| AssignStmt       |                    | ID ASSIGN Expr SEP(;) |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| BlockMaybeRepeat |                    |                       | Block RepeatTail       |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| RepeatTail       |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 | REPEAT Expr SEP(;) | ε      | ε        |
| Block            |                    |                       | SEP({) StmtList SEP(}) |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| FuncCallStmt     |                    |                       |                        | FuncCall SEP(;)               | FuncCall SEP(;)            |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| FuncCall         |                    |                       |                        | KEY(print) SEP(() Expr SEP()) | KEY(read) SEP(() ID SEP()) |                                           |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| IfStmt           |                    |                       |                        |                               |                            | KEY(if) SEP(() Expr SEP()) Block ElsePart |                                   |                                    |                                      |                                   |                                   |                                   |        |                 |                    |        |          |
| ElsePart         |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   | ε      | KEY(else) Block |                    |        |          |
| Expr             |                    |                       |                        |                               |                            |                                           | SEP(() Expr                       | NUM_INT Expr                       | NUM_FLOAT Expr                       | OP1(+) Expr                       | OP1(-) Expr                       | OP1(!) Expr                       |        |                 |                    |        |          |
| EqualityExpr     |                    |                       |                        |                               |                            |                                           | SEP(() RelationExpr EqualityExpr' | NUM_INT RelationExpr EqualityExpr' | NUM_FLOAT RelationExpr EqualityExpr' | OP1(+) RelationExpr EqualityExpr' | OP1(-) RelationExpr EqualityExpr' | OP1(!) RelationExpr EqualityExpr' |        |                 |                    |        |          |
| EqualityExpr'    |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   | ε      |                 |                    | ε      | ε        |
| RelationExpr     |                    |                       |                        |                               |                            |                                           | SEP(() AddExpr RelationExpr'      | NUM_INT AddExpr RelationExpr'      | NUM_FLOAT AddExpr RelationExpr'      | OP1(+) AddExpr RelationExpr'      | OP1(-) AddExpr RelationExpr'      | OP1(!) AddExpr RelationExpr'      |        |                 |                    |        |          |
| RelationExpr'    |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   | ε      |                 |                    | ε      | ε        |
| AddExpr          |                    |                       |                        |                               |                            |                                           | SEP(() MulExpr AddExpr'           | NUM_INT MulExpr AddExpr'           | NUM_FLOAT MulExpr AddExpr'           | OP1(+) MulExpr AddExpr'           | OP1(-) MulExpr AddExpr'           | OP1(!) MulExpr AddExpr'           |        |                 |                    |        |          |
| AddExpr'         |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      | OP1(+) MulExpr AddExpr'           | OP1(-) MulExpr AddExpr'           |                                   | ε      |                 |                    | ε      | ε        |
| MulExpr          |                    |                       |                        |                               |                            |                                           | SEP(() UnaryExpr MulExpr'         | NUM_INT UnaryExpr MulExpr'         | NUM_FLOAT UnaryExpr MulExpr'         | OP1(+) UnaryExpr MulExpr'         | OP1(-) UnaryExpr MulExpr'         | OP1(!) UnaryExpr MulExpr'         |        |                 |                    |        |          |
| MulExpr'         |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   | ε      |                 |                    | ε      | ε        |
| UnaryExpr        |                    |                       |                        |                               |                            |                                           | SEP(() PowerExpr                  | NUM_INT PowerExpr                  | NUM_FLOAT PowerExpr                  | OP1(+) UnaryExpr                  | OP1(-) UnaryExpr                  | OP1(!) UnaryExpr                  |        |                 |                    |        |          |
| PowerExpr        |                    |                       |                        |                               |                            |                                           | SEP(() PrimaryExpr PowerExpr'     | NUM_INT PrimaryExpr PowerExpr'     | NUM_FLOAT PrimaryExpr PowerExpr'     |                                   |                                   |                                   |        |                 |                    |        |          |
| PowerExpr'       |                    |                       |                        |                               |                            |                                           |                                   |                                    |                                      |                                   |                                   |                                   | ε      |                 |                    | ε      | ε        |
| PrimaryExpr      |                    |                       |                        |                               |                            |                                           | SEP(() Expr SEP())                | NUM_INT                            | NUM_FLOAT                            |                                   |                                   |                                   |        |                 |                    |        |          |


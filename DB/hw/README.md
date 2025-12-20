

# 数据库课程设计

09023321 巩皓锴

## 背景

随着计算机教育的普及与程序设计竞赛的兴起，传统的“人工阅卷”已无法满足高效、公正的评测需求。因此，开发一套 **OJ（Online Judge）在线评测系统** 具有深远的实际意义：

1. **自动化评测的高效性**：系统能够自动编译、运行用户提交的代码，并利用预设测试用例进行毫秒级的即时比对，极大地节省了人力成本。
2. **评价标准的客观性**：通过严格限制 **时间（CPU Time）** 与 **空间（Memory）**，OJ 为代码性能提供了量化的硬性指标，确保了结果的绝对公平。
3. **人才培养与筛选**：系统能够真实记录用户的解题进度与能力分布，为教学评估及企业技术人才招聘提供直观的数据支撑。

## 概念模型、数据库模式设计

### 总述本数据库

i.  本数据库基于程序设计竞赛与在线评测业务展开。描述了不同类型的赛事，如国际大学生程序设计竞赛（ICPC）、程序设计及语言期末考试等。除了以个人为单位参加的比赛外，系统还支持以组织（学院、班级）为单位进行管理。

ii. 对于所有比赛，都会有多个竞赛题目和多个参赛人员。对于正在进行的赛事，系统实时统计提交记录并生成实时排名；对于已结束的比赛，记录最终获奖情况与历史表现。

iii. 统计了题目与知识点的关联关系，通过标签系统分类（如：动态规划、图论、字符串）；统计了用户在不同阶段的解题行为与成长轨迹。

iv. 对于用户：设有基本信息表（账号密码）、所属组织（学校/班级）以及在系统内的练习历史和竞赛参与记录。

v.  对于组织机构：是管理用户的最小行政单位，支持父子层级结构，并通过 IP 地址段关联地理位置，用于规范化管理。

### 数据关系

i.   组织机构：包含组织 ID、名称、上级组织 ID、对应的位置 IP。

ii.  位置信息：包含 IP 地址主键及对应的详细物理地址。

iii. 用户信息：姓名、登录密码、所属组织、用户唯一编号。

iv.  登录日志：记录用户 ID、登录的具体时间、访问时的 IP 地址。

v.   题目信息：包含题目描述、运行时间限制（ms）、内存占用限制（MB）。

vi.  标签系统：包含标签 ID、知识点名称。

vii. 题目标签关联：通过题目 ID 与标签 ID 的映射，实现一道题目对应多个知识点。

viii. 竞赛信息：包含竞赛名称、统一的开始时间与结束时间。

ix.  竞赛题目关联：定义某场竞赛具体包含哪些题目及其在比赛中的编号。

x.   竞赛参与者：记录报名或参加某场比赛的用户 ID 与竞赛 ID。

xi.  提交记录（核心）：包含提交编号、用户 ID、题目 ID、所属竞赛 ID、提交时间、运行结果（AC/WA等）、消耗时间、消耗内存。

### ER图

![ER图](er.png)

### 数据库设计

- 组织机构
    - 组织ID (主键)
    - 名称
    - 上级组织ID
    - 位置IP地址
- 位置
    - 位置IP (主键)
    - 详细地址
- 用户
    - 用户ID (主键)
    - 姓名
    - 密码
    - 组织ID (外键)
- 登录日志
    - 日志ID (主键)
    - 用户ID 
    - 登录时间
    - 位置IP地址
- 题目
    - 题目ID (主键)
    - 题目描述
    - 时间限制
    - 空间限制
- 标签
    - 标签ID (主键)
    - 标签名称
- 题目标签关联
    - ID (主键)
    - 题目ID
    - 标签ID
- 提交记录
    - 提交ID (主键)
    - 用户ID
    - 题目ID
    - 竞赛ID
    - 提交时间
    - 结果
    - 耗时
    - 使用内存
- 竞赛
    - 竞赛ID (主键)
    - 竞赛名称
    - 开始时间
    - 结束时间
- 竞赛题目关联
    - ID (主键)
    - 竞赛ID
    - 题目ID
- 竞赛参与者
    - ID (主键)
    - 竞赛ID
    - 用户ID

```sql
-- 1. 组织机构
CREATE TABLE Organization (
    OrgID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    ParentOrgID INTEGER,
    LocationIP TEXT
);

-- 2. 位置
CREATE TABLE Location (
    LocationIP TEXT PRIMARY KEY,
    Address TEXT
);

-- 3. 用户
CREATE TABLE User (
    UserID INTEGER PRIMARY KEY AUTOINCREMENT,
    Username TEXT NOT NULL,
    Password TEXT NOT NULL,
    OrgID INTEGER
);

-- 4. 登录日志
CREATE TABLE LoginLog (
    LogID INTEGER PRIMARY KEY AUTOINCREMENT,
    UserID INTEGER,
    LoginTime DATETIME DEFAULT CURRENT_TIMESTAMP,
    LocationIP TEXT
);

-- 5. 题目
CREATE TABLE Problem (
    ProblemID INTEGER PRIMARY KEY AUTOINCREMENT,
    Description TEXT,
    TimeLimit INTEGER, -- 单位通常为 ms
    MemoryLimit INTEGER -- 单位通常为 KB
);

-- 6. 标签
CREATE TABLE Tag (
    TagID INTEGER PRIMARY KEY AUTOINCREMENT,
    TagName TEXT UNIQUE
);

-- 7. 题目标签关联 (多对多)
CREATE TABLE ProblemTagRelation (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ProblemID INTEGER,
    TagID INTEGER
);

-- 8. 提交记录
CREATE TABLE Submission (
    SubmissionID INTEGER PRIMARY KEY AUTOINCREMENT,
    UserID INTEGER,
    ProblemID INTEGER,
    ContestID INTEGER,
    SubmitTime DATETIME DEFAULT CURRENT_TIMESTAMP,
    Result TEXT,
    TimeUsed INTEGER,
    MemoryUsed INTEGER
);

-- 9. 竞赛
CREATE TABLE Contest (
    ContestID INTEGER PRIMARY KEY AUTOINCREMENT,
    ContestName TEXT,
    StartTime DATETIME,
    EndTime DATETIME
);

-- 10. 竞赛题目关联
CREATE TABLE ContestProblemRelation (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ContestID INTEGER,
    ProblemID INTEGER
);

-- 11. 竞赛参与者
CREATE TABLE ContestParticipant (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ContestID INTEGER,
    UserID INTEGER
);
```

数据库设计中没有使用外键，考虑到以下几点：
- 性能开销
    - 写操作变慢：每次插入、更新或删除数据时，数据库引擎都必须去扫描关联的表以确保存实性。这会产生额外的磁盘 I/O。

    - 锁竞争：外键约束可能导致数据库在检查约束时锁定相关表的行，在高并发场景下容易产生死锁或严重的阻塞。

- 开发与运维的灵活性差

    - 清理数据困难：如果想删除一个老旧组织，但它关联了成千上万的用户和日志，数据库会报错拒绝删除，必须手动处理复杂的级联关系。

    - 测试数据模拟难：在开发阶段，如果想快速伪造几条“提交记录”进行UI测试，外键会强迫必须先创建对应的“用户”和“题目”，增加了开发心智负担。

- 级联删除的风险
    - 外键支持级联删除。虽然方便，但极其危险。一旦误操作删除了一个父节点，可能会导致整个关联链路的数据瞬间被物理抹除，难以恢复。

### 典型任务场景

- 查询所有标签名称为 “Easy” 的题目 ID

$\Pi_{ProblemID}(\sigma_{TagName='Easy'}(Tag \bowtie ProblemTagRelation))$

- 查询用户 ID 为 10 的所有提交结果和耗时

$\Pi_{Result, TimeUsed}(\sigma_{UserID=10}(Submission))$

- 查询在“北京”地址（LocationIP 关联）登录过的用户名

$\Pi_{Username}(User \bowtie LoginLog \bowtie \sigma_{Address='北京'}(Location))$

- 查询参加了 ID 为 100 的竞赛的所有用户 ID

$\Pi_{UserID}(\sigma_{ContestID=100}(ContestParticipant))$

- 查询内存限制大于 128MB (131072 KB) 的题目描述

$\Pi_{Description}(\sigma_{MemoryLimit > 131072}(Problem))$

- 查找没有上级机构（根机构）的机构名称

$\Pi_{Name}(\sigma_{ParentOrgID = NULL}(Organization))$

-  查询竞赛“2023 Final”包含的所有题目 ID

$\Pi_{ProblemID}(ContestProblemRelation \bowtie \sigma_{ContestName='2023 Final'}(Contest))$

- 查询从未进行过任何提交的用户 ID

$\Pi_{UserID}(User) - \Pi_{UserID}(Submission)$

- 查询题目 ID 为 5 且结果为 'Accepted' 的所有提交 ID

$\Pi_{SubmissionID}(\sigma_{ProblemID=5 \land Result='Accepted'}(Submission))$

- 查询同时包含标签 ID 1 和标签 ID 2 的题目 ID

$\Pi_{ProblemID}(\sigma_{TagID=1}(ProblemTagRelation)) \cap \Pi_{ProblemID}(\sigma_{TagID=2}(ProblemTagRelation))$

- 参加了所有竞赛的用户 ID（除法运算）

$\Pi_{UserID, ContestID}(ContestParticipant) ÷ \Pi_{ContestID}(Contest)$

- 查询解决了竞赛 ID 为 100 的“所有”题目的用户 ID

$$\Pi_{UserID, ProblemID}(Submission \bowtie \sigma_{Result='Accepted'}(Submission)) \div \Pi_{ProblemID}(\sigma_{ContestID=100}(ContestProblemRelation))$$

- 查询从未在自己所属组织所在的 IP 地址登录过的用户 ID

$$\Pi_{UserID}(User) - \Pi_{UserID}(User \bowtie_{\text{User.OrgID = Org.OrgID}} Organization \bowtie_{\text{Org.LocationIP = Log.LocationIP}} LoginLog)$$

- 查询至少参加过两场不同竞赛的用户 ID

$$\Pi_{C_1.UserID}(\sigma_{C_1.UserID = C_2.UserID \land C_1.ContestID \neq C_2.ContestID}(C_1 \times C_2))$$

- 查询属于“二级机构”的用户提交过的题目 ID

$$\Pi_{ProblemID}(Submission \bowtie User \bowtie \sigma_{ParentOrgID \neq NULL}(Organization))$$

- 查询所有包含“动态规划”标签但不包含“数学”标签的题目 ID

$$\Pi_{ProblemID}(ProblemTagRelation \bowtie \sigma_{TagName='动态规划'}(Tag)) - \Pi_{ProblemID}(ProblemTagRelation \bowtie \sigma_{TagName='数学'}(Tag))$$

- 查询在所有登录过的位置中，至少有一个位置是“上海”的用户姓名

$$\Pi_{Username}(User \bowtie LoginLog \bowtie \sigma_{Address='上海'}(Location))$$

- 查询那些“所有提交记录均为 Accepted”的用户 ID

$$\Pi_{UserID}(Submission) - \Pi_{UserID}(\sigma_{Result \neq 'Accepted'}(Submission))$$

-  查询那些在竞赛期间之外对竞赛题目进行提交的用户 ID

$$\Pi_{UserID}(\sigma_{SubmitTime < StartTime \lor SubmitTime > EndTime}(Submission \bowtie Contest \bowtie ContestProblemRelation))$$

## SQL查询、数据库模式优化和实现

### 典型任务场景的 SQL 表达

-  查询所有标签名称为 “Easy” 的题目 ID 

```sql
SELECT DISTINCT R.ProblemID 
FROM Tag T JOIN ProblemTagRelation R ON T.TagID = R.TagID
WHERE T.TagName = 'Easy';
```

- 查询用户 ID 为 10 的所有提交结果和耗时

```sql
SELECT Result, TimeUsed 
FROM Submission 
WHERE UserID = 10;
```

- 查询在“北京”地址登录过的用户名

```sql
SELECT DISTINCT U.Username 
FROM User U 
JOIN LoginLog L ON U.UserID = L.UserID 
JOIN Location LOC ON L.LocationIP = LOC.LocationIP
WHERE LOC.Address = '北京';
```

- 查询参加了 ID 为 100 的竞赛的所有用户 ID

```sql
SELECT UserID 
FROM ContestParticipant 
WHERE ContestID = 100;
```

- 查询内存限制大于 128MB (131072 KB) 的题目描述


```sql
SELECT Description 
FROM Problem 
WHERE MemoryLimit > 131072;
```

- 查找没有上级机构（根机构）的机构名称


```sql
SELECT Name 
FROM Organization 
WHERE ParentOrgID IS NULL;
```


- 查询竞赛“2023 Final”包含的所有题目 ID


```sql
SELECT R.ProblemID 
FROM ContestProblemRelation R JOIN Contest C ON R.ContestID = C.ContestID
WHERE C.ContestName = '2023 Final';
```

- 查询从未进行过任何提交的用户 ID（差运算）

```sql
SELECT UserID FROM User
EXCEPT
SELECT UserID FROM Submission;
```

- 查询题目 ID 为 5 且结果为 'Accepted' 的所有提交 ID


```sql
SELECT SubmissionID 
FROM Submission 
WHERE ProblemID = 5 AND Result = 'Accepted';
```

- 查询同时包含标签 ID 1 和标签 ID 2 的题目 

```sql
SELECT ProblemID FROM ProblemTagRelation WHERE TagID = 1
INTERSECT
SELECT ProblemID FROM ProblemTagRelation WHERE TagID = 2;
```


- 查询参加了所有竞赛的用户 ID

```sql
SELECT UserID 
FROM User U
WHERE NOT EXISTS (
    SELECT ContestID FROM Contest
    EXCEPT
    SELECT ContestID FROM ContestParticipant CP WHERE CP.UserID = U.UserID
);
```

- 查询解决了竞赛 ID 为 100 的“所有”题目的用户 ID

```sql
SELECT S.UserID 
FROM Submission S
WHERE S.Result = 'Accepted' 
AND S.ProblemID IN (SELECT ProblemID FROM ContestProblemRelation WHERE ContestID = 100)
GROUP BY S.UserID
HAVING COUNT(DISTINCT S.ProblemID) = (SELECT COUNT(*) FROM ContestProblemRelation WHERE ContestID = 100);
```

- 查询从未在自己所属组织所在的 IP 地址登录过的用户 ID


```sql
SELECT UserID FROM User
EXCEPT
SELECT U.UserID 
FROM User U 
JOIN Organization O ON U.OrgID = O.OrgID 
JOIN LoginLog L ON U.UserID = L.UserID AND O.LocationIP = L.LocationIP;
```

- 查询至少参加过两场不同竞赛的用户 ID

```sql
SELECT DISTINCT C1.UserID 
FROM ContestParticipant C1, ContestParticipant C2 
WHERE C1.UserID = C2.UserID AND C1.ContestID != C2.ContestID;
```


- 查询属于“二级机构”的用户提交过的题目 ID


```sql
SELECT DISTINCT S.ProblemID 
FROM Submission S 
JOIN User U ON S.UserID = U.UserID 
JOIN Organization O ON U.OrgID = O.OrgID
WHERE O.ParentOrgID IS NOT NULL;
```

- 查询包含“动态规划”但不包含“数学”标签的题目 ID

```sql
SELECT R.ProblemID FROM ProblemTagRelation R JOIN Tag T ON R.TagID = T.TagID WHERE T.TagName = '动态规划'
EXCEPT
SELECT R.ProblemID FROM ProblemTagRelation R JOIN Tag T ON R.TagID = T.TagID WHERE T.TagName = '数学';
```

- 查询在所有登录位置中，至少有一个位置是“上海”的用户姓名

```sql
SELECT DISTINCT U.Username 
FROM User U 
JOIN LoginLog L ON U.UserID = L.UserID 
JOIN Location LOC ON L.LocationIP = LOC.LocationIP
WHERE LOC.Address = '上海';
```

- 查询那些“所有提交记录均为 Accepted”的用户 ID

```sql
SELECT UserID FROM Submission
EXCEPT
SELECT UserID FROM Submission WHERE Result != 'Accepted';
```

- 查询在竞赛期间之外对竞赛题目进行提交的用户 ID

```sql
SELECT DISTINCT S.UserID 
FROM Submission S 
JOIN Contest C ON S.ContestID = C.ContestID
JOIN ContestProblemRelation CPR ON S.ProblemID = CPR.ProblemID AND C.ContestID = CPR.ContestID
WHERE S.SubmitTime < C.StartTime OR S.SubmitTime > C.EndTime;
```

------

### 数据库模式设计评价与优化

虽然目前的表结构已经基本拆分，但仍存在以下潜在风险：

1. **更新异常**：在 `Organization` 表中，如果 `LocationIP` 发生变更（例如某个校区更换了出口 IP），由于 `Location` 描述依赖于 IP，若不慎在 `Organization` 中修改了 IP 而未同步 `Location` 表（或反之），会导致逻辑地址与物理地址脱节。

    优化方案：添加LocationID做主键。

    ```sql
    -- 1. 组织机构
    CREATE TABLE Organization (
        OrgID INTEGER PRIMARY KEY AUTOINCREMENT,
        Name TEXT NOT NULL,
        ParentOrgID INTEGER,
        LocationID INTEGER
    );
    
    -- 2. 位置
    CREATE TABLE Location (
        LocationID INTEGER PRIMARY KEY AUTOINCREMENT,
        LocationIP TEXT,
        Address TEXT
    );
    
    -- 4. 登录日志
    CREATE TABLE LoginLog (
        LogID INTEGER PRIMARY KEY AUTOINCREMENT,
        UserID INTEGER,
        LoginTime DATETIME DEFAULT CURRENT_TIMESTAMP,
        LocationID INTEGER
    );
    ```

    

2. **删除与插入异常**：通过不设置外键已经避免，逻辑交由应用处理。

### 函数依赖集

- **Organization (组织机构)**
    - $OrgID \to \{Name, ParentOrgID, LocationID\}$
    - *说明：通过组织 ID 可以唯一确定组织名称、父级组织及关联的位置 ID。*
- **Location (位置信息)**
    - $LocationID \to \{LocationIP, Address\}$
    - *说明：引入自增 ID 后，IP 地址不再作为主键，而是作为属性由 LocationID 决定。*
- **User (用户)**
    - $UserID \to \{Username, Password, OrgID\}$
- **LoginLog (登录日志)**
    - $LogID \to \{UserID, LoginTime, LocationID\}$
- **Problem (题目)**
    - $ProblemID \to \{Description, TimeLimit, MemoryLimit\}$
- **Submission (提交记录)**
    - $SubmissionID \to \{UserID, ProblemID, ContestID, SubmitTime, Result, TimeUsed, MemoryUsed\}$
- **Contest (竞赛)**
    - $ContestID \to \{ContestName, StartTime, EndTime\}$
- **Tag (标签)**
    - $TagID \to TagName$

------

### 关于范式的论证

通过单主属性与消灭外键，很显然的做到了“对于每一个非平凡的函数依赖，前件都是候选键”，所以达到BCNF。

- 优点

    - **彻底消除更新异常**：无论 IP 地址如何变动，或某个物理位置的名称（如“实验楼”更名为“图灵楼”）如何修改，都**只需更新一处**。

    - **数据零冗余**：每个事实（如地理位置、组织信息、用户信息）在数据库中只出现一次，极大地节省了存储空间并降低了数据不一致的风险。

    - **结构清晰**：表的功能高度解耦，`Location` 负责物理空间，`Organization` 负责行政逻辑，符合软件工程的“高内聚低耦合”原则。

- 缺点

    - **查询性能损耗（Join 较多）**：由于数据分散，查询一个用户的登录物理地址需要进行 $User \bowtie Organization \bowtie Location$ 三表连接。在高并发的 OJ 实时排名查询中，连接操作会带来额外的 CPU 开销。

    - **索引维护成本**：为了保证三表连接的速度，必须在 `OrgID`、`LocationID` 等所有外键关联字段上建立索引，这会稍微增加磁盘占用和插入数据的耗时。

    - **开发复杂度增加**：在后端代码中，不能直接从 `User` 对象获取 `Address`，必须编写更复杂的关联查询或在代码层进行多次查询。

### 索引优化后的SQL

```sql
-- 1. 位置表 (BCNF 核心：独立存储地理信息)
CREATE TABLE Location (
    LocationID INTEGER PRIMARY KEY AUTOINCREMENT,
    LocationIP TEXT NOT NULL,
    Address TEXT,
    -- 为 IP 建立唯一索引，防止重复录入同一网段
    CONSTRAINT idx_unique_ip UNIQUE (LocationIP)
);

-- 2. 组织机构表 (优化：LocationID 替代 LocationIP)
CREATE TABLE Organization (
    OrgID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    ParentOrgID INTEGER,
    LocationID INTEGER
);
-- 优化索引：加速组织层级查询及位置关联
CREATE INDEX idx_org_parent ON Organization(ParentOrgID);
CREATE INDEX idx_org_location ON Organization(LocationID);

-- 3. 用户表
CREATE TABLE User (
    UserID INTEGER PRIMARY KEY AUTOINCREMENT,
    Username TEXT NOT NULL UNIQUE,
    Password TEXT NOT NULL,
    OrgID INTEGER
);
-- 优化索引：加速登录验证及所属组织查询
CREATE INDEX idx_user_org ON User(OrgID);

-- 4. 登录日志表 (优化：关联 LocationID 而非 IP)
CREATE TABLE LoginLog (
    LogID INTEGER PRIMARY KEY AUTOINCREMENT,
    UserID INTEGER,
    LoginTime DATETIME DEFAULT CURRENT_TIMESTAMP,
    LocationID INTEGER
);
-- 优化索引：加速用户轨迹追踪
CREATE INDEX idx_log_user ON LoginLog(UserID);
CREATE INDEX idx_log_location ON LoginLog(LocationID);

-- 5. 题目表
CREATE TABLE Problem (
    ProblemID INTEGER PRIMARY KEY AUTOINCREMENT,
    Description TEXT,
    TimeLimit INTEGER,
    MemoryLimit INTEGER
);

-- 6. 标签表
CREATE TABLE Tag (
    TagID INTEGER PRIMARY KEY AUTOINCREMENT,
    TagName TEXT UNIQUE
);

-- 7. 题目标签关联表
CREATE TABLE ProblemTagRelation (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ProblemID INTEGER,
    TagID INTEGER
);
-- 优化索引：加速多对多标签检索
CREATE INDEX idx_ptr_problem ON ProblemTagRelation(ProblemID);
CREATE INDEX idx_ptr_tag ON ProblemTagRelation(TagID);

-- 8. 提交记录表 (核心表：数据量最大)
CREATE TABLE Submission (
    SubmissionID INTEGER PRIMARY KEY AUTOINCREMENT,
    UserID INTEGER,
    ProblemID INTEGER,
    ContestID INTEGER,
    SubmitTime DATETIME DEFAULT CURRENT_TIMESTAMP,
    Result TEXT,
    TimeUsed INTEGER,
    MemoryUsed INTEGER
);
-- 优化索引：复合索引加速特定竞赛下的题目统计，以及用户个人记录查询
CREATE INDEX idx_sub_user ON Submission(UserID);
CREATE INDEX idx_sub_contest_problem ON Submission(ContestID, ProblemID);
CREATE INDEX idx_sub_result ON Submission(Result); -- 加速统计 Accepted/WA 比例

-- 9. 竞赛表
CREATE TABLE Contest (
    ContestID INTEGER PRIMARY KEY AUTOINCREMENT,
    ContestName TEXT,
    StartTime DATETIME,
    EndTime DATETIME
);

-- 10. 竞赛题目关联表
CREATE TABLE ContestProblemRelation (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ContestID INTEGER,
    ProblemID INTEGER
);
CREATE INDEX idx_cpr_contest ON ContestProblemRelation(ContestID);

-- 11. 竞赛参与者表
CREATE TABLE ContestParticipant (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ContestID INTEGER,
    UserID INTEGER
);
CREATE INDEX idx_cp_contest_user ON ContestParticipant(ContestID, UserID);
```




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

注：同时改为 MySql 风格，加入更多NOT NULL约束

```sql
-- 1. 位置表
CREATE TABLE IF NOT EXISTS Location
(
    LocationID INT PRIMARY KEY AUTO_INCREMENT,
    LocationIP VARCHAR(45)  NOT NULL, -- 使用 VARCHAR(45) 以兼容 IPv6
    Address    VARCHAR(255) NOT NULL,
    UNIQUE INDEX idx_unique_ip (LocationIP)
) ENGINE = InnoDB;

-- 2. 组织机构表
CREATE TABLE IF NOT EXISTS Organization
(
    OrgID       INT PRIMARY KEY AUTO_INCREMENT,
    Name        VARCHAR(100) NOT NULL,
    ParentOrgID INT,
    LocationID  INT          NOT NULL,
    INDEX idx_org_parent (ParentOrgID),
    INDEX idx_org_location (LocationID)
) ENGINE = InnoDB;

-- 3. 用户表
CREATE TABLE IF NOT EXISTS User
(
    UserID   INT PRIMARY KEY AUTO_INCREMENT,
    Username VARCHAR(50)  NOT NULL UNIQUE,
    Password VARCHAR(255) NOT NULL,
    OrgID    INT,
    INDEX idx_user_org (OrgID)
) ENGINE = InnoDB;

-- 4. 登录日志表
CREATE TABLE IF NOT EXISTS LoginLog
(
    LogID      INT PRIMARY KEY AUTO_INCREMENT,
    UserID     INT NOT NULL,
    LoginTime  DATETIME DEFAULT CURRENT_TIMESTAMP,
    LocationID INT NOT NULL,
    INDEX idx_log_user (UserID),
    INDEX idx_log_location (LocationID)
) ENGINE = InnoDB;

-- 5. 题目表
CREATE TABLE IF NOT EXISTS Problem
(
    ProblemID   INT PRIMARY KEY AUTO_INCREMENT,
    Description TEXT NOT NULL,
    TimeLimit   INT,
    MemoryLimit INT
) ENGINE = InnoDB;

-- 6. 标签表
CREATE TABLE IF NOT EXISTS Tag
(
    TagID   INT PRIMARY KEY AUTO_INCREMENT,
    TagName VARCHAR(50) UNIQUE NOT NULL
) ENGINE = InnoDB;

-- 7. 题目标签关联表
CREATE TABLE IF NOT EXISTS ProblemTagRelation
(
    ID        INT PRIMARY KEY AUTO_INCREMENT,
    ProblemID INT NOT NULL,
    TagID     INT NOT NULL,
    INDEX idx_ptr_problem (ProblemID),
    INDEX idx_ptr_tag (TagID)
) ENGINE = InnoDB;

-- 8. 提交记录表
CREATE TABLE IF NOT EXISTS Submission
(
    SubmissionID INT PRIMARY KEY AUTO_INCREMENT,
    UserID       INT                                NOT NULL,
    ProblemID    INT                                NOT NULL,
    ContestID    INT,
    SubmitTime   DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    Result       VARCHAR(20),
    TimeUsed     INT                                NOT NULL,
    MemoryUsed   INT                                NOT NULL,
    INDEX idx_sub_user (UserID),
    INDEX idx_sub_contest_problem (ContestID, ProblemID),
    INDEX idx_sub_result (Result)
) ENGINE = InnoDB;

-- 9. 竞赛表
CREATE TABLE IF NOT EXISTS Contest
(
    ContestID   INT PRIMARY KEY AUTO_INCREMENT,
    ContestName VARCHAR(100) NOT NULL,
    StartTime   DATETIME,
    EndTime     DATETIME
) ENGINE = InnoDB;

-- 10. 竞赛题目关联表
CREATE TABLE IF NOT EXISTS ContestProblemRelation
(
    ID        INT PRIMARY KEY AUTO_INCREMENT,
    ContestID INT NOT NULL,
    ProblemID INT NOT NULL,
    INDEX idx_cpr_contest (ContestID)
) ENGINE = InnoDB;

-- 11. 竞赛参与者表
CREATE TABLE IF NOT EXISTS ContestParticipant
(
    ID        INT PRIMARY KEY AUTO_INCREMENT,
    ContestID INT NOT NULL,
    UserID    INT NOT NULL,
    INDEX idx_cp_contest_user (ContestID, UserID)
) ENGINE = InnoDB;
```

## Web 架构实现

使用MySQL8.0数据库，使用Rust作为后端，Vue作为前端

程序主要分为两部分：

- **SQL查询部分**：前端会发送用户的SQL请求到后端，由后端与数据库交互后返回并展示
- **现代查询方式演示**：后端会使用现代语言与工具的特性对数据库进行便捷访问，前端只需要通过特定接口即可获取数据

### 前端演示

进入网页后如图所示，左边的文本框可以进行SQL语句的编写，

![image-20251221082056721](./README/image-20251221082056721.png)

点击左下角的示例按钮可以快速填充SQL文本框，点击运行查询即可查询并在右边展示查询结果，

![image-20251221082315816](./README/image-20251221082315816.png)

右上角的按钮是调用后端提供的API来进行查询，更加现代与安全。

![image-20251221082350997](./README/image-20251221082350997.png)

### 数据库交互说明

对于常规的SQL语句转发请求，后端主要进行格式转换的工作，因为事先不可预知查询的数据表样式，对于Rust这类强类型语言来说较为繁琐

```rust
async fn handle_query(
    axum::extract::State(pool): axum::extract::State<MySqlPool>,
    Json(payload): Json<SqlRequest>,
) -> Json<serde_json::Value> {
    match sqlx::query(&payload.sql).fetch_all(&pool).await {
        Ok(rows) => {
            let mut results = Vec::new();
            for row in rows {
                let mut map = serde_json::Map::new();
                for column in row.columns() {
                    let name = column.name();
                    let value = if let Ok(v) = row.try_get::<String, _>(name) {
                        serde_json::Value::String(v)
                    } else if let Ok(v) = row.try_get::<i64, _>(name) {
                        serde_json::Value::Number(v.into())
                    } else if let Ok(v) = row.try_get::<f64, _>(name) {
                        serde_json::Number::from_f64(v)
                            .map(serde_json::Value::Number)
                            .unwrap_or(serde_json::Value::Null)
                    } else if let Ok(v) = row.try_get::<bool, _>(name) {
                        serde_json::Value::Bool(v)
                    } else if let Ok(v) = row.try_get::<chrono::NaiveDateTime, _>(name) {
                        serde_json::Value::String(v.to_string())
                    } else {
                        serde_json::Value::Null
                    };
                    map.insert(name.to_string(), value);
                }
                results.push(serde_json::Value::Object(map));
            }
            Json(serde_json::json!(results))
        }
        Err(e) => Json(serde_json::json!({ "error": e.to_string() })),
    }
}
```

然而现代语言及工具可以方便的将数据库映射到数据结构，例如下面的代码，对于已知返回结构的查询，可以将其绑定到特定的数据结构中，并且现代工具可以使用灵活的形式进行数据库的访问，例如：在Rust语言中，常常使用蛇形命名法，但数据库中不是，所以sqlx工具提供了rename注解，保持了语言自己的特色，同时解决了访问数据库时字段名称不一致的问题；另外，这种形式也可以对前端屏蔽部分敏感数据。

```rust
#[derive(Serialize, FromRow)]
pub struct User {
    #[sqlx(rename = "UserID")] // 匹配数据库的大写列名
    pub id: i32,
    #[sqlx(rename = "Username")]
    pub username: String,
    #[allow(dead_code)]
    #[serde(skip_serializing)] // 敏感信息不返回给前端
    #[sqlx(rename = "Password")]
    pub password: String,
    #[sqlx(rename = "OrgID")]
    pub org_id: Option<i32>,
}
// 获取所有用户
pub async fn get_users(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    match sqlx::query_as::<_, User>("SELECT * FROM User")
        .fetch_all(&pool)
        .await
    {
        Ok(users) => Json(serde_json::json!(users)),
        Err(e) => Json(serde_json::json!({ "error": e.to_string() })),
    }
}
```

## 列存储模式设计

### B+树设计

- 表的每一列单独存储为一个文件

- 对于每一列，存储其值与对应的行号

- 对于每个列，创建独立B+树，每个节点存储列值与行号，其中列值为键

- 建立行号到B+树节点指针的索引表

- 查询操作：二分定位到对应的节点，遍历其行号进行查询

- 插入操作：二分定位到对应的节点，如存在，新增行号，如不存在，新增节点；更新索引表

- 删除操作：使用索引表找到对应节点，删除其内的行号，删除后如果无行号，则删除节点

### 关系到文件映射

- 一个关系对应一个文件夹
- 一列对应一个文件

### 伪代码

- 数据结构说明

    ```rust
    // 逻辑行号类型
    type RowID = u64;
    
    // 每一列对应一个物理文件映射
    struct ColumnStore<K> {
        // 存储列值到行号的映射（B+树）
        bplus_tree: BPlusTree<K, Vec<RowID>>,
        // 方便通过 RowID 快速定位并删除数据
        rid_index: HashMap<RowID, LeafNodePointer>,
        // 每一列独立的文件句柄
        file_handle: File,
    }
    ```

- 查询

    ```rust
    impl<K: Ord> ColumnStore<K> {
        // 点查询：查找特定值的所有行号
        fn point_query(&self, target_value: K) -> Vec<RowID> {
            // 1. 二分定位：在B+树中搜索该键
            let leaf_node = self.bplus_tree.find_leaf(target_value);
            
            if let Some(node) = leaf_node {
                // 2. 遍历该节点的键值对，找到匹配的 RowID 集合
                if let Some(rids) = node.get_row_ids_for_key(target_value) {
                    return rids.clone();
                }
            }
            vec![] // 未找到
        }
    
        // 范围查询：二分定位起点，然后横向遍历
        fn range_query(&self, min_v: K, max_v: K) -> Vec<RowID> {
            let mut results = Vec::new();
            // 1. 定位到包含 min_v 的起始叶子节点
            let mut curr_node = self.bplus_tree.find_leaf(min_v);
    
            while let Some(node) = curr_node {
                for (key, rids) in node.iter_pairs() {
                    if key > max_v { return results; } // 超过范围
                    if key >= min_v {
                        results.extend(rids);
                    }
                }
                // 2. 沿着 B+ 树叶子节点的兄弟指针移动
                curr_node = node.next_sibling;
            }
            results
        }
    }
    ```

    - 插入操作

        ```rust
        impl<K: Ord> ColumnStore<K> {
            fn insert(&mut self, value: K, row_id: RowID) {
                // 1. 二分定位到叶子节点
                let (leaf_ptr, node) = self.bplus_tree.find_leaf_mut(value);
        
                if node.contains_key(value) {
                    // 情况 A: 键存在，直接在现有节点新增行号
                    node.add_rid_to_key(value, row_id);
                } else {
                    // 情况 B: 键不存在，新增节点（或在节点内新增键）
                    node.insert_new_key(value, row_id);
                    // 如果节点满了，执行 B+ 树分裂逻辑
                    if node.is_full() {
                        self.bplus_tree.split_and_rebalance(leaf_ptr);
                    }
                }
        
                // 2. 关键步骤：更新行号索引表，记录该 RowID 所在的节点指针
                self.rid_index.insert(row_id, leaf_ptr);
            }
        }
        ```

    - 删除操作

        ```rust
        impl<K: Ord> ColumnStore<K> {
            fn delete(&mut self, value: K, row_id: RowID) {
                // 1. 查找索引表：直接获取该行号所在的 B+ 树节点指针
                if let Some(leaf_ptr) = self.rid_index.get(&row_id) {
                    let node = self.bplus_tree.get_mut_node(leaf_ptr);
        
                    // 2. 在该节点内移除行号
                    node.remove_rid(value, row_id);
        
                    // 3. 检查：如果该 Key 下没有更多行号
                    if node.get_rids_count(value) == 0 {
                        node.remove_key(value); // 彻底移除该键
                        
                        // 4. B+ 树维护：如果节点因删除而过空，执行合并逻辑
                        if node.is_underflow() {
                            self.bplus_tree.coalesce_or_redistribute(leaf_ptr);
                        }
                    }
                    
                    // 5. 从索引表中移除该行记录
                    self.rid_index.remove(&row_id);
                }
            }
        }
        ```

        ### 更多思考

        上升到关系层级时，列存储模式的操作逻辑有更多的挑战，主要是如何保持不同列文件之间 RowID 的严格同步。

        可以设计：

        - 元数据管理器：维护表结构，记录该表包含哪些列，以及每列对应的物理文件路径。
        - 行号协调器：确保插入新行时，所有列使用相同的逻辑 RowID。
        - 行号索引表：现在需要管理 `RowID -> {ColA_NodePtr, ColB_NodePtr, ...}` 的映射。

        **伪代码如下：**

        - 关系点查询

        ```rust
        impl Relation {
            fn select(&self, projection: Vec<ColID>, condition_col: ColID, value: K) -> ResultSet {
                // 1. 在过滤列的 B+ 树中进行点查询，获取匹配的 RowID 集合
                let target_rids = self.columns[condition_col].point_query(value);
                
                let mut final_results = ResultSet::new();
        
                // 2. 根据获取的 RowID，分别去目标列中提取数据
                for rid in target_rids {
                    let mut row_data = Row::new();
                    for &col_id in &projection {
                        let val = self.columns[col_id].get_value_by_rid(rid);
                        row_data.push(val);
                    }
                    final_results.add_row(row_data);
                }
                final_results
            }
        }
        ```

        - 关系插入

        ```rust
        impl Relation {
            fn insert_row(&mut self, new_row: HashMap<ColID, Value>) -> Result<(), Error> {
                // 1. 生成全局唯一的递增行号
                let new_rid = self.generate_next_rid();
        
                // 2. 遍历表中所有列，同步执行插入
                for (col_id, col_index) in self.columns.iter_mut() {
                    let val = new_row.get(col_id).expect("Schema mismatch");
                    col_index.insert(val, new_rid);
                }
                
                Ok(())
            }
        }
        ```

        - 关系删除

        ```rust
        impl Relation {
            fn delete_rows(&mut self, condition_col: ColID, value: K) {
                // 1. 找到所有需要删除的 RowIDs
                let rids_to_delete = self.columns[condition_col].point_query(value);
        
                // 2. 级联删除：从所有列文件中移除这些行
                for rid in rids_to_delete {
                    for col_index in self.columns.values_mut() {
                        col_index.delete_by_rid(rid);
                    }
                }
            }
        }
        ```

        **优势与挑战：**

        - 优势：投影效率

        由于是列存储，如果一个表有 100 列，查询只涉及 2 列，该关系查询只会打开 2 个列文件。相比行存需要读取整行再抛弃无效字段，设计在 I/O 上节省了 **98%** 的开销。

        - 挑战：写放大

        由于每一列都有自己的 B+ 树和索引表，插入一行数据会导致多次磁盘随机 IO（每个列文件都要更新）。或许可以引入缓冲池：不直接修改磁盘上的 B+ 树，而是先将新行缓存在内存中，等积累到一定数量后，再批量刷入磁盘。

        ## 附录

### 前端核心代码

```vue
<template>
  <div class="sql-dashboard">
    <header class="navbar">
      <div class="logo">
        <span class="icon">⚡</span>
        <span class="text">OJ SQL Lab</span>
      </div>
      <div class="status-bar">
        <span class="status-dot"></span>
        Backend: Online
      </div>
    </header>

    <main class="content-wrapper">
      <section class="editor-pane">
        <div class="pane-header">SQL 请求编辑器</div>
        <div class="textarea-container">
          <textarea 
            v-model="sql" 
            spellcheck="false"
            placeholder="Write your SQL here..."
          ></textarea>
        </div>
        
        <button @click="runQuery" :disabled="loading" class="run-button">
          {{ loading ? '执行中...' : '运行查询' }}
        </button>

        <div class="examples-section">
          <div class="pane-header">一键示例</div>
          <div class="example-grid">
            <div 
              v-for="(item, idx) in examples" 
              :key="idx" 
              @click="sql = item.code"
              class="example-card"
            >
              {{ item.title }}
            </div>
          </div>
        </div>
      </section>

      <section class="result-pane">
        <div class="pane-header">一键打印</div>
        <div class="button-group">
          <button 
            v-for="(btn, idx) in button_groups" 
            :key="idx" 
            @click="fetchNative(btn.type)" 
            :disabled="loading" 
            class="run-button"
          >
            {{ btn.title }}
          </button>
        </div>
        <div class="pane-header">
          查询结果
          <span v-if="results.length" class="count-tag">{{ results.length }} rows</span>
        </div>
        
        <div class="table-container">
          <div v-if="error" class="error-box">
            <span class="error-title">SQL Error:</span>
            <p>{{ error }}</p>
          </div>

          <table v-else-if="results.length > 0">
            <thead>
              <tr>
                <th v-for="key in Object.keys(results[0])" :key="key">{{ key }}</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(row, i) in results" :key="i">
                <td v-for="(val, j) in Object.values(row)" :key="j">
                  <span :class="{ 'null-val': val === null }">
                    {{ val === null ? 'NULL' : val }}
                  </span>
                </td>
              </tr>
            </tbody>
          </table>

          <div v-else class="empty-state">
            <div class="empty-icon">📂</div>
            <p>没有数据，执行查询以查看结果。</p>
          </div>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup>
import { ref } from 'vue'

const sql = ref('-- 例：查找所有用户\nSELECT * FROM User;')
const results = ref([])
const error = ref(null)
const loading = ref(false)

const examples = [
  { title: "Easy题目", code: "SELECT DISTINCT R.ProblemID FROM Tag T JOIN ProblemTagRelation R ON T.TagID = R.TagID WHERE T.TagName = 'Easy';" },
  { title: "用户提交情况", code: "SELECT Result, TimeUsed FROM Submission WHERE UserID = 10;" },
  { title: "北京登录用户", code: "SELECT DISTINCT U.Username FROM User U JOIN LoginLog L ON U.UserID = L.UserID JOIN Location LOC ON L.LocationID = LOC.LocationID WHERE LOC.Address = '北京';" },
  { title: "竞赛参加者", code: "SELECT UserID FROM ContestParticipant WHERE ContestID = 1;" },
  { title: "内存限制查询", code: "SELECT Description FROM Problem WHERE MemoryLimit > 131072;" },
  { title: "根机构查询", code: "SELECT Name FROM Organization WHERE ParentOrgID IS NULL;" },
  { title: "竞赛题目", code: "SELECT R.ProblemID FROM ContestProblemRelation R JOIN Contest C ON R.ContestID = C.ContestID WHERE C.ContestName = '2023 Final';" },
  { title: "未提交用户", code: "SELECT UserID FROM User EXCEPT SELECT UserID FROM Submission;" },
  { title: "通过提交", code: "SELECT SubmissionID FROM Submission WHERE ProblemID = 1 AND Result = 'Accepted';" },
  { title: "交集标签", code: "SELECT ProblemID FROM ProblemTagRelation WHERE TagID = 1 INTERSECT SELECT ProblemID FROM ProblemTagRelation WHERE TagID = 3;" },
  { title: "全竞赛参加者", code: "SELECT UserID FROM User U WHERE NOT EXISTS (SELECT ContestID FROM Contest EXCEPT SELECT ContestID FROM ContestParticipant CP WHERE CP.UserID = U.UserID);" },
  { title: "竞赛全通", code: "SELECT S.UserID FROM Submission S WHERE S.Result = 'Accepted' AND S.ProblemID IN (SELECT ProblemID FROM ContestProblemRelation WHERE ContestID = 1) GROUP BY S.UserID HAVING COUNT(DISTINCT S.ProblemID) = (SELECT COUNT(*) FROM ContestProblemRelation WHERE ContestID = 1);" },
  { title: "异地登录", code: "SELECT UserID FROM User EXCEPT SELECT U.UserID FROM User U JOIN Organization O ON U.OrgID = O.OrgID JOIN LoginLog L ON U.UserID = L.UserID AND O.LocationID = L.LocationID;" },
  { title: "多竞赛参加", code: "SELECT DISTINCT C1.UserID FROM ContestParticipant C1, ContestParticipant C2 WHERE C1.UserID = C2.UserID AND C1.ContestID != C2.ContestID;" },
  { title: "二级机构题目", code: "SELECT DISTINCT S.ProblemID FROM Submission S JOIN User U ON S.UserID = U.UserID JOIN Organization O ON U.OrgID = O.OrgID WHERE O.ParentOrgID IS NOT NULL;" },
  { title: "标签差集", code: "SELECT R.ProblemID FROM ProblemTagRelation R JOIN Tag T ON R.TagID = T.TagID WHERE T.TagName = '动态规划' EXCEPT SELECT R.ProblemID FROM ProblemTagRelation R JOIN Tag T ON R.TagID = T.TagID WHERE T.TagName = '数学';" },
  { title: "上海登录用户", code: "SELECT DISTINCT U.Username FROM User U JOIN LoginLog L ON U.UserID = L.UserID JOIN Location LOC ON L.LocationID = LOC.LocationID WHERE LOC.Address = '上海';" },
  { title: "全通用户", code: "SELECT UserID FROM Submission EXCEPT SELECT UserID FROM Submission WHERE Result != 'Accepted';" },
  { title: "竞赛外提交", code: "SELECT DISTINCT S.UserID FROM Submission S JOIN Contest C ON S.ContestID = C.ContestID JOIN ContestProblemRelation CPR ON S.ProblemID = CPR.ProblemID AND C.ContestID = CPR.ContestID WHERE S.SubmitTime < C.StartTime OR S.SubmitTime > C.EndTime;" }
]

const button_groups = [
  { title: "用户", type: "users" },
  { title: "机构", type: "organizations" },
  { title: "竞赛", type: "contests" },
  { title: "标签", type: "tags" },
  { title: "日志", type: "login_logs" },
  { title: "位置", type: "locations" },
  { title: "题目", type: "problems" },
  { title: "提交", type: "submissions" },
  { title: "题目-标签", type: "problem_tag_relations" },
  { title: "竞赛-题目", type: "contest_problem_relations" },
  { title: "竞赛-用户", type: "contest_participants" }
]

async function runQuery() {
  loading.value = true
  error.value = null
  try {
    const res = await fetch('http://localhost:3000/api/query', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ sql: sql.value })
    })
    const data = await res.json()
    if (data.error) error.value = data.error
    else results.value = data
  } catch (err) {
    error.value = "Failed to connect to Rust backend at localhost:3000"
  } finally {
    loading.value = false
  }
}

// Vue 端的调用逻辑
async function fetchNative(type) {
  loading.value = true;
  error.value = null;
  try {
    const res = await fetch(`http://localhost:3000/api/${type}`);
    const data = await res.json();
    if (data.error) error.value = data.error;
    else results.value = data; // 结构体返回的 JSON 数组直接赋值给表格即可
  } catch (err) {
    error.value = "Connect failed";
  } finally {
    loading.value = false;
  }
}
</script>

<style scoped>
/* 基础重置 */
.sql-dashboard {
  height: 100vh;
  display: flex;
  flex-direction: column;
  font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
  background-color: #0f172a;
  color: #f1f5f9;
}

/* 顶部导航 */
.navbar {
  height: 60px;
  background-color: #1e293b;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 24px;
  border-bottom: 1px solid #334155;
}
.logo { display: flex; align-items: center; gap: 10px; font-weight: 800; font-size: 1.2rem; color: #38bdf8; }
.status-bar { font-size: 0.85rem; color: #94a3b8; display: flex; align-items: center; gap: 8px; }
.status-dot { width: 8px; height: 8px; background: #22c55e; border-radius: 50%; box-shadow: 0 0 8px #22c55e; }

/* 主体布局 */
.content-wrapper {
  flex: 1;
  display: flex;
  overflow: hidden; /* 关键：防止整体出现滚动条 */
}

/* 标题样式 */
.pane-header {
  padding: 12px 16px;
  font-size: 0.75rem;
  font-weight: 700;
  color: #ffffff;
  letter-spacing: 0.05em;
  background: #0f172a;
  border-radius: 10px;
}

/* 左侧编辑器 */
.editor-pane {
  width: 400px;
  border-right: 1px solid #334155;
  display: flex;
  flex-direction: column;
  padding: 16px;
  background: #1e293b;
}
.textarea-container { flex: 1; min-height: 200px; align-items: center; display: flex; justify-content: center; margin-top: 16px; }
textarea {
  width: 90%;
  height: 80%;
  background: #0f172a;
  border: 1px solid #334155;
  border-radius: 8px;
  color: #e2e8f0;
  padding: 16px;
  font-family: 'Fira Code', 'Monaco', monospace;
  font-size: 0.9rem;
  resize: none;
  outline: none;
}
textarea:focus { border-color: #38bdf8; }

.run-button {
  margin-top: 16px;
  padding: 14px;
  background: #38bdf8;
  color: #0f172a;
  border: none;
  border-radius: 8px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.2s;
}
.run-button:hover { background: #7dd3fc; transform: translateY(-1px); }
.run-button:disabled { background: #475569; cursor: not-allowed; }

.examples-section {
  flex: 1;
  overflow: auto;
  padding: 16px;
  scrollbar-width: thin;
  scrollbar-color: #475569 #1e293b;
}

.example-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
  margin-top: 8px;
}
.example-card {
  padding: 10px;
  background: #334155;
  border-radius: 6px;
  font-size: 0.75rem;
  cursor: pointer;
  text-align: center;
  transition: background 0.2s;
}
.example-card:hover { background: #475569; color: #38bdf8; }

/* 右侧结果集 */
.result-pane {
  flex: 1;
  display: flex;
  flex-direction: column;
  background: #0f172a;
}
.table-container {
  flex: 1;
  overflow: auto; /* 允许表格内部滚动 */
  padding: 0 16px 16px 16px;
  scrollbar-width: thin;
  scrollbar-color: #475569 #1e293b;
}

table {
  width: 100%;
  border-collapse: collapse;
  font-size: 0.85rem;
  color: #cbd5e1;
}
th {
  position: sticky; /* 表头吸顶 */
  top: 0;
  background: #1e293b;
  text-align: left;
  padding: 12px;
  border-bottom: 2px solid #334155;
  z-index: 10;
}
td {
  padding: 12px;
  border-bottom: 1px solid #1e293b;
}
tr:hover { background: #1e293b; }

.null-val { color: #64748b; font-style: italic; }
.count-tag { margin-left: 10px; color: #38bdf8; }

/* 状态展示 */
.error-box {
  background: #451a1a;
  border: 1px solid #7f1d1d;
  padding: 16px;
  border-radius: 8px;
  color: #fca5a5;
}
.error-title { font-weight: bold; display: block; margin-bottom: 8px; }

.empty-state {
  height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  color: #64748b;
}
.empty-icon { font-size: 3rem; margin-bottom: 16px; }

.button-group {
  display: flex;
  padding: 0 10px;
  gap: 8px;
}
.button-group .run-button {
  flex: 1;
}


</style>
```

### 后端核心代码

```rust
// main.rs
mod model;

use crate::model::{get_contest_participants, get_contest_problem_relations, get_contests, get_locations, get_login_logs, get_organizations, get_problem_tags, get_problems, get_submissions, get_tags, get_users};
use axum::routing::get;
use axum::{routing::post, Json, Router};
use serde::Deserialize;
use sqlx::mysql::MySqlPool;
use sqlx::{Column, Row};
use std::net::SocketAddr;
use tower_http::cors::CorsLayer;

#[derive(Deserialize)]
struct SqlRequest {
    sql: String,
}

#[tokio::main]
async fn main() {
    let database_url = "harkerhand://harkerhand:harkerhand@10.210.126.58/oj_db";
    let pool = MySqlPool::connect(database_url).await.unwrap();

    let app = Router::new()
        .route("/api/query", post(handle_query))
        .route("/api/users", get(get_users))
        .route("/api/submissions", get(get_submissions))
        .route("/api/organizations", get(get_organizations))
        .route("/api/contests", get(get_contests))
        .route("/api/tags", get(get_tags))
        .route("/api/login_logs", get(get_login_logs))
        .route("/api/locations", get(get_locations))
        .route("/api/problems", get(get_problems))
        .route("/api/problem_tag_relations", get(get_problem_tags))
        .route("/api/contest_problem_relations", get(get_contest_problem_relations))
        .route("/api/contest_participants", get(get_contest_participants))
        .layer(CorsLayer::permissive()) // 允许跨域
        .with_state(pool);

    let addr = SocketAddr::from(([127, 0, 0, 1], 3000));
    println!("监听端口: {}", addr);
    let listener = tokio::net::TcpListener::bind(addr).await.unwrap();
    axum::serve(listener, app).await.unwrap();
}

async fn handle_query(
    axum::extract::State(pool): axum::extract::State<MySqlPool>,
    Json(payload): Json<SqlRequest>,
) -> Json<serde_json::Value> {
    match sqlx::query(&payload.sql).fetch_all(&pool).await {
        Ok(rows) => {
            let mut results = Vec::new();
            for row in rows {
                let mut map = serde_json::Map::new();
                for column in row.columns() {
                    let name = column.name();
                    let value = if let Ok(v) = row.try_get::<String, _>(name) {
                        serde_json::Value::String(v)
                    } else if let Ok(v) = row.try_get::<i64, _>(name) {
                        serde_json::Value::Number(v.into())
                    } else if let Ok(v) = row.try_get::<f64, _>(name) {
                        serde_json::Number::from_f64(v)
                            .map(serde_json::Value::Number)
                            .unwrap_or(serde_json::Value::Null)
                    } else if let Ok(v) = row.try_get::<bool, _>(name) {
                        serde_json::Value::Bool(v)
                    } else if let Ok(v) = row.try_get::<chrono::NaiveDateTime, _>(name) {
                        serde_json::Value::String(v.to_string())
                    } else {
                        serde_json::Value::Null
                    };
                    map.insert(name.to_string(), value);
                }
                results.push(serde_json::Value::Object(map));
            }
            Json(serde_json::json!(results))
        }
        Err(e) => Json(serde_json::json!({ "error": e.to_string() })),
    }
}
// model.rs
use axum::extract::State;
use axum::Json;
use chrono::NaiveDateTime;
use serde::Serialize;
use sqlx::FromRow;
use sqlx::MySqlPool;

#[derive(Serialize, FromRow)]
pub struct User {
    #[sqlx(rename = "UserID")] // 匹配数据库的大写列名
    pub id: i32,
    #[sqlx(rename = "Username")]
    pub username: String,
    #[allow(dead_code)]
    #[serde(skip_serializing)] // 敏感信息不返回给前端
    #[sqlx(rename = "Password")]
    pub password: String,
    #[sqlx(rename = "OrgID")]
    pub org_id: Option<i32>,
}

#[derive(Serialize, FromRow)]
pub struct Organization {
    #[sqlx(rename = "OrgID")]
    pub id: i32,
    #[sqlx(rename = "Name")]
    pub name: String,
    #[sqlx(rename = "ParentOrgID")]
    pub parent_id: Option<i32>,
    #[sqlx(rename = "LocationID")]
    pub location_id: i32,
}

#[derive(Serialize, FromRow)]
pub struct Submission {
    #[sqlx(rename = "SubmissionID")]
    pub id: i32,
    #[sqlx(rename = "UserID")]
    pub user_id: i32,
    #[sqlx(rename = "ProblemID")]
    pub problem_id: i32,
    #[sqlx(rename = "Result")]
    pub result: String,
    #[sqlx(rename = "SubmitTime")]
    pub submit_time: NaiveDateTime,
}

#[derive(Serialize, FromRow)]
pub struct Location {
    #[sqlx(rename = "LocationID")]
    pub id: i32,
    #[sqlx(rename = "LocationIP")]
    pub ip: String,
    #[sqlx(rename = "Address")]
    pub address: String,
}

#[derive(Serialize, FromRow)]
pub struct Problem {
    #[sqlx(rename = "ProblemID")]
    pub id: i32,
    #[sqlx(rename = "Description")]
    pub description: String,
    #[sqlx(rename = "TimeLimit")]
    pub time_limit: i32,
    #[sqlx(rename = "MemoryLimit")]
    pub memory_limit: i32,
}

#[derive(Serialize, FromRow)]
pub struct Tag {
    #[sqlx(rename = "TagID")]
    pub id: i32,
    #[sqlx(rename = "TagName")]
    pub tag_name: String,
}

#[derive(Serialize, FromRow)]
pub struct Contest {
    #[sqlx(rename = "ContestID")]
    pub id: i32,
    #[sqlx(rename = "ContestName")]
    pub contest_name: String,
    #[sqlx(rename = "StartTime")]
    pub start_time: Option<NaiveDateTime>,
    #[sqlx(rename = "EndTime")]
    pub end_time: Option<NaiveDateTime>,
}

#[derive(Serialize, FromRow)]
pub struct ContestParticipant {
    #[sqlx(rename = "ID")]
    pub id: i32,
    #[sqlx(rename = "ContestID")]
    pub contest_id: i32,
    #[sqlx(rename = "UserID")]
    pub user_id: i32,
}

#[derive(Serialize, FromRow)]
pub struct LoginLog {
    #[sqlx(rename = "LogID")]
    pub id: i32,
    #[sqlx(rename = "UserID")]
    pub user_id: i32,
    #[sqlx(rename = "LoginTime")]
    pub login_time: NaiveDateTime,
    #[sqlx(rename = "LocationID")]
    pub location_id: i32,
}

#[derive(Serialize, FromRow)]
pub struct ProblemTag {
    #[sqlx(rename = "ID")]
    pub id: i32,
    #[sqlx(rename = "ProblemID")]
    pub problem_id: i32,
    #[sqlx(rename = "TagID")]
    pub tag_id: i32,
}

#[derive(Serialize, FromRow)]
pub struct ContestProblemRelation {
    #[sqlx(rename = "ID")]
    pub id: i32,
    #[sqlx(rename = "ContestID")]
    pub contest_id: i32,
    #[sqlx(rename = "ProblemID")]
    pub problem_id: i32,
}

// 获取所有用户
pub async fn get_users(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<User>(&pool, "SELECT * FROM User").await
}

// 获取所有机构
pub async fn get_organizations(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<Organization>(&pool, "SELECT * FROM Organization").await
}

// 获取所有提交记录
pub async fn get_submissions(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<Submission>(&pool, "SELECT * FROM Submission").await
}

// 获取所有题目
pub async fn get_problems(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<Problem>(&pool, "SELECT * FROM Problem").await
}

// 获取所有位置
pub async fn get_locations(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<Location>(&pool, "SELECT * FROM Location").await
}

// 获取所有竞赛
pub async fn get_contests(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<Contest>(&pool, "SELECT * FROM Contest").await
}

// 获取题目标签关联
pub async fn get_problem_tags(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<ProblemTag>(&pool, "SELECT * FROM ProblemTagRelation").await
}

pub async fn get_tags(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<Tag>(&pool, "SELECT * FROM Tag").await
}

pub async fn get_login_logs(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<LoginLog>(&pool, "SELECT * FROM LoginLog ORDER BY LoginTime DESC LIMIT 100").await
}

pub async fn get_contest_problem_relations(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<ContestProblemRelation>(&pool, "SELECT * FROM ContestProblemRelation").await
}

pub async fn get_contest_participants(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    fetch_all_as::<ContestParticipant>(&pool, "SELECT * FROM ContestParticipant").await
}


// 辅助函数：统一处理错误和序列化
pub async fn fetch_all_as<T>(pool: &MySqlPool, sql: &str) -> Json<serde_json::Value>
where
    T: for<'r> sqlx::FromRow<'r, sqlx::mysql::MySqlRow> + Serialize + Send + Unpin,
{
    match sqlx::query_as::<_, T>(sql).fetch_all(pool).await {
        Ok(items) => Json(serde_json::json!(items)),
        Err(e) => Json(serde_json::json!({ "error": e.to_string() })),
    }
}
```


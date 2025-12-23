-- 1. 位置数据
INSERT INTO Location (LocationIP, Address)
VALUES ('192.168.1.1', '北京'),
       ('192.168.1.2', '北京'),
       ('202.120.1.1', '上海'),
       ('202.112.1.1', '广州'),
       ('10.0.0.1', '深圳');

-- 2. 题目数据
INSERT INTO Problem (Description, TimeLimit, MemoryLimit)
VALUES ('A+B Problem', 1000, 65536),                -- ID 1
       ('Quick Sort Implementation', 2000, 131072), -- ID 2 (128MB)
       ('Dynamic Programming Task', 3000, 262144),  -- ID 3 (256MB)
       ('Math Geometry', 1500, 65536),              -- ID 4
       ('Easy Greed', 1000, 32768);
-- ID 5

-- 3. 标签数据
INSERT INTO Tag (TagName)
VALUES ('Easy'),
       ('动态规划'),
       ('数学'),
       ('排序');

-- 4. 题目标签关联
INSERT INTO ProblemTagRelation (ProblemID, TagID)
VALUES (1, 1),
       (1, 3), -- 题目1: Easy, 数学
       (2, 4), -- 题目2: 排序
       (3, 2), -- 题目3: 动态规划
       (5, 1);
-- 题目5: Easy

-- 5. 组织机构 (包含层级)
INSERT INTO Organization (Name, ParentOrgID, LocationID)
VALUES ('清华大学', NULL, 1), -- OrgID 1 (根机构, 北京)
       ('计算机系', 1, 1),    -- OrgID 2 (二级机构, 北京)
       ('复旦大学', NULL, 3), -- OrgID 3 (根机构, 上海)
       ('软件学院', 3, 3);
-- OrgID 4 (二级机构, 上海)

-- 6. 用户数据
INSERT INTO User (Username, Password, OrgID)
VALUES ('Alice', 'pass123', 2),    -- ID 1, 属于二级机构 (计算机系)
       ('Bob', 'pass123', 2),      -- ID 2, 属于二级机构
       ('Charlie', 'pass123', 4),-- ID 3, 属于二级机构 (软件学院)
       ('David', 'pass123', 1),    -- ID 4, 属于一级机构
       ('Eve', 'pass123', NULL),   -- ID 5, 无机构
       ('Frank', 'pass123', 2),    -- ID 6, 属于一级机构 (复旦大学)
       ('Grace', 'pass123', 3),    -- ID  7, 属于一级机构
       ('Heidi', 'pass123', NULL), -- ID 8, 无机构
       ('Ivan', 'pass123', 4),     -- ID 9, 属于二级机构
       ('Judy', 'pass123', 3);
-- ID 10, 属于一级机构


-- 7. 登录日志 (测试位置逻辑)
INSERT INTO LoginLog (UserID, LocationID)
VALUES (1, 1), -- Alice 在北京登录 (符合她所属机构)
       (2, 4), -- Bob 在深圳登录 (不符合他所属机构)
       (3, 3), -- Charlie 在上海登录 (符合)
       (1, 3);
-- Alice 也在上海登录过

-- 8. 竞赛数据
INSERT INTO Contest (ContestName, StartTime, EndTime)
VALUES ('2023 Final', '2023-12-01 09:00:00', '2023-12-01 12:00:00'), -- ID 1
       ('Spring Warmup', '2023-04-01 14:00:00', '2023-04-01 16:00:00');
-- ID 2

-- 9. 竞赛题目关联
INSERT INTO ContestProblemRelation (ContestID, ProblemID)
VALUES (1, 1),
       (1, 2),
       (1, 3), -- 2023 Final 有 1,2,3 三道题
       (2, 1),
       (2, 5);
-- Spring Warmup 有 1,5 两道题

-- 10. 竞赛参与者
INSERT INTO ContestParticipant (ContestID, UserID)
VALUES (1, 1),
       (1, 2),
       (1, 3), -- Alice, Bob, Charlie 参加了 Final
       (2, 1),
       (2, 4);
-- Alice, David 参加了 Warmup


-- 11. 提交记录 (Submission)
INSERT INTO Submission (UserID, ProblemID, ContestID, SubmitTime, Result, TimeUsed, MemoryUsed)
VALUES
-- Alice (ID 1): 解决了竞赛 1 的所有题
(1, 1, 1, '2023-12-01 10:00:00', 'Accepted', 100, 2048),
(1, 2, 1, '2023-12-01 10:30:00', 'Accepted', 200, 4096),
(1, 3, 1, '2023-12-01 11:00:00', 'Accepted', 500, 8192),

-- Bob (ID 2): 竞赛 1 错了一题，且有一次在竞赛时间外提交
(2, 1, 1, '2023-12-01 10:00:00', 'Accepted', 110, 2048),
(2, 2, 1, '2023-11-30 20:00:00', 'Wrong Answer', 0, 0), -- 提前提交 (非竞赛时间)
(2, 2, 1, '2023-12-01 10:15:00', 'Accepted', 150, 4000),

-- Charlie (ID 3): 只有 Accepted，但没做完
(3, 1, 1, '2023-12-01 10:00:00', 'Accepted', 120, 2048),

-- David (ID 4): 有 Wrong Answer 记录
(4, 1, 2, '2023-04-01 14:30:00', 'Accepted', 100, 2048),
(4, 5, 2, '2023-04-01 15:00:00', 'Wrong Answer', 50, 1024),

-- 注：Frank (ID 6) 从未提交过，用于测试 EXCEPT UserID FROM Submission

-- Judy (ID 10): 有多次提交记录
(10, 1, NULL, '2023-05-01 10:00:00', 'Wrong Answer', 0, 0),
(10, 1, NULL, '2023-05-01 10:10:00', 'Accepted', 90, 2000),
(10, 4, NULL, '2023-05-02 11:00:00', 'Accepted', 140, 3000);
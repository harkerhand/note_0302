-- 1. 位置表
CREATE TABLE IF NOT EXISTS Location (
                          LocationID INT PRIMARY KEY AUTO_INCREMENT,
                          LocationIP VARCHAR(45) NOT NULL, -- 使用 VARCHAR(45) 以兼容 IPv6
                          Address VARCHAR(255),
                          UNIQUE INDEX idx_unique_ip (LocationIP)
) ENGINE=InnoDB;

-- 2. 组织机构表
CREATE TABLE IF NOT EXISTS  Organization (
                              OrgID INT PRIMARY KEY AUTO_INCREMENT,
                              Name VARCHAR(100) NOT NULL,
                              ParentOrgID INT,
                              LocationID INT,
                              INDEX idx_org_parent (ParentOrgID),
                              INDEX idx_org_location (LocationID)
) ENGINE=InnoDB;

-- 3. 用户表
CREATE TABLE IF NOT EXISTS User (
                      UserID INT PRIMARY KEY AUTO_INCREMENT,
                      Username VARCHAR(50) NOT NULL UNIQUE,
                      Password VARCHAR(255) NOT NULL,
                      OrgID INT,
                      INDEX idx_user_org (OrgID)
) ENGINE=InnoDB;

-- 4. 登录日志表
CREATE TABLE IF NOT EXISTS  LoginLog (
                          LogID INT PRIMARY KEY AUTO_INCREMENT,
                          UserID INT,
                          LoginTime DATETIME DEFAULT CURRENT_TIMESTAMP,
                          LocationID INT,
                          INDEX idx_log_user (UserID),
                          INDEX idx_log_location (LocationID)
) ENGINE=InnoDB;

-- 5. 题目表
CREATE TABLE IF NOT EXISTS  Problem (
                         ProblemID INT PRIMARY KEY AUTO_INCREMENT,
                         Description TEXT,
                         TimeLimit INT,
                         MemoryLimit INT
) ENGINE=InnoDB;

-- 6. 标签表
CREATE TABLE IF NOT EXISTS  Tag (
                     TagID INT PRIMARY KEY AUTO_INCREMENT,
                     TagName VARCHAR(50) UNIQUE
) ENGINE=InnoDB;

-- 7. 题目标签关联表
CREATE TABLE IF NOT EXISTS  ProblemTagRelation (
                                    ID INT PRIMARY KEY AUTO_INCREMENT,
                                    ProblemID INT,
                                    TagID INT,
                                    INDEX idx_ptr_problem (ProblemID),
                                    INDEX idx_ptr_tag (TagID)
) ENGINE=InnoDB;

-- 8. 提交记录表
CREATE TABLE IF NOT EXISTS  Submission (
                            SubmissionID INT PRIMARY KEY AUTO_INCREMENT,
                            UserID INT,
                            ProblemID INT,
                            ContestID INT,
                            SubmitTime DATETIME DEFAULT CURRENT_TIMESTAMP,
                            Result VARCHAR(20),
                            TimeUsed INT,
                            MemoryUsed INT,
                            INDEX idx_sub_user (UserID),
                            INDEX idx_sub_contest_problem (ContestID, ProblemID),
                            INDEX idx_sub_result (Result)
) ENGINE=InnoDB;

-- 9. 竞赛表
CREATE TABLE IF NOT EXISTS  Contest (
                         ContestID INT PRIMARY KEY AUTO_INCREMENT,
                         ContestName VARCHAR(100),
                         StartTime DATETIME,
                         EndTime DATETIME
) ENGINE=InnoDB;

-- 10. 竞赛题目关联表
CREATE TABLE IF NOT EXISTS  ContestProblemRelation (
                                        ID INT PRIMARY KEY AUTO_INCREMENT,
                                        ContestID INT,
                                        ProblemID INT,
                                        INDEX idx_cpr_contest (ContestID)
) ENGINE=InnoDB;

-- 11. 竞赛参与者表
CREATE TABLE IF NOT EXISTS  ContestParticipant (
                                    ID INT PRIMARY KEY AUTO_INCREMENT,
                                    ContestID INT,
                                    UserID INT,
                                    INDEX idx_cp_contest_user (ContestID, UserID)
) ENGINE=InnoDB;
-- 一次性清空所有表并重置自增 ID
TRUNCATE TABLE Location;
TRUNCATE TABLE Organization;
TRUNCATE TABLE User;
TRUNCATE TABLE LoginLog;
TRUNCATE TABLE Problem;
TRUNCATE TABLE Tag;
TRUNCATE TABLE ProblemTagRelation;
TRUNCATE TABLE Submission;
TRUNCATE TABLE Contest;
TRUNCATE TABLE ContestProblemRelation;
TRUNCATE TABLE ContestParticipant;


DROP TABLE Location;
DROP TABLE Organization;
DROP TABLE User;
DROP TABLE LoginLog;
DROP TABLE Problem;
DROP TABLE Tag;
DROP TABLE ProblemTagRelation;
DROP TABLE Submission;
DROP TABLE Contest;
DROP TABLE ContestProblemRelation;
DROP TABLE ContestParticipant;
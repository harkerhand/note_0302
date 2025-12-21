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
    pub location_id: Option<i32>,
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


// --- 基础实体表 ---

#[derive(Serialize, FromRow)]
pub struct Location {
    #[sqlx(rename = "LocationID")]
    pub id: i32,
    #[sqlx(rename = "LocationIP")]
    pub ip: String,
    #[sqlx(rename = "Address")]
    pub address: Option<String>,
}

#[derive(Serialize, FromRow)]
pub struct Problem {
    #[sqlx(rename = "ProblemID")]
    pub id: i32,
    #[sqlx(rename = "Description")]
    pub description: Option<String>,
    #[sqlx(rename = "TimeLimit")]
    pub time_limit: Option<i32>,
    #[sqlx(rename = "MemoryLimit")]
    pub memory_limit: Option<i32>,
}

#[derive(Serialize, FromRow)]
pub struct Tag {
    #[sqlx(rename = "TagID")]
    pub id: i32,
    #[sqlx(rename = "TagName")]
    pub tag_name: Option<String>,
}

#[derive(Serialize, FromRow)]
pub struct Contest {
    #[sqlx(rename = "ContestID")]
    pub id: i32,
    #[sqlx(rename = "ContestName")]
    pub contest_name: Option<String>,
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
    pub contest_id: Option<i32>,
    #[sqlx(rename = "UserID")]
    pub user_id: Option<i32>,
}

// --- 日志与关联表 ---

#[derive(Serialize, FromRow)]
pub struct LoginLog {
    #[sqlx(rename = "LogID")]
    pub id: i32,
    #[sqlx(rename = "UserID")]
    pub user_id: Option<i32>,
    #[sqlx(rename = "LoginTime")]
    pub login_time: Option<NaiveDateTime>,
    #[sqlx(rename = "LocationID")]
    pub location_id: Option<i32>,
}

#[derive(Serialize, FromRow)]
pub struct ProblemTag {
    #[sqlx(rename = "ID")]
    pub id: i32,
    #[sqlx(rename = "ProblemID")]
    pub problem_id: Option<i32>,
    #[sqlx(rename = "TagID")]
    pub tag_id: Option<i32>,
}

#[derive(Serialize, FromRow)]
pub struct ContestProblemRelation {
    #[sqlx(rename = "ID")]
    pub id: i32,
    #[sqlx(rename = "ContestID")]
    pub contest_id: Option<i32>,
    #[sqlx(rename = "ProblemID")]
    pub problem_id: Option<i32>,
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

// 获取所有机构
pub async fn get_organizations(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    match sqlx::query_as::<_, Organization>("SELECT * FROM Organization")
        .fetch_all(&pool)
        .await
    {
        Ok(orgs) => Json(serde_json::json!(orgs)),
        Err(e) => Json(serde_json::json!({ "error": e.to_string() })),
    }
}

// 获取所有提交记录
pub async fn get_submissions(State(pool): State<MySqlPool>) -> Json<serde_json::Value> {
    match sqlx::query_as::<_, Submission>("SELECT * FROM Submission ORDER BY SubmitTime DESC LIMIT 100")
        .fetch_all(&pool)
        .await
    {
        Ok(subs) => Json(serde_json::json!(subs)),
        Err(e) => Json(serde_json::json!({ "error": e.to_string() })),
    }
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
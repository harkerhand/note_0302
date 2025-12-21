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

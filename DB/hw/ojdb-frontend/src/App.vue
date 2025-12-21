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
          {{ loading ? 'EXECUTING...' : 'RUN QUERY' }}
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
            <p>No data to display. Execute a query to see results.</p>
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
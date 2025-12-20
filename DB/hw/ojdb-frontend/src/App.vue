<template>
  <div class="sql-dashboard">
    <header class="navbar">
      <div class="logo">
        <span class="icon">⚡</span>
        <span class="text">OJ SQL Lab</span>
      </div>
      <div class="status-bar">
        <span class="status-dot"></span>
        Rust Backend: Online
      </div>
    </header>

    <main class="content-wrapper">
      <section class="editor-pane">
        <div class="pane-header">SQL QUERY EDITOR</div>
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
          <div class="pane-header mini">SCENARIO EXAMPLES</div>
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
        <div class="pane-header">
          QUERY RESULTS 
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

const sql = ref('-- Select all users to start\nSELECT * FROM User;')
const results = ref([])
const error = ref(null)
const loading = ref(false)

const examples = [
  { title: "Easy Problems", code: "SELECT DISTINCT R.ProblemID FROM Tag T JOIN ProblemTagRelation R ON T.TagID = R.TagID WHERE T.TagName = 'Easy';" },
  { title: "Inactive Users", code: "SELECT UserID FROM User EXCEPT SELECT UserID FROM Submission;" },
  { title: "Beijing Logins", code: "SELECT DISTINCT U.Username FROM User U JOIN LoginLog L ON U.UserID = L.UserID JOIN Location LOC ON L.LocationIP = LOC.LocationIP WHERE LOC.Address = '北京';" },
  { title: "Sub-Orgs", code: "SELECT Name FROM Organization WHERE ParentOrgID IS NOT NULL;" }
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
  color: #64748b;
  letter-spacing: 0.05em;
  background: #0f172a;
}
.pane-header.mini { margin-top: 20px; }

/* 左侧编辑器 */
.editor-pane {
  width: 400px;
  border-right: 1px solid #334155;
  display: flex;
  flex-direction: column;
  padding: 16px;
  background: #1e293b;
}
.textarea-container { flex: 1; min-height: 200px; }
textarea {
  width: 100%;
  height: 100%;
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
</style>
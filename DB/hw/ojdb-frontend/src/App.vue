<template>
  <div class="p-6 max-w-7xl mx-auto flex flex-col h-screen gap-4">
    <h1 class="text-2xl font-bold border-b pb-2">OJ SQL 演练台</h1>
    
    <div class="flex flex-1 gap-6 overflow-hidden">
      <div class="w-1/3 flex flex-col gap-4">
        <div class="flex-1 flex flex-col">
          <label class="font-medium mb-1">SQL 编辑器:</label>
          <textarea 
            v-model="sql" 
            class="flex-1 p-3 font-mono text-sm border rounded bg-gray-50 focus:ring-2 focus:ring-blue-500"
            placeholder="在此输入 SQL..."
          ></textarea>
        </div>
        
        <button @click="runQuery" class="bg-green-600 text-white py-2 rounded hover:bg-green-700 font-bold">
          运行查询 (Execute)
        </button>

        <div class="mt-4">
          <h3 class="font-medium mb-2 text-gray-700">场景示例:</h3>
          <div class="grid grid-cols-1 gap-2">
            <button v-for="(item, idx) in examples" :key="idx" 
                    @click="sql = item.code"
                    class="text-left text-xs p-2 border rounded hover:bg-blue-50 transition">
              {{ item.title }}
            </button>
          </div>
        </div>
      </div>

      <div class="w-2/3 border rounded bg-white overflow-auto p-4 shadow-inner">
        <div v-if="error" class="text-red-500 font-mono text-sm">{{ error }}</div>
        <table v-else-if="results.length > 0" class="min-w-full text-sm">
          <thead class="bg-gray-100">
            <tr>
              <th v-for="key in Object.keys(results[0])" :key="key" class="border p-2 text-left">{{ key }}</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="(row, i) in results" :key="i" class="hover:bg-gray-50">
              <td v-for="val in Object.values(row)" :key="val" class="border p-2">{{ val ?? 'NULL' }}</td>
            </tr>
          </tbody>
        </table>
        <div v-else class="text-gray-400 text-center mt-20">暂无数据，请运行 SQL 查询</div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

const sql = ref('-- 查询所有题目\nSELECT * FROM Problem;')
const results = ref([])
const error = ref(null)

const examples = [
  { title: "所有 Easy 标签的题目", code: "SELECT DISTINCT R.ProblemID FROM Tag T JOIN ProblemTagRelation R ON T.TagID = R.TagID WHERE T.TagName = 'Easy';" },
  { title: "从未提交的用户 (EXCEPT)", code: "SELECT UserID FROM User EXCEPT SELECT UserID FROM Submission;" },
  { title: "解决了竞赛1所有题的用户", code: "SELECT S.UserID FROM Submission S WHERE S.Result = 'Accepted' AND S.ProblemID IN (SELECT ProblemID FROM ContestProblemRelation WHERE ContestID = 1) GROUP BY S.UserID HAVING COUNT(DISTINCT S.ProblemID) = (SELECT COUNT(*) FROM ContestProblemRelation WHERE ContestID = 1);" },
  { title: "查询二级机构用户", code: "SELECT U.Username, O.Name FROM User U JOIN Organization O ON U.OrgID = O.OrgID WHERE O.ParentOrgID IS NOT NULL;" }
]

async function runQuery() {
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
    error.value = "无法连接到 Rust 后端"
  }
}
</script>
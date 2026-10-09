const API = "";

// Переключение вкладок
document.querySelectorAll(".tab").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(b => b.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById("tab-" + btn.dataset.tab).classList.add("active");
  });
});

async function post(path, body) {
  let res;
  try {
    res = await fetch(API + path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body)
    });
  } catch (e) {
    throw new Error("Сервер недоступен. Запущен ли aicoach.exe? " + e.message);
  }
  const text = await res.text();
  let data;
  try { data = JSON.parse(text); }
  catch { throw new Error("Сервер вернул не-JSON: " + text.slice(0, 200)); }
  if (!res.ok) throw new Error("HTTP " + res.status + ": " + JSON.stringify(data));
  return data;
}

function el(tag, cls, text) {
  const e = document.createElement(tag);
  if (cls) e.className = cls;
  if (text !== undefined) e.textContent = text;
  return e;
}

function pick(data, ...names) {
  for (const n of names) if (data[n]) return data[n];
  return null;
}

function loadingBox(text) {
  const p = el("p", "loading-box");
  p.appendChild(el("span", "loading"));
  p.appendChild(document.createTextNode(text));
  return p;
}

function renderList(title, arr) {
  const box = el("div", "card");
  const h = el("h3");
  h.appendChild(document.createTextNode(title));
  const btn = el("button", "copy", "📋 Копировать");
  btn.addEventListener("click", () => {
    const text = (arr || []).map(x => typeof x === "string" ? x : JSON.stringify(x)).join("\n");
    navigator.clipboard.writeText(text);
    btn.textContent = "✓ Скопировано";
    setTimeout(() => btn.textContent = "📋 Копировать", 1500);
  });
  h.appendChild(btn);
  box.appendChild(h);
  if (!arr || arr.length === 0) {
    box.appendChild(el("p", "empty", "(пусто)"));
  } else {
    const ul = el("ul");
    arr.forEach(item => {
      let text;
      if (typeof item === "string") text = item;
      else if (typeof item === "object" && item !== null) {
        const vals = Object.values(item);
        text = vals.map(v => (typeof v === "string" ? v : JSON.stringify(v))).join(": ");
      } else text = String(item);
      ul.appendChild(el("li", null, text));
    });
    box.appendChild(ul);
  }
  return box;
}

function renderText(title, text) {
  const box = el("div", "card");
  const h = el("h3");
  h.appendChild(document.createTextNode(title));
  const btn = el("button", "copy", "📋 Копировать");
  btn.addEventListener("click", () => {
    navigator.clipboard.writeText(text || "");
    btn.textContent = "✓ Скопировано";
    setTimeout(() => btn.textContent = "📋 Копировать", 1500);
  });
  h.appendChild(btn);
  box.appendChild(h);
  box.appendChild(el("p", null, text || "(пусто)"));
  return box;
}

function renderObject(title, obj) {
  const box = el("div", "card");
  box.appendChild(el("h3", null, title));
  if (!obj || Object.keys(obj).length === 0) {
    box.appendChild(el("p", "empty", "(пусто)"));
  } else {
    const ul = el("ul");
    for (const [k, v] of Object.entries(obj)) {
      ul.appendChild(el("li", null, k + ": " + (typeof v === "string" ? v : JSON.stringify(v))));
    }
    box.appendChild(ul);
  }
  return box;
}

async function loadTasks() {
  const out = document.getElementById("tasks");
  out.innerHTML = "";
  out.appendChild(loadingBox("Загружаю задания..."));
  try {
    const data = await post("/api/tasks", {
      exam: document.getElementById("exam").value,
      section: document.getElementById("section").value,
      difficulty: document.getElementById("difficulty").value,
      count: 30
    });
    out.innerHTML = "";
    if (!data.tasks || !data.tasks.length) { out.textContent = "Ничего не найдено."; return; }
    data.tasks.forEach(t => {
      const box = el("div", "card");
      box.appendChild(el("h3", null, t.section + " [" + t.difficulty + "]"));
      box.appendChild(el("p", null, t.prompt));
      if (t.meta) box.appendChild(el("small", null, JSON.stringify(t.meta)));
      out.appendChild(box);
    });
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

async function coachEssay() {
  const out = document.getElementById("essayOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("Анализирую эссе... (30–60 секунд)"));
  try {
    const data = await post("/api/essay/coach", {
      user_id: "u1",
      university: document.getElementById("uni").value,
      essay_type: document.getElementById("essayType").value,
      prompt: document.getElementById("prompt").value,
      draft: document.getElementById("draft").value
    });
    out.innerHTML = "";
    if (data.error) { out.appendChild(renderText("Ошибка", data.error + (data.raw ? "\n\nСырой ответ: " + data.raw : ""))); return; }
    out.appendChild(renderList("📋 План эссе", pick(data, "outline", "план", "схема", "plan", "структура")));
    out.appendChild(renderList("💭 Вопросы для размышления", pick(data, "reflection_questions", "вопросы-размышления", "вопросы")));
    out.appendChild(renderList("✅ Чек-лист для университета", pick(data, "university_checklist", "чек-лист", "университет")));
    out.appendChild(renderList("⚠️ Слабые места", pick(data, "weak_points", "слабые_места", "слабые")));
    out.appendChild(renderList("🎭 Заметки о тоне", pick(data, "tone_notes", "тон", "tone")));
    out.appendChild(renderList("📚 Ресурсы", pick(data, "resources", "ресурсы")));
    if (data.word_budget && Object.keys(data.word_budget).length) out.appendChild(renderObject("📊 Бюджет слов", data.word_budget));
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

async function makePlan() {
  const out = document.getElementById("planOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("Составляю план... (30–60 секунд)"));
  try {
    const data = await post("/api/study-plan", {
      user_id: "u1",
      target_exam: document.getElementById("pExam").value,
      target_score: parseFloat(document.getElementById("pScore").value),
      current_level: document.getElementById("pLevel").value,
      native_language: document.getElementById("pNative").value,
      weeks_available: parseInt(document.getElementById("pWeeks").value),
      hours_per_week: parseFloat(document.getElementById("pHours").value)
    });
    out.innerHTML = "";
    if (data.error) { out.appendChild(renderText("Ошибка", data.error)); return; }
    out.appendChild(renderList("🎯 Цели по неделям", pick(data, "weekly_goals", "цели", "еженедельные_цели")));
    out.appendChild(renderList("📅 Ежедневные задачи", pick(data, "daily_tasks", "ежедневные_задачи", "задачи")));
    out.appendChild(renderList("📚 Ресурсы", pick(data, "resources", "ресурсы")));
    out.appendChild(renderText("💡 Обоснование", pick(data, "rationale", "обоснование", "reasoning")));
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

async function buildPortfolio() {
  const out = document.getElementById("pfOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("Собираю портфолио... (30–60 секунд)"));
  const unis = document.getElementById("pfUnis").value.split(",").map(s => s.trim()).filter(Boolean);
  try {
    const data = await post("/api/portfolio", {
      user_id: "u1",
      major: document.getElementById("pfMajor").value,
      universities: unis,
      target_exam: document.getElementById("pfExam").value,
      target_score: parseFloat(document.getElementById("pfScore").value),
      weeks_available: parseInt(document.getElementById("pfWeeks").value)
    });
    out.innerHTML = "";
    if (data.error) { out.appendChild(renderText("Ошибка", data.error)); return; }
    out.appendChild(renderList("📂 Разделы портфолио", pick(data, "sections", "разделы", "секции")));
    out.appendChild(renderList("🎯 Что делать", pick(data, "action_items", "действия", "шаги", "задачи")));
    if (data.university_requirements && Object.keys(data.university_requirements).length) {
      out.appendChild(renderObject("🏛 Требования вузов", data.university_requirements));
    }
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

// ===== Переключатель языка =====
const I18N = {
  ru: {
    title: "AI Coach — IELTS / SAT / Поступление",
    header_title: "🎓 AI Coach",
    header_sub: "Подготовка к IELTS / SAT, эссе, портфолио, поступление за границу",
    tab_tasks: "📚 Задания",
    tab_essay: "✍️ Эссе-коуч",
    tab_plan: "🗓 План тренировок",
    tab_portfolio: "🎯 Портфолио",
    tasks_h2: "Задания IELTS / SAT",
    exam: "Экзамен",
    section: "Секция",
    difficulty: "Сложность",
    btn_get: "Получить",
    essay_h2: "Разбор эссе (без написания за вас)",
    university: "Университет",
    essay_type: "Тип эссе",
    prompt: "Промпт",
    draft: "Черновик",
    btn_analyze: "Разобрать",
    plan_h2: "Индивидуальный план тренировок",
    target_score: "Целевой балл",
    current_level: "Текущий уровень",
    native_lang: "Родной язык",
    weeks: "Недель",
    hours: "Часов в неделю",
    btn_make_plan: "Составить план",
    portfolio_h2: "Портфолио для поступления",
    major: "Специальность",
    universities: "Вузы (через запятую)",
    target_exam: "Целевой экзамен",
    weeks_to_deadline: "Недель до дедлайна",
    btn_build_portfolio: "Собрать план портфолио"
  },
  en: {
    title: "AI Coach — IELTS / SAT / Admissions",
    header_title: "🎓 AI Coach",
    header_sub: "IELTS / SAT prep, essays, portfolio, study abroad admissions",
    tab_tasks: "📚 Tasks",
    tab_essay: "✍️ Essay Coach",
    tab_plan: "🗓 Study Plan",
    tab_portfolio: "🎯 Portfolio",
    tasks_h2: "IELTS / SAT Tasks",
    exam: "Exam",
    section: "Section",
    difficulty: "Difficulty",
    btn_get: "Get",
    essay_h2: "Essay Analysis (no writing for you)",
    university: "University",
    essay_type: "Essay type",
    prompt: "Prompt",
    draft: "Draft",
    btn_analyze: "Analyze",
    plan_h2: "Personal Study Plan",
    target_score: "Target score",
    current_level: "Current level",
    native_lang: "Native language",
    weeks: "Weeks",
    hours: "Hours per week",
    btn_make_plan: "Make plan",
    portfolio_h2: "Admissions Portfolio",
    major: "Major",
    universities: "Universities (comma-separated)",
    target_exam: "Target exam",
    weeks_to_deadline: "Weeks to deadline",
    btn_build_portfolio: "Build portfolio"
  }
};

function setLang(lang) {
  const dict = I18N[lang] || I18N.ru;
  document.querySelectorAll("[data-i18n]").forEach(el => {
    const key = el.getAttribute("data-i18n");
    if (dict[key]) {
      if (el.tagName === "TITLE") document.title = dict[key];
      else el.textContent = dict[key];
    }
  });
  document.querySelectorAll(".lang").forEach(btn => {
    btn.classList.toggle("active", btn.dataset.lang === lang);
  });
  localStorage.setItem("lang", lang);
}

document.querySelectorAll(".lang").forEach(btn => {
  btn.addEventListener("click", () => setLang(btn.dataset.lang));
});

// Применяем сохранённый язык при загрузке
setLang(localStorage.getItem("lang") || "ru");
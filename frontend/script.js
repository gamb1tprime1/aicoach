const API = "";

// ============================================================
// ЛОКАЛИЗАЦИЯ
// ============================================================
const translations = {
  ru: {
    subtitle: "Подготовка к IELTS / SAT, эссе, портфолио, поступление за границу",
    tab_tasks: "📚 Задания",
    tab_essay: "✍️ Эссе-коуч",
    tab_plan: "🗓 План тренировок",
    tab_portfolio: "🎯 Портфолио",
    tab_history: "📜 История",
    history_title: "История запросов",
    btn_load_history: "Обновить",
    history_empty: "Запросов ещё не было.",
    history_essay: "Эссе",
    history_plan: "План",
    history_portfolio: "Портфолио",
    history_unknown: "Другое",
    tasks_title: "Задания IELTS / SAT",
    essay_title: "Разбор эссе (без написания за вас)",
    plan_title: "Индивидуальный план тренировок",
    portfolio_title: "Портфолио для поступления",
    label_exam: "Экзамен",
    label_section: "Секция",
    label_difficulty: "Сложность",
    label_uni: "Университет",
    label_essay_type: "Тип эссе",
    label_prompt: "Промпт",
    label_draft: "Черновик",
    label_target: "Целевой балл",
    label_level: "Текущий уровень",
    label_native: "Родной язык",
    label_weeks: "Недель",
    label_hours: "Часов в неделю",
    label_major: "Специальность",
    label_unis: "Вузы (через запятую)",
    label_deadline: "Недель до дедлайна",
    btn_get_tasks: "Получить",
    label_count: "Сколько",
    btn_generate: "Сгенерировать",
    btn_analyze: "Разобрать",
    btn_plan: "Составить план",
    btn_portfolio: "Собрать портфолио",
    ph_section: "Writing Task 2 / Math / Reading",
    ph_difficulty: "band7 / band9 / hard",
    ph_draft: "Вставь свой черновик...",
    loading: "Загружаю...",
    analyzing: "Анализирую... (30–60 секунд)",
    planning: "Составляю... (30–60 секунд)",
    building: "Собираю... (30–60 секунд)",
    nothing_found: "Ничего не найдено.",
    copy: "Копировать",
    copied: "✓ Скопировано",
    error_prefix: "Ошибка",
    empty: "(пусто)",
    card_plan: "План эссе",
    card_questions: "Вопросы для размышления",
    card_checklist: "Чек-лист для университета",
    card_weak: "Слабые места",
    card_tone: "Заметки о тоне",
    card_resources: "Ресурсы",
    card_budget: "Бюджет слов",
    card_goals: "Цели по неделям",
    card_daily: "Ежедневные задачи",
    card_rationale: "Обоснование",
    card_sections: "Разделы портфолио",
    card_actions: "Что делать",
    card_uni_req: "Требования вузов",
    gen_fallback: "Модель не справилась, показаны задания из базы."
  },
  en: {
    subtitle: "Preparation for IELTS / SAT, essays, portfolio, study abroad",
    tab_tasks: "📚 Tasks",
    tab_essay: "✍️ Essay Coach",
    tab_plan: "🗓 Study Plan",
    tab_portfolio: "🎯 Portfolio",
    tab_history: "📜 History",
    history_title: "Request History",
    btn_load_history: "Refresh",
    history_empty: "No requests yet.",
    history_essay: "Essay",
    history_plan: "Plan",
    history_portfolio: "Portfolio",
    history_unknown: "Other",
    tasks_title: "IELTS / SAT Tasks",
    essay_title: "Essay review (no ghostwriting)",
    plan_title: "Personal study plan",
    portfolio_title: "Admissions portfolio",
    label_exam: "Exam",
    label_section: "Section",
    label_difficulty: "Difficulty",
    label_uni: "University",
    label_essay_type: "Essay type",
    label_prompt: "Prompt",
    label_draft: "Draft",
    label_target: "Target score",
    label_level: "Current level",
    label_native: "Native language",
    label_weeks: "Weeks",
    label_hours: "Hours per week",
    label_major: "Major",
    label_unis: "Universities (comma-separated)",
    label_deadline: "Weeks until deadline",
    btn_get_tasks: "Get tasks",
    label_count: "Count",
    btn_generate: "Generate",
    btn_analyze: "Analyze",
    btn_plan: "Build plan",
    btn_portfolio: "Build portfolio",
    ph_section: "Writing Task 2 / Math / Reading",
    ph_difficulty: "band7 / band9 / hard",
    ph_draft: "Paste your draft here...",
    loading: "Loading...",
    analyzing: "Analyzing... (30–60 seconds)",
    planning: "Building... (30–60 seconds)",
    building: "Building... (30–60 seconds)",
    nothing_found: "Nothing found.",
    copy: "Copy",
    copied: "✓ Copied",
    error_prefix: "Error",
    empty: "(empty)",
    card_plan: "Essay outline",
    card_questions: "Reflection questions",
    card_checklist: "University checklist",
    card_weak: "Weak points",
    card_tone: "Tone notes",
    card_resources: "Resources",
    card_budget: "Word budget",
    card_goals: "Weekly goals",
    card_daily: "Daily tasks",
    card_rationale: "Rationale",
    card_sections: "Portfolio sections",
    card_actions: "Action items",
    card_uni_req: "University requirements",
    gen_fallback: "Model failed, showing static tasks."
  }
};

let currentLang = localStorage.getItem("lang") || "ru";
function t(key) { return translations[currentLang][key] || key; }

function setLang(lang) {
  currentLang = lang;
  localStorage.setItem("lang", lang);
  document.documentElement.lang = lang;

  document.querySelectorAll("[data-i18n]").forEach(el => {
    const key = el.getAttribute("data-i18n");
    if (translations[lang][key]) el.textContent = translations[lang][key];
  });

  document.querySelectorAll("[data-i18n-placeholder]").forEach(el => {
    const key = el.getAttribute("data-i18n-placeholder");
    if (translations[lang][key]) el.placeholder = translations[lang][key];
  });

  const btnRu = document.getElementById("btn-ru");
  const btnEn = document.getElementById("btn-en");
  if (btnRu) btnRu.classList.toggle("active", lang === "ru");
  if (btnEn) btnEn.classList.toggle("active", lang === "en");
}

// ============================================================
// ВКЛАДКИ
// ============================================================
document.querySelectorAll(".tab").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(b => b.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById("tab-" + btn.dataset.tab).classList.add("active");
  });
});

// ============================================================
// HTTP
// ============================================================
async function post(path, body) {
  let res;
  try {
    res = await fetch(API + path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body)
    });
  } catch (e) {
    throw new Error((currentLang === "ru" ? "Сервер недоступен. " : "Server unreachable. ") + e.message);
  }
  const text = await res.text();
  let data;
  try { data = JSON.parse(text); }
  catch { throw new Error("Non-JSON response: " + text.slice(0, 200)); }
  if (!res.ok) throw new Error("HTTP " + res.status + ": " + JSON.stringify(data));
  return data;
}

// ============================================================
// ХЕЛПЕРЫ РЕНДЕРА
// ============================================================
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

function renderList(title, arr) {
  const box = el("div", "card");
  const h = el("h3", null, title);
  const btn = el("button", "copy", t("copy"));
  btn.addEventListener("click", () => {
    const text = (arr || []).map(x => typeof x === "string" ? x : JSON.stringify(x)).join("\n");
    navigator.clipboard.writeText(text);
    btn.textContent = t("copied");
    setTimeout(() => btn.textContent = t("copy"), 1500);
  });
  h.appendChild(btn);
  box.appendChild(h);
  if (!arr || arr.length === 0) {
    box.appendChild(el("p", "empty", t("empty")));
  } else {
    const ul = el("ul", "pretty-list");
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
  const h = el("h3", null, title);
  const btn = el("button", "copy", t("copy"));
  btn.addEventListener("click", () => {
    navigator.clipboard.writeText(text || "");
    btn.textContent = t("copied");
    setTimeout(() => btn.textContent = t("copy"), 1500);
  });
  h.appendChild(btn);
  box.appendChild(h);
  const p = el("p", "pretty-text", text || t("empty"));
  box.appendChild(p);
  return box;
}

function renderObject(title, obj) {
  const box = el("div", "card");
  box.appendChild(el("h3", null, title));
  if (!obj || Object.keys(obj).length === 0) {
    box.appendChild(el("p", "empty", t("empty")));
  } else {
    const ul = el("ul");
    for (const [k, v] of Object.entries(obj)) {
      ul.appendChild(el("li", null, k + ": " + (typeof v === "string" ? v : JSON.stringify(v))));
    }
    box.appendChild(ul);
  }
  return box;
}

function loadingBox(msgKey) {
  const box = el("div", "loading-box");
  box.appendChild(el("span", "loading"));
  box.appendChild(document.createTextNode(t(msgKey)));
  return box;
}

function renderTaskCard(item) {
  const box = el("div", "card");
  box.appendChild(el("h3", null, item.section + " [" + item.difficulty + "]"));
  box.appendChild(el("p", null, item.prompt));
  if (item.meta) {
    const m = item.meta;
    const parts = [];
    if (m.topic)        parts.push("📌 " + m.topic);
    if (m.min_words)    parts.push("✍️ min " + m.min_words + " words");
    if (m.time_minutes) parts.push("⏱ " + m.time_minutes + " min");
    if (m.questions)    parts.push("❓ " + m.questions + " questions");
    if (m.answers)      parts.push("✓ " + JSON.stringify(m.answers));
    if (parts.length) box.appendChild(el("small", null, parts.join("  ·  ")));
  }
  return box;
}

// ============================================================
// 1) ЗАДАНИЯ
// ============================================================
async function loadTasks() {
  const out = document.getElementById("tasks");
  out.innerHTML = "";
  out.appendChild(loadingBox("loading"));
  try {
    const data = await post("/api/tasks", {
      exam: document.getElementById("exam").value,
      section: document.getElementById("section").value,
      difficulty: document.getElementById("difficulty").value,
      count: 20
    });
    out.innerHTML = "";
    if (!data.tasks || !data.tasks.length) {
      out.textContent = t("nothing_found");
      return;
    }
    data.tasks.forEach(item => out.appendChild(renderTaskCard(item)));
  } catch (e) { out.textContent = "❌ " + e.message; }
}

async function generateTasks() {
  const out = document.getElementById("tasks");
  out.innerHTML = "";
  out.appendChild(loadingBox("loading"));
  try {
    const countInput = document.getElementById("count");
    const count = countInput ? (parseInt(countInput.value) || 5) : 5;
    const data = await post("/api/tasks/generate", {
      exam: document.getElementById("exam").value,
      section: document.getElementById("section").value,
      difficulty: document.getElementById("difficulty").value,
      count: count,
      lang: currentLang
    });
    out.innerHTML = "";
    if (data.error) {
      out.textContent = "❌ " + data.error;
      return;
    }
    if (!data.tasks || !data.tasks.length) {
      out.textContent = t("nothing_found");
      return;
    }
    if (data.fallback) {
      out.appendChild(el("p", "empty", t("gen_fallback")));
    }
    data.tasks.forEach(item => out.appendChild(renderTaskCard(item)));
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

// ============================================================
// 2) ЭССЕ-КОУЧ
// ============================================================
async function coachEssay() {
  const out = document.getElementById("essayOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("analyzing"));
  try {
    const data = await post("/api/essay/coach", {
      user_id: "u1",
      university: document.getElementById("uni").value,
      essay_type: document.getElementById("essayType").value,
      prompt: document.getElementById("prompt").value,
      draft: document.getElementById("draft").value,
      lang: currentLang
    });
    out.innerHTML = "";
    if (data.error) {
      out.appendChild(renderText(t("error_prefix"), data.error + (data.raw ? "\n\nRaw: " + data.raw : "")));
      return;
    }
    out.appendChild(renderList(t("card_plan"), pick(data, "outline", "план", "plan", "схема")));
    out.appendChild(renderList(t("card_questions"), pick(data, "reflection_questions", "вопросы-размышления", "вопросы")));
    out.appendChild(renderList(t("card_checklist"), pick(data, "university_checklist", "чек-лист", "университет")));
    out.appendChild(renderList(t("card_weak"), pick(data, "weak_points", "слабые_места", "слабые")));
    out.appendChild(renderList(t("card_tone"), pick(data, "tone_notes", "тон", "tone")));
    out.appendChild(renderList(t("card_resources"), pick(data, "resources", "ресурсы")));
    if (data.word_budget && Object.keys(data.word_budget).length) {
      out.appendChild(renderObject(t("card_budget"), data.word_budget));
    }
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

// ============================================================
// 3) ПЛАН
// ============================================================
async function makePlan() {
  const out = document.getElementById("planOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("planning"));
  try {
    const data = await post("/api/study-plan", {
      user_id: "u1",
      target_exam: document.getElementById("pExam").value,
      target_score: parseFloat(document.getElementById("pScore").value),
      current_level: document.getElementById("pLevel").value,
      native_language: document.getElementById("pNative").value,
      weeks_available: parseInt(document.getElementById("pWeeks").value),
      hours_per_week: parseFloat(document.getElementById("pHours").value),
      lang: currentLang
    });
    out.innerHTML = "";
    if (data.error) {
      out.appendChild(renderText(t("error_prefix"), data.error));
      return;
    }
    out.appendChild(renderList(t("card_goals"), pick(data, "weekly_goals", "цели", "еженедельные_цели")));
    out.appendChild(renderList(t("card_daily"), pick(data, "daily_tasks", "ежедневные_задачи", "задачи")));
    out.appendChild(renderList(t("card_resources"), pick(data, "resources", "ресурсы")));
    out.appendChild(renderText(t("card_rationale"), pick(data, "rationale", "обоснование", "reasoning")));
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

// ============================================================
// 4) ПОРТФОЛИО
// ============================================================
async function buildPortfolio() {
  const out = document.getElementById("pfOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("building"));
  const unis = document.getElementById("pfUnis").value.split(",").map(s => s.trim()).filter(Boolean);
  try {
    const data = await post("/api/portfolio", {
      user_id: "u1",
      major: document.getElementById("pfMajor").value,
      universities: unis,
      target_exam: document.getElementById("pfExam").value,
      target_score: parseFloat(document.getElementById("pfScore").value),
      weeks_available: parseInt(document.getElementById("pfWeeks").value),
      lang: currentLang
    });
    out.innerHTML = "";
    if (data.error) {
      out.appendChild(renderText(t("error_prefix"), data.error));
      return;
    }
    out.appendChild(renderList(t("card_sections"), pick(data, "sections", "разделы", "секции")));
    out.appendChild(renderList(t("card_actions"), pick(data, "action_items", "действия", "шаги", "задачи")));
    if (data.university_requirements && Object.keys(data.university_requirements).length) {
      out.appendChild(renderObject(t("card_uni_req"), data.university_requirements));
    }
  } catch (e) { out.innerHTML = ""; out.textContent = "❌ " + e.message; }
}

// ============================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================
// ============================================================
// 5) ИСТОРИЯ
// ============================================================
async function loadHistory() {
  const out = document.getElementById("historyOut");
  out.innerHTML = "";
  out.appendChild(loadingBox("loading"));
  try {
    const res = await fetch("/api/history?user_id=u1");
    const data = await res.json();
    out.innerHTML = "";
    if (!data.items || !data.items.length) {
      out.textContent = t("history_empty");
      return;
    }
    data.items.forEach(item => {
      const box = el("div", "card");

      // Название эндпоинта
      let epName = t("history_unknown");
      let epIcon = "📌";
      if (item.endpoint === "essay")          { epName = t("history_essay");     epIcon = "✍️"; }
      else if (item.endpoint === "study_plan"){ epName = t("history_plan");      epIcon = "🗓"; }
      else if (item.endpoint === "portfolio") { epName = t("history_portfolio"); epIcon = "🎯"; }

      // Дата
      const date = item.created_at
        ? new Date(item.created_at * 1000).toLocaleString(currentLang === "ru" ? "ru-RU" : "en-US")
        : "";

      box.appendChild(el("h3", null, epIcon + " " + epName + "  ·  " + date));

      // Что запросили — только ключевые поля
      const req = item.request || {};
      const reqLines = [];
      if (req.university)    reqLines.push((currentLang === "ru" ? "🏫 Университет: " : "🏫 University: ") + req.university);
      if (req.major)         reqLines.push((currentLang === "ru" ? "🎓 Специальность: " : "🎓 Major: ") + req.major);
      if (req.essay_type)    reqLines.push((currentLang === "ru" ? "📝 Тип эссе: " : "📝 Essay type: ") + req.essay_type);
      if (req.target_exam)   reqLines.push((currentLang === "ru" ? "📋 Экзамен: " : "📋 Exam: ") + req.target_exam);
      if (req.target_score)  reqLines.push((currentLang === "ru" ? "🎯 Целевой балл: " : "🎯 Target score: ") + req.target_score);
      if (req.exam)          reqLines.push((currentLang === "ru" ? "📋 Экзамен: " : "📋 Exam: ") + req.exam);
      if (req.count)         reqLines.push((currentLang === "ru" ? "🔢 Количество: " : "🔢 Count: ") + req.count);
      if (req.draft && req.draft.length > 0) {
        const short = req.draft.length > 200 ? req.draft.slice(0, 200) + "..." : req.draft;
        reqLines.push((currentLang === "ru" ? "📄 Черновик: " : "📄 Draft: ") + short);
      }
      if (req.universities && req.universities.length) {
        reqLines.push((currentLang === "ru" ? "🏫 Вузы: " : "🏫 Universities: ") + req.universities.join(", "));
      }

      if (reqLines.length) {
        box.appendChild(el("p", "pretty-text", reqLines.join("\n")));
      }

      // Что вернулось — компактная сводка
      const resp = item.response || {};
      const respLines = [];

      if (resp.outline && resp.outline.length) {
        respLines.push((currentLang === "ru" ? "📝 План: " : "📝 Outline: ") + resp.outline.length + " " +
          (currentLang === "ru" ? "пунктов" : "items"));
      }
      if (resp.weekly_goals && resp.weekly_goals.length) {
        respLines.push((currentLang === "ru" ? "🎯 Цели: " : "🎯 Weekly goals: ") + resp.weekly_goals.length + " " +
          (currentLang === "ru" ? "шт." : "items"));
      }
      if (resp.daily_tasks && resp.daily_tasks.length) {
        respLines.push((currentLang === "ru" ? "📅 Задачи: " : "📅 Daily tasks: ") + resp.daily_tasks.length + " " +
          (currentLang === "ru" ? "шт." : "items"));
      }
      if (resp.sections && resp.sections.length) {
        respLines.push((currentLang === "ru" ? "📚 Разделы: " : "📚 Sections: ") + resp.sections.length + " " +
          (currentLang === "ru" ? "шт." : "items"));
      }
      if (resp.action_items && resp.action_items.length) {
        respLines.push((currentLang === "ru" ? "✅ Действия: " : "✅ Actions: ") + resp.action_items.length + " " +
          (currentLang === "ru" ? "шт." : "items"));
      }
      if (resp.tasks && resp.tasks.length) {
        respLines.push((currentLang === "ru" ? "📚 Сгенерировано заданий: " : "📚 Generated tasks: ") + resp.tasks.length);
      }
      if (resp.fallback !== undefined) {
        respLines.push((currentLang === "ru" ? "⚠️ Fallback: " : "⚠️ Fallback: ") + resp.fallback);
      }

      if (respLines.length) {
        box.appendChild(el("p", "pretty-text", respLines.join("\n")));
      }

      // Кнопка «показать полный JSON»
      const details = el("details");
      const summary = el("summary", "copy", currentLang === "ru" ? "Показать полный ответ" : "Show full response");
      summary.style.cursor = "pointer";
      details.appendChild(summary);
      const pre = el("pre", null, JSON.stringify(resp, null, 2));
      pre.style.background = "#f8f9fc";
      pre.style.padding = "10px";
      pre.style.borderRadius = "6px";
      pre.style.fontSize = "12px";
      pre.style.overflow = "auto";
      pre.style.maxHeight = "300px";
      details.appendChild(pre);
      box.appendChild(details);

      out.appendChild(box);
    });
  } catch (e) {
    out.innerHTML = "";
    out.textContent = "❌ " + e.message;
  }
}
document.addEventListener("DOMContentLoaded", () => setLang(currentLang));
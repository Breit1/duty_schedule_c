const API_BASE = '/api';

let currentTab = 'schedule';
let tasks = [];
let users = [];
let duties = [];

// Инициализация
document.addEventListener('DOMContentLoaded', () => {
    setupTabs();
    loadData();
    setDefaultDates();
});

function setupTabs() {
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const tab = btn.dataset.tab;
            switchTab(tab);
        });
    });
}

function switchTab(tab) {
    currentTab = tab;
    
    // Обновляем кнопки
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.tab === tab);
    });
    
    // Обновляем контент
    document.querySelectorAll('.tab-content').forEach(content => {
        content.classList.toggle('active', content.id === tab);
    });
    
    // Загружаем данные для вкладки
    if (tab === 'schedule') {
        loadSchedule();
    } else if (tab === 'tasks') {
        loadTasks();
    } else if (tab === 'users') {
        loadUsers();
    } else if (tab === 'statistics') {
        loadStatistics();
    }
}

function setDefaultDates() {
    const today = new Date();
    const nextMonth = new Date(today);
    nextMonth.setMonth(nextMonth.getMonth() + 1);
    
    document.getElementById('startDate').value = formatDate(today);
    document.getElementById('endDate').value = formatDate(nextMonth);
}

function formatDate(date) {
    const d = new Date(date);
    const year = d.getFullYear();
    const month = String(d.getMonth() + 1).padStart(2, '0');
    const day = String(d.getDate()).padStart(2, '0');
    return `${year}-${month}-${day}`;
}

// API функции
async function apiCall(endpoint, method = 'GET', body = null) {
    const options = {
        method,
        headers: {
            'Content-Type': 'application/json'
        }
    };
    
    if (body) {
        options.body = JSON.stringify(body);
    }
    
    const response = await fetch(`${API_BASE}${endpoint}`, options);
    return response.json();
}

async function loadData() {
    await Promise.all([
        loadTasks(),
        loadUsers(),
        loadSchedule()
    ]);
}

// Задачи
async function loadTasks() {
    try {
        tasks = await apiCall('/tasks');
        renderTasks();
    } catch (error) {
        console.error('Ошибка загрузки задач:', error);
    }
}

function renderTasks() {
    const container = document.getElementById('tasksList');
    
    if (tasks.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">📝</div>
                <p>Нет задач. Добавьте первую задачу!</p>
            </div>
        `;
        return;
    }
    
    container.innerHTML = tasks.map(task => `
        <div class="task-card">
            <div class="card-header">
                <div>
                    <div class="card-title">${escapeHtml(task.name)}</div>
                    <div class="card-subtitle">${escapeHtml(task.description || 'Без описания')}</div>
                </div>
                <span class="badge badge-category">${escapeHtml(task.category)}</span>
            </div>
            <div class="card-info">
                <div class="card-info-item">
                    <span>🔄</span>
                    <span>Каждые ${task.frequencyDays} ${getDayWord(task.frequencyDays)}</span>
                </div>
                <div class="card-info-item">
                    <span>⏱️</span>
                    <span>${task.durationMinutes} минут</span>
                </div>
            </div>
            <div class="card-actions">
                <button class="btn btn-danger" onclick="deleteTask(${task.id})">🗑️ Удалить</button>
            </div>
        </div>
    `).join('');
}

function getDayWord(days) {
    if (days === 1) return 'день';
    if (days >= 2 && days <= 4) return 'дня';
    return 'дней';
}

function showTaskModal(taskId = null) {
    const modal = document.getElementById('taskModal');
    const form = document.getElementById('taskForm');
    const title = document.getElementById('taskModalTitle');
    
    if (taskId) {
        const task = tasks.find(t => t.id === taskId);
        if (task) {
            document.getElementById('taskId').value = task.id;
            document.getElementById('taskName').value = task.name;
            document.getElementById('taskDescription').value = task.description || '';
            document.getElementById('taskFrequency').value = task.frequencyDays;
            document.getElementById('taskDuration').value = task.durationMinutes;
            document.getElementById('taskCategory').value = task.category;
            title.textContent = 'Редактировать задачу';
        }
    } else {
        form.reset();
        document.getElementById('taskId').value = '';
        title.textContent = 'Добавить задачу';
    }
    
    modal.style.display = 'block';
}

function closeTaskModal() {
    document.getElementById('taskModal').style.display = 'none';
}

async function saveTask(event) {
    event.preventDefault();
    
    const taskId = document.getElementById('taskId').value;
    const taskData = {
        name: document.getElementById('taskName').value,
        description: document.getElementById('taskDescription').value,
        frequencyDays: parseInt(document.getElementById('taskFrequency').value),
        durationMinutes: parseInt(document.getElementById('taskDuration').value),
        category: document.getElementById('taskCategory').value
    };
    
    try {
        if (taskId) {
            // Редактирование пока не реализовано в API
            alert('Редактирование задач пока не поддерживается');
        } else {
            await apiCall('/tasks', 'POST', taskData);
            closeTaskModal();
            await loadTasks();
            await loadSchedule();
        }
    } catch (error) {
        console.error('Ошибка сохранения задачи:', error);
        alert('Ошибка при сохранении задачи');
    }
}

async function deleteTask(taskId) {
    if (!confirm('Вы уверены, что хотите удалить эту задачу?')) {
        return;
    }
    
    try {
        await apiCall(`/tasks/${taskId}`, 'DELETE');
        await loadTasks();
        await loadSchedule();
    } catch (error) {
        console.error('Ошибка удаления задачи:', error);
        alert('Ошибка при удалении задачи');
    }
}

// Пользователи
async function loadUsers() {
    try {
        users = await apiCall('/users');
        renderUsers();
        updateUserFilter();
    } catch (error) {
        console.error('Ошибка загрузки пользователей:', error);
    }
}

function renderUsers() {
    const container = document.getElementById('usersList');
    
    if (users.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">👥</div>
                <p>Нет пользователей. Добавьте первого пользователя!</p>
            </div>
        `;
        return;
    }
    
    container.innerHTML = users.map(user => `
        <div class="user-card">
            <div class="card-header">
                <div>
                    <div class="card-title">${escapeHtml(user.name)}</div>
                    <div class="card-subtitle">${escapeHtml(user.email || 'Без email')}</div>
                </div>
                <span class="badge ${user.active ? 'badge-completed' : 'badge-skipped'}">
                    ${user.active ? 'Активен' : 'Неактивен'}
                </span>
            </div>
            <div class="card-actions">
                <button class="btn btn-secondary" onclick="editUser(${user.id})">✏️ Редактировать</button>
                <button class="btn btn-danger" onclick="deleteUser(${user.id})">🗑️ Удалить</button>
            </div>
        </div>
    `).join('');
}

function updateUserFilter() {
    const select = document.getElementById('userFilter');
    select.innerHTML = '<option value="">Все пользователи</option>' +
        users.map(user => 
            `<option value="${user.id}">${escapeHtml(user.name)}</option>`
        ).join('');
}

function showUserModal(userId = null) {
    const modal = document.getElementById('userModal');
    const form = document.getElementById('userForm');
    const title = document.getElementById('userModalTitle');
    
    if (userId) {
        const user = users.find(u => u.id === userId);
        if (user) {
            document.getElementById('userId').value = user.id;
            document.getElementById('userName').value = user.name;
            document.getElementById('userEmail').value = user.email || '';
            document.getElementById('userActive').checked = user.active;
            title.textContent = 'Редактировать пользователя';
        }
    } else {
        form.reset();
        document.getElementById('userId').value = '';
        document.getElementById('userActive').checked = true;
        title.textContent = 'Добавить пользователя';
    }
    
    modal.style.display = 'block';
}

function closeUserModal() {
    document.getElementById('userModal').style.display = 'none';
}

async function saveUser(event) {
    event.preventDefault();
    
    const userId = document.getElementById('userId').value;
    const userData = {
        name: document.getElementById('userName').value,
        email: document.getElementById('userEmail').value,
        active: document.getElementById('userActive').checked
    };
    
    try {
        if (userId) {
            await apiCall(`/users/${userId}`, 'PUT', userData);
        } else {
            await apiCall('/users', 'POST', userData);
        }
        closeUserModal();
        await loadUsers();
        await loadSchedule();
    } catch (error) {
        console.error('Ошибка сохранения пользователя:', error);
        alert('Ошибка при сохранении пользователя');
    }
}

function editUser(userId) {
    showUserModal(userId);
}

async function deleteUser(userId) {
    if (!confirm('Вы уверены, что хотите удалить этого пользователя?')) {
        return;
    }
    
    try {
        await apiCall(`/users/${userId}`, 'DELETE');
        await loadUsers();
        await loadSchedule();
    } catch (error) {
        console.error('Ошибка удаления пользователя:', error);
        alert('Ошибка при удалении пользователя');
    }
}

// Расписание
async function loadSchedule() {
    try {
        const startDate = document.getElementById('startDate').value;
        const endDate = document.getElementById('endDate').value;
        const userId = document.getElementById('userFilter').value;
        
        let endpoint = `/duties?startDate=${startDate}&endDate=${endDate}`;
        if (userId) {
            endpoint += `&userId=${userId}`;
        }
        
        duties = await apiCall(endpoint);
        renderSchedule();
    } catch (error) {
        console.error('Ошибка загрузки расписания:', error);
    }
}

function filterSchedule() {
    loadSchedule();
}

function renderSchedule() {
    const container = document.getElementById('scheduleList');
    
    if (duties.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">📅</div>
                <p>Нет дежурств. Сгенерируйте расписание!</p>
            </div>
        `;
        return;
    }
    
    // Группируем по датам
    const grouped = {};
    duties.forEach(duty => {
        if (!grouped[duty.date]) {
            grouped[duty.date] = [];
        }
        grouped[duty.date].push(duty);
    });
    
    const sortedDates = Object.keys(grouped).sort();
    
    container.innerHTML = sortedDates.map(date => {
        const dateDuties = grouped[date];
        const dateObj = new Date(date);
        const dateStr = dateObj.toLocaleDateString('ru-RU', { 
            weekday: 'long', 
            year: 'numeric', 
            month: 'long', 
            day: 'numeric' 
        });
        
        return `
            <div class="date-group">
                <h3 style="margin: 20px 0 10px 0; color: var(--primary-color);">${dateStr}</h3>
                ${dateDuties.map(duty => renderDutyCard(duty)).join('')}
            </div>
        `;
    }).join('');
}

function renderDutyCard(duty) {
    const statusClass = duty.status;
    const statusText = {
        'pending': 'Ожидает',
        'completed': 'Выполнено',
        'skipped': 'Пропущено'
    };
    
    const statusBadge = {
        'pending': 'badge-pending',
        'completed': 'badge-completed',
        'skipped': 'badge-skipped'
    };
    
    return `
        <div class="duty-card ${statusClass}">
            <div class="card-header">
                <div>
                    <div class="card-title">${escapeHtml(duty.taskName || 'Задача')}</div>
                    <div class="card-subtitle">${escapeHtml(duty.userName || 'Пользователь')}</div>
                </div>
                <span class="badge ${statusBadge[duty.status]}">${statusText[duty.status]}</span>
            </div>
            <div class="card-info">
                <div class="card-info-item">
                    <span>📅</span>
                    <span>${duty.date}</span>
                </div>
                ${duty.taskCategory ? `
                    <div class="card-info-item">
                        <span class="badge badge-category">${escapeHtml(duty.taskCategory)}</span>
                    </div>
                ` : ''}
            </div>
            ${duty.notes ? `
                <div style="margin-top: 10px; padding: 10px; background: var(--bg-color); border-radius: 6px; font-size: 0.9em;">
                    <strong>Заметки:</strong> ${escapeHtml(duty.notes)}
                </div>
            ` : ''}
            <div class="card-actions">
                ${duty.status === 'pending' ? `
                    <button class="btn btn-success" onclick="completeDuty(${duty.id})">✅ Выполнено</button>
                    <button class="btn btn-warning" onclick="skipDuty(${duty.id})">⏭️ Пропустить</button>
                ` : ''}
                <button class="btn btn-danger" onclick="deleteDuty(${duty.id})">🗑️ Удалить</button>
            </div>
        </div>
    `;
}

function showNotesModal(dutyId, action) {
    const modal = document.getElementById('notesModal');
    const form = document.getElementById('notesForm');
    const submitBtn = document.getElementById('notesSubmitBtn');
    
    document.getElementById('notesDutyId').value = dutyId;
    document.getElementById('notesAction').value = action;
    document.getElementById('notesText').value = '';
    
    submitBtn.textContent = action === 'complete' ? '✅ Отметить выполненным' : '⏭️ Пропустить';
    
    modal.style.display = 'block';
}

function closeNotesModal() {
    document.getElementById('notesModal').style.display = 'none';
}

async function saveNotes(event) {
    event.preventDefault();
    
    const dutyId = parseInt(document.getElementById('notesDutyId').value);
    const action = document.getElementById('notesAction').value;
    const notes = document.getElementById('notesText').value;
    
    try {
        if (action === 'complete') {
            await apiCall(`/duties/${dutyId}/complete`, 'POST', { notes });
        } else {
            await apiCall(`/duties/${dutyId}/skip`, 'POST', { notes });
        }
        closeNotesModal();
        await loadSchedule();
    } catch (error) {
        console.error('Ошибка обновления дежурства:', error);
        alert('Ошибка при обновлении дежурства');
    }
}

function completeDuty(dutyId) {
    showNotesModal(dutyId, 'complete');
}

function skipDuty(dutyId) {
    showNotesModal(dutyId, 'skip');
}

async function deleteDuty(dutyId) {
    if (!confirm('Вы уверены, что хотите удалить это дежурство?')) {
        return;
    }
    
    try {
        await apiCall(`/duties/${dutyId}`, 'DELETE');
        await loadSchedule();
    } catch (error) {
        console.error('Ошибка удаления дежурства:', error);
        alert('Ошибка при удалении дежурства');
    }
}

async function generateSchedule() {
    if (!confirm('Сгенерировать расписание на 30 дней вперед?')) {
        return;
    }
    
    try {
        await apiCall('/duties/generate', 'POST', { daysAhead: 30 });
        alert('Расписание успешно сгенерировано!');
        await loadSchedule();
    } catch (error) {
        console.error('Ошибка генерации расписания:', error);
        alert('Ошибка при генерации расписания');
    }
}

// Статистика
async function loadStatistics() {
    try {
        const startDate = document.getElementById('startDate').value;
        const endDate = document.getElementById('endDate').value;
        
        const stats = await apiCall(`/statistics?startDate=${startDate}&endDate=${endDate}`);
        renderStatistics(stats);
    } catch (error) {
        console.error('Ошибка загрузки статистики:', error);
    }
}

function renderStatistics(stats) {
    const container = document.getElementById('statisticsContent');
    
    const total = stats.total || 0;
    const completed = stats.completed || 0;
    const pending = stats.pending || 0;
    const skipped = stats.skipped || 0;
    const completionRate = total > 0 ? Math.round((completed / total) * 100) : 0;
    
    container.innerHTML = `
        <div class="stat-card">
            <p>Всего дежурств</p>
            <h3>${total}</h3>
        </div>
        <div class="stat-card" style="background: linear-gradient(135deg, #50c878 0%, #3fb863 100%);">
            <p>Выполнено</p>
            <h3>${completed}</h3>
        </div>
        <div class="stat-card" style="background: linear-gradient(135deg, #f39c12 0%, #d68910 100%);">
            <p>Ожидает</p>
            <h3>${pending}</h3>
        </div>
        <div class="stat-card" style="background: linear-gradient(135deg, #7f8c8d 0%, #6c7a7b 100%);">
            <p>Пропущено</p>
            <h3>${skipped}</h3>
        </div>
        <div class="stat-card" style="background: linear-gradient(135deg, #e74c3c 0%, #c0392b 100%);">
            <p>Процент выполнения</p>
            <h3>${completionRate}%</h3>
        </div>
    `;
}

// Утилиты
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Закрытие модальных окон при клике вне их
window.onclick = function(event) {
    const modals = ['taskModal', 'userModal', 'notesModal'];
    modals.forEach(modalId => {
        const modal = document.getElementById(modalId);
        if (event.target === modal) {
            modal.style.display = 'none';
        }
    });
}



#include "duty_manager.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <climits>

DutyManager::DutyManager() : nextTaskId(1), nextUserId(1), nextDutyId(1) {
    addUser("Пользователь 1", "user1@example.com");
    addUser("Пользователь 2", "user2@example.com");
}

std::string DutyManager::getCurrentDate() const {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::ostringstream oss;
    oss << std::setfill('0') << (now->tm_year + 1900) << "-"
        << std::setw(2) << (now->tm_mon + 1) << "-"
        << std::setw(2) << now->tm_mday;
    return oss.str();
}

std::string DutyManager::addDays(const std::string& date, int days) const {
    std::tm tm = {};
    std::istringstream iss(date);
    char dash;
    iss >> tm.tm_year >> dash >> tm.tm_mon >> dash >> tm.tm_mday;
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    
    std::time_t t = std::mktime(&tm);
    t += days * 24 * 60 * 60;
    std::tm* newTm = std::localtime(&t);
    
    std::ostringstream oss;
    oss << std::setfill('0') << (newTm->tm_year + 1900) << "-"
        << std::setw(2) << (newTm->tm_mon + 1) << "-"
        << std::setw(2) << newTm->tm_mday;
    return oss.str();
}

bool DutyManager::isDateBefore(const std::string& date1, const std::string& date2) const {
    return date1 < date2;
}

int DutyManager::getDaysDifference(const std::string& date1, const std::string& date2) const {
    std::tm tm1 = {}, tm2 = {};
    std::istringstream iss1(date1), iss2(date2);
    char dash;
    iss1 >> tm1.tm_year >> dash >> tm1.tm_mon >> dash >> tm1.tm_mday;
    iss2 >> tm2.tm_year >> dash >> tm2.tm_mon >> dash >> tm2.tm_mday;
    tm1.tm_year -= 1900;
    tm1.tm_mon -= 1;
    tm2.tm_year -= 1900;
    tm2.tm_mon -= 1;
    
    std::time_t t1 = std::mktime(&tm1);
    std::time_t t2 = std::mktime(&tm2);
    return (t2 - t1) / (24 * 60 * 60);
}

std::map<int, int> DutyManager::getUserDutyCounts(const std::string& startDate, const std::string& endDate) const {
    std::map<int, int> counts;
    for (const auto& user : users) {
        if (user.active) {
            counts[user.id] = 0;
        }
    }
    
    for (const auto& duty : duties) {
        if (duty.date >= startDate && duty.date <= endDate && duty.status == "pending") {
            counts[duty.userId]++;
        }
    }
    return counts;
}

int DutyManager::addTask(const std::string& name, const std::string& description, 
                         int frequencyDays, int durationMinutes, const std::string& category) {
    Task task(nextTaskId++, name, description, frequencyDays, durationMinutes, category);
    tasks.push_back(task);
    return task.id;
}

bool DutyManager::removeTask(int taskId) {
    auto it = std::remove_if(tasks.begin(), tasks.end(),
        [taskId](const Task& t) { return t.id == taskId; });
    if (it != tasks.end()) {
        tasks.erase(it, tasks.end());
        duties.erase(std::remove_if(duties.begin(), duties.end(),
            [taskId](const Duty& d) { return d.taskId == taskId; }), duties.end());
        return true;
    }
    return false;
}

std::vector<Task> DutyManager::getTasks() const {
    return tasks;
}

Task* DutyManager::getTask(int taskId) {
    for (auto& task : tasks) {
        if (task.id == taskId) {
            return &task;
        }
    }
    return nullptr;
}

int DutyManager::addUser(const std::string& name, const std::string& email) {
    User user(nextUserId++, name, email);
    users.push_back(user);
    return user.id;
}

bool DutyManager::removeUser(int userId) {
    auto it = std::remove_if(users.begin(), users.end(),
        [userId](const User& u) { return u.id == userId; });
    if (it != users.end()) {
        users.erase(it, users.end());
        duties.erase(std::remove_if(duties.begin(), duties.end(),
            [userId](const Duty& d) { return d.userId == userId; }), duties.end());
        return true;
    }
    return false;
}

bool DutyManager::updateUser(int userId, const std::string& name, const std::string& email, bool active) {
    for (auto& user : users) {
        if (user.id == userId) {
            user.name = name;
            user.email = email;
            user.active = active;
            return true;
        }
    }
    return false;
}

std::vector<User> DutyManager::getUsers() const {
    return users;
}

User* DutyManager::getUser(int userId) {
    for (auto& user : users) {
        if (user.id == userId) {
            return &user;
        }
    }
    return nullptr;
}

void DutyManager::generateSchedule(int daysAhead) {
    if (tasks.empty() || users.empty()) {
        return;
    }
    
    std::vector<User> activeUsers;
    for (const auto& user : users) {
        if (user.active) {
            activeUsers.push_back(user);
        }
    }
    
    if (activeUsers.empty()) {
        return;
    }
    
    std::string currentDate = getCurrentDate();
    std::string endDate = addDays(currentDate, daysAhead);
    
    for (const auto& task : tasks) {
        std::string taskDate = currentDate;
        
        std::string lastDutyDate = "";
        for (const auto& duty : duties) {
            if (duty.taskId == task.id && duty.date > lastDutyDate) {
                lastDutyDate = duty.date;
            }
        }
        
        if (lastDutyDate.empty()) {
            taskDate = currentDate;
        } else {
            taskDate = addDays(lastDutyDate, task.frequencyDays);
        }
        
        while (isDateBefore(taskDate, endDate) || taskDate == endDate) {
            bool exists = false;
            for (const auto& duty : duties) {
                if (duty.taskId == task.id && duty.date == taskDate && duty.status != "completed") {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                auto counts = getUserDutyCounts(taskDate, addDays(taskDate, task.frequencyDays));
                
                int minCount = INT_MAX;
                int selectedUserId = activeUsers[0].id;
                for (const auto& user : activeUsers) {
                    int count = counts[user.id];
                    if (count < minCount) {
                        minCount = count;
                        selectedUserId = user.id;
                    }
                }
                
                Duty duty(nextDutyId++, task.id, selectedUserId, taskDate);
                duties.push_back(duty);
            }
            
            taskDate = addDays(taskDate, task.frequencyDays);
        }
    }
}

std::vector<Duty> DutyManager::getDuties(const std::string& startDate, const std::string& endDate) const {
    std::vector<Duty> result;
    for (const auto& duty : duties) {
        if (startDate.empty() || duty.date >= startDate) {
            if (endDate.empty() || duty.date <= endDate) {
                result.push_back(duty);
            }
        }
    }
    std::sort(result.begin(), result.end(), 
        [](const Duty& a, const Duty& b) { return a.date < b.date; });
    return result;
}

std::vector<Duty> DutyManager::getDutiesByUser(int userId, const std::string& startDate, const std::string& endDate) const {
    std::vector<Duty> result;
    for (const auto& duty : duties) {
        if (duty.userId == userId) {
            if (startDate.empty() || duty.date >= startDate) {
                if (endDate.empty() || duty.date <= endDate) {
                    result.push_back(duty);
                }
            }
        }
    }
    std::sort(result.begin(), result.end(), 
        [](const Duty& a, const Duty& b) { return a.date < b.date; });
    return result;
}

bool DutyManager::completeDuty(int dutyId, const std::string& notes) {
    for (auto& duty : duties) {
        if (duty.id == dutyId) {
            duty.status = "completed";
            duty.notes = notes;
            duty.completedAt = getCurrentDate();
            return true;
        }
    }
    return false;
}

bool DutyManager::skipDuty(int dutyId, const std::string& notes) {
    for (auto& duty : duties) {
        if (duty.id == dutyId) {
            duty.status = "skipped";
            duty.notes = notes;
            return true;
        }
    }
    return false;
}

bool DutyManager::deleteDuty(int dutyId) {
    auto it = std::remove_if(duties.begin(), duties.end(),
        [dutyId](const Duty& d) { return d.id == dutyId; });
    if (it != duties.end()) {
        duties.erase(it, duties.end());
        return true;
    }
    return false;
}

std::map<std::string, int> DutyManager::getStatistics(const std::string& startDate, const std::string& endDate) const {
    std::map<std::string, int> stats;
    stats["total"] = 0;
    stats["completed"] = 0;
    stats["pending"] = 0;
    stats["skipped"] = 0;
    
    for (const auto& duty : duties) {
        if (startDate.empty() || duty.date >= startDate) {
            if (endDate.empty() || duty.date <= endDate) {
                stats["total"]++;
                if (duty.status == "completed") stats["completed"]++;
                else if (duty.status == "pending") stats["pending"]++;
                else if (duty.status == "skipped") stats["skipped"]++;
            }
        }
    }
    return stats;
}

std::vector<std::map<std::string, std::string>> DutyManager::getUserStatistics(int userId) const {
    std::vector<std::map<std::string, std::string>> result;
    int completed = 0, pending = 0, skipped = 0;
    
    for (const auto& duty : duties) {
        if (duty.userId == userId) {
            if (duty.status == "completed") completed++;
            else if (duty.status == "pending") pending++;
            else if (duty.status == "skipped") skipped++;
        }
    }
    
    std::map<std::string, std::string> stats;
    stats["userId"] = std::to_string(userId);
    stats["completed"] = std::to_string(completed);
    stats["pending"] = std::to_string(pending);
    stats["skipped"] = std::to_string(skipped);
    result.push_back(stats);
    
    return result;
}


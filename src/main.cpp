#include "duty_manager.h"
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using json = nlohmann::json;
using namespace httplib;

DutyManager dutyManager;

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

json taskToJson(const Task& task) {
    json j;
    j["id"] = task.id;
    j["name"] = task.name;
    j["description"] = task.description;
    j["frequencyDays"] = task.frequencyDays;
    j["durationMinutes"] = task.durationMinutes;
    j["category"] = task.category;
    return j;
}

json userToJson(const User& user) {
    json j;
    j["id"] = user.id;
    j["name"] = user.name;
    j["email"] = user.email;
    j["active"] = user.active;
    return j;
}

json dutyToJson(const Duty& duty) {
    json j;
    j["id"] = duty.id;
    j["taskId"] = duty.taskId;
    j["userId"] = duty.userId;
    j["date"] = duty.date;
    j["status"] = duty.status;
    j["completedAt"] = duty.completedAt;
    j["notes"] = duty.notes;
    
    // Добавляем информацию о задаче и пользователе
    Task* task = dutyManager.getTask(duty.taskId);
    if (task) {
        j["taskName"] = task->name;
        j["taskCategory"] = task->category;
    }
    
    User* user = dutyManager.getUser(duty.userId);
    if (user) {
        j["userName"] = user->name;
    }
    
    return j;
}

int main() {
    Server svr;
    
    // CORS заголовки
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });
    
    // Обработка OPTIONS запросов
    svr.Options(".*", [](const Request&, Response& res) {
        return;
    });
    
    // Статические файлы
    svr.Get("/", [](const Request&, Response& res) {
        std::string content = readFile("web/index.html");
        if (content.empty()) {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        } else {
            res.set_content(content, "text/html");
        }
    });
    
    svr.Get("/style.css", [](const Request&, Response& res) {
        std::string content = readFile("web/style.css");
        if (content.empty()) {
            res.status = 404;
        } else {
            res.set_content(content, "text/css");
        }
    });
    
    svr.Get("/app.js", [](const Request&, Response& res) {
        std::string content = readFile("web/app.js");
        if (content.empty()) {
            res.status = 404;
        } else {
            res.set_content(content, "application/javascript");
        }
    });
    
    // API: Задачи
    svr.Get("/api/tasks", [](const Request&, Response& res) {
        auto tasks = dutyManager.getTasks();
        json result = json::array();
        for (const auto& task : tasks) {
            result.push_back(taskToJson(task));
        }
        res.set_content(result.dump(), "application/json");
    });
    
    svr.Post("/api/tasks", [](const Request& req, Response& res) {
        try {
            auto body = json::parse(req.body);
            int id = dutyManager.addTask(
                body["name"].get<std::string>(),
                body.value("description", ""),
                body.value("frequencyDays", 1),
                body.value("durationMinutes", 30),
                body.value("category", "general")
            );
            
            // Автоматически генерируем расписание
            dutyManager.generateSchedule(30);
            
            json result;
            result["id"] = id;
            result["success"] = true;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
    
    svr.Delete("/api/tasks/(\\d+)", [](const Request& req, Response& res) {
        int taskId = std::stoi(req.matches[1]);
        bool success = dutyManager.removeTask(taskId);
        json result;
        result["success"] = success;
        res.set_content(result.dump(), "application/json");
    });
    
    // API: Пользователи
    svr.Get("/api/users", [](const Request&, Response& res) {
        auto users = dutyManager.getUsers();
        json result = json::array();
        for (const auto& user : users) {
            result.push_back(userToJson(user));
        }
        res.set_content(result.dump(), "application/json");
    });
    
    svr.Post("/api/users", [](const Request& req, Response& res) {
        try {
            auto body = json::parse(req.body);
            int id = dutyManager.addUser(
                body["name"].get<std::string>(),
                body.value("email", "")
            );
            json result;
            result["id"] = id;
            result["success"] = true;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
    
    svr.Put("/api/users/(\\d+)", [](const Request& req, Response& res) {
        try {
            int userId = std::stoi(req.matches[1]);
            auto body = json::parse(req.body);
            bool success = dutyManager.updateUser(
                userId,
                body["name"].get<std::string>(),
                body.value("email", ""),
                body.value("active", true)
            );
            json result;
            result["success"] = success;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
    
    svr.Delete("/api/users/(\\d+)", [](const Request& req, Response& res) {
        int userId = std::stoi(req.matches[1]);
        bool success = dutyManager.removeUser(userId);
        json result;
        result["success"] = success;
        res.set_content(result.dump(), "application/json");
    });
    
    // API: Дежурства
    svr.Get("/api/duties", [](const Request& req, Response& res) {
        std::string startDate = req.get_param_value("startDate");
        std::string endDate = req.get_param_value("endDate");
        std::string userId = req.get_param_value("userId");
        
        std::vector<Duty> duties;
        if (!userId.empty()) {
            duties = dutyManager.getDutiesByUser(std::stoi(userId), startDate, endDate);
        } else {
            duties = dutyManager.getDuties(startDate, endDate);
        }
        
        json result = json::array();
        for (const auto& duty : duties) {
            result.push_back(dutyToJson(duty));
        }
        res.set_content(result.dump(), "application/json");
    });
    
    svr.Post("/api/duties/generate", [](const Request& req, Response& res) {
        try {
            auto body = json::parse(req.body);
            int daysAhead = body.value("daysAhead", 30);
            dutyManager.generateSchedule(daysAhead);
            json result;
            result["success"] = true;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
    
    svr.Post("/api/duties/(\\d+)/complete", [](const Request& req, Response& res) {
        try {
            int dutyId = std::stoi(req.matches[1]);
            auto body = json::parse(req.body);
            std::string notes = body.value("notes", "");
            bool success = dutyManager.completeDuty(dutyId, notes);
            json result;
            result["success"] = success;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
    
    svr.Post("/api/duties/(\\d+)/skip", [](const Request& req, Response& res) {
        try {
            int dutyId = std::stoi(req.matches[1]);
            auto body = json::parse(req.body);
            std::string notes = body.value("notes", "");
            bool success = dutyManager.skipDuty(dutyId, notes);
            json result;
            result["success"] = success;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
    
    svr.Delete("/api/duties/(\\d+)", [](const Request& req, Response& res) {
        int dutyId = std::stoi(req.matches[1]);
        bool success = dutyManager.deleteDuty(dutyId);
        json result;
        result["success"] = success;
        res.set_content(result.dump(), "application/json");
    });
    
    // API: Статистика
    svr.Get("/api/statistics", [](const Request& req, Response& res) {
        std::string startDate = req.get_param_value("startDate");
        std::string endDate = req.get_param_value("endDate");
        auto stats = dutyManager.getStatistics(startDate, endDate);
        json result;
        for (const auto& [key, value] : stats) {
            result[key] = value;
        }
        res.set_content(result.dump(), "application/json");
    });
    
    std::cout << "Сервер запущен на http://localhost:8080\n";
    std::cout << "Откройте браузер и перейдите по адресу http://localhost:8080\n";
    
    svr.listen("0.0.0.0", 8080);
    
    return 0;
}


#ifndef DUTY_MANAGER_H
#define DUTY_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <algorithm>
#include <memory>

struct Task {
    int id;
    std::string name;
    std::string description;
    int frequencyDays;  // Периодичность в днях
    int durationMinutes; // Длительность в минутах
    std::string category; // Категория (уборка, кухня, ванна и т.д.)
    
    Task() : id(0), frequencyDays(1), durationMinutes(30) {}
    Task(int id, const std::string& name, const std::string& desc, 
         int freq, int dur, const std::string& cat)
        : id(id), name(name), description(desc), 
          frequencyDays(freq), durationMinutes(dur), category(cat) {}
};

struct User {
    int id;
    std::string name;
    std::string email;
    bool active;
    
    User() : id(0), active(true) {}
    User(int id, const std::string& name, const std::string& email)
        : id(id), name(name), email(email), active(true) {}
};

struct Duty {
    int id;
    int taskId;
    int userId;
    std::string date;  // YYYY-MM-DD
    std::string status; // "pending", "completed", "skipped"
    std::string completedAt;
    std::string notes;
    
    Duty() : id(0), taskId(0), userId(0), status("pending") {}
    Duty(int id, int taskId, int userId, const std::string& date)
        : id(id), taskId(taskId), userId(userId), date(date), status("pending") {}
};

class DutyManager {
private:
    std::vector<Task> tasks;
    std::vector<User> users;
    std::vector<Duty> duties;
    int nextTaskId;
    int nextUserId;
    int nextDutyId;
    
    std::string getCurrentDate() const;
    std::string addDays(const std::string& date, int days) const;
    bool isDateBefore(const std::string& date1, const std::string& date2) const;
    int getDaysDifference(const std::string& date1, const std::string& date2) const;
    std::map<int, int> getUserDutyCounts(const std::string& startDate, const std::string& endDate) const;
    
public:
    DutyManager();
    
    // Управление задачами
    int addTask(const std::string& name, const std::string& description, 
                int frequencyDays, int durationMinutes, const std::string& category);
    bool removeTask(int taskId);
    std::vector<Task> getTasks() const;
    Task* getTask(int taskId);
    
    // Управление пользователями
    int addUser(const std::string& name, const std::string& email);
    bool removeUser(int userId);
    bool updateUser(int userId, const std::string& name, const std::string& email, bool active);
    std::vector<User> getUsers() const;
    User* getUser(int userId);
    
    // Управление дежурствами
    void generateSchedule(int daysAhead = 30);
    std::vector<Duty> getDuties(const std::string& startDate = "", const std::string& endDate = "") const;
    std::vector<Duty> getDutiesByUser(int userId, const std::string& startDate = "", const std::string& endDate = "") const;
    bool completeDuty(int dutyId, const std::string& notes = "");
    bool skipDuty(int dutyId, const std::string& notes = "");
    bool deleteDuty(int dutyId);
    
    // Статистика
    std::map<std::string, int> getStatistics(const std::string& startDate = "", const std::string& endDate = "") const;
    std::vector<std::map<std::string, std::string>> getUserStatistics(int userId) const;
};

#endif // DUTY_MANAGER_H



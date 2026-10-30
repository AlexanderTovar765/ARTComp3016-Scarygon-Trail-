#pragma once
#include <chrono>
#include <fstream>
#include <string>

// This part of the code is technically optional, anonymous playtest log. When switched on (data/settings.txt or the L key on the
// title screen) the game writes one CSV file per launch into playtest_logs/ with the choices
// made and how each run ended. It records nothing personal: just a random session id, the
// date, and elapsed time. Testers can send the file back as evidence for the playtesting
// section of the report.
class PlaytestLog {
public:
    ~PlaytestLog();

    // This part of the code opens a new log file in 'folder' (created if needed). Returns false if it cannot.
    bool start(const std::string& folder, const std::string& buildLabel);
    void stop();
    bool isOpen() const { return m_file.is_open(); }
    const std::string& sessionId() const { return m_sessionId; }
    const std::string& filePath() const { return m_path; }

    void logRunStart(const std::string& note = "new run");
    void logChoice(int day, const std::string& part, const std::string& variant, int choiceNumber,
                   const std::string& label, int food, int health, int sanity, int wagon,
                   const std::string& narrator, int alive);
    void logRunEnd(const std::string& outcome, int partsPlayed, int survivors,
                   int food, int health, int sanity, int wagon);

    // This part of the code quotes a field for CSV (doubles any quotes, wraps it if it holds a comma or quote).
    static std::string csvField(const std::string& text);

private:
    void row(const std::string& type, const std::string& day, const std::string& part,
             const std::string& variant, const std::string& choice, const std::string& label,
             const std::string& food, const std::string& health, const std::string& sanity,
             const std::string& wagon, const std::string& narrator, const std::string& alive,
             const std::string& note);

    std::ofstream m_file;
    std::string m_sessionId;
    std::string m_path;
    int m_run = 0;
    std::chrono::steady_clock::time_point m_started;
};

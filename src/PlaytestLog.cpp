#include "PlaytestLog.h"

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>

namespace {
    std::string randomSessionId() {
        std::random_device device;
        std::mt19937 rng(device());
        std::uniform_int_distribution<int> digit(0, 15);
        std::string id;
        for (int i = 0; i < 6; ++i) id += "0123456789abcdef"[digit(rng)];
        return id;
    }

    std::string utcDate() {
        const std::time_t now = std::time(nullptr);
        std::tm parts{};
#ifdef _WIN32
        gmtime_s(&parts, &now);
#else
        gmtime_r(&now, &parts);
#endif
        std::ostringstream out;
        out << std::put_time(&parts, "%Y-%m-%d");
        return out.str();
    }
}

PlaytestLog::~PlaytestLog() {
    stop();
}

std::string PlaytestLog::csvField(const std::string& text) {
    if (text.find_first_of(",\"\n\r") == std::string::npos) return text;
    std::string out = "\"";
    for (char c : text) {
        if (c == '"') out += "\"\"";
        else if (c != '\n' && c != '\r') out += c;
    }
    out += '"';
    return out;
}

bool PlaytestLog::start(const std::string& folder, const std::string& buildLabel) {
    stop();

    std::error_code error;
    std::filesystem::create_directories(folder, error);
    if (error) return false;

    m_sessionId = randomSessionId();
    m_path = (std::filesystem::path(folder) / ("session_" + m_sessionId + ".csv")).string();
    m_file.open(m_path, std::ios::out | std::ios::trunc);
    if (!m_file.is_open()) return false;

    m_run = 0;
    m_started = std::chrono::steady_clock::now();
    m_file << "# The Scarygon Trail - anonymous playtest log\n"
           << "# session: " << m_sessionId << "\n"
           << "# build: " << buildLabel << "\n"
           << "# date (UTC): " << utcDate() << "\n"
           << "seconds,run,type,day,part,variant,choice,label,food,health,sanity,wagon,narrator,alive,note\n";
    m_file.flush();
    return true;
}

void PlaytestLog::stop() {
    if (m_file.is_open()) {
        row("QUIT", "", "", "", "", "", "", "", "", "", "", "", "log closed");
        m_file.close();
    }
}

void PlaytestLog::row(const std::string& type, const std::string& day, const std::string& part,
                      const std::string& variant, const std::string& choice, const std::string& label,
                      const std::string& food, const std::string& health, const std::string& sanity,
                      const std::string& wagon, const std::string& narrator, const std::string& alive,
                      const std::string& note) {
    if (!m_file.is_open()) return;
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_started).count();
    m_file << std::fixed << std::setprecision(1) << seconds << "," << m_run << "," << type << "," << day << ","
           << csvField(part) << "," << csvField(variant) << "," << choice << "," << csvField(label) << ","
           << food << "," << health << "," << sanity << "," << wagon << "," << narrator << "," << alive << ","
           << csvField(note) << "\n";
    m_file.flush();  // flush every row so a crash or force-quit still leaves a usable log
}

void PlaytestLog::logRunStart() {
    ++m_run;
    row("START", "", "", "", "", "", "", "", "", "", "", "", "new run");
}

void PlaytestLog::logChoice(int day, const std::string& part, const std::string& variant, int choiceNumber,
                            const std::string& label, int food, int health, int sanity, int wagon,
                            const std::string& narrator, int alive) {
    row("CHOICE", std::to_string(day), part, variant, std::to_string(choiceNumber), label,
        std::to_string(food), std::to_string(health), std::to_string(sanity), std::to_string(wagon),
        narrator, std::to_string(alive), "");
}

void PlaytestLog::logRunEnd(const std::string& outcome, int partsPlayed, int survivors,
                            int food, int health, int sanity, int wagon) {
    row("END", "", "", "", "", "", std::to_string(food), std::to_string(health), std::to_string(sanity),
        std::to_string(wagon), "", std::to_string(survivors),
        outcome + "; parts played: " + std::to_string(partsPlayed));
}

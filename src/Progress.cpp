#include "Progress.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
    const char* const kEndingNames[kEndingCount] = {
        "intact", "lost_one", "half", "lone", "everyone_died", "sanity_lost", "wagon_lost",
    };
    const char* const kEndingTitles[kEndingCount] = {
        "The family is intact. Suspiciously.", "A small loss", "Half a family", "One survivor",
        "Nobody left", "Sanity lost", "Wagon lost",
    };

    std::string trim(const std::string& s) {
        std::size_t a = 0, b = s.size();
        while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
        return s.substr(a, b - a);
    }

    std::string lower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    std::vector<std::string> splitList(const std::string& text) {
        std::vector<std::string> out;
        std::stringstream ss(text);
        std::string item;
        while (std::getline(ss, item, ',')) {
            item = trim(item);
            if (!item.empty()) out.push_back(item);
        }
        return out;
    }
}

const char* endingName(EndingId id) {
    const int i = static_cast<int>(id);
    return (i >= 0 && i < kEndingCount) ? kEndingNames[i] : "intact";
}

const char* endingTitle(EndingId id) {
    const int i = static_cast<int>(id);
    return (i >= 0 && i < kEndingCount) ? kEndingTitles[i] : "";
}

EndingId Progress::classify(bool survived, int aliveCount, int sanity, int wagon) {
    if (survived) {
        switch (aliveCount) {
        case 4:  return EndingId::Intact;
        case 3:  return EndingId::LostOne;
        case 2:  return EndingId::Half;
        default: return EndingId::Lone;
        }
    }
    if (aliveCount <= 0) return EndingId::EveryoneDied;
    if (sanity <= 0)     return EndingId::SanityLost;
    if (wagon <= 0)      return EndingId::WagonLost;
    return EndingId::EveryoneDied;
}

Progress Progress::load(const std::string& path, std::vector<std::string>* warnings) {
    Progress p;
    std::ifstream file(path);
    if (!file.is_open()) return p;   // first run: nothing saved yet

    auto warn = [&](int line, const std::string& message) {
        if (warnings) warnings->push_back(path + " line " + std::to_string(line) + ": " + message);
    };

    std::string raw;
    int lineNumber = 0;
    while (std::getline(file, raw)) {
        ++lineNumber;
        const std::string line = trim(raw);
        if (line.empty() || line[0] == '#') continue;
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) { warn(lineNumber, "expected 'key = value'; ignored"); continue; }
        const std::string key = lower(trim(line.substr(0, equals)));
        const std::string value = trim(line.substr(equals + 1));

        if (key == "difficulty") {
            if (!parseDifficulty(value, p.difficulty)) warn(lineNumber, "unknown difficulty '" + value + "'; using Normal");
        } else if (key == "runs") {
            try { p.runs = std::max(0, std::stoi(value)); } catch (...) { warn(lineNumber, "'runs' must be a number"); }
        } else if (key == "scenes") {
            for (const std::string& name : splitList(value)) {
                SceneId id;
                if (parseScene(name, id)) p.m_scenes[static_cast<int>(id)] = true;
                else warn(lineNumber, "unknown scene '" + name + "'; ignored");
            }
        } else if (key == "actions") {
            for (const std::string& name : splitList(value)) {
                ActionId id;
                if (parseAction(name, id) && id != ActionId::None) p.m_actions[static_cast<int>(id)] = true;
                else warn(lineNumber, "unknown action '" + name + "'; ignored");
            }
        } else if (key == "endings") {
            for (const std::string& name : splitList(value)) {
                bool found = false;
                for (int i = 0; i < kEndingCount; ++i) {
                    if (lower(name) == kEndingNames[i]) { p.m_endings[i] = true; found = true; }
                }
                if (!found) warn(lineNumber, "unknown ending '" + name + "'; ignored");
            }
        } else if (key == "cleared") {
            for (const std::string& name : splitList(value)) {
                Difficulty d;
                if (parseDifficulty(name, d)) p.m_cleared[static_cast<int>(d)] = true;
                else warn(lineNumber, "unknown difficulty '" + name + "'; ignored");
            }
        } else {
            warn(lineNumber, "unknown key '" + key + "'; ignored");
        }
    }
    return p;
}

bool Progress::save(const std::string& path) const {
    std::error_code ec;
    const std::filesystem::path file(path);
    if (file.has_parent_path()) std::filesystem::create_directories(file.parent_path(), ec);

    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) return false;

    auto list = [&](const char* key, int count, auto nameOf, auto isSet) {
        out << key << " = ";
        bool first = true;
        for (int i = 0; i < count; ++i) {
            if (!isSet(i)) continue;
            out << (first ? "" : ",") << nameOf(i);
            first = false;
        }
        out << "\n";
    };

    out << "# Saved by The Scarygon Trail. Safe to delete: the game just starts a fresh record.\n";
    out << "difficulty = " << lower(difficultyName(difficulty)) << "\n";
    out << "runs = " << runs << "\n";
    list("scenes", kSceneCount, [](int i) { return sceneName(static_cast<SceneId>(i)); },
         [&](int i) { return m_scenes[i]; });
    list("actions", kActionCount + 1, [](int i) { return actionName(static_cast<ActionId>(i)); },
         [&](int i) { return i > 0 && m_actions[i]; });
    list("endings", kEndingCount, [](int i) { return endingName(static_cast<EndingId>(i)); },
         [&](int i) { return m_endings[i]; });
    list("cleared", 3, [](int i) { return lower(difficultyName(static_cast<Difficulty>(i))); },
         [&](int i) { return m_cleared[i]; });
    return out.good();
}

bool Progress::markScene(SceneId id) {
    const int i = static_cast<int>(id);
    if (i < 0 || i >= kSceneCount || m_scenes[i]) return false;
    m_scenes[i] = true;
    return true;
}

bool Progress::markAction(ActionId id) {
    const int i = static_cast<int>(id);
    if (i <= 0 || i > kActionCount || m_actions[i]) return false;
    m_actions[i] = true;
    return true;
}

bool Progress::markEnding(EndingId id) {
    const int i = static_cast<int>(id);
    if (i < 0 || i >= kEndingCount || m_endings[i]) return false;
    m_endings[i] = true;
    return true;
}

void Progress::markCleared(Difficulty d) { m_cleared[static_cast<int>(d)] = true; }

bool Progress::sceneSeen(SceneId id) const   { const int i = static_cast<int>(id); return i >= 0 && i < kSceneCount && m_scenes[i]; }
bool Progress::actionSeen(ActionId id) const { const int i = static_cast<int>(id); return i > 0 && i <= kActionCount && m_actions[i]; }
bool Progress::endingSeen(EndingId id) const { const int i = static_cast<int>(id); return i >= 0 && i < kEndingCount && m_endings[i]; }
bool Progress::cleared(Difficulty d) const   { return m_cleared[static_cast<int>(d)]; }

int Progress::scenesSeen() const  { int n = 0; for (bool b : m_scenes)  n += b ? 1 : 0; return n; }
int Progress::actionsSeen() const { int n = 0; for (int i = 1; i <= kActionCount; ++i) n += m_actions[i] ? 1 : 0; return n; }
int Progress::endingsSeen() const { int n = 0; for (bool b : m_endings) n += b ? 1 : 0; return n; }
int Progress::clearedCount() const { int n = 0; for (bool b : m_cleared) n += b ? 1 : 0; return n; }

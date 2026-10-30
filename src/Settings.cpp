#include "Settings.h"

#include <cctype>
#include <fstream>

namespace {
    std::string trim(const std::string& s) {
        std::size_t start = 0, end = s.size();
        while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    std::string lower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    bool parseBool(const std::string& text, bool& out) {
        const std::string v = lower(text);
        if (v == "on" || v == "true" || v == "yes" || v == "1")  { out = true;  return true; }
        if (v == "off" || v == "false" || v == "no" || v == "0") { out = false; return true; }
        return false;
    }
}

Settings Settings::load(const std::string& path) {
    Settings settings;
    std::ifstream file(path);
    if (!file.is_open()) return settings;  // This part of the code is for optional file: defaults should be fine

    std::string raw;
    int lineNumber = 0;
    while (std::getline(file, raw)) {
        ++lineNumber;
        if (lineNumber == 1 && raw.compare(0, 3, "\xEF\xBB\xBF") == 0) raw.erase(0, 3);  // This part of the code is for the UTF-8 BOM
        const std::string line = trim(raw);
        if (line.empty() || line[0] == '#') continue;

        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            settings.warnings.push_back(path + " line " + std::to_string(lineNumber) + ": expected 'key = value'");
            continue;
        }
        const std::string key = lower(trim(line.substr(0, equals)));
        const std::string value = trim(line.substr(equals + 1));

        bool flag = false;
        if (key == "audio" || key == "playtest_log") {
            if (!parseBool(value, flag)) {
                settings.warnings.push_back(path + " line " + std::to_string(lineNumber) +
                                            ": '" + key + "' must be on or off");
            } else if (key == "audio") {
                settings.audio = flag;
            } else {
                settings.playtestLog = flag;
            }
        } else {
            settings.warnings.push_back(path + " line " + std::to_string(lineNumber) +
                                        ": unknown setting '" + key + "'");
        }
    }
    return settings;
}

#pragma once
#include <string>
#include <vector>

// Optional game settings read from data/settings.txt (lines like "audio = on").
// A missing file just means defaults; a bad line is reported and ignored.
struct Settings {
    bool audio = true;          // play sound (only if irrKlang is available)
    bool playtestLog = false;   // write an anonymous log of the choices made (see PlaytestLog)
    std::vector<std::string> warnings;

    static Settings load(const std::string& path);
};

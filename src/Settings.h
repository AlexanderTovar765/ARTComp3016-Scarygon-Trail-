#pragma once
#include <string>
#include <vector>

// This part of the code is for optional game settings read from data/settings.txt (lines like "audio = on").
// A missing file just means defaults; a bad line is reported and ignored.
struct Settings {
    bool audio = true;          // This part of the code will play sound (only if irrKlang is available)
    bool playtestLog = false;   // This part of the code is for writing an anonymous log of the choices made (see PlaytestLog)
    std::vector<std::string> warnings;

    static Settings load(const std::string& path);
};

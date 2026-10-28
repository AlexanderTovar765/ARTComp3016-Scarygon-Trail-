#include "Animations.h"

#include <cctype>

namespace {
    const char* const kSceneNames[kSceneCount] = {
        "trail", "oxen_stare", "map_man", "deer", "herd", "ritual", "figure", "crowd", "tidy_wagon",
        "river", "travellers", "oxen_circle", "voice", "new_road", "cook_camp",
    };
    const char* const kActionNames[kActionCount + 1] = {
        "none", "feed", "eat", "rush", "ask", "buy", "refuse", "flee", "fight", "hide", "pray", "steal",
        "sleep", "give", "cross", "inspect", "shoot", "join", "wait", "build", "burn",
    };

    std::string normalise(const std::string& text) {
        std::string out;
        for (char c : text) {
            if (c == '-' || c == ' ') c = '_';
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return out;
    }
}

const char* sceneName(SceneId id) {
    const int i = static_cast<int>(id);
    return (i >= 0 && i < kSceneCount) ? kSceneNames[i] : "trail";
}

const char* actionName(ActionId id) {
    const int i = static_cast<int>(id);
    return (i >= 0 && i <= kActionCount) ? kActionNames[i] : "none";
}

bool parseScene(const std::string& text, SceneId& out) {
    const std::string v = normalise(text);
    for (int i = 0; i < kSceneCount; ++i) {
        if (v == kSceneNames[i]) { out = static_cast<SceneId>(i); return true; }
    }
    return false;
}

bool parseAction(const std::string& text, ActionId& out) {
    const std::string v = normalise(text);
    for (int i = 0; i <= kActionCount; ++i) {
        if (v == kActionNames[i]) { out = static_cast<ActionId>(i); return true; }
    }
    return false;
}

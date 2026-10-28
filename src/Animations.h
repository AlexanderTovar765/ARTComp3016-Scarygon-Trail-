#pragma once
#include <string>

// The vocabulary of the top-panel animations. events.txt picks from it by name:
//   scene  = ...   (an [EVENT] key)  the backdrop and its "actor" while the event is on screen
//   action = ...   (a [CHOICE] key)  what the family does when that choice is picked
// The pictures are drawn in code by SceneRenderer, so there are no image files.
enum class SceneId {
    Trail = 0,    // the default: the wagon and its oxen on the road
    OxenStare,    // the oxen, staring
    MapMan,       // a roadside map seller
    Deer,         // a very calm deer
    Herd,         // the deer brought friends (one is wearing your hat)
    Ritual,       // the son's circle of candles
    Figure,       // something in your coat at the edge of the firelight
    Crowd,        // lots of somethings in your coat
    TidyWagon,    // an overnight repack
    River,        // SHALLOW (sign) / NOT ANYMORE (smaller sign)
    Travellers,   // smiling at the same speed
    OxenCircle,   // the oxen's meeting
    Voice,        // a voice in the dark that sounds like you
    NewRoad,      // a road that is a little too smooth
    CookCamp,     // a camp with a pot of something wonderful
    Count
};

enum class ActionId {
    None = 0,
    Feed, Eat, Rush, Ask, Buy, Refuse, Flee, Fight, Hide, Pray, Steal, Sleep, Give,
    Cross, Inspect, Shoot, Join, Wait, Build, Burn,
    Count
};

constexpr int kSceneCount  = static_cast<int>(SceneId::Count);
constexpr int kActionCount = static_cast<int>(ActionId::Count) - 1;   // None is not a discoverable animation

const char* sceneName(SceneId id);                              // "oxen_stare", "map_man", ...
const char* actionName(ActionId id);                            // "feed", "flee", ...
bool parseScene(const std::string& text, SceneId& out);         // case-insensitive; '-' and ' ' count as '_'
bool parseAction(const std::string& text, ActionId& out);

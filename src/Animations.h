#pragma once
#include <string>

// This part of the code is for setting the animations on each event. (e.g. if the user chooses to feed the oxen, 
// the animation will register that moment instead of the other 3.)
enum class SceneId {
    Trail = 0,    
    OxenStare,  
    MapMan,       
    Deer,         
    Herd,         
    Ritual,       
    Figure,       
    Crowd,       
    TidyWagon,    
    River,       
    Travellers,   
    OxenCircle,   
    Voice,        
    NewRoad,     
    CookCamp,    
    Count
};

enum class ActionId {
    None = 0,
    Feed, Eat, Rush, Ask, Buy, Refuse, Flee, Fight, Hide, Pray, Steal, Sleep, Give,
    Cross, Inspect, Shoot, Join, Wait, Build, Burn,
    Count
};

constexpr int kSceneCount  = static_cast<int>(SceneId::Count);
constexpr int kActionCount = static_cast<int>(ActionId::Count) - 1;   

const char* sceneName(SceneId id);                              
const char* actionName(ActionId id);                           
bool parseScene(const std::string& text, SceneId& out);        
bool parseAction(const std::string& text, ActionId& out);

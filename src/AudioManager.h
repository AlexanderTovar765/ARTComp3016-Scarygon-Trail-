#pragma once
#include <memory>
#include <string>

// Short sound effects.
enum class Sfx { Click, Good, Bad, Warning, Death, DayEnd, StateChange, Start };

// Looping background mood. It follows the narrator: calm while the party is sane,
// uneasy when the game starts to lie, dread during a breakdown.
enum class Mood { Silent, Calm, Uneasy, Dread };

// Plays sound through irrKlang (one of the libraries the module allows). irrKlang is only
// used if the project was built with SCARYGON_USE_IRRKLANG (the build files switch it on when
// vendored/irrKlang exists; run tools/get-irrklang.ps1 to fetch it). Without it, or if there is
// no audio device, every call here is a harmless no-op and the game stays silent.
class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // 'folder' holds the .wav files (assets/audio). 'useNullDevice' is for automated tests:
    // it runs the engine without making any noise. Returns true if sound is available.
    bool init(const std::string& folder, bool useNullDevice = false);

    bool isAvailable() const;
    static bool builtWithIrrKlang();

    void setMood(Mood mood);
    void play(Sfx sfx);

    void setMuted(bool muted);
    bool isMuted() const;
    Mood getMood() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

#pragma once
#include <memory>
#include <string>

// This part of the code is for the short sound effects.
enum class Sfx { Click, Good, Bad, Warning, Death, DayEnd, StateChange, Start };

// This part of the code is for the looping background mood. It follows the narrator: calm while the party is sane,
// uneasy when the game begins to lie, dread during a breakdown.
enum class Mood { Silent, Calm, Uneasy, Dread };

// This part of the code is for playing sound through irrKlang (one of the libraries the module allows). irrKlang is only
// used if the project was built with SCARYGON_USE_IRRKLANG (the build files switch it on when
// vendored/irrKlang exists; run tools/get-irrklang.ps1 to fetch it). Without it, or if there is
// no audio device, every call here will not activate but not effect the game, itll just be silent.
class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // This part of the code is for the 'folder' to hold the .wav files (assets/audio). 'useNullDevice' is for automated tests:
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

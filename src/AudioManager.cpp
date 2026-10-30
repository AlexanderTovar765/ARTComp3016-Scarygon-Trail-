#include "AudioManager.h"

#include <iostream>

#ifdef SCARYGON_USE_IRRKLANG
#include <irrKlang.h>
#endif

struct AudioManager::Impl {
    std::string folder;
    Mood mood = Mood::Silent;
    bool muted = false;
    bool available = false;

#ifdef SCARYGON_USE_IRRKLANG
    irrklang::ISoundEngine* engine = nullptr;
    irrklang::ISound* music = nullptr;
#endif

    static const char* moodFile(Mood mood) {
        switch (mood) {
        case Mood::Calm:   return "ambient_calm.wav";
        case Mood::Uneasy: return "ambient_uneasy.wav";
        case Mood::Dread:  return "ambient_dread.wav";
        case Mood::Silent: break;
        }
        return nullptr;
    }

    static const char* sfxFile(Sfx sfx) {
        switch (sfx) {
        case Sfx::Click:       return "click.wav";
        case Sfx::Good:        return "good.wav";
        case Sfx::Bad:         return "bad.wav";
        case Sfx::Warning:     return "warning.wav";
        case Sfx::Death:       return "death.wav";
        case Sfx::DayEnd:      return "day_end.wav";
        case Sfx::StateChange: return "state_change.wav";
        case Sfx::Start:       return "start.wav";
        }
        return nullptr;
    }

#ifdef SCARYGON_USE_IRRKLANG
    void stopMusic() {
        if (music) {
            music->stop();
            music->drop();  // This part of the code is for the irrKlang objects, it makes sure they are reference counted: drop() and releases our held.
            music = nullptr;
        }
    }

    void startMusic() {
        stopMusic();
        const char* file = moodFile(mood);
        if (!engine || !file) return;
        // This part of the code is for the loop = true, start paused = false, track = true section (so theres an ISound* to stop later).
        music = engine->play2D((folder + "/" + file).c_str(), true, false, true);
    }
#endif
};

AudioManager::AudioManager() : m_impl(std::make_unique<Impl>()) {}

AudioManager::~AudioManager() {
#ifdef SCARYGON_USE_IRRKLANG
    if (m_impl) {
        m_impl->stopMusic();
        if (m_impl->engine) {
            m_impl->engine->drop();
            m_impl->engine = nullptr;
        }
    }
#endif
}

bool AudioManager::builtWithIrrKlang() {
#ifdef SCARYGON_USE_IRRKLANG
    return true;
#else
    return false;
#endif
}

bool AudioManager::init(const std::string& folder, bool useNullDevice) {
    m_impl->folder = folder;

#ifdef SCARYGON_USE_IRRKLANG
    m_impl->engine = irrklang::createIrrKlangDevice(
        useNullDevice ? irrklang::ESOD_NULL : irrklang::ESOD_AUTO_DETECT);
    if (!m_impl->engine) {
        std::cerr << "[audio] could not start irrKlang (no sound device?). Continuing without sound.\n";
        return false;
    }
    m_impl->engine->setSoundVolume(0.8f);
    m_impl->available = true;
    return true;
#else
    (void)useNullDevice;
    std::cerr << "[audio] built without irrKlang, so the game will be silent "
                 "(run tools/get-irrklang.ps1 to add it).\n";
    return false;
#endif
}

bool AudioManager::isAvailable() const { return m_impl->available; }
bool AudioManager::isMuted() const     { return m_impl->muted; }
Mood AudioManager::getMood() const     { return m_impl->mood; }

void AudioManager::setMood(Mood mood) {
    if (mood == m_impl->mood) return;
    m_impl->mood = mood;
#ifdef SCARYGON_USE_IRRKLANG
    if (m_impl->available && !m_impl->muted) m_impl->startMusic();
#endif
}

void AudioManager::play(Sfx sfx) {
#ifdef SCARYGON_USE_IRRKLANG
    if (!m_impl->available || m_impl->muted) return;
    const char* file = Impl::sfxFile(sfx);
    if (file) m_impl->engine->play2D((m_impl->folder + "/" + file).c_str());  // This part of the code is for the fire and forget.
#else
    (void)sfx;
#endif
}

void AudioManager::setMuted(bool muted) {
    m_impl->muted = muted;
#ifdef SCARYGON_USE_IRRKLANG
    if (!m_impl->available) return;
    if (muted) m_impl->stopMusic();
    else       m_impl->startMusic();
#endif
}

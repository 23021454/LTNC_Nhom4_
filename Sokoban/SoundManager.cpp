#include "SoundManager.h"
#include <iostream>

SoundManager::SoundManager() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

SoundManager::~SoundManager() {
    cleanUp();
    Mix_CloseAudio();
}

bool SoundManager::loadSound(const std::string& id, const std::string& filePath) {
    Mix_Chunk* sound = Mix_LoadWAV(filePath.c_str());
    if (sound == nullptr) {
        std::cout << "Failed to load sound: " << filePath << " Error: " << Mix_GetError() << std::endl;
        return false;
    }

    m_sounds[id] = sound;
    return true;
}

void SoundManager::playSound(const std::string& id, int loops) {
    if (m_sounds.count(id)) {
        Mix_PlayChannel(-1, m_sounds[id], loops);
    }
}

void SoundManager::stopSound(const std::string& id) {
    // Stop all channels playing this sound (optional: can be expanded)
    Mix_HaltChannel(-1);
}

void SoundManager::cleanUp() {
    for (auto& pair : m_sounds) {
        Mix_FreeChunk(pair.second);
    }
    m_sounds.clear();
}

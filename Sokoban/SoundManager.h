#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SDL_mixer.h>
#include <string>
#include <map>

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    bool loadSound(const std::string& id, const std::string& filePath);
    void playSound(const std::string& id, int loops = 0);
    void stopSound(const std::string& id);
    void cleanUp();

private:
    std::map<std::string, Mix_Chunk*> m_sounds;
};

#endif // SOUNDMANAGER_H

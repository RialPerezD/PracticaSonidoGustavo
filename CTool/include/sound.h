#pragma once
#include <../deps/OpenAL/include/AL/al.h>
#include <../deps/OpenAL/include/AL/alc.h>
#include <string>
#include <vector>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool Init();
    int LoadWav(const std::string& filename);
    void Play(int index, bool loop = true);
    void Stop(int index);
    void Close();

    void Update(float deltaTime);
    void Crossfade(int fromIndex, int toIndex, float duration);

private:
    struct Fade {
        int from = -1;
        int to = -1;
        float duration = 0.0f;
        float elapsed = 0.0f;
        bool active = false;
    };

    ALCdevice* device_;
    ALCcontext* context_;
    std::vector<ALuint> sources_;
    std::vector<ALuint> buffers_;
    Fade fade_;
};

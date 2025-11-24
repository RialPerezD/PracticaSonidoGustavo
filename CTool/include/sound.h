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

    void Play(int index, bool loop = false);
    void Stop(int index);
    void Close();
    void SetVolume(int index, float gain);

    void Update(float deltaTime);
    void Crossfade(int fromIndex, int toIndex, float duration);

    void UpdateSpatial2D(float listenerX, float listenerY);
    void SetSourcePosition(int index, float x, float y);
    void Register2DSound(int index, float x, float y, float maxDistance);

private:
    struct Fade {
        int from = -1;
        int to = -1;
        float duration = 0.0f;
        float elapsed = 0.0f;
        bool active = false;
    };

    struct SoundSource2D {
        int sourceIndex;
        float x, y;
        float maxDistance;
    };

    ALCdevice* device_;
    ALCcontext* context_;
    std::vector<ALuint> sources_;
    std::vector<ALuint> buffers_;
    std::vector<SoundSource2D> spatialSources_;
    Fade fade_;
};

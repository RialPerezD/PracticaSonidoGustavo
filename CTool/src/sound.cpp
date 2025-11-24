#include <sound.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <algorithm>

AudioManager::AudioManager()
    : device_(nullptr), context_(nullptr) {
}

AudioManager::~AudioManager() {
    Close();
}


void CheckErrors() {
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        const char* errMsg = "";
        switch (err) {
        case AL_INVALID_NAME:      errMsg = "AL_INVALID_NAME"; break;
        case AL_INVALID_ENUM:      errMsg = "AL_INVALID_ENUM"; break;
        case AL_INVALID_VALUE:     errMsg = "AL_INVALID_VALUE"; break;
        case AL_INVALID_OPERATION: errMsg = "AL_INVALID_OPERATION"; break;
        case AL_OUT_OF_MEMORY:     errMsg = "AL_OUT_OF_MEMORY"; break;
        default:                   errMsg = "Unknown OpenAL error"; break;
        }
        std::cout << "OpenAL Error: " << errMsg << std::endl;
    }
}


bool AudioManager::Init() {
    device_ = alcOpenDevice(nullptr);
    if (!device_) return false;

    context_ = alcCreateContext(device_, nullptr);
    if (!context_) {
        alcCloseDevice(device_);
        device_ = nullptr;
        return false;
    }

    if (!alcMakeContextCurrent(context_)) {
        alcDestroyContext(context_);
        alcCloseDevice(device_);
        context_ = nullptr;
        device_ = nullptr;
        return false;
    }

    return true;
}


int AudioManager::LoadWav(const std::string& filename) {
    ALuint buffer;
    alGenBuffers(1, &buffer);

    std::ifstream file(filename, std::ios::binary);
    if (!file) return -1;

    char riff[4]; file.read(riff, 4);
    file.ignore(4);
    char wave[4]; file.read(wave, 4);

    short audioFormat = 0, numChannels = 0, bitsPerSample = 0;
    int sampleRate = 0;

    bool fmtFound = false;
    bool dataFound = false;
    ALenum format = 0;
    std::vector<char> data;

    while (!file.eof() && (!fmtFound || !dataFound)) {
        char chunkId[4];
        int chunkSize = 0;
        file.read(chunkId, 4);
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (std::strncmp(chunkId, "fmt ", 4) == 0) {
            fmtFound = true;
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            file.ignore(6);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);
            if (chunkSize > 16) file.ignore(chunkSize - 16);
        }
        else if (std::strncmp(chunkId, "data", 4) == 0) {
            dataFound = true;
            data.resize(chunkSize);
            file.read(data.data(), chunkSize);
        }
        else {
            file.ignore(chunkSize);
        }
    }

    if (!fmtFound || !dataFound) return -1;

    if (numChannels == 1 && bitsPerSample == 8) format = AL_FORMAT_MONO8;
    else if (numChannels == 1 && bitsPerSample == 16) format = AL_FORMAT_MONO16;
    else if (numChannels == 2 && bitsPerSample == 8) format = AL_FORMAT_STEREO8;
    else if (numChannels == 2 && bitsPerSample == 16) format = AL_FORMAT_STEREO16;
    else return -1;

    alBufferData(buffer, format, data.data(), static_cast<ALsizei>(data.size()), sampleRate);
    CheckErrors();

    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, 1.0f);

    buffers_.push_back(buffer);
    sources_.push_back(source);

    return static_cast<int>(sources_.size() - 1);
}


void AudioManager::Play(int index, bool loop) {
    if (index < 0 || index >= static_cast<int>(sources_.size())) return;
    alSourcei(sources_[index], AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcePlay(sources_[index]);
}


void AudioManager::Stop(int index) {
    if (index < 0 || index >= static_cast<int>(sources_.size())) return;
    alSourceStop(sources_[index]);
}


void AudioManager::Update(float deltaTime) {
    if (fade_.active) {
        fade_.elapsed += deltaTime;
        float t = fade_.elapsed / fade_.duration;
        if (t >= 1.0f) {
            t = 1.0f;
            fade_.active = false;
            alSourceStop(sources_[fade_.from]);
        }
        float gainFrom = 1.0f - t;
        float gainTo = t;
        alSourcef(sources_[fade_.from], AL_GAIN, gainFrom);
        alSourcef(sources_[fade_.to], AL_GAIN, gainTo);
    }
}


void AudioManager::Crossfade(int fromIndex, int toIndex, float duration) {
    if (fromIndex < 0 || fromIndex >= static_cast<int>(sources_.size())) return;
    if (toIndex < 0 || toIndex >= static_cast<int>(sources_.size())) return;

    fade_.from = fromIndex;
    fade_.to = toIndex;
    fade_.duration = duration;
    fade_.elapsed = 0.0f;
    fade_.active = true;

    alSourcePlay(sources_[toIndex]);
}


void AudioManager::Close() {
    for (auto src : sources_) alDeleteSources(1, &src);
    for (auto buf : buffers_) alDeleteBuffers(1, &buf);
    sources_.clear();
    buffers_.clear();
    if (context_) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context_);
        context_ = nullptr;
    }
    if (device_) {
        alcCloseDevice(device_);
        device_ = nullptr;
    }
}


void AudioManager::SetVolume(int index, float gain) {
    if (index < 0 || index >= sources_.size()) return;
    alSourcef(sources_[index], AL_GAIN, gain);
}


void AudioManager::Register2DSound(int index, float x, float y, float maxDistance) {
    spatialSources_.push_back({ index, x, y, maxDistance });
}


void AudioManager::SetSourcePosition(int index, float x, float y) {
    for (auto& s : spatialSources_) {
        if (s.sourceIndex == index) {
            s.x = x;
            s.y = y;
            return;
        }
    }
}


void AudioManager::UpdateSpatial2D(float listenerX, float listenerY) {
    float fx = -1.0f;
    float fy = 0.0f;

    float rightX = 1.0f;
    float rightY = 0.0f;

    for (auto& s : spatialSources_) {

        float dx = s.x - listenerX;
        float dy = s.y - listenerY;
        float distance = std::sqrt(dx * dx + dy * dy);

        float d = std::clamp(1.0f - (distance / s.maxDistance), 0.0f, 1.0f);

        float dotRight = dx * rightX + dy * rightY;

        float panning = std::clamp(dotRight / s.maxDistance, -1.0f, 1.0f);

        alSource3f(sources_[s.sourceIndex], AL_POSITION, panning, 0.0f, 0.0f);
        alSourcef(sources_[s.sourceIndex], AL_GAIN, d);
    }
}


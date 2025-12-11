/**
 * @file sound.cpp
 * @brief Implementation of the AudioManager class for OpenAL-based audio management.
 *
 * This file provides functionality for initializing OpenAL, loading WAV files,
 * playing, stopping, controlling volume, and handling 2D spatial audio.
 */

#include <sound.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath> // For std::sqrt

 /**
  * @brief Default constructor for AudioManager.
  *
  * Initializes the OpenAL device and context pointers to null.
  */
AudioManager::AudioManager()
    : device_(nullptr), context_(nullptr) {
}

/**
 * @brief Destructor for AudioManager.
 *
 * Calls the Close method to properly release all OpenAL resources.
 */
AudioManager::~AudioManager() {
    Close();
}

/**
 * @brief Checks for and reports any OpenAL errors.
 *
 * Queries the current OpenAL error state and prints a descriptive message
 * to the console if an error is detected. This function is generally used
 * internally after OpenAL calls to ensure operation integrity.
 */
void CheckErrors() {
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        const char* errMsg = "";
        switch (err) {
        case AL_INVALID_NAME:        errMsg = "AL_INVALID_NAME"; break;
        case AL_INVALID_ENUM:        errMsg = "AL_INVALID_ENUM"; break;
        case AL_INVALID_VALUE:       errMsg = "AL_INVALID_VALUE"; break;
        case AL_INVALID_OPERATION:   errMsg = "AL_INVALID_OPERATION"; break;
        case AL_OUT_OF_MEMORY:       errMsg = "AL_OUT_OF_MEMORY"; break;
        default:                     errMsg = "Unknown OpenAL error"; break;
        }
        std::cout << "OpenAL Error: " << errMsg << std::endl;
    }
}

/**
 * @brief Initializes the OpenAL device and context.
 *
 * Attempts to open the default audio device, creates a rendering context,
 * and makes the context current on the calling thread. All resources are
 * cleaned up if any step fails.
 *
 * @return True if initialization is successful, false otherwise.
 */
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

/**
 * @brief Loads audio data from a WAV file into an OpenAL buffer and creates a source.
 *
 * This function parses the RIFF/WAVE file structure to extract format information
 * (channels, bits per sample, sample rate) and raw audio data. It then creates
 * an OpenAL buffer, uploads the data, creates an OpenAL source, and links them.
 *
 * @param filename The path to the WAV file.
 * @return The index of the newly created source in the internal vector, or -1 on failure.
 */
int AudioManager::LoadWav(const std::string& filename) {
    ALuint buffer;
    alGenBuffers(1, &buffer);

    std::ifstream file(filename, std::ios::binary);
    if (!file) return -1;

    // Check RIFF and WAVE header
    char riff[4]; file.read(riff, 4);
    file.ignore(4); // Chunk size
    char wave[4]; file.read(wave, 4);

    short audioFormat = 0, numChannels = 0, bitsPerSample = 0;
    int sampleRate = 0;

    bool fmtFound = false;
    bool dataFound = false;
    ALenum format = 0;
    std::vector<char> data;

    // Iterate through chunks to find "fmt " and "data"
    while (!file.eof() && (!fmtFound || !dataFound)) {
        char chunkId[4];
        int chunkSize = 0;
        file.read(chunkId, 4);
        if (file.eof()) break;
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (std::strncmp(chunkId, "fmt ", 4) == 0) {
            fmtFound = true;
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            file.ignore(6); // ByteRate and BlockAlign
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);
            if (chunkSize > 16) file.ignore(chunkSize - 16); // Skip extra format info
        }
        else if (std::strncmp(chunkId, "data", 4) == 0) {
            dataFound = true;
            data.resize(chunkSize);
            file.read(data.data(), chunkSize);
        }
        else {
            file.ignore(chunkSize); // Skip unknown chunk
        }
    }

    if (!fmtFound || !dataFound) return -1;

    // Determine the OpenAL format
    if (numChannels == 1 && bitsPerSample == 8) format = AL_FORMAT_MONO8;
    else if (numChannels == 1 && bitsPerSample == 16) format = AL_FORMAT_MONO16;
    else if (numChannels == 2 && bitsPerSample == 8) format = AL_FORMAT_STEREO8;
    else if (numChannels == 2 && bitsPerSample == 16) format = AL_FORMAT_STEREO16;
    else return -1; // Unsupported format

    // Buffer the audio data
    alBufferData(buffer, format, data.data(), static_cast<ALsizei>(data.size()), sampleRate);
    CheckErrors();

    // Create a source and attach the buffer
    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, 1.0f); // Default volume

    // Store the buffer and source handles
    buffers_.push_back(buffer);
    sources_.push_back(source);

    return static_cast<int>(sources_.size() - 1);
}

/**
 * @brief Starts playback for the audio source at the specified index.
 *
 * @param index The index of the audio source to play (returned by LoadWav).
 * @param loop If true, the sound will loop continuously.
 */
void AudioManager::Play(int index, bool loop) {
    if (index < 0 || index >= static_cast<int>(sources_.size())) return;
    alSourcei(sources_[index], AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcePlay(sources_[index]);
}

/**
 * @brief Stops playback for the audio source at the specified index.
 *
 * @param index The index of the audio source to stop.
 */
void AudioManager::Stop(int index) {
    if (index < 0 || index >= static_cast<int>(sources_.size())) return;
    alSourceStop(sources_[index]);
}

/**
 * @brief Updates the state of ongoing effects, primarily crossfading.
 *
 * This method must be called regularly within the main game loop to progress
 * timed audio effects like crossfades.
 *
 * @param deltaTime The time elapsed since the last update, typically in seconds.
 */
void AudioManager::Update(float deltaTime) {
    if (fade_.active) {
        fade_.elapsed += deltaTime;
        float t = fade_.elapsed / fade_.duration; // Normalized time [0, 1]

        if (t >= 1.0f) {
            t = 1.0f;
            fade_.active = false;
            // Ensure the 'from' source is stopped once fade is complete
            alSourceStop(sources_[fade_.from]);
        }

        // Apply fading gains (linear interpolation)
        float gainFrom = 1.0f - t;
        float gainTo = t;
        alSourcef(sources_[fade_.from], AL_GAIN, gainFrom);
        alSourcef(sources_[fade_.to], AL_GAIN, gainTo);
    }
}

/**
 * @brief Initiates a crossfade transition between two audio sources.
 *
 * Starts the 'to' sound and gradually decreases the volume of the 'from' sound
 * while increasing the volume of the 'to' sound over the specified duration.
 *
 * @param fromIndex The index of the source to fade out.
 * @param toIndex The index of the source to fade in.
 * @param duration The length of the transition in seconds.
 */
void AudioManager::Crossfade(int fromIndex, int toIndex, float duration) {
    if (fromIndex < 0 || fromIndex >= static_cast<int>(sources_.size())) return;
    if (toIndex < 0 || toIndex >= static_cast<int>(sources_.size())) return;

    // Configure and activate the fade state
    fade_.from = fromIndex;
    fade_.to = toIndex;
    fade_.duration = duration;
    fade_.elapsed = 0.0f;
    fade_.active = true;

    // Start the destination sound immediately
    alSourcePlay(sources_[toIndex]);
}

/**
 * @brief Releases all OpenAL resources and closes the device and context.
 *
 * Deletes all sources and buffers, destroys the context, and closes the device.
 * This should be called before application exit.
 */
void AudioManager::Close() {
    // Delete sources and buffers
    for (auto src : sources_) alDeleteSources(1, &src);
    for (auto buf : buffers_) alDeleteBuffers(1, &buf);
    sources_.clear();
    buffers_.clear();

    // Destroy context and close device
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

/**
 * @brief Sets the volume (gain) for a specific audio source.
 *
 * The gain value is linear, where 1.0 is default volume.
 *
 * @param index The index of the audio source.
 * @param gain The desired volume level (e.g., 0.0 for mute, 1.0 for full volume).
 */
void AudioManager::SetVolume(int index, float gain) {
    if (index < 0 || index >= sources_.size()) return;
    alSourcef(sources_[index], AL_GAIN, gain);
}

/**
 * @brief Registers an existing audio source as a 2D spatial sound.
 *
 * This stores the initial position and maximum audible distance for the source,
 * allowing its volume and panning to be calculated based on the listener's position.
 *
 * @param index The index of the audio source (returned by LoadWav).
 * @param x The initial X-coordinate of the sound source in world units.
 * @param y The initial Y-coordinate of the sound source in world units.
 * @param maxDistance The distance at which the sound is completely attenuated.
 */
void AudioManager::Register2DSound(int index, float x, float y, float maxDistance) {
    spatialSources_.push_back({ index, x, y, maxDistance });
}

/**
 * @brief Updates the world position of a registered 2D spatial sound source.
 *
 * @param index The index of the spatial sound source to update.
 * @param x The new X-coordinate.
 * @param y The new Y-coordinate.
 */
void AudioManager::SetSourcePosition(int index, float x, float y) {
    for (auto& s : spatialSources_) {
        if (s.sourceIndex == index) {
            s.x = x;
            s.y = y;
            return;
        }
    }
}

/**
 * @brief Updates the gain and panning for all registered 2D spatial sounds.
 *
 * Calculates distance-based attenuation (volume) and left/right panning
 * (position) relative to the listener's position and orientation.
 *
 * **Note on Listener Orientation:** In this 2D system, the listener is assumed
 * to be facing in the negative X direction (fx = -1.0f) with 'right' being in the
 * positive X direction (rightX = 1.0f), which simplifies the calculation of panning
 * based on the sound's relative X-position to the listener.
 *
 * @param listenerX The listener's X-coordinate.
 * @param listenerY The listener's Y-coordinate.
 */
void AudioManager::UpdateSpatial2D(float listenerX, float listenerY) {
    // Listener Orientation (Fixed for 2D top-down view)
    float fx = -1.0f; // Forward direction (e.g., facing left)
    float fy = 0.0f;

    float rightX = 1.0f; // Right direction (e.g., facing right)
    float rightY = 0.0f;

    for (auto& s : spatialSources_) {

        // 1. Calculate Distance and Attenuation (Volume)
        float dx = s.x - listenerX;
        float dy = s.y - listenerY;
        float distance = std::sqrt(dx * dx + dy * dy);

        // Calculate gain: 1.0 at distance 0, 0.0 at maxDistance, clamped between 0 and 1
        float d = std::clamp(1.0f - (distance / s.maxDistance), 0.0f, 1.0f);

        // 2. Calculate Panning (Position)
        // Dot product of (sound-listener vector) and listener's 'right' vector
        float dotRight = dx * rightX + dy * rightY;

        // Normalize dot product to [-1.0, 1.0] for position/panning
        float panning = std::clamp(dotRight / s.maxDistance, -1.0f, 1.0f);

        // Set OpenAL Source Properties
        // AL_POSITION takes (x, y, z) in listener space. For 2D panning, we use X as lateral position.
        alSource3f(sources_[s.sourceIndex], AL_POSITION, panning, 0.0f, 0.0f);
        alSourcef(sources_[s.sourceIndex], AL_GAIN, d);
    }
    // 
}
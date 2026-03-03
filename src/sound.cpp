#include "sound.h"
#include <iostream>
#include <cmath>

SoundSystem::SoundSystem() 
    : audioDevice(0), initialized(false), musicPlaying(false) {}

SoundSystem::~SoundSystem() {
    cleanup();
}

bool SoundSystem::init() {
    // Setup audio specification
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = nullptr; // Use queue audio mode
    
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &audioSpec, 0);
    
    if (audioDevice == 0) {
        std::cout << "[SOUND] Failed to open audio device: " << SDL_GetError() << "\n";
        return false;
    }
    
    // Start playing
    SDL_PauseAudioDevice(audioDevice, 0);
    
    initialized = true;
    std::cout << "[SOUND] Audio system initialized successfully\n";
    return true;
}

void SoundSystem::cleanup() {
    if (audioDevice != 0) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
    initialized = false;
}

void SoundSystem::generateKickBeep(std::vector<Sint16>& buffer) {
    // Short punchy kick sound (80Hz bass punch)
    const int duration = audioSpec.freq * 0.1; // 100ms
    buffer.resize(duration);
    
    for (int i = 0; i < duration; i++) {
        float t = (float)i / audioSpec.freq;
        float freq = 80.0f + (200.0f * exp(-t * 20.0f)); // Pitch drop
        float envelope = exp(-t * 30.0f); // Quick decay
        buffer[i] = (Sint16)(sin(2.0f * M_PI * freq * t) * 15000 * envelope);
    }
}

void SoundSystem::generateGoalBeep(std::vector<Sint16>& buffer) {
    // Celebration sound (ascending tone)
    const int duration = audioSpec.freq * 0.5; // 500ms
    buffer.resize(duration);
    
    for (int i = 0; i < duration; i++) {
        float t = (float)i / audioSpec.freq;
        float freq = 400.0f + (600.0f * t); // Rising pitch
        float envelope = 1.0f - (t * 2.0f);
        if (envelope < 0) envelope = 0;
        
        // Add harmonics for richer sound
        float sound = sin(2.0f * M_PI * freq * t) * 0.7f;
        sound += sin(2.0f * M_PI * freq * 2.0f * t) * 0.3f;
        
        buffer[i] = (Sint16)(sound * 20000 * envelope);
    }
}

void SoundSystem::generateWhistleBeep(std::vector<Sint16>& buffer) {
    // Whistle sound (high pitched)
    const int duration = audioSpec.freq * 0.3; // 300ms
    buffer.resize(duration);
    
    for (int i = 0; i < duration; i++) {
        float t = (float)i / audioSpec.freq;
        float freq = 2000.0f; // High whistle
        float envelope = 1.0f;
        
        // Fade in and out
        if (t < 0.05f) envelope = t / 0.05f;
        if (t > 0.25f) envelope = (0.3f - t) / 0.05f;
        
        buffer[i] = (Sint16)(sin(2.0f * M_PI * freq * t) * 18000 * envelope);
    }
}

void SoundSystem::queueAudio(const std::vector<Sint16>& buffer) {
    if (!initialized || buffer.empty()) return;
    
    // Clear queue if too much is pending
    Uint32 queued = SDL_GetQueuedAudioSize(audioDevice);
    if (queued > audioSpec.freq * 2) { // More than 2 seconds queued
        SDL_ClearQueuedAudio(audioDevice);
    }
    
    SDL_QueueAudio(audioDevice, buffer.data(), buffer.size() * sizeof(Sint16));
}

void SoundSystem::playKickSound() {
    if (!initialized) return;
    
    std::vector<Sint16> buffer;
    generateKickBeep(buffer);
    queueAudio(buffer);
    
    std::cout << "[SOUND] Playing kick sound\n";
}

void SoundSystem::playGoalSound() {
    if (!initialized) return;
    
    std::vector<Sint16> buffer;
    generateGoalBeep(buffer);
    queueAudio(buffer);
    
    std::cout << "[SOUND] Playing goal sound!\n";
}

void SoundSystem::playWhistleSound() {
    if (!initialized) return;
    
    std::vector<Sint16> buffer;
    generateWhistleBeep(buffer);
    queueAudio(buffer);
    
    std::cout << "[SOUND] Playing whistle sound\n";
}

void SoundSystem::generateBackgroundMusic(std::vector<Sint16>& buffer) {
    // Gentle, slow melody for football game (4 notes, about 2 seconds)
    const int noteCount = 4;
    const float noteDuration = 0.5f; // 500ms per note - slower tempo
    const int samplesPerNote = (int)(audioSpec.freq * noteDuration);
    
    // Simple calming melody: C5, G4, E5, C5
    float frequencies[noteCount] = {523.25f, 392.00f, 659.25f, 523.25f};
    
    buffer.resize(samplesPerNote * noteCount);
    
    for (int note = 0; note < noteCount; note++) {
        float freq = frequencies[note];
        int offset = note * samplesPerNote;
        
        for (int i = 0; i < samplesPerNote; i++) {
            float t = (float)i / audioSpec.freq;
            
            // Envelope for smooth notes (attack, sustain, release)
            float envelope = 1.0f;
            float attackTime = 0.02f;
            float releaseTime = 0.05f;
            
            if (t < attackTime) {
                envelope = t / attackTime;
            } else if (t > noteDuration - releaseTime) {
                envelope = (noteDuration - t) / releaseTime;
            }
            
            // Generate sine wave with envelope
            float sample = std::sin(2.0f * M_PI * freq * t) * envelope;
            buffer[offset + i] = (Sint16)(sample * 2000); // Gentle volume for background music
        }
    }
}

void SoundSystem::playBackgroundMusic() {
    if (!initialized || musicPlaying) return;
    
    std::vector<Sint16> buffer;
    generateBackgroundMusic(buffer);
    queueAudio(buffer);
    
    musicPlaying = true;
    std::cout << "[SOUND] Background music started (looping melody)\n";
}

void SoundSystem::stopBackgroundMusic() {
    if (!initialized || !musicPlaying) return;
    
    SDL_ClearQueuedAudio(audioDevice);
    musicPlaying = false;
    std::cout << "[SOUND] Background music stopped\n";
}

void SoundSystem::updateMusic() {
    if (!initialized || !musicPlaying) return;
    
    // Check if audio queue is getting low
    Uint32 queuedBytes = SDL_GetQueuedAudioSize(audioDevice);
    
    // If queue has less than 0.5 seconds of audio, add more
    Uint32 minBytes = audioSpec.freq * sizeof(Sint16) * 0.5f; // 0.5 seconds
    
    if (queuedBytes < minBytes) {
        std::vector<Sint16> buffer;
        generateBackgroundMusic(buffer);
        queueAudio(buffer);
    }
}

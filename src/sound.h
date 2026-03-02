#pragma once
#include <SDL2/SDL.h>
#include <vector>

// Simple sound system using SDL_audio
class SoundSystem {
public:
    SoundSystem();
    ~SoundSystem();
    
    bool init();
    void cleanup();
    
    // Play different sound effects
    void playKickSound();
    void playGoalSound();
    void playWhistleSound();
    void playBackgroundMusic();
    void stopBackgroundMusic();
    void updateMusic(); // Update music loop
    
private:
    SDL_AudioDeviceID audioDevice;
    SDL_AudioSpec audioSpec;
    bool initialized;
    bool musicPlaying;
    
    // Generate simple beep sounds
    void generateKickBeep(std::vector<Sint16>& buffer);
    void generateGoalBeep(std::vector<Sint16>& buffer);
    void generateWhistleBeep(std::vector<Sint16>& buffer);
    void generateBackgroundMusic(std::vector<Sint16>& buffer);
    
    // Queue audio for playback
    void queueAudio(const std::vector<Sint16>& buffer);
};

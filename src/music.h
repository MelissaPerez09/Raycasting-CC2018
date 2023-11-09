#include <SDL.h>

bool isMusicPlaying = true;


int backgroundMusic() {
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    if (SDL_LoadWAV("../assets/background.wav", &wavSpec, &wavBuffer, &wavLength) == nullptr) {
        return 1;
    }

    if (SDL_OpenAudio(&wavSpec, nullptr) < 0) {
        return 1;
    }

    while (isMusicPlaying) {
        SDL_QueueAudio(1, wavBuffer, wavLength);
        SDL_PauseAudio(0);

        SDL_Delay(wavLength * 1000 / wavSpec.freq);

        SDL_QueueAudio(1, wavBuffer, wavLength);
        SDL_PauseAudio(0);
    }

    SDL_CloseAudio();
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();

    return 0;
}

int playSoundEffect() {
    SDL_AudioSpec soundSpec;
    Uint32 soundLength;
    Uint8* soundBuffer;

    if (SDL_LoadWAV("assets/camera-shutter.wav", &soundSpec, &soundBuffer, &soundLength) == nullptr) {
        return 1; // Handle error loading the sound effect
    }

    if (SDL_OpenAudio(&soundSpec, nullptr) < 0) {
        return 1; // Handle error opening the audio device for sound effects
    }

    // Play the sound effect
    SDL_QueueAudio(1, soundBuffer, soundLength);
    SDL_PauseAudio(0);

    // Wait for the sound effect to finish
    SDL_Delay(soundLength * 1000 / soundSpec.freq);

    SDL_CloseAudio();
    SDL_FreeWAV(soundBuffer);

    return 0;
}

#include "raylib.h"

Music sandboxMusic;

void InitSandboxMusic() {
    InitAudioDevice(); 
    sandboxMusic = LoadMusicStream("assets/The Samunaut.mp3");
    sandboxMusic.looping = true;
    PlayMusicStream(sandboxMusic);
    SetMusicVolume(sandboxMusic, 0.5f);
}

void UpdateSandboxMusic() {
    UpdateMusicStream(sandboxMusic); // call every frame
}

void StopSandboxMusic() {
    StopMusicStream(sandboxMusic);
    UnloadMusicStream(sandboxMusic);
    CloseAudioDevice(); // at program end
}

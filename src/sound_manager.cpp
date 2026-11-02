// src/sound_manager.cpp
#include "splashkit.h"
#include "sound_manager.h"

void load_game_sounds()
{
    // Load every short effect once before the game loop starts.
    load_sound_effect("merge", "Resources/audio/merge_pop.wav");
    load_sound_effect("order", "Resources/audio/confirmation.ogg");
    load_sound_effect("timeout", "Resources/audio/error.ogg");
    load_sound_effect("drop", "Resources/audio/drop.ogg");
    load_sound_effect("click", "Resources/audio/click.ogg");
    load_sound_effect("question", "Resources/audio/question.ogg");
    load_sound_effect("tick", "Resources/audio/tick.ogg");

    // Story narration for Mia and the festival guest.
    load_sound_effect("story_mia_intro", "Resources/audio/story_mia_intro.mp3");
    load_sound_effect("story_festival_guest", "Resources/audio/story_festival_guest.mp3");
    load_sound_effect("story_mia_invitation", "Resources/audio/story_mia_invitation.mp3");
}

void stop_story_voice()
{
    stop_sound_effect("story_mia_intro");
    stop_sound_effect("story_festival_guest");
    stop_sound_effect("story_mia_invitation");
}

void play_story_voice(const GameState &game, int story_line)
{
    stop_story_voice();
    if (!game.soundEffectsOn) return;

    if (story_line == 0)
        play_sound_effect("story_mia_intro", 0.85);
    else if (story_line == 1)
        play_sound_effect("story_festival_guest", 0.85);
    else if (story_line == 2)
        play_sound_effect("story_mia_invitation", 0.85);
}

// Play one effect, but only when the player has sound effects switched on.
static void play_effect(const GameState &game, const string &name, double volume)
{
    if (game.soundEffectsOn)
        play_sound_effect(name, volume);
}

// Keep effects slightly below full volume so they do not overpower the game.
void play_merge_sound(const GameState &game)
{
    play_effect(game, "merge", 0.75);
}

void play_order_sound(const GameState &game)
{
    play_effect(game, "order", 0.7);
}

void play_timeout_sound(const GameState &game)
{
    play_effect(game, "timeout", 0.55);
}

void play_drop_sound(const GameState &game)
{
    play_effect(game, "drop", 0.45);
}

void play_click_sound(const GameState &game)
{
    play_effect(game, "click", 0.45);
}

void play_question_sound(const GameState &game)
{
    play_effect(game, "question", 0.45);
}

void play_tick_sound(const GameState &game)
{
    play_effect(game, "tick", 0.35);
}

// Load and play a soft bakery theme throughout the game.
static music background_music;
void load_background_music()
{
    background_music = load_music("bakery_bgm", "Resources/audio/bakery_bgm.mp3");
}

void play_background_music()
{
    play_music(background_music, -1, 0.20);
}

// The music keeps its place in the song while it is paused.
void apply_music_setting(const GameState &game)
{
    if (game.musicOn)
        resume_music();
    else
        pause_music();
}

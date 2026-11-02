// src/sound_manager.h
#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "splashkit.h"
#include "game_state.h"

void load_game_sounds();
void play_story_voice(const GameState &game, int story_line);
void stop_story_voice();
void load_background_music();
void play_background_music();
// Pause or resume the music to match the player's music setting.
void apply_music_setting(const GameState &game);
void play_merge_sound(const GameState &game);
void play_order_sound(const GameState &game);
void play_timeout_sound(const GameState &game);
void play_drop_sound(const GameState &game);
void play_click_sound(const GameState &game);
void play_question_sound(const GameState &game);
void play_tick_sound(const GameState &game);

#endif

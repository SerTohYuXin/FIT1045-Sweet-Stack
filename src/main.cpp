// src/main.cpp
#include "splashkit.h"
#include "game_state.h"
#include "game_logic.h"
#include "game_view.h"
#include "input.h"
#include "sound_manager.h"
#include "score_manager.h"
#include "game_layout.h"

static void record_round_if_finished(GameState &game)
{
    bool hasEnded = (game.screen == WON_SCREEN || game.screen == GAME_OVER_SCREEN);
    if (hasEnded && !game.resultRecorded)
    {
        game.newHighScore = record_high_score(game.mode, game.score);
        game.resultRecorded = true;
    }
}

int main()
{
    GameState game;
    start_new_game(game);
    open_window("Sweet Stack - Cake Merge", WINDOW_WIDTH, WINDOW_HEIGHT);
    load_game_art();
    load_game_sounds();
    play_story_voice(game, game.storyLine);
    load_background_music();
    play_background_music();

    while (!quit_requested())
    {
        // Handle input, update game rules, then draw the current screen.
        process_events();

        unsigned int now = current_ticks();
        update_game(game, now);
        record_round_if_finished(game);
        handle_key_presses(game, now);
        if (mouse_clicked(LEFT_BUTTON))
        {
            handle_mouse_click(game, mouse_x(), mouse_y(), now);
        }
        record_round_if_finished(game);
        draw_game(game, now);
        refresh_screen(60);
    }

    close_all_windows();
    return 0;
}

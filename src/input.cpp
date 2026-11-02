// src/input.cpp
#include "splashkit.h"
#include "input.h"
#include "game_logic.h"
#include "game_layout.h"
#include "sound_manager.h"
#include "utilities.h"

// Stop the current narration when the player advances, then speak the next subtitle.
static void advance_story_with_voice(game_state &game, unsigned int now)
{
    stop_story_voice();
    advance_screen(game, now);
    if (game.screen == STORY_SCREEN)
        play_story_voice(game, game.story_line);
}

// Return which generator was clicked, or NO_ITEM if the click missed them all.
static item_category generator_at(double mouse_x, double mouse_y)
{
    if (point_in_rectangle(mouse_x, mouse_y, DOUGH_BUTTON_X, GENERATOR_Y, GENERATOR_WIDTH, GENERATOR_HEIGHT))
        return DOUGH;

    if (point_in_rectangle(mouse_x, mouse_y, TART_BUTTON_X, GENERATOR_Y, GENERATOR_WIDTH, GENERATOR_HEIGHT))
        return TART;

    if (point_in_rectangle(mouse_x, mouse_y, CAKE_BUTTON_X, GENERATOR_Y, GENERATOR_WIDTH, GENERATOR_HEIGHT))
        return CAKE;

    return NO_ITEM;
}

// Handle a click inside the settings panel. The two switches work independently,
// and clicks outside the panel do nothing while it is open.
static void handle_settings_click(game_state &game, double mouse_x, double mouse_y)
{
    if (point_in_rectangle(mouse_x, mouse_y, SETTINGS_TOGGLE_X, SETTINGS_MUSIC_Y,
            SETTINGS_TOGGLE_WIDTH, SETTINGS_TOGGLE_HEIGHT))
    {
        toggle_music(game);
        apply_music_setting(game);
        play_click_sound(game);
    }
    else if (point_in_rectangle(mouse_x, mouse_y, SETTINGS_TOGGLE_X, SETTINGS_EFFECTS_Y,
            SETTINGS_TOGGLE_WIDTH, SETTINGS_TOGGLE_HEIGHT))
    {
        toggle_sound_effects(game);
        play_click_sound(game);   // only heard when effects have just been switched ON
    }
    else if (point_in_rectangle(mouse_x, mouse_y, SETTINGS_CLOSE_X, SETTINGS_CLOSE_Y,
            SETTINGS_CLOSE_WIDTH, SETTINGS_CLOSE_HEIGHT))
    {
        play_click_sound(game);
        close_settings(game);
    }
}

// Handle a click on the four mode cards. Choosing a mode starts gameplay at once.
static void handle_menu_click(game_state &game, double mouse_x, double mouse_y, unsigned int now)
{
    if (game.settings_open)
    {
        handle_settings_click(game, mouse_x, mouse_y);
        return;
    }

    if (point_in_rectangle(mouse_x, mouse_y, SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y,
            SETTINGS_BUTTON_WIDTH, SETTINGS_BUTTON_HEIGHT))
    {
        play_click_sound(game);
        open_settings(game);
        return;
    }

    game_mode chosen = BEGINNER_MODE;
    bool clicked_card = true;

    if (point_in_rectangle(mouse_x, mouse_y, MODE_LEFT_X, MODE_TOP_Y, MODE_CARD_WIDTH, MODE_CARD_HEIGHT))
        chosen = BEGINNER_MODE;
    else if (point_in_rectangle(mouse_x, mouse_y, MODE_RIGHT_X, MODE_TOP_Y, MODE_CARD_WIDTH, MODE_CARD_HEIGHT))
        chosen = CLASSIC_MODE;
    else if (point_in_rectangle(mouse_x, mouse_y, MODE_LEFT_X, MODE_BOTTOM_Y, MODE_CARD_WIDTH, MODE_CARD_HEIGHT))
        chosen = RUSH_MODE;
    else if (point_in_rectangle(mouse_x, mouse_y, MODE_RIGHT_X, MODE_BOTTOM_Y, MODE_CARD_WIDTH, MODE_CARD_HEIGHT))
        chosen = RECIPE_MODE;
    else
        clicked_card = false;

    if (clicked_card)
    {
        play_click_sound(game);
        choose_mode(game, chosen, now);
    }
}

// Handle a click on the 4 x 4 board: select, move or merge.
static void handle_board_click(game_state &game, double mouse_x, double mouse_y, unsigned int now)
{
    if (!point_in_rectangle(mouse_x, mouse_y, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE))
        return;

    int column = (int)(mouse_x - BOARD_X) / (CELL_SIZE + CELL_GAP);
    int row = (int)(mouse_y - BOARD_Y) / (CELL_SIZE + CELL_GAP);
    int in_cell_x = (int)(mouse_x - BOARD_X) % (CELL_SIZE + CELL_GAP);
    int in_cell_y = (int)(mouse_y - BOARD_Y) % (CELL_SIZE + CELL_GAP);

    // Ignore the small gaps between board squares.
    if (in_cell_x < CELL_SIZE && in_cell_y < CELL_SIZE)
    {
        select_or_move_cake(game, row, column, now);
    }
}

void handle_mouse_click(game_state &game, double mouse_x, double mouse_y, unsigned int now)
{
    if (game.screen == MODE_MENU)
    {
        handle_menu_click(game, mouse_x, mouse_y, now);
        return;
    }

    if (game.screen == STORY_SCREEN)
    {
        play_click_sound(game);
        advance_story_with_voice(game, now);
        return;
    }

    if (game.screen == WON_SCREEN || game.screen == GAME_OVER_SCREEN)
    {
        if (point_in_rectangle(mouse_x, mouse_y, END_REPLAY_BUTTON_X, END_BUTTON_Y,
                END_BUTTON_WIDTH, END_BUTTON_HEIGHT))
        {
            play_click_sound(game);
            choose_mode(game, game.mode, now);
        }
        else if (point_in_rectangle(mouse_x, mouse_y, END_MENU_BUTTON_X, END_BUTTON_Y,
                END_BUTTON_WIDTH, END_BUTTON_HEIGHT))
        {
            play_click_sound(game);
            show_mode_menu(game);
        }
        else
        {
            play_click_sound(game);
            advance_screen(game, now);
        }
        return;
    }

    if (game.screen != PLAYING_SCREEN)
        return;

    // Pause/Resume remains available while the game is paused.
    if (point_in_rectangle(mouse_x, mouse_y, PAUSE_BUTTON_X, PAUSE_BUTTON_Y,
            PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT))
    {
        play_click_sound(game);
        toggle_pause(game, now);
        return;
    }

    // The BACK button returns to mode selection without closing the game.
    if (point_in_rectangle(mouse_x, mouse_y, BACK_X, BACK_Y, BACK_WIDTH, BACK_HEIGHT))
    {
        play_click_sound(game);
        show_mode_menu(game);
        return;
    }

    if (game.paused)
        return;

    // A generator button puts its food in a random empty square straight away.
    item_category generator = generator_at(mouse_x, mouse_y);

    if (generator != NO_ITEM)
    {
        if (generate_item(game, generator))
            play_click_sound(game);
        else
            play_timeout_sound(game); // the board is full
        return;
    }

    handle_board_click(game, mouse_x, mouse_y, now);
}

// Keyboard controls: 1-4 pick a mode, SPACE continues, R goes back to the modes.
void handle_key_presses(game_state &game, unsigned int now)
{
    if (game.screen == MODE_MENU)
    {
        // While the settings panel is open only ESC works.
        if (game.settings_open)
        {
            if (key_typed(ESCAPE_KEY))
                close_settings(game);
            return;
        }

        if (key_typed(NUM_1_KEY))
        {
            play_click_sound(game);
            choose_mode(game, BEGINNER_MODE, now);
        }
        else if (key_typed(NUM_2_KEY))
        {
            play_click_sound(game);
            choose_mode(game, CLASSIC_MODE, now);
        }
        else if (key_typed(NUM_3_KEY))
        {
            play_click_sound(game);
            choose_mode(game, RUSH_MODE, now);
        }
        else if (key_typed(NUM_4_KEY))
        {
            play_click_sound(game);
            choose_mode(game, RECIPE_MODE, now);
        }
        return;
    }

    if (game.screen == PLAYING_SCREEN && (key_typed(P_KEY) || key_typed(ESCAPE_KEY)))
    {
        play_click_sound(game);
        toggle_pause(game, now);
        return;
    }

    if (game.paused)
    {
        if (key_typed(R_KEY))
            show_mode_menu(game);
        return;
    }

    if (key_typed(SPACE_KEY))
    {
        if (game.screen == STORY_SCREEN)
            advance_story_with_voice(game, now);
        else
            advance_screen(game, now);
    }

    // R returns to the four modes (never back to the story).
    if (key_typed(R_KEY) && game.screen != STORY_SCREEN)
    {
        show_mode_menu(game);
    }
}

// src/game_state.cpp
#include "splashkit.h"
#include "game_state.h"
#include "sound_manager.h"

int order_count_for_mode(game_mode mode)
{
    return (mode == CLASSIC_MODE) ? CLASSIC_ORDER_COUNT : ORDER_COUNT;
}

// Generate one shuffled pass through all three food categories.
static void shuffled_categories(item_category categories[3])
{
    categories[0] = DOUGH;
    categories[1] = TART;
    categories[2] = CAKE;
    for (int i = 2; i > 0; i--)
    {
        int swap_index = rnd(i + 1);   // rnd(n) gives 0 to n - 1, so this is 0 to i
        item_category temp = categories[i];
        categories[i] = categories[swap_index];
        categories[swap_index] = temp;
    }
}

// Store the orders once when a mode starts so they stay stable during play.
static void setup_orders(game_state &game, game_mode mode)
{
    item_category categories[3];
    if (mode == BEGINNER_MODE)
    {
        categories[0] = DOUGH;
        categories[1] = TART;
        categories[2] = CAKE;
    }
    else
    {
        shuffled_categories(categories);
    }

    for (int i = 0; i < order_count_for_mode(mode); i++)
    {
        order_spec &order = game.orders[i];
        order.category = (mode == BEGINNER_MODE) ? (item_category)(DOUGH + i) : categories[i % ORDER_COUNT];
        order.tier = (mode == BEGINNER_MODE) ? ((i == 2) ? 3 : 2) : 1 + rnd(2);
        order.quantity = 1;
        order.secondary_category = NO_ITEM;
        order.secondary_tier = -1;
        order.secondary_quantity = 0;
        order.move_limit = 12 + i * 4;

        if (mode == CLASSIC_MODE)
            order.quantity = 1 + rnd(2);   // 1 or 2 items
        else if (mode == RECIPE_MODE)
        {
            order.quantity = 1 + rnd(2);   // 1 or 2 items of the first food
            order.secondary_category = categories[(i + 1) % ORDER_COUNT];
            order.secondary_tier = 1 + rnd(2);
            order.secondary_quantity = 1;
        }
    }
}

// Empty every square of the board.
static void setup_board(game_state &game)
{
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int column = 0; column < GRID_SIZE; column++)
        {
            game.board[row][column].category = NO_ITEM;
            game.board[row][column].tier = 0;
            game.recipe_food_pending[row][column] = false;
            game.recipe_food_delivered_at[row][column] = 0;
        }
    }
}

// Reset every value that changes during a round.
static void reset_round(game_state &game)
{
    game.mode = BEGINNER_MODE;
    game.score = 0;
    game.moves = 0;
    game.order_index = 0;
    game.delivered_count = 0;
    game.satisfaction = 3;
    game.recipe_first_delivered = 0;
    game.recipe_second_delivered = 0;
    game.moves_left = 0;
    game.story_line = 0;
    game.order_started_at = 0;
    game.lost_by_timer = false;
    game.lost_by_move_limit = false;
    game.low_time_alert_played = false;
    game.order_just_completed = false;
    game.paused = false;
    game.paused_at = 0;
    game.order_completed_at = 0;
    game.served_food_removed = false;
    game.last_action = "";
    game.settings_open = false;   // the panel is closed, but music/effects settings are kept
    game.new_high_score = false;
    game.result_recorded = false;
    game.selected_row = -1;
    game.selected_column = -1;
    setup_orders(game, BEGINNER_MODE);
    setup_board(game);
}

// Called once when the program starts: the story scene comes first.
void start_new_game(game_state &game)
{
    reset_round(game);
    game.music_on = true;           // both switches start ON; reset_round never changes them
    game.sound_effects_on = true;
    game.screen = STORY_SCREEN;
}

// Used by the Back button, the R key and the end screens.
// The story is not shown again.
void show_mode_menu(game_state &game)
{
    if (game.paused)
        apply_music_setting(game);
    reset_round(game);
    game.screen = MODE_MENU;
}

// Start the selected mode and go straight to gameplay.
void choose_mode(game_state &game, game_mode mode, unsigned int now)
{
    reset_round(game);
    game.mode = mode;
    setup_orders(game, mode);
    game.moves_left = (mode == RECIPE_MODE) ? game.orders[0].move_limit : 0;
    game.order_started_at = now;
    game.screen = PLAYING_SCREEN;
}

void open_settings(game_state &game)
{
    game.settings_open = true;
}

void close_settings(game_state &game)
{
    game.settings_open = false;
}

// Each switch only changes its own setting.
void toggle_music(game_state &game)
{
    game.music_on = !game.music_on;
}

void toggle_sound_effects(game_state &game)
{
    game.sound_effects_on = !game.sound_effects_on;
}

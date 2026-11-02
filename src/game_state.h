// src/game_state.h
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "splashkit.h"

const int GRID_SIZE = 4;
// The food atlas has four images per category, using tiers 0 through 3.
const int MAX_ITEM_TIER = 3;
const int ORDER_COUNT = 3;
const int CLASSIC_ORDER_COUNT = 5;
const int MAX_ORDER_COUNT = CLASSIC_ORDER_COUNT;

// Each food chain has its own category.
enum item_category
{
    NO_ITEM,
    DOUGH,
    TART,
    CAKE
};

// Screens and game modes.
enum game_mode 
{ 
    BEGINNER_MODE, 
    CLASSIC_MODE, 
    RUSH_MODE, 
    RECIPE_MODE 
};

int order_count_for_mode(game_mode mode);

enum screen_state
{
    MODE_MENU,
    STORY_SCREEN,
    PLAYING_SCREEN,
    WON_SCREEN,
    GAME_OVER_SCREEN
};

// A board square stores both the food category and its tier.
struct board_item
{
    item_category category;
    int tier;
};

struct order_spec
{
    item_category category;
    int tier;
    int quantity;
    item_category secondary_category;
    int secondary_tier;
    int secondary_quantity;
    int move_limit;
};

struct game_state
{
    // A square with category NO_ITEM is empty.
    board_item board[GRID_SIZE][GRID_SIZE];
    int selected_row, selected_column;
    int score, moves, order_index, delivered_count, satisfaction;
    int recipe_first_delivered, recipe_second_delivered, moves_left;
    order_spec orders[MAX_ORDER_COUNT];
    unsigned int order_started_at;
    string last_action;

    game_mode mode;
    screen_state screen;

    int story_line;
    bool lost_by_timer, lost_by_move_limit, low_time_alert_played;

    // True while "Order complete" is showing and the customer is leaving.
    // The board is locked and the Rush Hour timer is paused until it ends.
    bool order_just_completed;
    bool paused;
    unsigned int paused_at;
    unsigned int order_completed_at;
    bool served_food_removed;   // true once the served food has been taken off the board
    bool recipe_food_pending[GRID_SIZE][GRID_SIZE];
    unsigned int recipe_food_delivered_at[GRID_SIZE][GRID_SIZE];

    // Player settings. Starting a new round does NOT reset these, so they
    // stay the same until the player changes them.
    bool music_on;
    bool sound_effects_on;
    bool settings_open;        // true while the settings panel is showing
    bool new_high_score;        // true when this round sets a new local record
    bool result_recorded;      // prevents saving the same round more than once
};

// Program start: reset everything and show the story first.
void start_new_game(game_state &game);
// Reset everything and show the four-mode selection screen.
void show_mode_menu(game_state &game);
// Start the chosen mode's gameplay immediately.
void choose_mode(game_state &game, game_mode mode, unsigned int now);
item_category current_order_category(const game_state &game);
item_category current_order_secondary_category(const game_state &game);
bool generate_item(game_state &game, item_category category);

// Settings panel on the mode-selection screen. Music and sound effects
// are two separate switches.
void open_settings(game_state &game);
void close_settings(game_state &game);
void toggle_music(game_state &game);
void toggle_sound_effects(game_state &game);

#endif

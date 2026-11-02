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
enum ItemCategory
{
    NO_ITEM,
    DOUGH,
    TART,
    CAKE
};

// Screens and game modes.
enum GameMode 
{ 
    BEGINNER_MODE, 
    CLASSIC_MODE, 
    RUSH_MODE, 
    RECIPE_MODE 
};

int order_count_for_mode(GameMode mode);

enum ScreenState
{
    MODE_MENU,
    STORY_SCREEN,
    PLAYING_SCREEN,
    WON_SCREEN,
    GAME_OVER_SCREEN
};

// A board square stores both the food category and its tier.
struct BoardItem
{
    ItemCategory category;
    int tier;
};

struct OrderSpec
{
    ItemCategory category;
    int tier;
    int quantity;
    ItemCategory secondaryCategory;
    int secondaryTier;
    int secondaryQuantity;
    int moveLimit;
};

struct GameState
{
    // A square with category NO_ITEM is empty.
    BoardItem board[GRID_SIZE][GRID_SIZE];
    int selectedRow, selectedColumn;
    int score, moves, orderIndex, deliveredCount, satisfaction;
    int recipeFirstDelivered, recipeSecondDelivered, movesLeft;
    OrderSpec orders[MAX_ORDER_COUNT];
    unsigned int orderStartedAt;
    string lastAction;

    GameMode mode;
    ScreenState screen;

    int storyLine;
    bool lostByTimer, lostByMoveLimit, lowTimeAlertPlayed;

    // True while "Order complete" is showing and the customer is leaving.
    // The board is locked and the Rush Hour timer is paused until it ends.
    bool orderJustCompleted;
    bool paused;
    unsigned int pausedAt;
    unsigned int orderCompletedAt;
    bool servedFoodRemoved;   // true once the served food has been taken off the board
    bool recipeFoodPending[GRID_SIZE][GRID_SIZE];
    unsigned int recipeFoodDeliveredAt[GRID_SIZE][GRID_SIZE];

    // Player settings. Starting a new round does NOT reset these, so they
    // stay the same until the player changes them.
    bool musicOn;
    bool soundEffectsOn;
    bool settingsOpen;        // true while the settings panel is showing
    bool newHighScore;        // true when this round sets a new local record
    bool resultRecorded;      // prevents saving the same round more than once
};

// Program start: reset everything and show the story first.
void start_new_game(GameState &game);
// Reset everything and show the four-mode selection screen.
void show_mode_menu(GameState &game);
// Start the chosen mode's gameplay immediately.
void choose_mode(GameState &game, GameMode mode, unsigned int now);
ItemCategory current_order_category(const GameState &game);
ItemCategory current_order_secondary_category(const GameState &game);
bool generate_item(GameState &game, ItemCategory category);

// Settings panel on the mode-selection screen. Music and sound effects
// are two separate switches.
void open_settings(GameState &game);
void close_settings(GameState &game);
void toggle_music(GameState &game);
void toggle_sound_effects(GameState &game);

#endif

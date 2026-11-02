// src/game_state.cpp
#include "splashkit.h"
#include "game_state.h"
#include "sound_manager.h"

int order_count_for_mode(GameMode mode)
{
    return (mode == CLASSIC_MODE) ? CLASSIC_ORDER_COUNT : ORDER_COUNT;
}

// Generate one shuffled pass through all three food categories.
static void shuffled_categories(ItemCategory categories[3])
{
    categories[0] = DOUGH;
    categories[1] = TART;
    categories[2] = CAKE;
    for (int i = 2; i > 0; i--)
    {
        int swapIndex = rnd(i + 1);   // rnd(n) gives 0 to n - 1, so this is 0 to i
        ItemCategory temp = categories[i];
        categories[i] = categories[swapIndex];
        categories[swapIndex] = temp;
    }
}

// Store the orders once when a mode starts so they stay stable during play.
static void setup_orders(GameState &game, GameMode mode)
{
    ItemCategory categories[3];
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
        OrderSpec &order = game.orders[i];
        order.category = (mode == BEGINNER_MODE) ? (ItemCategory)(DOUGH + i) : categories[i % ORDER_COUNT];
        order.tier = (mode == BEGINNER_MODE) ? ((i == 2) ? 3 : 2) : 1 + rnd(2);
        order.quantity = 1;
        order.secondaryCategory = NO_ITEM;
        order.secondaryTier = -1;
        order.secondaryQuantity = 0;
        order.moveLimit = 12 + i * 4;

        if (mode == CLASSIC_MODE)
            order.quantity = 1 + rnd(2);   // 1 or 2 items
        else if (mode == RECIPE_MODE)
        {
            order.quantity = 1 + rnd(2);   // 1 or 2 items of the first food
            order.secondaryCategory = categories[(i + 1) % ORDER_COUNT];
            order.secondaryTier = 1 + rnd(2);
            order.secondaryQuantity = 1;
        }
    }
}

// Empty every square of the board.
static void setup_board(GameState &game)
{
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int column = 0; column < GRID_SIZE; column++)
        {
            game.board[row][column].category = NO_ITEM;
            game.board[row][column].tier = 0;
            game.recipeFoodPending[row][column] = false;
            game.recipeFoodDeliveredAt[row][column] = 0;
        }
    }
}

// Reset every value that changes during a round.
static void reset_round(GameState &game)
{
    game.mode = BEGINNER_MODE;
    game.score = 0;
    game.moves = 0;
    game.orderIndex = 0;
    game.deliveredCount = 0;
    game.satisfaction = 3;
    game.recipeFirstDelivered = 0;
    game.recipeSecondDelivered = 0;
    game.movesLeft = 0;
    game.storyLine = 0;
    game.orderStartedAt = 0;
    game.lostByTimer = false;
    game.lostByMoveLimit = false;
    game.lowTimeAlertPlayed = false;
    game.orderJustCompleted = false;
    game.paused = false;
    game.pausedAt = 0;
    game.orderCompletedAt = 0;
    game.servedFoodRemoved = false;
    game.lastAction = "";
    game.settingsOpen = false;   // the panel is closed, but music/effects settings are kept
    game.newHighScore = false;
    game.resultRecorded = false;
    game.selectedRow = -1;
    game.selectedColumn = -1;
    setup_orders(game, BEGINNER_MODE);
    setup_board(game);
}

// Called once when the program starts: the story scene comes first.
void start_new_game(GameState &game)
{
    reset_round(game);
    game.musicOn = true;           // both switches start ON; reset_round never changes them
    game.soundEffectsOn = true;
    game.screen = STORY_SCREEN;
}

// Used by the Back button, the R key and the end screens.
// The story is not shown again.
void show_mode_menu(GameState &game)
{
    if (game.paused)
        apply_music_setting(game);
    reset_round(game);
    game.screen = MODE_MENU;
}

// Start the selected mode and go straight to gameplay.
void choose_mode(GameState &game, GameMode mode, unsigned int now)
{
    reset_round(game);
    game.mode = mode;
    setup_orders(game, mode);
    game.movesLeft = (mode == RECIPE_MODE) ? game.orders[0].moveLimit : 0;
    game.orderStartedAt = now;
    game.screen = PLAYING_SCREEN;
}

void open_settings(GameState &game)
{
    game.settingsOpen = true;
}

void close_settings(GameState &game)
{
    game.settingsOpen = false;
}

// Each switch only changes its own setting.
void toggle_music(GameState &game)
{
    game.musicOn = !game.musicOn;
}

void toggle_sound_effects(GameState &game)
{
    game.soundEffectsOn = !game.soundEffectsOn;
}

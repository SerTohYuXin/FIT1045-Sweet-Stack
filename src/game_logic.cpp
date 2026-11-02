// src/game_logic.cpp
#include "splashkit.h"
#include "game_state.h"
#include "game_logic.h"
#include "sound_manager.h"
#include "utilities.h"

static void clear_selection(GameState &game)
{
    game.selectedRow = -1;
    game.selectedColumn = -1;
}

// Story: move through the dialogue, then show the four modes.
// Win or game-over screen: return to the four modes.
void advance_screen(GameState &game, unsigned int now)
{
    (void)now; // kept so existing calls stay the same

    if (game.screen == STORY_SCREEN)
    {
        if (game.storyLine < 2)
            game.storyLine++;
        else
            show_mode_menu(game);
    }
    else if (game.screen == WON_SCREEN || game.screen == GAME_OVER_SCREEN)
    {
        show_mode_menu(game);
    }
}

ItemCategory current_order_category(const GameState &game)
{
    if (game.orderIndex >= order_count_for_mode(game.mode)) return NO_ITEM;

    return game.orders[game.orderIndex].category;
}

int current_order_tier(const GameState &game)
{
    if (game.orderIndex >= order_count_for_mode(game.mode)) return -1;

    return game.orders[game.orderIndex].tier;
}

int current_order_quantity(const GameState &game)
{
    if (game.orderIndex >= order_count_for_mode(game.mode)) return 0;

    return game.orders[game.orderIndex].quantity;
}

ItemCategory current_order_secondary_category(const GameState &game)
{
    if (game.mode != RECIPE_MODE || game.orderIndex >= order_count_for_mode(game.mode))
        return NO_ITEM;
    return game.orders[game.orderIndex].secondaryCategory;
}

int current_order_secondary_tier(const GameState &game)
{
    if (game.mode != RECIPE_MODE || game.orderIndex >= order_count_for_mode(game.mode))
        return -1;
    return game.orders[game.orderIndex].secondaryTier;
}

int current_order_secondary_quantity(const GameState &game)
{
    if (game.mode != RECIPE_MODE || game.orderIndex >= order_count_for_mode(game.mode))
        return 0;
    return game.orders[game.orderIndex].secondaryQuantity;
}

unsigned int current_order_time_left(const GameState &game, unsigned int now)
{
    if (game.mode != RUSH_MODE || game.screen != PLAYING_SCREEN)
        return ORDER_TIME_LIMIT;
    // The timer is paused while the customer is leaving.
    unsigned int clock = game.orderJustCompleted ? game.orderCompletedAt : (game.paused ? game.pausedAt : now);
    unsigned int elapsed = clock - game.orderStartedAt;
    return (elapsed >= ORDER_TIME_LIMIT) ? 0 : ORDER_TIME_LIMIT - elapsed;
}

int current_move_limit(const GameState &game)
{
    if (game.mode != RECIPE_MODE || game.orderIndex >= order_count_for_mode(game.mode)) return 0;
    return game.orders[game.orderIndex].moveLimit;
}

// Rush Hour: hearts the current customer has left, based on how long they have waited.
static int hearts_left(const GameState &game, unsigned int now)
{
    // The clock is paused while the customer is leaving.
    unsigned int clock = game.orderJustCompleted ? game.orderCompletedAt : (game.paused ? game.pausedAt : now);
    int heartsLost = (int)((clock - game.orderStartedAt) / CUSTOMER_HEART_TIME);
    if (heartsLost >= MAX_HEARTS) return 0;
    return MAX_HEARTS - heartsLost;
}

bool customer_has_left(const GameState &game, unsigned int now)
{
    unsigned int clock = game.paused ? game.pausedAt : now;
    return game.orderJustCompleted && clock - game.orderCompletedAt >= ORDER_COMPLETE_MESSAGE_TIME;
}

static bool can_make_a_move(const GameState &game)
{
    // An empty square means the player can still generate or move an item.
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (game.board[row][col].category == NO_ITEM)
                return true;
        }
    }

    // Matching categories and tiers mean that a merge is possible.
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            BoardItem first = game.board[row][col];
            if (first.category == NO_ITEM || first.tier >= MAX_ITEM_TIER)
                continue;
            for (int otherRow = 0; otherRow < GRID_SIZE; otherRow++)
            {
                for (int otherCol = 0; otherCol < GRID_SIZE; otherCol++)
                {
                    if (row == otherRow && col == otherCol)
                        continue;
                    BoardItem second = game.board[otherRow][otherCol];
                    if (first.category == second.category && first.tier == second.tier)
                        return true;
                }
            }
        }
    }
    return false;
}

static bool recipe_food_is_pending(const GameState &game, int row, int column)
{
    return game.recipeFoodPending[row][column];
}

static void update_recipe_food_collection(GameState &game, unsigned int now)
{
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int column = 0; column < GRID_SIZE; column++)
        {
            if (game.recipeFoodPending[row][column] &&
                now - game.recipeFoodDeliveredAt[row][column] >= ORDER_COMPLETE_MESSAGE_TIME)
            {
                game.board[row][column].category = NO_ITEM;
                game.board[row][column].tier = 0;
                game.recipeFoodPending[row][column] = false;
            }
        }
    }
}
// Take up to "count" foods of one kind off the board (rows first, then columns).
static void remove_foods(GameState &game, ItemCategory category, int tier, int count)
{
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (count > 0 && game.board[row][col].category == category && game.board[row][col].tier == tier)
            {
                game.board[row][col].category = NO_ITEM;
                game.board[row][col].tier = 0;
                count--;
            }
        }
    }
}

// The foods that were counted for the current order are taken away
static void remove_served_food(GameState &game)
{
    if (game.orderIndex >= order_count_for_mode(game.mode)) return;

    // Recipe items are removed as soon as each exact requirement is delivered.
    // Other modes still remove their completed items when the customer leaves.
    if (game.mode != RECIPE_MODE)
        remove_foods(game, current_order_category(game), current_order_tier(game), game.deliveredCount);
}

// Generates the requested food type in a randomly chosen empty square.
// Returns false if the board has no empty square.
bool generate_item(GameState &game, ItemCategory category)
{
    if (game.screen != PLAYING_SCREEN || category == NO_ITEM)
        return false;
    int emptyCount = 0;
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (game.board[row][col].category == NO_ITEM)
                emptyCount++;
        }
    }

    if (emptyCount == 0)
        return false;

    // Pick one of the empty squares at random.
    int chosenEmpty = rnd(emptyCount);   // rnd(n) gives 0 to n - 1
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (game.board[row][col].category == NO_ITEM)
            {
                if (chosenEmpty == 0)
                {
                    game.board[row][col].category = category;
                    game.board[row][col].tier = 0;
                    game.lastAction = "";
                    play_drop_sound(game);
                    return true;
                }
                chosenEmpty--;
            }
        }
    }
    return false;
}

static bool order_is_complete(const GameState &game)
{
    if (game.mode == RECIPE_MODE)
    {
        return game.recipeFirstDelivered >= current_order_quantity(game) && game.recipeSecondDelivered >= current_order_secondary_quantity(game);
    }
    return game.deliveredCount >= current_order_quantity(game);
}

// The next customer arrives with the next order.
static void finish_order_transition(GameState &game, unsigned int now)
{
    game.orderJustCompleted = false;
    game.servedFoodRemoved = false;
    game.orderIndex++;
    game.deliveredCount = 0;
    game.recipeFirstDelivered = 0;
    game.recipeSecondDelivered = 0;
    game.lastAction = "";
    game.orderStartedAt = now;   // Rush Hour: the new order gets a full timer
    game.lowTimeAlertPlayed = false;
    game.satisfaction = MAX_HEARTS;   // every new customer starts with full hearts

    if (game.mode == RECIPE_MODE)
        game.movesLeft = current_move_limit(game);

    play_question_sound(game);
}

static void complete_order(GameState &game, unsigned int now)
{
    if (!order_is_complete(game)) 
        return;
    play_order_sound(game);
    game.score += 100 * (game.orderIndex + 1);

    if (game.mode == RUSH_MODE)
        game.score += (int)(current_order_time_left(game, now) / 1000) * 2;

    // The last order goes straight to the win screen.
    if (game.orderIndex + 1 >= order_count_for_mode(game.mode))
    {
        game.orderIndex++;
        game.screen = WON_SCREEN;
        return;
    }

    // Keep the hearts this customer had when they were served.
    if (game.mode == RUSH_MODE)
        game.satisfaction = hearts_left(game, now);

    // Otherwise show "Order complete" and let the customer leave first.
    // The order number changes when the next customer arrives.
    game.orderJustCompleted = true;
    game.servedFoodRemoved = false;
    game.orderCompletedAt = now;
}

static void record_item_for_order(GameState &game, ItemCategory category, int tier,
                                  int row, int column, unsigned int now)
{
    // Only an exact category and tier advances an order. Recipe deliveries are
    // consumed immediately so a completed ingredient cannot be merged again.
    bool counted = false;
    bool requestedCategory = false;
    bool requirementAlreadyMet = false;
    int requestedTier = -1;

    if (game.mode == RECIPE_MODE)
    {
        if (category == current_order_category(game))
        {
            requestedCategory = true;
            requestedTier = current_order_tier(game);
            requirementAlreadyMet = game.recipeFirstDelivered >= current_order_quantity(game);
            if (tier == requestedTier && !requirementAlreadyMet)
            {
                game.recipeFirstDelivered++;
                counted = true;
            }
        }
        else if (category == current_order_secondary_category(game))
        {
            requestedCategory = true;
            requestedTier = current_order_secondary_tier(game);
            requirementAlreadyMet = game.recipeSecondDelivered >= current_order_secondary_quantity(game);
            if (tier == requestedTier && !requirementAlreadyMet)
            {
                game.recipeSecondDelivered++;
                counted = true;
            }
        }
    }
    else if (category == current_order_category(game))
    {
        requestedCategory = true;
        requestedTier = current_order_tier(game);
        requirementAlreadyMet = game.deliveredCount >= current_order_quantity(game);
        if (tier == requestedTier && !requirementAlreadyMet)
        {
            game.deliveredCount++;
            counted = true;
        }
    }

    if (counted)
    {
        game.lastAction = "Counted: level " + number_text(tier + 1) + ".";
        if (game.mode == RECIPE_MODE)
        {
            game.recipeFoodPending[row][column] = true;
            game.recipeFoodDeliveredAt[row][column] = now;
        }
    }
    else if (requirementAlreadyMet)
        game.lastAction = "That recipe item is already complete.";
    else if (requestedCategory)
        game.lastAction = "Not counted: needs level " + number_text(requestedTier + 1) + ".";
    else
        game.lastAction = "This food is not in the order.";

    complete_order(game, now);
}

static void use_move(GameState &game)
{
    game.moves++;
    if (game.mode == RECIPE_MODE)
        game.movesLeft--;
}

// Recipe Challenge: running out of moves before the recipe is finished ends the game.
static void check_recipe_limit(GameState &game)
{
    if (game.mode == RECIPE_MODE &&
        game.screen == PLAYING_SCREEN &&
        !game.orderJustCompleted &&
        game.movesLeft <= 0 &&
        game.orderIndex < order_count_for_mode(game.mode))
    {
        play_timeout_sound(game);
        game.screen = GAME_OVER_SCREEN;
        game.lostByMoveLimit = true;
    }
}

void select_or_move_cake(GameState &game, int row, int column, unsigned int now)
{
    // The board is locked while the customer is leaving.
    if (game.screen != PLAYING_SCREEN ||
        game.orderJustCompleted ||
        !is_in_range(row, 0, GRID_SIZE - 1) ||
        !is_in_range(column, 0, GRID_SIZE - 1))
        return;

    if (game.mode == RECIPE_MODE && recipe_food_is_pending(game, row, column))
    {
        clear_selection(game);
        game.lastAction = "This item is being served.";
        return;
    }

    BoardItem target = game.board[row][column];
    // First click selects an occupied square.
    if (game.selectedRow == -1)
    {
        if (target.category != NO_ITEM)
        {
            game.lastAction = "";
            game.selectedRow = row;
            game.selectedColumn = column;
        }
        return;
    }

    int fromRow = game.selectedRow;
    int fromColumn = game.selectedColumn;

    if (fromRow == row && fromColumn == column)
    {
        game.lastAction = "";
        clear_selection(game);
        return;
    }
    BoardItem moving = game.board[fromRow][fromColumn];

    // Move an item into an empty square.
    if (target.category == NO_ITEM)
    {
        game.lastAction = "";
        game.board[row][column] = moving;
        game.board[fromRow][fromColumn].category = NO_ITEM;
        game.board[fromRow][fromColumn].tier = 0;

        play_drop_sound(game);
        use_move(game);
        check_recipe_limit(game);
        clear_selection(game);
        return;
    }

    // Merge only when both the category and tier match.
    if (target.category == moving.category && target.tier == moving.tier && moving.tier < MAX_ITEM_TIER)
    {
        int mergedTier = moving.tier + 1;
        game.board[row][column].tier = mergedTier;
        game.board[fromRow][fromColumn].category = NO_ITEM;
        game.board[fromRow][fromColumn].tier = 0;

        play_merge_sound(game);
        game.score += mergedTier * 10;
        use_move(game);
        record_item_for_order(game, target.category, mergedTier, row, column, now);
        check_recipe_limit(game);
        clear_selection(game);
        return;
    }

    // Clicking a different occupied square selects that square instead.
    game.lastAction = "";
    game.selectedRow = row;
    game.selectedColumn = column;
}

void toggle_pause(GameState &game, unsigned int now)
{
    if (game.screen != PLAYING_SCREEN)
        return;

    if (!game.paused)
    {
        game.paused = true;
        game.pausedAt = now;
        pause_music();
    }
    else
    {
        unsigned int pausedDuration = now - game.pausedAt;
        for (int row = 0; row < GRID_SIZE; row++)
            for (int column = 0; column < GRID_SIZE; column++)
                if (game.recipeFoodPending[row][column])
                    game.recipeFoodDeliveredAt[row][column] += pausedDuration;
        game.orderStartedAt += pausedDuration;
        if (game.orderJustCompleted)
            game.orderCompletedAt += pausedDuration;
        game.paused = false;
        game.pausedAt = 0;
        apply_music_setting(game);
    }
}

// Called every frame: timers, the customer leaving, and the "no moves left" check.
void update_game(GameState &game, unsigned int now)
{
    if (game.screen != PLAYING_SCREEN || game.paused) return;
    if (game.mode == RECIPE_MODE)
        update_recipe_food_collection(game, now);
    // While the customer is leaving nothing else can happen (the timer is paused).
    if (game.orderJustCompleted)
    {
        // When the customer leaves, they take the food they ordered with them.
        if (customer_has_left(game, now) && !game.servedFoodRemoved)
        {
            remove_served_food(game);
            game.servedFoodRemoved = true;
        }

        if (now - game.orderCompletedAt >= CUSTOMER_TRANSITION_TIME)
            finish_order_transition(game, now);

        return;
    }

    if (game.mode == RUSH_MODE)
    {
        // A heart is lost every 10 seconds, with a tick to tell the player.
        int hearts = hearts_left(game, now);

        if (hearts < game.satisfaction)
            play_tick_sound(game);

        game.satisfaction = hearts;

        // Last 10 seconds: one more warning tick.
        if (!game.lowTimeAlertPlayed && current_order_time_left(game, now) <= 10000 && current_order_time_left(game, now) > 0)
        {
            play_tick_sound(game);
            game.lowTimeAlertPlayed = true;
        }

        // After 30 seconds the customer leaves and the game is over.
        if (current_order_time_left(game, now) == 0)
        {
            play_timeout_sound(game);
            game.screen = GAME_OVER_SCREEN;
            game.lostByTimer = true;
            return;
        }
    }

    if (game.screen == PLAYING_SCREEN && !can_make_a_move(game))
        game.screen = GAME_OVER_SCREEN;
}

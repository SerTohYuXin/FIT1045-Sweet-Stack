// src/game_logic.cpp
#include "splashkit.h"
#include "game_state.h"
#include "game_logic.h"
#include "sound_manager.h"
#include "utilities.h"

static void clear_selection(game_state &game)
{
    game.selected_row = -1;
    game.selected_column = -1;
}

// Story: move through the dialogue, then show the four modes.
// Win or game-over screen: return to the four modes.
void advance_screen(game_state &game, unsigned int now)
{
    (void)now; // kept so existing calls stay the same

    if (game.screen == STORY_SCREEN)
    {
        if (game.story_line < 2)
            game.story_line++;
        else
            show_mode_menu(game);
    }
    else if (game.screen == WON_SCREEN || game.screen == GAME_OVER_SCREEN)
    {
        show_mode_menu(game);
    }
}

item_category current_order_category(const game_state &game)
{
    if (game.order_index >= order_count_for_mode(game.mode)) return NO_ITEM;

    return game.orders[game.order_index].category;
}

int current_order_tier(const game_state &game)
{
    if (game.order_index >= order_count_for_mode(game.mode)) return -1;

    return game.orders[game.order_index].tier;
}

int current_order_quantity(const game_state &game)
{
    if (game.order_index >= order_count_for_mode(game.mode)) return 0;

    return game.orders[game.order_index].quantity;
}

item_category current_order_secondary_category(const game_state &game)
{
    if (game.mode != RECIPE_MODE || game.order_index >= order_count_for_mode(game.mode))
        return NO_ITEM;
    return game.orders[game.order_index].secondary_category;
}

int current_order_secondary_tier(const game_state &game)
{
    if (game.mode != RECIPE_MODE || game.order_index >= order_count_for_mode(game.mode))
        return -1;
    return game.orders[game.order_index].secondary_tier;
}

int current_order_secondary_quantity(const game_state &game)
{
    if (game.mode != RECIPE_MODE || game.order_index >= order_count_for_mode(game.mode))
        return 0;
    return game.orders[game.order_index].secondary_quantity;
}

unsigned int current_order_time_left(const game_state &game, unsigned int now)
{
    if (game.mode != RUSH_MODE || game.screen != PLAYING_SCREEN)
        return ORDER_TIME_LIMIT;
    // The timer is paused while the customer is leaving.
    unsigned int clock = game.order_just_completed ? game.order_completed_at : (game.paused ? game.paused_at : now);
    unsigned int elapsed = clock - game.order_started_at;
    return (elapsed >= ORDER_TIME_LIMIT) ? 0 : ORDER_TIME_LIMIT - elapsed;
}

int current_move_limit(const game_state &game)
{
    if (game.mode != RECIPE_MODE || game.order_index >= order_count_for_mode(game.mode)) return 0;
    return game.orders[game.order_index].move_limit;
}

// Rush Hour: hearts the current customer has left, based on how long they have waited.
static int hearts_left(const game_state &game, unsigned int now)
{
    // The clock is paused while the customer is leaving.
    unsigned int clock = game.order_just_completed ? game.order_completed_at : (game.paused ? game.paused_at : now);
    int hearts_lost = (int)((clock - game.order_started_at) / CUSTOMER_HEART_TIME);
    if (hearts_lost >= MAX_HEARTS) return 0;
    return MAX_HEARTS - hearts_lost;
}

bool customer_has_left(const game_state &game, unsigned int now)
{
    unsigned int clock = game.paused ? game.paused_at : now;
    return game.order_just_completed && clock - game.order_completed_at >= ORDER_COMPLETE_MESSAGE_TIME;
}

static bool can_make_a_move(const game_state &game)
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
            board_item first = game.board[row][col];
            if (first.category == NO_ITEM || first.tier >= MAX_ITEM_TIER)
                continue;
            for (int other_row = 0; other_row < GRID_SIZE; other_row++)
            {
                for (int other_col = 0; other_col < GRID_SIZE; other_col++)
                {
                    if (row == other_row && col == other_col)
                        continue;
                    board_item second = game.board[other_row][other_col];
                    if (first.category == second.category && first.tier == second.tier)
                        return true;
                }
            }
        }
    }
    return false;
}

static bool recipe_food_is_pending(const game_state &game, int row, int column)
{
    return game.recipe_food_pending[row][column];
}

static void update_recipe_food_collection(game_state &game, unsigned int now)
{
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int column = 0; column < GRID_SIZE; column++)
        {
            if (game.recipe_food_pending[row][column] &&
                now - game.recipe_food_delivered_at[row][column] >= ORDER_COMPLETE_MESSAGE_TIME)
            {
                game.board[row][column].category = NO_ITEM;
                game.board[row][column].tier = 0;
                game.recipe_food_pending[row][column] = false;
            }
        }
    }
}
// Take up to "count" foods of one kind off the board (rows first, then columns).
static void remove_foods(game_state &game, item_category category, int tier, int count)
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
static void remove_served_food(game_state &game)
{
    if (game.order_index >= order_count_for_mode(game.mode)) return;

    // Recipe items are removed as soon as each exact requirement is delivered.
    // Other modes still remove their completed items when the customer leaves.
    if (game.mode != RECIPE_MODE)
        remove_foods(game, current_order_category(game), current_order_tier(game), game.delivered_count);
}

// Generates the requested food type in a randomly chosen empty square.
// Returns false if the board has no empty square.
bool generate_item(game_state &game, item_category category)
{
    if (game.screen != PLAYING_SCREEN || category == NO_ITEM)
        return false;
    int empty_count = 0;
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (game.board[row][col].category == NO_ITEM)
                empty_count++;
        }
    }

    if (empty_count == 0)
        return false;

    // Pick one of the empty squares at random.
    int chosen_empty = rnd(empty_count);   // rnd(n) gives 0 to n - 1
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (game.board[row][col].category == NO_ITEM)
            {
                if (chosen_empty == 0)
                {
                    game.board[row][col].category = category;
                    game.board[row][col].tier = 0;
                    game.last_action = "";
                    play_drop_sound(game);
                    return true;
                }
                chosen_empty--;
            }
        }
    }
    return false;
}

static bool order_is_complete(const game_state &game)
{
    if (game.mode == RECIPE_MODE)
    {
        return game.recipe_first_delivered >= current_order_quantity(game) && game.recipe_second_delivered >= current_order_secondary_quantity(game);
    }
    return game.delivered_count >= current_order_quantity(game);
}

static void count_prepared_items_for_order(game_state &game, unsigned int now);
static void complete_order(game_state &game, unsigned int now);

// The next customer arrives with the next order.
static void finish_order_transition(game_state &game, unsigned int now)
{
    game.order_just_completed = false;
    game.served_food_removed = false;
    game.order_index++;
    game.delivered_count = 0;
    game.recipe_first_delivered = 0;
    game.recipe_second_delivered = 0;
    game.last_action = "";
    game.order_started_at = now;   // Rush Hour: the new order gets a full timer
    game.low_time_alert_played = false;
    game.satisfaction = MAX_HEARTS;   // every new customer starts with full hearts

    if (game.mode == RECIPE_MODE)
        game.moves_left = current_move_limit(game);

    play_question_sound(game);
    count_prepared_items_for_order(game, now);
    complete_order(game, now);
}

static void complete_order(game_state &game, unsigned int now)
{
    if (!order_is_complete(game)) 
        return;
    play_order_sound(game);
    game.score += 100 * (game.order_index + 1);

    if (game.mode == RUSH_MODE)
        game.score += (int)(current_order_time_left(game, now) / 1000) * 2;

    // The last order goes straight to the win screen.
    if (game.order_index + 1 >= order_count_for_mode(game.mode))
    {
        game.order_index++;
        game.screen = WON_SCREEN;
        return;
    }

    // Keep the hearts this customer had when they were served.
    if (game.mode == RUSH_MODE)
        game.satisfaction = hearts_left(game, now);

    // Otherwise show "Order complete" and let the customer leave first.
    // The order number changes when the next customer arrives.
    game.order_just_completed = true;
    game.served_food_removed = false;
    game.order_completed_at = now;
}

// Count exact matching items that were prepared before this customer arrived.
static void count_prepared_items_for_order(game_state &game, unsigned int now)
{
    if (game.order_index >= order_count_for_mode(game.mode))
        return;

    bool counted_any = false;
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int column = 0; column < GRID_SIZE; column++)
        {
            board_item &item = game.board[row][column];
            if (item.category == NO_ITEM ||
                (game.mode == RECIPE_MODE && recipe_food_is_pending(game, row, column)))
                continue;

            if (game.mode == RECIPE_MODE)
            {
                if (item.category == current_order_category(game) &&
                    item.tier == current_order_tier(game) &&
                    game.recipe_first_delivered < current_order_quantity(game))
                {
                    game.recipe_first_delivered++;
                    game.recipe_food_pending[row][column] = true;
                    game.recipe_food_delivered_at[row][column] = now;
                    counted_any = true;
                }
                else if (item.category == current_order_secondary_category(game) &&
                         item.tier == current_order_secondary_tier(game) &&
                         game.recipe_second_delivered < current_order_secondary_quantity(game))
                {
                    game.recipe_second_delivered++;
                    game.recipe_food_pending[row][column] = true;
                    game.recipe_food_delivered_at[row][column] = now;
                    counted_any = true;
                }
            }
            else if (item.category == current_order_category(game) &&
                     item.tier == current_order_tier(game) &&
                     game.delivered_count < current_order_quantity(game))
            {
                game.delivered_count++;
                counted_any = true;
            }
        }
    }

    if (counted_any)
        game.last_action = "Prepared item counted.";
}
static void record_item_for_order(game_state &game, item_category category, int tier,
                                  int row, int column, unsigned int now)
{
    // Only an exact category and tier advances an order. Recipe deliveries are
    // consumed immediately so a completed ingredient cannot be merged again.
    bool counted = false;
    bool requested_category = false;
    bool requirement_already_met = false;
    int requested_tier = -1;

    if (game.mode == RECIPE_MODE)
    {
        if (category == current_order_category(game))
        {
            requested_category = true;
            requested_tier = current_order_tier(game);
            requirement_already_met = game.recipe_first_delivered >= current_order_quantity(game);
            if (tier == requested_tier && !requirement_already_met)
            {
                game.recipe_first_delivered++;
                counted = true;
            }
        }
        else if (category == current_order_secondary_category(game))
        {
            requested_category = true;
            requested_tier = current_order_secondary_tier(game);
            requirement_already_met = game.recipe_second_delivered >= current_order_secondary_quantity(game);
            if (tier == requested_tier && !requirement_already_met)
            {
                game.recipe_second_delivered++;
                counted = true;
            }
        }
    }
    else if (category == current_order_category(game))
    {
        requested_category = true;
        requested_tier = current_order_tier(game);
        requirement_already_met = game.delivered_count >= current_order_quantity(game);
        if (tier == requested_tier && !requirement_already_met)
        {
            game.delivered_count++;
            counted = true;
        }
    }

    if (counted)
    {
        game.last_action = "Counted: level " + number_text(tier + 1) + ".";
        if (game.mode == RECIPE_MODE)
        {
            game.recipe_food_pending[row][column] = true;
            game.recipe_food_delivered_at[row][column] = now;
        }
    }
    else if (requirement_already_met)
        game.last_action = "That recipe item is already complete.";
    else if (requested_category)
        game.last_action = "Not counted: needs level " + number_text(requested_tier + 1) + ".";
    else
        game.last_action = "This food is not in the order.";

    complete_order(game, now);
}

static void use_move(game_state &game)
{
    game.moves++;
    if (game.mode == RECIPE_MODE)
        game.moves_left--;
}

// Recipe Challenge: running out of moves before the recipe is finished ends the game.
static void check_recipe_limit(game_state &game)
{
    if (game.mode == RECIPE_MODE &&
        game.screen == PLAYING_SCREEN &&
        !game.order_just_completed &&
        game.moves_left <= 0 &&
        game.order_index < order_count_for_mode(game.mode))
    {
        play_timeout_sound(game);
        game.screen = GAME_OVER_SCREEN;
        game.lost_by_move_limit = true;
    }
}

void select_or_move_cake(game_state &game, int row, int column, unsigned int now)
{
    // The board is locked while the customer is leaving.
    if (game.screen != PLAYING_SCREEN ||
        game.order_just_completed ||
        !is_in_range(row, 0, GRID_SIZE - 1) ||
        !is_in_range(column, 0, GRID_SIZE - 1))
        return;

    if (game.mode == RECIPE_MODE && recipe_food_is_pending(game, row, column))
    {
        clear_selection(game);
        game.last_action = "This item is being served.";
        return;
    }

    board_item target = game.board[row][column];
    // First click selects an occupied square.
    if (game.selected_row == -1)
    {
        if (target.category != NO_ITEM)
        {
            game.last_action = "";
            game.selected_row = row;
            game.selected_column = column;
        }
        return;
    }

    int from_row = game.selected_row;
    int from_column = game.selected_column;

    if (from_row == row && from_column == column)
    {
        game.last_action = "";
        clear_selection(game);
        return;
    }
    board_item moving = game.board[from_row][from_column];

    // Move an item into an empty square.
    if (target.category == NO_ITEM)
    {
        game.last_action = "";
        game.board[row][column] = moving;
        game.board[from_row][from_column].category = NO_ITEM;
        game.board[from_row][from_column].tier = 0;

        play_drop_sound(game);
        use_move(game);
        check_recipe_limit(game);
        clear_selection(game);
        return;
    }

    // Merge only when both the category and tier match.
    if (target.category == moving.category && target.tier == moving.tier && moving.tier < MAX_ITEM_TIER)
    {
        int merged_tier = moving.tier + 1;
        game.board[row][column].tier = merged_tier;
        game.board[from_row][from_column].category = NO_ITEM;
        game.board[from_row][from_column].tier = 0;

        play_merge_sound(game);
        game.score += merged_tier * 10;
        use_move(game);
        record_item_for_order(game, target.category, merged_tier, row, column, now);
        check_recipe_limit(game);
        clear_selection(game);
        return;
    }

    // Clicking a different occupied square selects that square instead.
    game.last_action = "";
    game.selected_row = row;
    game.selected_column = column;
}

void toggle_pause(game_state &game, unsigned int now)
{
    if (game.screen != PLAYING_SCREEN)
        return;

    if (!game.paused)
    {
        game.paused = true;
        game.paused_at = now;
        pause_music();
    }
    else
    {
        unsigned int paused_duration = now - game.paused_at;
        for (int row = 0; row < GRID_SIZE; row++)
            for (int column = 0; column < GRID_SIZE; column++)
                if (game.recipe_food_pending[row][column])
                    game.recipe_food_delivered_at[row][column] += paused_duration;
        game.order_started_at += paused_duration;
        if (game.order_just_completed)
            game.order_completed_at += paused_duration;
        game.paused = false;
        game.paused_at = 0;
        apply_music_setting(game);
    }
}

// Called every frame: timers, the customer leaving, and the "no moves left" check.
void update_game(game_state &game, unsigned int now)
{
    if (game.screen != PLAYING_SCREEN || game.paused) return;
    if (game.mode == RECIPE_MODE)
        update_recipe_food_collection(game, now);
    // While the customer is leaving nothing else can happen (the timer is paused).
    if (game.order_just_completed)
    {
        // When the customer leaves, they take the food they ordered with them.
        if (customer_has_left(game, now) && !game.served_food_removed)
        {
            remove_served_food(game);
            game.served_food_removed = true;
        }

        if (now - game.order_completed_at >= CUSTOMER_TRANSITION_TIME)
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
        if (!game.low_time_alert_played && current_order_time_left(game, now) <= 10000 && current_order_time_left(game, now) > 0)
        {
            play_tick_sound(game);
            game.low_time_alert_played = true;
        }

        // After 30 seconds the customer leaves and the game is over.
        if (current_order_time_left(game, now) == 0)
        {
            play_timeout_sound(game);
            game.screen = GAME_OVER_SCREEN;
            game.lost_by_timer = true;
            return;
        }
    }

    if (game.screen == PLAYING_SCREEN && !can_make_a_move(game))
        game.screen = GAME_OVER_SCREEN;
}

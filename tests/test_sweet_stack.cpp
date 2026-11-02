// tests/test_sweet_stack.cpp
#include "splashkit.h"
#include "game_state.h"
#include "game_logic.h"
#include "sound_manager.h"
#include "game_layout.h"
#include "utilities.h"
#include "input.h"

// Print PASS or FAIL for one check.
void check(bool condition, string test_name)
{
    if (condition)
    {
        write_line("PASS: " + test_name);
    }
    else
    {
        write_line("FAIL: " + test_name);
    }
}

// Count how many board squares hold food.
int count_items(const GameState &game)
{
    int count = 0;
    for (int row = 0; row < GRID_SIZE; row++)
        for (int col = 0; col < GRID_SIZE; col++)
            if (game.board[row][col].category != NO_ITEM)
                count++;
    return count;
}

// Put one item on the board by hand.
void place(GameState &game, int row, int col, ItemCategory category, int tier)
{
    game.board[row][col].category = category;
    game.board[row][col].tier = tier;
}

void test_screen_flow()
{
    GameState game = GameState();   // every value starts at zero
    start_new_game(game);
    check(game.screen == STORY_SCREEN, "program starts on the story screen");

    advance_screen(game, 0);
    advance_screen(game, 0);
    check(game.screen == STORY_SCREEN, "story is still showing before the last line is finished");
    advance_screen(game, 0);
    check(game.screen == MODE_MENU, "finishing the story shows the mode menu");

    choose_mode(game, RUSH_MODE, 1000);
    check(game.screen == PLAYING_SCREEN, "choosing a mode starts gameplay immediately");
    check(game.mode == RUSH_MODE, "chosen mode is stored");
    check(game.orderStartedAt == 1000, "timer starts when the mode starts");

    show_mode_menu(game);
    check(game.screen == MODE_MENU, "Back returns to the mode menu (not the story)");

    choose_mode(game, RECIPE_MODE, 2000);
    check(game.screen == PLAYING_SCREEN && game.movesLeft == 12, "choosing another mode starts it immediately with its own settings");
}

void test_generators()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, BEGINNER_MODE, 0);

    check(generate_item(game, DOUGH), "dough generator succeeds");
    check(count_items(game) == 1, "dough generator adds exactly one item");

    check(generate_item(game, TART), "tart generator succeeds");
    check(generate_item(game, CAKE), "cake generator succeeds");
    check(count_items(game) == 3, "three generator clicks give three items");

    bool allTierZero = true;
    for (int row = 0; row < GRID_SIZE; row++)
        for (int col = 0; col < GRID_SIZE; col++)
            if (game.board[row][col].category != NO_ITEM && game.board[row][col].tier != 0)
                allTierZero = false;
    check(allTierZero, "new items start at tier 0");

    for (int i = 0; i < 13; i++)
        generate_item(game, DOUGH);
    check(count_items(game) == GRID_SIZE * GRID_SIZE, "board can be filled completely");
    check(!generate_item(game, DOUGH), "generator fails when the board is full");
}

void test_move_and_merge()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, CLASSIC_MODE, 0);

    place(game, 0, 0, DOUGH, 0);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 2, 2, 0);
    check(game.board[2][2].category == DOUGH && game.board[0][0].category == NO_ITEM, "item moves into an empty square");
    check(game.moves == 1, "a move is counted");

    place(game, 0, 0, DOUGH, 0);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 2, 2, 0);
    check(game.board[2][2].tier == 1 && game.board[0][0].category == NO_ITEM, "two matching items merge into a higher tier");
    check(game.score == 10, "merge adds score");

    place(game, 0, 0, TART, 1);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 2, 2, 0);
    check(game.board[2][2].category == DOUGH && game.board[0][0].category == TART, "different foods do not merge");
}

// Remove every food from the board so a test starts from a known position.
void empty_board(GameState &game)
{
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            game.board[row][column].category = NO_ITEM;
            game.board[row][column].tier = 0;
        }
    }
}

// Finish the current Beginner/Classic/Rush order by merging two foods
// that are one tier lower than the tier the customer asked for.
void finish_current_order(GameState &game, unsigned int now)
{
    ItemCategory food = current_order_category(game);
    int tier = current_order_tier(game);

    empty_board(game);
    game.deliveredCount = current_order_quantity(game) - 1;
    place(game, 0, 0, food, tier - 1);
    place(game, 0, 1, food, tier - 1);
    select_or_move_cake(game, 0, 0, now);
    select_or_move_cake(game, 0, 1, now);
}

// Move one food from one square to another empty square.
void move_food(GameState &game, int fromRow, int fromColumn, int toRow, int toColumn)
{
    select_or_move_cake(game, fromRow, fromColumn, 0);
    select_or_move_cake(game, toRow, toColumn, 0);
}

void test_order_complete()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, BEGINNER_MODE, 0);

    // First order needs one tier 2 dough. Merge two tier 1 dough to make it.
    place(game, 0, 0, DOUGH, 1);
    place(game, 0, 1, DOUGH, 1);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 0, 1, 0);
    check(game.orderJustCompleted, "making the ordered item completes the order");
    check(game.score == 120, "completed order adds the order bonus plus merge score");
}

void test_recipe_rejects_higher_tier()
{
    GameState game = GameState();
    choose_mode(game, RECIPE_MODE, 0);

    game.orders[0].category = DOUGH;
    game.orders[0].tier = 1;
    game.orders[0].quantity = 1;
    game.orders[0].secondaryCategory = CAKE;
    game.orders[0].secondaryTier = 1;
    game.orders[0].secondaryQuantity = 1;
    game.orders[0].moveLimit = 24;
    game.movesLeft = 24;

    empty_board(game);
    place(game, 0, 0, DOUGH, 1);
    place(game, 0, 1, DOUGH, 1);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 0, 1, 0);
    check(game.recipeFirstDelivered == 0 && !game.orderJustCompleted,
          "a higher-tier merge does not satisfy a lower-tier recipe item");

    empty_board(game);
    place(game, 0, 0, DOUGH, 0);
    place(game, 0, 1, DOUGH, 0);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 0, 1, 0);
    check(game.recipeFirstDelivered == 1 && !game.orderJustCompleted,
          "an exact-tier recipe merge advances only its matching item");
}

void test_customer_leaving()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, BEGINNER_MODE, 0);

    finish_current_order(game, 1000);
    check(game.orderJustCompleted && game.screen == PLAYING_SCREEN, "finished order starts the customer transition");
    check(game.orderIndex == 0, "order number stays the same until the next customer arrives");
    check(!customer_has_left(game, 1500), "customer is still shown during the Order complete message");
    check(customer_has_left(game, 1000 + ORDER_COMPLETE_MESSAGE_TIME), "customer has left after the message");

    // The board is locked, so no second order can be completed by accident.
    int movesBefore = game.moves;
    empty_board(game);
    place(game, 2, 0, DOUGH, 0);
    place(game, 2, 1, DOUGH, 0);
    select_or_move_cake(game, 2, 0, 1200);
    select_or_move_cake(game, 2, 1, 1200);
    check(game.board[2][0].category == DOUGH && game.board[2][1].category == DOUGH,"board is locked while the customer is leaving");
    check(game.moves == movesBefore, "locked clicks do not count as moves");

    update_game(game, 1000 + CUSTOMER_TRANSITION_TIME - 1);
    check(game.orderJustCompleted && game.orderIndex == 0, "next customer does not arrive too early");

    update_game(game, 1000 + CUSTOMER_TRANSITION_TIME);
    check(!game.orderJustCompleted && game.orderIndex == 1, "next customer arrives with the next order");
    check(game.deliveredCount == 0, "new order starts with no progress");

    select_or_move_cake(game, 2, 0, 5000);
    select_or_move_cake(game, 2, 1, 5000);
    check(game.board[2][1].tier == 1, "board works again for the next customer");
}

void test_final_order_wins()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, CLASSIC_MODE, 0);
    unsigned int now = 0;

    for (int i = 0; i < order_count_for_mode(game.mode) - 1; i++)
    {
        finish_current_order(game, now);
        update_game(game, now + CUSTOMER_TRANSITION_TIME);
        now += 10000;
    }
    check(game.orderIndex == order_count_for_mode(game.mode) - 1 && game.screen == PLAYING_SCREEN, "first two orders each bring a new customer");

    finish_current_order(game, now);
    check(game.screen == WON_SCREEN, "final order shows the win screen");
    check(!game.orderJustCompleted, "final order has no customer transition");
}

void test_rush_hour_timer()
{
    check(ORDER_TIME_LIMIT == 30000, "Rush Hour gives each customer 30 seconds");
    check(CUSTOMER_HEART_TIME == 10000, "a heart fades every 10 seconds");
    check(MAX_HEARTS * CUSTOMER_HEART_TIME == ORDER_TIME_LIMIT, "three hearts last exactly one order");

    GameState game = GameState();
    choose_mode(game, RUSH_MODE, 0);
    check(game.satisfaction == 3, "Rush Hour customer starts with three hearts");
    check(current_order_time_left(game, 5000) == 25000, "timer counts down from 30 seconds");

    // One heart is lost every 10 seconds (the last heart goes when time is up).
    update_game(game, 9999);
    check(game.satisfaction == 3, "still three hearts just before 10 seconds");
    update_game(game, 10000);
    check(game.satisfaction == 2, "first heart is lost at 10 seconds");
    update_game(game, 19999);
    check(game.satisfaction == 2, "still two hearts just before 20 seconds");
    update_game(game, 20000);
    check(game.satisfaction == 1, "second heart is lost at 20 seconds");
    update_game(game, 29999);
    check(game.screen == PLAYING_SCREEN && game.satisfaction == 1, "customer waits for the full 30 seconds");
    update_game(game, ORDER_TIME_LIMIT);
    check(game.screen == GAME_OVER_SCREEN && game.lostByTimer, "customer leaves at 30 seconds and the game is over");

    // Hearts and timer are frozen while the customer is leaving.
    choose_mode(game, RUSH_MODE, 0);
    finish_current_order(game, 15000);
    check(game.satisfaction == 2, "customer keeps the hearts they had when served");
    update_game(game, 15500);
    check(game.satisfaction == 2, "hearts do not change while the customer is leaving");
    check(current_order_time_left(game, 95000) == ORDER_TIME_LIMIT - 15000, "timer is frozen during the transition");

    // A late order must not be punished by the transition time.
    choose_mode(game, RUSH_MODE, 0);
    finish_current_order(game, 29000);
    update_game(game, 30500);
    check(game.screen == PLAYING_SCREEN, "no game over while the customer is leaving");

    // The next customer gets full hearts and a full timer.
    update_game(game, 29000 + CUSTOMER_TRANSITION_TIME);
    check(game.satisfaction == 3, "next customer starts with three hearts");
    check(current_order_time_left(game, 29000 + CUSTOMER_TRANSITION_TIME) == ORDER_TIME_LIMIT, "next customer gets a full timer");
}

void test_recipe_move_limit()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, RECIPE_MODE, 0);
    int limit = game.movesLeft;
    check(limit == 12, "Recipe Challenge starts with 12 moves for the first customer");

    empty_board(game);
    place(game, 0, 0, DOUGH, 0);
    for (int i = 0; i < limit - 1; i++)
    {
        if (i % 2 == 0)
            move_food(game, 0, 0, 0, 1);
        else
            move_food(game, 0, 1, 0, 0);
    }
    check(game.movesLeft == 1 && game.screen == PLAYING_SCREEN, "moves are counted down while the game continues");

    if ((limit - 1) % 2 == 0)
        move_food(game, 0, 0, 0, 1);
    else
        move_food(game, 0, 1, 0, 0);
    check(game.screen == GAME_OVER_SCREEN && game.lostByMoveLimit, "using the last move without finishing the recipe ends the game");

    // Finishing the order on the very last move must not end the game.
    choose_mode(game, RECIPE_MODE, 0);
    empty_board(game);
    game.movesLeft = 1;
    game.recipeFirstDelivered = current_order_quantity(game) - 1;
    game.recipeSecondDelivered = current_order_secondary_quantity(game);
    place(game, 0, 0, current_order_category(game), current_order_tier(game) - 1);
    place(game, 0, 1, current_order_category(game), current_order_tier(game) - 1);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 0, 1, 0);
    check(game.orderJustCompleted && game.screen == PLAYING_SCREEN, "finishing an order on the last move is not a loss");
    update_game(game, ORDER_COMPLETE_MESSAGE_TIME);
    check(game.board[0][1].category == NO_ITEM, "recipe food disappears when the customer leaves");
}

void test_served_food_disappears()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, BEGINNER_MODE, 0);
    finish_current_order(game, 1000);
    place(game, 3, 3, TART, 0);   // a food that is not part of the order

    ItemCategory food = current_order_category(game);
    int tier = current_order_tier(game);

    check(game.board[0][1].category == food && game.board[0][1].tier == tier, "ordered food is still on the board while the customer is thanked");

    update_game(game, 1000 + ORDER_COMPLETE_MESSAGE_TIME - 1);
    check(game.board[0][1].category == food, "food stays until the customer leaves");

    update_game(game, 1000 + ORDER_COMPLETE_MESSAGE_TIME);
    check(game.board[0][1].category == NO_ITEM, "ordered food disappears when the customer leaves");
    check(game.board[3][3].category == TART, "other foods stay on the board");
}

// Beginner, Classic and Recipe have no hearts and no timer.
void test_hearts_only_in_rush_hour()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, BEGINNER_MODE, 0);
    update_game(game, 500000);
    check(game.screen == PLAYING_SCREEN, "Beginner has no timer");

    choose_mode(game, CLASSIC_MODE, 0);
    update_game(game, 500000);
    check(game.screen == PLAYING_SCREEN, "Classic Bakery has no timer");

    choose_mode(game, RECIPE_MODE, 0);
    update_game(game, 500000);
    check(game.screen == PLAYING_SCREEN, "Recipe Challenge has no timer");
    check(current_move_limit(game) == game.movesLeft, "Recipe Challenge starts with the full move limit");
}

void test_back_to_menu()
{
    GameState game = GameState();   // every value starts at zero
    choose_mode(game, RUSH_MODE, 0);
    finish_current_order(game, 1000);

    show_mode_menu(game);
    check(game.screen == MODE_MENU, "Back returns to the mode menu");

    choose_mode(game, BEGINNER_MODE, 2000);
    check(game.screen == PLAYING_SCREEN && !game.orderJustCompleted && game.orderIndex == 0, "choosing a mode after Back starts a fresh game");
}

void test_utilities()
{
    check(number_text(0) == "0", "number_text of 0");
    check(number_text(1250) == "1250", "number_text of 1250");

    check(is_in_range(5, 1, 10), "5 is inside 1 to 10");
    check(is_in_range(1, 1, 10) && is_in_range(10, 1, 10), "range includes both ends");
    check(!is_in_range(0, 1, 10) && !is_in_range(11, 1, 10), "values outside the range are rejected");

    check(estimated_text_width("Hello") == 5 * TEXT_CHAR_WIDTH, "text width is estimated per character");
    check(centered_text_x("abcd", 100, 100) == 100 + (100 - 4 * TEXT_CHAR_WIDTH) / 2, "text is centred in a box");

    check(point_in_rectangle(10, 10, 10, 10, 50, 20), "top-left corner is inside a rectangle");
    check(point_in_rectangle(59, 29, 10, 10, 50, 20), "bottom-right pixel is inside a rectangle");
    check(!point_in_rectangle(60, 20, 10, 10, 50, 20), "just right of a rectangle is outside");
    check(!point_in_rectangle(9, 20, 10, 10, 50, 20), "just left of a rectangle is outside");
}

// The screen layout is shared by drawing and clicking, so check it makes sense.
void test_layout()
{
    check(BOARD_SIZE == 4 * CELL_SIZE + 3 * CELL_GAP, "board size matches cells and gaps");
    check(BOARD_Y + BOARD_SIZE <= GENERATOR_Y, "generator buttons start below the board");
    check(GENERATOR_Y + GENERATOR_HEIGHT <= WINDOW_HEIGHT, "generator buttons fit inside the window");
    check(DOUGH_BUTTON_X + GENERATOR_WIDTH <= TART_BUTTON_X, "dough and tart buttons do not overlap");
    check(TART_BUTTON_X + GENERATOR_WIDTH <= CAKE_BUTTON_X, "tart and cake buttons do not overlap");
    check(CAKE_BUTTON_X + GENERATOR_WIDTH <= BOARD_X + BOARD_SIZE, "buttons are no wider than the board");
    check(FOOD_IMAGE_SIZE + 2 * FOOD_MARGIN == CELL_SIZE, "food picture is centred in its square");
    check(GENERATOR_ICON_SIZE + 8 <= GENERATOR_HEIGHT, "generator icon fits inside its button");
    check(BOARD_X + BOARD_SIZE < RIGHT_X, "board does not overlap the right-hand panels");
    check(RIGHT_X + CUSTOMER_BOX_WIDTH <= ORDER_BOX_X, "customer box does not overlap the order box");
    check(ORDER_BOX_X + ORDER_BOX_WIDTH <= WINDOW_WIDTH, "order box fits inside the window");
    check(BACK_X + BACK_WIDTH <= WINDOW_WIDTH, "Back button fits inside the window");

    // Text must keep a margin of at least 16 pixels inside its box.
    check(estimated_text_width("Orders ask for 1 or 2 items") + 2 * 20 <= MODE_CARD_WIDTH, "longest mode-card line fits its card");
    check(estimated_text_width("+ 1 x level 4 tart") + 2 * 16 <= ORDER_BOX_WIDTH, "recipe order line fits the order box");
    check(estimated_text_width("ORDER COMPLETE!") + 2 * 16 <= ORDER_BOX_WIDTH, "order-complete message fits the order box");
    check(estimated_text_width("Click or press SPACE to choose a mode again.") + 2 * 16 <= END_BOX_WIDTH, "end-screen hint fits its panel");
    check(MODE_RIGHT_X + MODE_CARD_WIDTH <= WINDOW_WIDTH, "mode cards fit inside the window");

    // Settings button and panel.
    check(SETTINGS_BUTTON_X + SETTINGS_BUTTON_WIDTH <= WINDOW_WIDTH, "Settings button fits inside the window");
    check(SETTINGS_BOX_X + SETTINGS_BOX_WIDTH <= WINDOW_WIDTH && SETTINGS_BOX_Y + SETTINGS_BOX_HEIGHT <= WINDOW_HEIGHT,
          "settings panel fits inside the window");
    check(SETTINGS_TOGGLE_X + SETTINGS_TOGGLE_WIDTH + 16 <= SETTINGS_BOX_X + SETTINGS_BOX_WIDTH, "ON/OFF buttons fit inside the panel");
    check(SETTINGS_MUSIC_Y + SETTINGS_TOGGLE_HEIGHT <= SETTINGS_EFFECTS_Y, "the two switches do not overlap");
    check(SETTINGS_EFFECTS_Y + SETTINGS_TOGGLE_HEIGHT < SETTINGS_CLOSE_Y, "Close button is below the switches");
    check(SETTINGS_CLOSE_Y + SETTINGS_CLOSE_HEIGHT + 16 <= SETTINGS_BOX_Y + SETTINGS_BOX_HEIGHT, "Close button fits inside the panel");
    check(SETTINGS_BOX_X + 30 + estimated_text_width("Background music") + 16 <= SETTINGS_TOGGLE_X, "music label does not touch its button");
    check(estimated_text_width("Changes stay until you change them.") + 2 * 16 <= SETTINGS_BOX_WIDTH, "settings hint fits the panel");

    // Story screen text.
    check(estimated_text_width("Mia: Hi, I'm Mia, the baker who runs this shop.") + 2 * 30 <= 570, "story introduction fits the dialogue box");
    check(estimated_text_width("MIA - SHOP OWNER") + 2 * 16 <= STORY_NAME_WIDTH, "name plate fits under the portrait");
    check(STORY_NAME_Y >= 416, "name plate does not cover the portrait");
}

// True when one order has sensible values for the given mode.
bool order_is_valid(const OrderSpec &order, GameMode mode)
{
    bool ok = is_in_range(order.category, DOUGH, CAKE) &&
              is_in_range(order.tier, 1, MAX_ITEM_TIER) &&
              is_in_range(order.quantity, 1, 2) &&
              order.moveLimit > 0;

    if (mode == RECIPE_MODE)
    {
        ok = ok && is_in_range(order.secondaryCategory, DOUGH, CAKE) &&
             order.secondaryCategory != order.category &&
             is_in_range(order.secondaryTier, 1, MAX_ITEM_TIER) &&
             is_in_range(order.secondaryQuantity, 1, 2);
    }
    else
    {
        ok = ok && order.secondaryCategory == NO_ITEM && order.secondaryQuantity == 0;
    }

    return ok;
}

// Each of the three orders asks for a different food category.
bool orders_use_each_category_once(const GameState &game)
{
    int seen[4] = {0, 0, 0, 0};

    for (int i = 0; i < ORDER_COUNT; i++)
        seen[game.orders[i].category]++;

    return seen[DOUGH] == 1 && seen[TART] == 1 && seen[CAKE] == 1;
}

// Random orders are different every time, so these checks only test that
// every possible result is valid, never that two runs differ.
void test_random_orders()
{
    GameMode modes[3] = {CLASSIC_MODE, RUSH_MODE, RECIPE_MODE};
    string names[3] = {"Classic Bakery", "Rush Hour", "Recipe Challenge"};

    for (int m = 0; m < 3; m++)
    {
        bool allValid = true;
        bool eachCategoryOnce = true;
        bool oldOrdersReplaced = true;
        bool movesMatchFirstOrder = true;
        bool rushAlwaysOne = true;
        bool sawOne = false;
        bool sawTwo = false;
        bool firstOrderSeen[4] = {false, false, false, false};

        for (int attempt = 0; attempt < 200; attempt++)
        {
            GameState game = GameState();
            choose_mode(game, modes[m], 0);

            // Damage the orders, then start the mode again: it must make a fresh set.
            for (int i = 0; i < order_count_for_mode(modes[m]); i++)
            {
                game.orders[i].quantity = 99;
                game.orders[i].tier = -5;
            }
            choose_mode(game, modes[m], 0);

            for (int i = 0; i < order_count_for_mode(modes[m]); i++)
            {
                if (!order_is_valid(game.orders[i], modes[m])) allValid = false;
                if (game.orders[i].quantity == 99 || game.orders[i].tier == -5) oldOrdersReplaced = false;
                if (modes[m] == RUSH_MODE && game.orders[i].quantity != 1) rushAlwaysOne = false;
                if (game.orders[i].quantity == 1) sawOne = true;
                if (game.orders[i].quantity == 2) sawTwo = true;
            }

            firstOrderSeen[game.orders[0].category] = true;
            if (!orders_use_each_category_once(game)) eachCategoryOnce = false;
            if (modes[m] == RECIPE_MODE && game.movesLeft != game.orders[0].moveLimit) movesMatchFirstOrder = false;
        }

        check(allValid, names[m] + ": every random order has valid values");
        check(oldOrdersReplaced, names[m] + ": starting the mode makes a fresh set of orders");
        check(eachCategoryOnce, names[m] + ": the three orders use each food once");
        check(firstOrderSeen[DOUGH] && firstOrderSeen[TART] && firstOrderSeen[CAKE],
              names[m] + ": any food can be the first order (fair shuffle)");

        if (modes[m] == RUSH_MODE)
            check(rushAlwaysOne, "Rush Hour orders ask for one item");
        if (modes[m] == CLASSIC_MODE)
        {
            check(order_count_for_mode(CLASSIC_MODE) == 5, "Classic Bakery serves five customers");
            check(sawOne && sawTwo, "Classic Bakery orders can ask for one or two items");
        }
        if (modes[m] == RECIPE_MODE)
            check(movesMatchFirstOrder, "Recipe Challenge starts with the first order's move limit");
    }

    // Beginner mode is the guided mode, so its orders never change.
    GameState game = GameState();
    choose_mode(game, BEGINNER_MODE, 0);
    check(game.orders[0].category == DOUGH && game.orders[1].category == TART && game.orders[2].category == CAKE,
          "Beginner orders are always dough, tart, cake");
    check(game.orders[0].tier == 2 && game.orders[1].tier == 2 && game.orders[2].tier == 3,
          "Beginner order tiers are fixed");
}

void test_item_tiers_and_random_squares()
{
    GameState game = GameState();
    choose_mode(game, BEGINNER_MODE, 0);

    // Sixteen generator clicks must fill all sixteen squares (every empty square can be chosen).
    int created = 0;
    bool tiersOk = true;
    for (int i = 0; i < 16; i++)
    {
        if (generate_item(game, (ItemCategory)(DOUGH + i % 3))) created++;
    }
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (game.board[row][col].category == NO_ITEM) created--;
            if (!is_in_range(game.board[row][col].tier, 0, MAX_ITEM_TIER)) tiersOk = false;
        }
    }
    check(created == 16, "sixteen generator clicks fill the whole board");
    check(tiersOk, "generated items stay within the tier limit");
    check(!generate_item(game, DOUGH), "a full board does not accept another item");

    // When only one square is empty, that square must be the one chosen.
    game.board[3][2].category = NO_ITEM;
    game.board[3][2].tier = 0;
    check(generate_item(game, TART) && game.board[3][2].category == TART, "the only empty square is always chosen");

    // With two empty squares, either of them can be chosen
    // (over 100 tries both are seen; the chance of missing one is far too small to matter).
    bool sawFirstSquare = false;
    bool sawSecondSquare = false;
    for (int attempt = 0; attempt < 100; attempt++)
    {
        empty_board(game);
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                place(game, row, col, DOUGH, 0);
        game.board[0][0].category = NO_ITEM;
        game.board[3][3].category = NO_ITEM;
        generate_item(game, TART);
        if (game.board[0][0].category == TART) sawFirstSquare = true;
        if (game.board[3][3].category == TART) sawSecondSquare = true;
    }
    check(sawFirstSquare && sawSecondSquare, "every empty square can be chosen, including the last one");

    // Two foods already at the top tier cannot merge into a tier that has no picture.
    empty_board(game);
    place(game, 0, 0, DOUGH, MAX_ITEM_TIER);
    place(game, 0, 1, DOUGH, MAX_ITEM_TIER);
    select_or_move_cake(game, 0, 0, 0);
    select_or_move_cake(game, 0, 1, 0);
    check(game.board[0][0].tier == MAX_ITEM_TIER && game.board[0][1].tier == MAX_ITEM_TIER &&
          game.board[0][0].category == DOUGH && game.board[0][1].category == DOUGH,
          "top-tier foods do not merge any further");

    // Random play in every mode never creates a tier outside 0 to MAX_ITEM_TIER.
    GameMode modes[4] = {BEGINNER_MODE, CLASSIC_MODE, RUSH_MODE, RECIPE_MODE};
    bool allInRange = true;
    for (int m = 0; m < 4; m++)
    {
        choose_mode(game, modes[m], 0);
        for (int step = 0; step < 1500; step++)
        {
            if (step % 3 == 0) generate_item(game, (ItemCategory)(DOUGH + rnd(3)));
            select_or_move_cake(game, rnd(4), rnd(4), 0);
            update_game(game, 0);
            if (game.screen != PLAYING_SCREEN) choose_mode(game, modes[m], 0);

            for (int row = 0; row < 4; row++)
                for (int col = 0; col < 4; col++)
                    if (!is_in_range(game.board[row][col].tier, 0, MAX_ITEM_TIER)) allInRange = false;
        }
    }
    check(allInRange, "random play never makes a tier above MAX_ITEM_TIER");
}

void test_settings_switches()
{
    GameState game = GameState();
    start_new_game(game);
    check(game.musicOn && game.soundEffectsOn, "music and sound effects both start ON");
    check(!game.settingsOpen, "settings panel starts closed");

    toggle_music(game);
    check(!game.musicOn && game.soundEffectsOn, "turning music off leaves sound effects on");
    toggle_sound_effects(game);
    check(!game.musicOn && !game.soundEffectsOn, "both can be off at the same time");
    toggle_music(game);
    check(game.musicOn && !game.soundEffectsOn, "turning music on leaves sound effects off");
    toggle_sound_effects(game);
    check(game.musicOn && game.soundEffectsOn, "both can be back on");

    open_settings(game);
    check(game.settingsOpen, "open_settings shows the panel");
    close_settings(game);
    check(!game.settingsOpen, "close_settings hides the panel");

    // Settings stay until the player changes them.
    toggle_music(game);   // music off, effects on
    show_mode_menu(game);
    check(!game.musicOn && game.soundEffectsOn, "settings survive going back to the menu");
    choose_mode(game, RUSH_MODE, 0);
    game.screen = GAME_OVER_SCREEN;
    show_mode_menu(game);
    choose_mode(game, RECIPE_MODE, 0);
    check(!game.musicOn && game.soundEffectsOn, "settings survive a whole round");

    open_settings(game);
    show_mode_menu(game);
    check(!game.settingsOpen, "returning to the menu closes the panel");
}

// Click on the settings button, switches and Close button the way a player would.
void click(GameState &game, int x, int y)
{
    handle_mouse_click(game, x, y, 0);
}

void test_settings_panel_clicks()
{
    GameState game = GameState();
    start_new_game(game);
    show_mode_menu(game);

    click(game, SETTINGS_BUTTON_X + 5, SETTINGS_BUTTON_Y + 5);
    check(game.settingsOpen && game.screen == MODE_MENU, "Settings button opens the panel");

    click(game, SETTINGS_TOGGLE_X + 10, SETTINGS_MUSIC_Y + 10);
    check(!game.musicOn && game.soundEffectsOn, "music switch only changes music");
    click(game, SETTINGS_TOGGLE_X + 10, SETTINGS_EFFECTS_Y + 10);
    check(!game.musicOn && !game.soundEffectsOn, "effects switch only changes sound effects");
    click(game, SETTINGS_TOGGLE_X + 10, SETTINGS_MUSIC_Y + 10);
    check(game.musicOn && !game.soundEffectsOn, "music can be switched back on by itself");

    // While the panel is open the mode cards behind it cannot be clicked.
    click(game, MODE_LEFT_X + 10, MODE_TOP_Y + 10);
    click(game, SETTINGS_BOX_X - 5, SETTINGS_BOX_Y - 5);
    check(game.screen == MODE_MENU && game.settingsOpen, "clicks outside the panel do nothing");

    click(game, SETTINGS_CLOSE_X + 10, SETTINGS_CLOSE_Y + 10);
    check(!game.settingsOpen && game.screen == MODE_MENU, "Close button hides the panel");
    check(game.musicOn && !game.soundEffectsOn, "closing the panel keeps the settings");

    click(game, MODE_LEFT_X + 10, MODE_TOP_Y + 10);
    check(game.screen == PLAYING_SCREEN && game.mode == BEGINNER_MODE, "mode cards work again after closing");
    check(game.musicOn && !game.soundEffectsOn, "settings are unchanged during play");
}

int main()
{
    load_game_sounds(); // the game logic plays sound effects

    test_utilities();
    test_layout();
    test_screen_flow();
    test_generators();
    test_move_and_merge();
    test_order_complete();
    test_recipe_rejects_higher_tier();
    test_customer_leaving();
    test_final_order_wins();
    test_rush_hour_timer();
    test_recipe_move_limit();
    test_served_food_disappears();
    test_hearts_only_in_rush_hour();
    test_back_to_menu();
    test_random_orders();
    test_item_tiers_and_random_squares();
    test_settings_switches();
    test_settings_panel_clicks();

    return 0;
}

// src/game_view.cpp
#include "splashkit.h"
#include "game_view.h"
#include "sound_manager.h"
#include "score_manager.h"
#include "game_logic.h"
#include "game_layout.h"
#include "utilities.h"

// Keep the colour creation in one place so the rest of the file stays readable.
static color rgb(int r, int g, int b)
{
    return rgb_color(r, g, b);
}

static const string UI_FONT_NAME = "sweetstack_ui";
static const int UI_FONT_SIZE = 14;
static bool ui_font_loaded = false;

static void draw_game_text(const string &text, const color &colour, double x, double y)
{
    if (ui_font_loaded)
        draw_text(text, colour, UI_FONT_NAME, UI_FONT_SIZE, x, y);
    else
        draw_text(text, colour, x, y);
}

static int centered_ui_text_x(const string &text, int box_x, int box_width)
{
    if (ui_font_loaded)
        return box_x + (box_width - text_width(text, UI_FONT_NAME, UI_FONT_SIZE)) / 2;
    return centered_text_x(text, box_x, box_width);
}

// Return a readable name for each food chain.
static string item_name(ItemCategory category)
{
    if (category == DOUGH)
    {
        return "Dough";
    }
    else if (category == TART)
    {
        return "Tart";
    }
    else if (category == CAKE)
    {
        return "Cake";
    }

    return "food";
}

// Load one picture and tell the player in the terminal if the file was not found.
static void load_picture(const string &name, const string &path)
{
    load_bitmap(name, path);

    if (!has_bitmap(name))
    {
        write_line("Could not load picture: " + path);
        write_line("Run the game from inside the SweetStack folder.");
    }
}

// Load the background, food pictures, shop owner, and customer portraits.
void load_game_art()
{
    load_font(UI_FONT_NAME, "Resources/fonts/Coiny-Regular.ttf");
    ui_font_loaded = has_font(UI_FONT_NAME);
    if (!ui_font_loaded)
        write_line("Could not load font: Resources/fonts/Coiny-Regular.ttf");

    load_picture("bakery_background", "Resources/images/background.png");
    load_picture("shop_owner", "Resources/images/shop_owner_story.png");
    for (int row = 0; row < 3; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            string name = "food_" + number_text(row) + "_" + number_text(column);
            load_picture(name, "Resources/images/food/" + name + ".png");
        }
    }

    // generator_0 = dough mixer, generator_1 = tart oven, generator_2 = cake mixer.
    for (int i = 0; i < 3; i++)
    {
        string name = "generator_" + number_text(i);
        load_picture(name, "Resources/images/food/" + name + ".png");
    }

    // The customer portraits used on the play screen.
    for (int i = 1; i <= 5; i++)
    {
        string file = "customer_0" + number_text(i);
        load_picture("customer_" + number_text(i),
            "Resources/images/customers/" + file + ".png");
    }
    load_picture("customer_story_1", "Resources/images/customers/customer_01_story.png");
}

// Convert the current mode into a label for the screen.
static string mode_name(GameMode mode)
{
    if (mode == BEGINNER_MODE)
    {
        return "BEGINNER";
    }
    else if (mode == CLASSIC_MODE)
    {
        return "CLASSIC BAKERY";
    }
    else if (mode == RUSH_MODE)
    {
        return "RUSH HOUR";
    }

    return "RECIPE CHALLENGE";
}

// Draw a food item from the atlas.
// Each row represents a food category and each column represents its tier.
static void draw_item(ItemCategory category, int tier, int x, int y)
{
    if (category == NO_ITEM)
    {
        return;
    }

    int atlasRow = 0;
    if (category == DOUGH)
    {
        atlasRow = 0;
    }
    else if (category == TART)
    {
        atlasRow = 1;
    }
    else if (category == CAKE)
    {
        atlasRow = 2;
    }

    // Higher tiers reuse the final picture.
    int atlasColumn = tier;
    if (atlasColumn > 3)
    {
        atlasColumn = 3;
    }

    if (atlasColumn < 0)
    {
        atlasColumn = 0;
    }

    string name = "food_" + number_text(atlasRow) + "_" + number_text(atlasColumn);
    draw_bitmap(name, x + FOOD_MARGIN, y + FOOD_MARGIN);
}

// Draw a customer or the shop owner at a chosen position.
static void draw_character(const string &name, int x, int y)
{
    if (has_bitmap(name))
    {
        draw_bitmap(name, x, y);
    }
    else
    {
        // The picture is missing: draw a plain marker so the problem is easy to see.
        fill_circle(rgb(211, 93, 127), x + 69, y + 63, 40);
        draw_game_text("?", rgb(255, 255, 255), x + 65, y + 55);
    }
}

// Draw the title bar used by each screen.
static void draw_header(const string &subtitle)
{
    fill_rectangle(rgb(255, 232, 221), 0, 0, WINDOW_WIDTH, 112);
    draw_game_text("SWEET STACK", rgb(121, 69, 84), 54, 22);
    draw_game_text(subtitle, rgb(146, 109, 116), 56, 62);
}

// Draw one mode choice on the mode-selection screen.
static void mode_card(int x, int y, const char *title, const char *lineA, const char *lineB, color accent)
{
    fill_rectangle(rgb(255, 255, 255), x, y, MODE_CARD_WIDTH, MODE_CARD_HEIGHT);
    draw_rectangle(accent, x, y, MODE_CARD_WIDTH, MODE_CARD_HEIGHT);
    draw_game_text(title, accent, x + 20, y + 16);
    draw_game_text(lineA, rgb(91, 78, 82), x + 20, y + 55);
    draw_game_text(lineB, rgb(126, 116, 116), x + 20, y + 86);
}

// Draw all four game-mode choices.
// Draw one ON/OFF button for the settings panel.
static void draw_toggle(int y, bool isOn)
{
    color colour = rgb(176, 166, 166);
    string label = "OFF";

    if (isOn)
    {
        colour = rgb(105, 157, 126);
        label = "ON";
    }

    fill_rectangle(colour, SETTINGS_TOGGLE_X, y, SETTINGS_TOGGLE_WIDTH, SETTINGS_TOGGLE_HEIGHT);
    draw_game_text(label, rgb(255, 255, 255),
        centered_ui_text_x(label, SETTINGS_TOGGLE_X, SETTINGS_TOGGLE_WIDTH), y + 12);
}

// Draw the settings panel on top of the mode-selection screen.
static void draw_settings_panel(const GameState &game)
{
    int labelX = SETTINGS_BOX_X + 30;

    // A darker copy offset behind the panel gives it a shadow.
    fill_rectangle(rgb(121, 69, 84), SETTINGS_BOX_X + 6, SETTINGS_BOX_Y + 6,
        SETTINGS_BOX_WIDTH, SETTINGS_BOX_HEIGHT);
    fill_rectangle(rgb(255, 255, 255), SETTINGS_BOX_X, SETTINGS_BOX_Y,
        SETTINGS_BOX_WIDTH, SETTINGS_BOX_HEIGHT);
    draw_rectangle(rgb(121, 69, 84), SETTINGS_BOX_X, SETTINGS_BOX_Y,
        SETTINGS_BOX_WIDTH, SETTINGS_BOX_HEIGHT);

    draw_game_text("SETTINGS", rgb(121, 69, 84),
        centered_ui_text_x("SETTINGS", SETTINGS_BOX_X, SETTINGS_BOX_WIDTH), SETTINGS_BOX_Y + 20);

    draw_game_text("Background music", rgb(91, 78, 82), labelX, SETTINGS_MUSIC_Y + 12);
    draw_toggle(SETTINGS_MUSIC_Y, game.musicOn);

    draw_game_text("Sound effects", rgb(91, 78, 82), labelX, SETTINGS_EFFECTS_Y + 12);
    draw_toggle(SETTINGS_EFFECTS_Y, game.soundEffectsOn);

    draw_game_text("Changes stay until you change them.", rgb(126, 116, 116),
        centered_ui_text_x("Changes stay until you change them.", SETTINGS_BOX_X, SETTINGS_BOX_WIDTH),
        SETTINGS_BOX_Y + 200);

    fill_rectangle(rgb(121, 69, 84), SETTINGS_CLOSE_X, SETTINGS_CLOSE_Y,
        SETTINGS_CLOSE_WIDTH, SETTINGS_CLOSE_HEIGHT);
    draw_game_text("CLOSE", rgb(255, 255, 255),
        centered_ui_text_x("CLOSE", SETTINGS_CLOSE_X, SETTINGS_CLOSE_WIDTH), SETTINGS_CLOSE_Y + 14);
}

static void draw_menu(const GameState &game)
{
    draw_header("Choose how you want to bake");

    fill_rectangle(rgb(121, 69, 84), SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y,
        SETTINGS_BUTTON_WIDTH, SETTINGS_BUTTON_HEIGHT);
    draw_game_text("SETTINGS", rgb(255, 255, 255),
        centered_ui_text_x("SETTINGS", SETTINGS_BUTTON_X, SETTINGS_BUTTON_WIDTH), SETTINGS_BUTTON_Y + 10);

    mode_card(MODE_LEFT_X, MODE_TOP_Y, "1  BEGINNER",
        "Learn as you play", "One-item orders; no timer", rgb(105, 157, 126));

    mode_card(MODE_RIGHT_X, MODE_TOP_Y, "2  CLASSIC BAKERY",
        "Five random customers", "Orders ask for 1 or 2 items", rgb(116, 139, 184));

    mode_card(MODE_LEFT_X, MODE_BOTTOM_Y, "3  RUSH HOUR",
        "30s per order", "Random; 10s per heart fades", rgb(207, 127, 99));

    mode_card(MODE_RIGHT_X, MODE_BOTTOM_Y, "4  RECIPE CHALLENGE",
        "Limited moves for each order", "Random two-food recipes", rgb(165, 117, 179));

    fill_rectangle(rgb(121, 69, 84), 350, 535, 260, 56);
    draw_game_text("CLICK A MODE (OR PRESS 1-4)", rgb(255, 255, 255), 372, 554);

    if (game.settingsOpen)
        draw_settings_panel(game);
}

// Draw the story scene with the shop owner and the customer.
static void draw_story(const GameState &game)
{
    draw_header("A little story before baking");

    if (game.storyLine == 1)
    {
        draw_character("customer_story_1", 52, 212);
    }
    else
    {
        draw_character("shop_owner", 52, 212);
    }

    // Name plate under the speaker, so the player knows who is talking.
    string speaker = "MIA - SHOP OWNER";
    if (game.storyLine == 1)
        speaker = "FESTIVAL GUEST";

    draw_game_text(speaker, rgb(121, 69, 84),
        centered_ui_text_x(speaker, STORY_NAME_X, STORY_NAME_WIDTH), STORY_NAME_Y + 5);

    fill_rectangle(rgb(255, 255, 255), 295, 232, 570, 184);
    draw_rectangle(rgb(231, 207, 207), 295, 232, 570, 184);
    draw_game_text("Mia's Moonlight Bakery", rgb(121, 69, 84), 325, 253);

    if (game.storyLine == 0)
    {
        draw_game_text("Mia: Hi, I'm Mia, the baker who runs this shop.", rgb(91, 78, 82), 325, 289);
        draw_game_text("The Moonlight Festival is tonight!", rgb(91, 78, 82), 325, 325);
        draw_game_text("Our bakery has just reopened.", rgb(91, 78, 82), 325, 358);
    }
    else if (game.storyLine == 1)
    {
        draw_game_text("Customer: We need treats for the festival.", rgb(91, 78, 82), 325, 299);
        draw_game_text("Can you help us prepare the orders?", rgb(91, 78, 82), 325, 330);
    }
    else
    {
        draw_game_text("Mia: Choose your challenge and let's bake!", rgb(91, 78, 82), 325, 299);
        draw_game_text("Every successful recipe helps the festival.", rgb(91, 78, 82), 325, 330);
    }

    fill_rectangle(rgb(121, 69, 84), 390, 482, 180, 54);
    draw_game_text("CONTINUE", rgb(255, 255, 255), 435, 500);
    draw_game_text("Click or press SPACE", rgb(255, 255, 255), 395, 555);
}

// Draw one generator button and a small preview of the food it creates.
static void draw_generator_button(const char *label, ItemCategory category, int x)
{
    fill_rectangle(rgb(255, 242, 235), x, GENERATOR_Y, GENERATOR_WIDTH, GENERATOR_HEIGHT);
    draw_rectangle(rgb(201, 157, 145), x, GENERATOR_Y, GENERATOR_WIDTH, GENERATOR_HEIGHT);

    // The generator atlas has three square sprites in one horizontal row.
    int spriteColumn = 0;
    if (category == TART)
    {
        spriteColumn = 1;
    }
    else if (category == CAKE)
    {
        spriteColumn = 2;
    }

    draw_bitmap("generator_" + number_text(spriteColumn), x + 4, GENERATOR_Y + 4);
    draw_game_text(label, rgb(121, 69, 84), x + 58, GENERATOR_Y + 19);
}

// Draw the three generators below the 4 by 4 board.
static void draw_generators()
{
    draw_generator_button("DOUGH", DOUGH, DOUGH_BUTTON_X);
    draw_generator_button("TART", TART, TART_BUTTON_X);
    draw_generator_button("CAKE", CAKE, CAKE_BUTTON_X);
}

// Beginner mode arrows point to a useful matching pair, or the needed generator.
static int draw_beginner_guidance(const GameState &game)
{
    if (game.mode != BEGINNER_MODE || game.orderJustCompleted || game.orderIndex >= order_count_for_mode(game.mode))
        return 3;

    ItemCategory needed = current_order_category(game);
    int requiredTier = current_order_tier(game);
    int firstRow = -1, firstCol = -1, secondRow = -1, secondCol = -1;

    if (game.selectedRow >= 0 && game.selectedColumn >= 0)
    {
        BoardItem selected = game.board[game.selectedRow][game.selectedColumn];
        if (selected.category == needed && selected.tier < requiredTier)
        {
            for (int row = 0; row < GRID_SIZE; row++)
            {
                for (int col = 0; col < GRID_SIZE; col++)
                {
                    BoardItem item = game.board[row][col];
                    if ((row != game.selectedRow || col != game.selectedColumn) &&
                        item.category == selected.category && item.tier == selected.tier)
                    {
                        secondRow = row;
                        secondCol = col;
                        break;
                    }
                }
                if (secondRow >= 0) break;
            }
            if (secondRow >= 0)
            {
                int x = BOARD_X + secondCol * (CELL_SIZE + CELL_GAP) + CELL_SIZE / 2;
                int y = BOARD_Y + secondRow * (CELL_SIZE + CELL_GAP);
                draw_line(rgb(221, 142, 66), x, y - 28, x, y - 13);
                fill_triangle(rgb(221, 142, 66), x - 8, y - 17, x + 8, y - 17, x, y - 4);
                return 2;
            }
        }
    }

    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            BoardItem first = game.board[row][col];
            if (first.category != needed || first.tier >= requiredTier) continue;
            for (int otherRow = row; otherRow < GRID_SIZE; otherRow++)
            {
                int startCol = (otherRow == row) ? col + 1 : 0;
                for (int otherCol = startCol; otherCol < GRID_SIZE; otherCol++)
                {
                    BoardItem second = game.board[otherRow][otherCol];
                    if (second.category == first.category && second.tier == first.tier)
                    {
                        firstRow = row; firstCol = col;
                        secondRow = otherRow; secondCol = otherCol;
                        break;
                    }
                }
                if (secondRow >= 0) break;
            }
            if (secondRow >= 0) break;
        }
        if (secondRow >= 0) break;
    }

    if (secondRow >= 0)
    {
        int firstX = BOARD_X + firstCol * (CELL_SIZE + CELL_GAP) + CELL_SIZE / 2;
        int firstY = BOARD_Y + firstRow * (CELL_SIZE + CELL_GAP);
        int secondX = BOARD_X + secondCol * (CELL_SIZE + CELL_GAP) + CELL_SIZE / 2;
        int secondY = BOARD_Y + secondRow * (CELL_SIZE + CELL_GAP);
        draw_line(rgb(221, 142, 66), firstX, firstY - 28, firstX, firstY - 13);
        fill_triangle(rgb(221, 142, 66), firstX - 8, firstY - 17, firstX + 8, firstY - 17, firstX, firstY - 4);
        draw_line(rgb(221, 142, 66), secondX, secondY - 28, secondX, secondY - 13);
        fill_triangle(rgb(221, 142, 66), secondX - 8, secondY - 17, secondX + 8, secondY - 17, secondX, secondY - 4);
        return 1;
    }

    bool hasEmpty = false;
    for (int row = 0; row < GRID_SIZE; row++)
        for (int col = 0; col < GRID_SIZE; col++)
            if (game.board[row][col].category == NO_ITEM) hasEmpty = true;
    if (!hasEmpty) return 3;

    int buttonX = (needed == DOUGH) ? DOUGH_BUTTON_X : (needed == TART) ? TART_BUTTON_X : CAKE_BUTTON_X;
    int x = buttonX + GENERATOR_WIDTH / 2;
    int y = GENERATOR_Y;
    draw_line(rgb(221, 142, 66), x, y - 28, x, y - 13);
    fill_triangle(rgb(221, 142, 66), x - 8, y - 17, x + 8, y - 17, x, y - 4);
    return 0;
}

// Draw the customer on the left and their order on the right.
static void draw_customer_and_order(const GameState &game, unsigned int now)
{
    bool leaving = customer_has_left(game, now);
    int textX = ORDER_BOX_X + 16;

    // CUSTOMER box and portrait (the portrait is missing once the customer has left).
    fill_rectangle(rgb(255, 255, 255), RIGHT_X, CUSTOMER_BOX_Y, CUSTOMER_BOX_WIDTH, CUSTOMER_BOX_HEIGHT);
    draw_rectangle(rgb(231, 207, 207), RIGHT_X, CUSTOMER_BOX_Y, CUSTOMER_BOX_WIDTH, CUSTOMER_BOX_HEIGHT);

    if (!leaving)
    {
        int customerNumber = (game.orderIndex % 5) + 1;
        draw_character("customer_" + number_text(customerNumber), RIGHT_X + 4, CUSTOMER_BOX_Y + 9);
    }

    // ORDER box, right next to the customer.
    fill_rectangle(rgb(255, 255, 255), ORDER_BOX_X, CUSTOMER_BOX_Y,
        ORDER_BOX_WIDTH, CUSTOMER_BOX_HEIGHT);
    draw_rectangle(rgb(231, 207, 207), ORDER_BOX_X, CUSTOMER_BOX_Y,
        ORDER_BOX_WIDTH, CUSTOMER_BOX_HEIGHT);

    if (leaving)
    {
        draw_game_text("Next customer...", rgb(126, 116, 116), textX, CUSTOMER_BOX_Y + 60);
        return;
    }

    if (game.orderJustCompleted)
    {
        draw_game_text("ORDER COMPLETE!", rgb(105, 157, 126), textX, CUSTOMER_BOX_Y + 42);
        draw_game_text("Thank you!", rgb(91, 78, 82), textX, CUSTOMER_BOX_Y + 72);
        return;
    }

    draw_game_text("CUSTOMER ORDER", rgb(121, 69, 84), textX, CUSTOMER_BOX_Y + 12);

    string orderText = number_text(current_order_quantity(game));
    orderText += " x level ";
    orderText += number_text(current_order_tier(game) + 1);
    orderText += " ";
    orderText += item_name(current_order_category(game));

    draw_game_text(orderText, rgb(91, 78, 82), textX, CUSTOMER_BOX_Y + 42);

    int firstReady = game.deliveredCount;
    if (game.mode == RECIPE_MODE)
    {
        firstReady = game.recipeFirstDelivered;
    }

    string readyText = "Ready: ";
    readyText += number_text(firstReady);
    readyText += " / ";
    readyText += number_text(current_order_quantity(game));

    draw_game_text(readyText, rgb(126, 116, 116), textX, CUSTOMER_BOX_Y + 68);

    if (game.mode == RECIPE_MODE)
    {
        string secondOrderText = "+ ";
        secondOrderText += number_text(current_order_secondary_quantity(game));
        secondOrderText += " x level ";
        secondOrderText += number_text(current_order_secondary_tier(game) + 1);
        secondOrderText += " ";
        secondOrderText += item_name(current_order_secondary_category(game));

        draw_game_text(secondOrderText, rgb(91, 78, 82), textX, CUSTOMER_BOX_Y + 96);

        string secondReadyText = "Ready: ";
        secondReadyText += number_text(game.recipeSecondDelivered);
        secondReadyText += " / ";
        secondReadyText += number_text(current_order_secondary_quantity(game));

        draw_game_text(secondReadyText, rgb(126, 116, 116), textX, CUSTOMER_BOX_Y + 120);
    }
}

// Draw the progress bar used for the Rush Hour timer and the Recipe moves.
static void draw_bar(color barColour, int x, int y, double fraction)
{
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;

    fill_rectangle(rgb(245, 229, 225), x, y, 260, 19);
    fill_rectangle(barColour, x, y, (int)(260.0 * fraction), 19);
}

// The panel under the order. It shows what matters in the chosen mode:
//   Rush Hour - the customer's hearts and the 60 second timer
//   Recipe - how many moves are left
//   Beginner/Classic - customer number and no timer.
static void draw_status_panel(const GameState &game, unsigned int now)
{
    int textX = RIGHT_X + 20;
    fill_rectangle(rgb(255, 255, 255), RIGHT_X, STATUS_BOX_Y, RIGHT_WIDTH, STATUS_BOX_HEIGHT);
    draw_rectangle(rgb(236, 218, 218), RIGHT_X, STATUS_BOX_Y, RIGHT_WIDTH, STATUS_BOX_HEIGHT);

    if (game.mode == RUSH_MODE)
    {
        draw_game_text("CUSTOMER SATISFACTION", rgb(121, 69, 84), textX, STATUS_BOX_Y + 15);

        for (int heart = 0; heart < MAX_HEARTS; heart++)
        {
            color heartColour = rgb(221, 211, 211);
            if (heart < game.satisfaction)
                heartColour = rgb(229, 87, 116);
            fill_circle(heartColour, RIGHT_X + 39 + heart * 36, STATUS_BOX_Y + 59, 12);
        }

        unsigned int left = current_order_time_left(game, now);
        color timerColour = rgb(228, 167, 101);
        if (left < 15000)
            timerColour = rgb(221, 94, 103);

        draw_bar(timerColour, textX, STATUS_BOX_Y + 86, (double)left / ORDER_TIME_LIMIT);
        draw_game_text(number_text(left / 1000) + " seconds left",
            rgb(91, 78, 82), textX, STATUS_BOX_Y + 112);
    }
    else if (game.mode == RECIPE_MODE)
    {
        draw_game_text("MOVES LEFT", rgb(121, 69, 84), textX, STATUS_BOX_Y + 15);
        draw_game_text(number_text(game.movesLeft) + " of " + number_text(current_move_limit(game)) + " moves", rgb(91, 78, 82), textX, STATUS_BOX_Y + 48);

        color movesColour = rgb(165, 117, 179);
        if (game.movesLeft <= 5)
            movesColour = rgb(221, 94, 103);

        draw_bar(movesColour, textX, STATUS_BOX_Y + 78, (double)game.movesLeft / current_move_limit(game));
        draw_game_text("Out of moves ends the game.", rgb(126, 116, 116), textX, STATUS_BOX_Y + 106);
    }
    else
    {
        draw_game_text("CUSTOMER " + number_text(game.orderIndex + 1) + " OF " + number_text(order_count_for_mode(game.mode)), rgb(121, 69, 84), textX, STATUS_BOX_Y + 15);
        draw_game_text("No timer in this mode.", rgb(91, 78, 82), textX, STATUS_BOX_Y + 55);
        draw_game_text("Take your time!", rgb(126, 116, 116), textX, STATUS_BOX_Y + 85);
    }
}

// Show the selected food and its requested level in every game mode.
static void draw_selection_panel(const GameState &game)
{
    fill_rectangle(rgb(255, 255, 255), RIGHT_X, 470, RIGHT_WIDTH, 130);
    draw_rectangle(rgb(236, 218, 218), RIGHT_X, 470, RIGHT_WIDTH, 130);
    draw_game_text("SELECTED FOOD", rgb(121, 69, 84), RIGHT_X + 20, 482);

    if (game.selectedRow >= 0 && game.selectedColumn >= 0)
    {
        BoardItem selected = game.board[game.selectedRow][game.selectedColumn];
        draw_item(selected.category, selected.tier, RIGHT_X + 8, 488);
        draw_game_text(item_name(selected.category), rgb(91, 78, 82), RIGHT_X + 126, 502);
        draw_game_text("Level " + number_text(selected.tier + 1), rgb(91, 78, 82), RIGHT_X + 126, 528);
        int neededTier = -1;
        if (selected.category == current_order_category(game))
            neededTier = current_order_tier(game);
        else if (game.mode == RECIPE_MODE && selected.category == current_order_secondary_category(game))
            neededTier = current_order_secondary_tier(game);

        if (neededTier >= 0)
            draw_game_text("Order needs level " + number_text(neededTier + 1) + ".",
                rgb(126, 116, 116), RIGHT_X + 126, 554);
        else
            draw_game_text("Not part of this order.", rgb(126, 116, 116), RIGHT_X + 126, 554);
    }
    else if (game.lastAction != "")
    {
        draw_game_text(game.lastAction, rgb(91, 78, 82), RIGHT_X + 20, 528);
    }
    else
    {
        draw_game_text("Nothing selected.", rgb(91, 78, 82), RIGHT_X + 20, 522);
        draw_game_text("Click a food tile on the tray to inspect it.",
            rgb(126, 116, 116), RIGHT_X + 20, 550);
    }
}

// Draw the game board, customer order, and status information.
static void draw_play(const GameState &game, unsigned int now)
{
    draw_header(mode_name(game.mode));

    // Keep a visible way back to the mode menu during play.
    fill_rectangle(rgb(121, 69, 84), BACK_X, BACK_Y, BACK_WIDTH, BACK_HEIGHT);
    draw_game_text("BACK", rgb(255, 255, 255), BACK_X + 33, BACK_Y + 10);

    draw_game_text("SCORE  " + number_text(game.score), rgb(121, 69, 84), 690, 25);
    draw_game_text("MOVES  " + number_text(game.moves), rgb(121, 69, 84), 690, 66);

    fill_rectangle(rgb(121, 69, 84), PAUSE_BUTTON_X, PAUSE_BUTTON_Y,
        PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT);
    string pauseLabel = game.paused ? "RESUME" : "PAUSE";
    draw_game_text(pauseLabel, rgb(255, 255, 255),
        centered_ui_text_x(pauseLabel, PAUSE_BUTTON_X, PAUSE_BUTTON_WIDTH), PAUSE_BUTTON_Y + 10);

    draw_game_text("BAKERY TRAY", rgb(121, 69, 84), BOARD_X, 130);

    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            int x = BOARD_X + col * (CELL_SIZE + CELL_GAP);
            int y = BOARD_Y + row * (CELL_SIZE + CELL_GAP);
            fill_rectangle(rgb(255, 255, 255), x, y, CELL_SIZE, CELL_SIZE);
            draw_rectangle(rgb(239, 224, 220), x, y, CELL_SIZE, CELL_SIZE);

            BoardItem item = game.board[row][col];
            draw_item(item.category, item.tier, x, y);
            if (row == game.selectedRow && col == game.selectedColumn)
            {
                fill_rectangle(rgba_color(211, 93, 127, 45), x, y, CELL_SIZE, CELL_SIZE);
                draw_rectangle(rgb(211, 93, 127), x + 2, y + 2, CELL_SIZE - 4, CELL_SIZE - 4);
                draw_rectangle(rgb(211, 93, 127), x + 4, y + 4, CELL_SIZE - 8, CELL_SIZE - 8);
            }
        }
    }

    draw_generators();
    draw_beginner_guidance(game);
    draw_customer_and_order(game, now);
    draw_status_panel(game, now);

    draw_selection_panel(game);

    if (game.paused)
    {
        fill_rectangle(rgba_color(54, 42, 47, 155), 0, 112, WINDOW_WIDTH, WINDOW_HEIGHT - 112);
        fill_rectangle(rgb(255, 247, 241), PAUSE_PANEL_X, PAUSE_PANEL_Y,
            PAUSE_PANEL_WIDTH, PAUSE_PANEL_HEIGHT);
        draw_rectangle(rgb(211, 93, 127), PAUSE_PANEL_X, PAUSE_PANEL_Y,
            PAUSE_PANEL_WIDTH, PAUSE_PANEL_HEIGHT);
        draw_game_text("PAUSED", rgb(121, 69, 84),
            centered_ui_text_x("PAUSED", PAUSE_PANEL_X, PAUSE_PANEL_WIDTH), PAUSE_PANEL_Y + 32);
        draw_game_text("Click RESUME or press P to continue.", rgb(91, 78, 82),
            centered_ui_text_x("Click RESUME or press P to continue.", PAUSE_PANEL_X, PAUSE_PANEL_WIDTH), PAUSE_PANEL_Y + 78);
    }
}

// Draw one centred line of text inside the end-screen panel.
static void draw_end_line(const string &text, color colour, int y)
{
    draw_game_text(text, colour, centered_ui_text_x(text, END_BOX_X, END_BOX_WIDTH), y);
}

// Draw the win or game-over screen on a white panel so the text is easy to read.
static void draw_end(const GameState &game)
{
    draw_header(mode_name(game.mode) + " RESULTS");
    fill_rectangle(rgb(255, 255, 255), END_BOX_X, END_BOX_Y, END_BOX_WIDTH, END_BOX_HEIGHT);
    draw_rectangle(rgb(231, 207, 207), END_BOX_X, END_BOX_Y, END_BOX_WIDTH, END_BOX_HEIGHT);

    if (game.screen == WON_SCREEN)
    {
        draw_end_line("FESTIVAL SAVED!", rgb(105, 157, 126), END_BOX_Y + 36);
        draw_end_line("You served every order at Mia's bakery.", rgb(91, 78, 82), END_BOX_Y + 86);
    }
    else if (game.lostByTimer)
    {
        draw_end_line("THE CUSTOMER LEFT", rgb(164, 78, 89), END_BOX_Y + 36);
        draw_end_line("They waited too long for their order.", rgb(91, 78, 82), END_BOX_Y + 86);
    }
    else if (game.lostByMoveLimit)
    {
        draw_end_line("OUT OF MOVES", rgb(164, 78, 89), END_BOX_Y + 36);
        draw_end_line("You ran out of moves for this recipe.", rgb(91, 78, 82), END_BOX_Y + 86);
    }
    else
    {
        draw_end_line("NO MOVES LEFT", rgb(164, 78, 89), END_BOX_Y + 36);
        draw_end_line("The board is full and nothing can merge.", rgb(91, 78, 82), END_BOX_Y + 86);
    }


    draw_end_line("Final score: " + number_text(game.score), rgb(91, 78, 82), END_BOX_Y + 126);
    if (game.mode != BEGINNER_MODE)
    {
        if (game.newHighScore)
            draw_end_line("NEW PERSONAL HIGHEST MARK!", rgb(105, 157, 126), END_BOX_Y + 157);
        else
            draw_end_line("Personal highest mark: " + number_text(high_score_for_mode(game.mode)), rgb(146, 109, 116), END_BOX_Y + 157);
    }

    fill_rectangle(rgb(121, 69, 84), END_REPLAY_BUTTON_X, END_BUTTON_Y,
        END_BUTTON_WIDTH, END_BUTTON_HEIGHT);
    draw_rectangle(rgb(121, 69, 84), END_REPLAY_BUTTON_X, END_BUTTON_Y,
        END_BUTTON_WIDTH, END_BUTTON_HEIGHT);
    draw_game_text("PLAY AGAIN", rgb(255, 255, 255),
        centered_ui_text_x("PLAY AGAIN", END_REPLAY_BUTTON_X, END_BUTTON_WIDTH), END_BUTTON_Y + 13);

    fill_rectangle(rgb(214, 171, 157), END_MENU_BUTTON_X, END_BUTTON_Y,
        END_BUTTON_WIDTH, END_BUTTON_HEIGHT);
    draw_rectangle(rgb(214, 171, 157), END_MENU_BUTTON_X, END_BUTTON_Y,
        END_BUTTON_WIDTH, END_BUTTON_HEIGHT);
    draw_game_text("BACK TO MENU", rgb(255, 255, 255),
        centered_ui_text_x("BACK TO MENU", END_MENU_BUTTON_X, END_BUTTON_WIDTH), END_BUTTON_Y + 13);
}

// Draw the background and whichever screen is currently active.
void draw_game(const GameState &game, unsigned int now)
{
    clear_screen(rgb(255, 247, 241));
    // The background picture is already 960 x 700, the same size as the window.
    draw_bitmap("bakery_background", 0, 0);
    if (game.screen == MODE_MENU)
    {
        draw_menu(game);
    }
    else if (game.screen == STORY_SCREEN)
    {
        draw_story(game);
    }
    else if (game.screen == PLAYING_SCREEN)
    {
        draw_play(game, now);
    }
    else
    {
        draw_end(game);
    }
}

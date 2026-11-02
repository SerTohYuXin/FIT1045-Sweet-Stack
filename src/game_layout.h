// src/game_layout.h
#ifndef GAME_LAYOUT_H
#define GAME_LAYOUT_H

#include "splashkit.h"
const int TEXT_CHAR_WIDTH = 8;

// All screen positions and sizes.
const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 700;

// The 4 x 4 board.
const int BOARD_X = 54;
const int BOARD_Y = 160;
const int CELL_SIZE = 112;
const int CELL_GAP = 10;
const int BOARD_SIZE = 4 * CELL_SIZE + 3 * CELL_GAP;   

// Food pictures.
const int FOOD_IMAGE_SIZE = 96;
const int FOOD_MARGIN = (CELL_SIZE - FOOD_IMAGE_SIZE) / 2;

// The three generator buttons sit directly under the board.
const int GENERATOR_Y = BOARD_Y + BOARD_SIZE + 4;      
const int GENERATOR_WIDTH = 150;
const int GENERATOR_HEIGHT = 54;                       
const int GENERATOR_GAP = 14;
const int GENERATOR_ICON_SIZE = 46;                    
const int DOUGH_BUTTON_X = BOARD_X;
const int TART_BUTTON_X = BOARD_X + GENERATOR_WIDTH + GENERATOR_GAP;
const int CAKE_BUTTON_X = BOARD_X + 2 * (GENERATOR_WIDTH + GENERATOR_GAP);

// Back button in the top-right corner of the play screen.
const int BACK_X = 825;
const int BACK_Y = 14;
const int BACK_WIDTH = 110;
const int BACK_HEIGHT = 36;

// Pause control in the play-screen header.
const int PAUSE_BUTTON_X = 825;
const int PAUSE_BUTTON_Y = 60;
const int PAUSE_BUTTON_WIDTH = 110;
const int PAUSE_BUTTON_HEIGHT = 36;

// Right-hand column of the play screen: customer picture, order, hearts, tips.
const int RIGHT_X = 552;
const int RIGHT_WIDTH = 378;
const int CUSTOMER_BOX_Y = 150;
const int CUSTOMER_BOX_WIDTH = 146;    
const int CUSTOMER_BOX_HEIGHT = 144;
const int ORDER_BOX_X = RIGHT_X + CUSTOMER_BOX_WIDTH + 8;
const int ORDER_BOX_WIDTH = RIGHT_X + RIGHT_WIDTH - ORDER_BOX_X;

// The status panel under the order.
const int STATUS_BOX_Y = 310;
const int STATUS_BOX_HEIGHT = 135;

// Settings button (top-right of the mode-selection screen) and the settings panel.
const int SETTINGS_BUTTON_X = 825;
const int SETTINGS_BUTTON_Y = 14;
const int SETTINGS_BUTTON_WIDTH = 110;
const int SETTINGS_BUTTON_HEIGHT = 36;
const int SETTINGS_BOX_X = 280;
const int SETTINGS_BOX_Y = 170;
const int SETTINGS_BOX_WIDTH = 400;
const int SETTINGS_BOX_HEIGHT = 310;
const int SETTINGS_TOGGLE_X = SETTINGS_BOX_X + 270;      // the ON/OFF buttons
const int SETTINGS_TOGGLE_WIDTH = 100;
const int SETTINGS_TOGGLE_HEIGHT = 40;
const int SETTINGS_MUSIC_Y = SETTINGS_BOX_Y + 80;
const int SETTINGS_EFFECTS_Y = SETTINGS_BOX_Y + 140;
const int SETTINGS_CLOSE_X = SETTINGS_BOX_X + 120;
const int SETTINGS_CLOSE_Y = SETTINGS_BOX_Y + 240;
const int SETTINGS_CLOSE_WIDTH = 160;
const int SETTINGS_CLOSE_HEIGHT = 44;

// Name plate under the story character.
const int STORY_NAME_X = 52;
const int STORY_NAME_Y = 424;
const int STORY_NAME_WIDTH = 223;
const int STORY_NAME_HEIGHT = 34;

// The four mode cards on the mode-selection screen.
const int MODE_CARD_WIDTH = 310;
const int MODE_CARD_HEIGHT = 137;
const int MODE_LEFT_X = 150;
const int MODE_RIGHT_X = 500;
const int MODE_TOP_Y = 165;
const int MODE_BOTTOM_Y = 335;

// The panel behind the text on the win and game-over screens.
const int END_BOX_X = 160;
const int END_BOX_Y = 200;
const int END_BOX_WIDTH = 640;
const int END_BOX_HEIGHT = 250;

// Buttons shown after a round ends.
const int END_REPLAY_BUTTON_X = 300;
const int END_MENU_BUTTON_X = 490;
const int END_BUTTON_Y = END_BOX_Y + 190;
const int END_BUTTON_WIDTH = 170;
const int END_BUTTON_HEIGHT = 44;
const int PAUSE_PANEL_X = 280;
const int PAUSE_PANEL_Y = 265;
const int PAUSE_PANEL_WIDTH = 400;
const int PAUSE_PANEL_HEIGHT = 140;


#endif

// src/game_logic.h
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "splashkit.h"
#include "game_state.h"

// Rush Hour gives the player 30 seconds to complete each order.
const unsigned int ORDER_TIME_LIMIT = 30000;

// After an order is finished (times in milliseconds):
// 0 - 1000: "Order complete" message, customer still shown
// 1000 - 1600: customer has left, waiting for the next one
// 1600: next customer arrives with a new order
const unsigned int ORDER_COMPLETE_MESSAGE_TIME = 1000;

// Rush Hour: every customer has 3 hearts. One heart is lost every 10 seconds
// of waiting, and the customer leaves (game over) after 30 seconds.
const int MAX_HEARTS = 3;
const unsigned int CUSTOMER_HEART_TIME = 10000;

const unsigned int CUSTOMER_TRANSITION_TIME = 1600;

// Change screens, update timers and handle cake movement and merging.
void advance_screen(GameState &game, unsigned int now);
void update_game(GameState &game, unsigned int now);
void toggle_pause(GameState &game, unsigned int now);
void select_or_move_cake(GameState &game, int row, int column, unsigned int now);

// Return the current order requirements for the user interface.
int current_order_tier(const GameState &game);
int current_order_quantity(const GameState &game);
int current_order_secondary_tier(const GameState &game);
int current_order_secondary_quantity(const GameState &game);
unsigned int current_order_time_left(const GameState &game, unsigned int now);

// After an order is finished the customer leaves, then the next one arrives.
// True once the "Order complete" message is over and the customer has gone.
// Recipe Challenge: how many moves the current order allows in total.
int current_move_limit(const GameState &game);

bool customer_has_left(const GameState &game, unsigned int now);

#endif

// src/input.h
#ifndef INPUT_H
#define INPUT_H

#include "splashkit.h"
#include "game_state.h"

// Route a mouse click to a menu button or a board cell.
void handle_mouse_click(GameState &game, double mouseX, double mouseY, unsigned int now);

// Handle the keyboard controls for the current screen.
void handle_key_presses(GameState &game, unsigned int now);

#endif

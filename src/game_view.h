// src/game_view.h
#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include "splashkit.h"
#include "game_state.h"

void draw_game(const GameState &game, unsigned int now);
void load_game_art();

#endif

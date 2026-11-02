//src/score_manager.h
#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

#include "game_state.h"

int high_score_for_mode(game_mode mode);
bool record_high_score(game_mode mode, int score);

#endif

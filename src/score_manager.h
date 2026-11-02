#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

#include "game_state.h"

int high_score_for_mode(GameMode mode);
bool record_high_score(GameMode mode, int score);

#endif

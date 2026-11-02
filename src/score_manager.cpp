//src/score_manager.cpp
#include "splashkit.h"
#include "score_manager.h"
#include <fstream>

static const int MODE_COUNT = 3;
static int high_scores[MODE_COUNT] = {0, 0, 0};
static bool scores_loaded = false;

static int mode_index(GameMode mode)
{
    // Beginner has a fixed learning recipe and does not keep a high mark.
    switch (mode)
    {
        case CLASSIC_MODE: return 0;
        case RUSH_MODE: return 1;
        case RECIPE_MODE: return 2;
        default: return -1;
    }
}

static void load_high_scores()
{
    if (scores_loaded) return;

    std::ifstream input("high_scores.txt");
    for (int i = 0; i < MODE_COUNT; i++)
    {
        int score = 0;
        if (input >> score && score >= 0)
            high_scores[i] = score;
    }
    scores_loaded = true;
}

static void save_high_scores()
{
    std::ofstream output("high_scores.txt", std::ios::trunc);
    if (!output)
    {
        write_line("Could not save high scores. Run the game from its project folder.");
        return;
    }

    for (int i = 0; i < MODE_COUNT; i++)
        output << high_scores[i] << '\n';
}

int high_score_for_mode(GameMode mode)
{
    load_high_scores();
    int index = mode_index(mode);
    return (index < 0) ? 0 : high_scores[index];
}

bool record_high_score(GameMode mode, int score)
{
    load_high_scores();
    int index = mode_index(mode);
    if (index < 0 || score <= high_scores[index]) return false;

    high_scores[index] = score;
    save_high_scores();
    return true;
}

# Sweet Stack 🍰
Sweet Stack is a C++ bakery puzzle game built with SplashKit. Help customers by merging matching foods on a 4 × 4 board, completing their orders and keeping the bakery running.

![Sweet Stack mode selection](Resources/images/readme/mode_selection.png)

## Features

- **Four game modes**
  - **Beginner** introduces the core merge mechanic with a fixed sequence of orders, no timer and on-screen guidance.
  - **Classic Bakery** challenges you to serve five customers with randomly generated orders and no timer.
  - **Rush Hour** gives you 30 seconds per order. A heart fades every 10 seconds, and completing an order early earns a time bonus.
  - **Recipe Challenge** asks you to complete two-food recipes within a move limit that increases from 12 to 16, then 20 moves per customer.
- **Merge and upgrade** dough, tarts and cakes through four levels. Only the exact food and level requested by an order will count.
- **Meet five festival customers**, each with their own portrait and randomly assigned orders in the non-Beginner modes.
- **Follow the story** with illustrated characters, subtitles and character dialogue audio.
- **Personal Highest Marks** are saved locally for Classic Bakery, Rush Hour and Recipe Challenge. Beginner uses a fixed learning sequence and does not track a mark.
- **Customise audio** with separate settings for background music and sound effects.
- **Pause, replay or return to the menu** from the game and results screens.

## Gameplay screenshots

### Rush Hour

Serve each customer before the timer runs out. Customer satisfaction fades as time passes.

![Rush Hour gameplay with customer order and timer](Resources/images/readme/rush_hour_gameplay.png)

### Recipe Challenge

Plan merges to complete both requested foods within the move limit.

![Recipe Challenge board showing two recipe requirements and moves remaining](Resources/images/readme/recipe_challenge.png)

### Results

Review the final score and Personal Highest Mark, then replay or return to the menu.

![Rush Hour results screen showing the final score and Personal Highest Mark](Resources/images/readme/results_screen.png)
## How the game works

The board contains 16 squares. Use a food generator to add an item, then move items or merge two matching items into the next level. For example, two Level 1 dough items merge into Level 2 dough. Continue merging until you create the exact item requested by the customer.

An order can request more than one item. In Recipe Challenge, each completed requirement is counted separately and the delivered item is collected after the same one-second acknowledgement used for regular orders. Items of the wrong category or level do not satisfy the order.

### Scoring
- A merge awards **10 points per resulting level**: Level 2 gives 10 points, Level 3 gives 20, and Level 4 gives 30.
- Completing an order awards **100 × its order number**: the first order gives 100 points, the second gives 200, and so on.
- Rush Hour adds **2 points for each whole second remaining** when an order is completed.
- Moving or generating items does not award points.
- Classic Bakery, Rush Hour and Recipe Challenge save their highest scores to `high_scores.txt` in the project folder. The saved values are ordered Classic Bakery, Rush Hour, then Recipe Challenge.

## Under the hood

- **Data-driven game state:** board contents, current mode, customer orders, score and round progress are kept in `GameState`, separately from drawing and input handling.
- **Merge and order validation:** a merge advances an order only when both the food category and tier match the requested item. Each requirement is counted once.
- **Randomised order generation:** Classic Bakery, Rush Hour and Recipe Challenge select varied foods, levels and quantities when a round starts. Beginner keeps its guided order sequence fixed.
- **Mode-specific pacing:** Rush Hour uses a per-order timer and heart countdown. Recipe Challenge uses a per-customer move limit. Completing an order pauses its transition timer while the customer leaves.
- **Persistent local records:** the score manager loads and saves three separate Personal Highest Marks in a small text file in the game folder.
- **Modular C++ structure:** game state, game logic, rendering, input, utilities, sound and score saving are separated into source modules.

## Controls

| Input | Action |
|---|---|
| Mouse click | Advance the story; choose a mode; use a generator; select, move or merge food; or press an on-screen button |
| `1`–`4` | Choose Beginner, Classic Bakery, Rush Hour or Recipe Challenge from the mode-selection screen |
| `Space` | Advance the story or return to mode selection from a results screen |
| `P` or `Esc` while playing | Pause or resume |
| `R` while playing or paused | Return to mode selection |
| `Esc` on the mode-selection screen | Close the Settings panel |
| Results-screen buttons | Choose **Play Again** or **Back to Menu** |

## Build and run

### Requirements

- C++11-compatible compiler (`clang++` is used by the Makefile)
- SplashKit installed and configured for the compiler
- GNU Make

Build and launch from the project folder so SplashKit can find the resources:

```bash
make
make run
```

To build and run the automated checks:

```bash
make test
```

To remove generated executables and test output:

```bash
make clean
```

## Project structure

## Project structure

```text
SweetStack/
├── Resources/
│   ├── audio/       # Music, dialogue and sound effects
│   ├── fonts/       # Coiny font and its licence
│   └── images/      # Bakery, food, customers and README screenshots
├── src/
│   ├── game_state.*     # State and order setup
│   ├── game_logic.*     # Merging, scoring and mode rules
│   ├── game_view.*      # Screens and game drawing
│   ├── input.*          # Keyboard and mouse controls
│   ├── sound_manager.*  # Music and sound effects
│   ├── score_manager.*  # Personal Highest Mark persistence
│   └── utilities.*      # Shared helpers
├── tests/               # Automated game-rule and screen-state checks
├── high_scores.txt      # Saved marks for modes 2–4
├── Makefile
├── README.md
└── SweetStack_Consolidated_Code.cpp
```
## Credits

- Built with **C++11** and **SplashKit**.
- **Coiny** font; see `Resources/fonts/Coiny-OFL.txt` for its licence.

**Author:** Ser Toh Yu Xin  
**Student ID:** 36818402  
**Unit:** FIT1045 Introduction to Programming
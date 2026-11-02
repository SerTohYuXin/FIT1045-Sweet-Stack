CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -l SplashKit

TARGET = sweet_stack.exe
TEST_TARGET = sweet_stack_tests.exe

SOURCES = src/main.cpp \
          src/game_state.cpp \
          src/game_logic.cpp \
          src/game_view.cpp \
          src/input.cpp \
          src/utilities.cpp \
          src/sound_manager.cpp \
          src/score_manager.cpp

# The checks cover game rules, inputs, settings, score tracking and screen flow.
TEST_SOURCES = tests/test_sweet_stack.cpp \
               src/game_state.cpp \
               src/game_logic.cpp \
               src/input.cpp \
               src/utilities.cpp \
               src/sound_manager.cpp \
               src/score_manager.cpp
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

# Builds the checks and runs them. No pipe is used, so a problem cannot be hidden:
#  - if the test program itself crashes, make stops with an error
#  - if any line starts with FAIL, make prints how many and stops with an error
test: $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) -I src $(TEST_SOURCES) -o $(TEST_TARGET) $(LDFLAGS)
	./$(TEST_TARGET) > test_results.txt || { cat test_results.txt; exit 1; }
	@cat test_results.txt
	@if grep -q '^FAIL' test_results.txt; then \
		echo "Failed checks: $$(grep -c '^FAIL' test_results.txt)"; exit 1; \
	else \
		echo "All checks passed: $$(grep -c '^PASS' test_results.txt)"; \
	fi

clean:
	rm -f $(TARGET) $(TEST_TARGET) test_results.txt

.PHONY: all run clean test

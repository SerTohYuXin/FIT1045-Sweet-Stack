// src/utilities.h
#ifndef UTILITIES_H
#define UTILITIES_H

#include "splashkit.h"

// Small helper functions shared by the game and the checks.

// Turn a whole number into text for drawing on screen, e.g. 42 -> "42".
string number_text(int number);

// True when min <= value <= max (the idea behind read_integer_range without any typing).
bool is_in_range(int value, int min, int max);

// True when the point (x, y) is inside the rectangle that starts at (rect_x, rect_y) and is width pixels wide and height pixels high.
bool point_in_rectangle(double x, double y, int rect_x, int rect_y, int width, int height);

// Estimated width in pixels of a line of text.
int estimated_text_width(string text);

// The x position that centres a line of text inside a box.
int centered_text_x(string text, int box_x, int box_width);

#endif

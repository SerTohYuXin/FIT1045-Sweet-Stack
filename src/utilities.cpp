// src/utilities.cpp
#include "splashkit.h"
#include "utilities.h"
#include "game_layout.h"

string number_text(int number)
{
    return to_string(number);
}

bool is_in_range(int value, int min, int max)
{
    return value >= min && value <= max;
}

bool point_in_rectangle(double x, double y, int rect_x, int rect_y, int width, int height)
{
    return x >= rect_x && x < rect_x + width && y >= rect_y && y < rect_y + height;
}

int estimated_text_width(string text)
{
    return (int)text.length() * TEXT_CHAR_WIDTH;
}

int centered_text_x(string text, int box_x, int box_width)
{
    return box_x + (box_width - estimated_text_width(text)) / 2;
}

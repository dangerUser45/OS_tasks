#pragma once

#define RESET           "\x1B[0m"
#define RED             "\x1B[38;2;255;0;0m"
#define GREEN           "\x1B[38;2;0;255;0m"
#define YELLOW          "\x1B[38;2;255;255;0m"
#define BLUE            "\x1B[38;2;0;0;255m"
#define MAGENTA         "\x1B[35;1m"
#define CYAN            "\x1B[36;1m"
#define WHITE           "\x1B[37;1m"
#define INDIGO          "\x1B[38;2;75;0;130m"
#define ORANGE          "\x1B[38;2;255;165;0m"
#define SKY_BLUE        "\x1B[38;2;82;169;254m"

#define RED_BGD         "\x1B[48;2;255;0;0m\x1B[38;2;255;255;255m"
#define YELLOW_BGD      "\x1B[48;2;255;255;0m\x1B[38;2;255;255;255m"

#define COLOR_TEXT(color, text) color text RESET

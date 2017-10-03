#pragma once

#include <random>
#include "globals.h"


namespace factories {
std::string characters_string(size_t min_length = constants::min_capacity,
                              size_t max_length = constants::max_capacity);


std::string non_zero_characters(char min_character = constants::min_character,
                                char max_character = constants::max_character);
}

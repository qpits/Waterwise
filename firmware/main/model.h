#ifndef MODEL_H
#define MODEL_H

#include <array>
#include <cstdint>
bool setup_model();
std::array<float, 2> predict(int16_t samples[]);

#endif
#include "graphics/draw.h"
#include <stdint.h>

static uint32_t debug_y = 500;
static const uint32_t debug_x = 500;

void shell_execute(kstring* line){
    draw_kstring(*line, debug_x, debug_y, BLUE);
    debug_y += 8 * m_scale;
}



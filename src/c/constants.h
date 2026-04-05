#pragma once

#define COLOR_ACCENT PBL_IF_COLOR_ELSE(GColorPurple, GColorBlack)
#define COLOR_ON_ACCENT GColorWhite
#define COLOR_MODAL PBL_IF_COLOR_ELSE(GColorIslamicGreen, GColorBlack)

// For some reason, graphics_text_layout_get_content_size doesn't reserve
// enough space for characters that draw below the baseline, so here we are.
#define DETAIL_TEXT_ADDITIONAL_HEIGHT 4

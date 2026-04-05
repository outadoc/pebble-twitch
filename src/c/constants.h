#pragma once

#define COLOR_ACCENT GColorPurple
#define COLOR_ON_ACCENT GColorWhite

// For some reason, graphics_text_layout_get_content_size doesn't reserve
// enough space for characters that draw below the baseline, so here we are.
#define DETAIL_TEXT_ADDITIONAL_HEIGHT 4

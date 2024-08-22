#pragma once

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_memfile.h>

#include <atomic>

#include "csv_file_structure.h"

// Utilizes resources!
class CSV_drawer : public CSV_Item {
	static constexpr int base_w = 1200;
	static constexpr int base_h = 600;
	static constexpr float screen_scale = 0.6f;
	// for better resolution on fonts and such:
	static constexpr int max_base_resolution_multiplier = 32;
	static constexpr int base_draw_w = base_w * max_base_resolution_multiplier;
	static constexpr int base_draw_h = base_h * max_base_resolution_multiplier;
	static constexpr int font_factor_16 = 16 * max_base_resolution_multiplier;
	static constexpr int font_factor_10 = 10 * max_base_resolution_multiplier;

	static std::atomic_uint64_t m_self_counter_auto_destroy;

	static ALLEGRO_FONT *m_font_16, *m_font_10;

	void _destroy_static_refs();
public:
	CSV_drawer(const CSV_drawer&) = delete;
	CSV_drawer(CSV_drawer&&) = delete;
	void operator=(const CSV_drawer&) = delete;
	void operator=(CSV_drawer&&) = delete;

	CSV_drawer();
	~CSV_drawer();

	void plot_to(const std::string& save_path, const uint16_t resolution = 16, float* tracker_progress = nullptr);
};
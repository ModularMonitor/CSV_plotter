#include "csv_drawer.h"
#include "resources.h"

ALLEGRO_FONT* CSV_drawer::m_font_16 = nullptr;
ALLEGRO_FONT* CSV_drawer::m_font_10 = nullptr;
std::atomic_uint64_t CSV_drawer::m_self_counter_auto_destroy{ 0 };


void CSV_drawer::_destroy_static_refs()
{
	if (const auto ptr = al_get_current_display(); ptr) 
		al_destroy_display(ptr);

	if (m_font_16) { al_destroy_font(m_font_16); m_font_16 = nullptr; }
	if (m_font_10) { al_destroy_font(m_font_10); m_font_10 = nullptr; }
}

CSV_drawer::CSV_drawer()
{
	++m_self_counter_auto_destroy;
}

CSV_drawer::~CSV_drawer()
{
	if (--m_self_counter_auto_destroy == 0) _destroy_static_refs();
}

struct tm utc_to_time(const uint64_t utc_t)
{
	struct tm res {};
	const time_t conv = static_cast<time_t>(utc_t / 1000ULL);
	localtime_s(&res, &conv);
	return res;
}

void CSV_drawer::plot_to(const std::string& save_path, const uint16_t resolution, float* tracker_progress)
{
	const size_t data_size = calc_data_size();
	const auto prog = [&tracker_progress] (const float pr) {
		if (tracker_progress) *tracker_progress = pr;
	};

	if (resolution == 0)
		throw std::invalid_argument("Resolution multiplier cannot be 0!");
	if (resolution > max_base_resolution_multiplier)
		throw std::invalid_argument("Resolution multiplier too high! Do not exceed "
			+ std::to_string(max_base_resolution_multiplier) + "!");

	if (data_size < 10)
		throw std::runtime_error("Data amount was less than 10! Cannot plot tiny or empty dataset!");

	prog(0.0f);

	if (!al_is_system_installed()) {
		al_init();
		al_init_image_addon();
		al_init_primitives_addon();
		al_init_font_addon();
		al_init_ttf_addon();
	}

	static Resources my_resources;
	if (!my_resources.get_pointer("IDR_FONT"))
		my_resources.load("IDR_FONT", "BIN");

	if (!m_font_16) {
		ALLEGRO_FILE* fmem = al_open_memfile(
			my_resources.get_pointer("IDR_FONT"),
			my_resources.get_size("IDR_FONT"),
			"r");

		m_font_16 = al_load_ttf_font_f(fmem, "font.ttf", font_factor_16, 0);
		// al_load_ttf_font_f eats the ALLEGRO_FILE* and destroys it by itself.
	}
	if (!m_font_10) {
		ALLEGRO_FILE* fmem = al_open_memfile(
			my_resources.get_pointer("IDR_FONT"),
			my_resources.get_size("IDR_FONT"),
			"r");

		m_font_10 = al_load_ttf_font_f(fmem, "font.ttf", font_factor_10, 0);
		// al_load_ttf_font_f eats the ALLEGRO_FILE* and destroys it by itself.
	}

	prog(0.03f);

	ALLEGRO_DISPLAY* m_disp = al_get_current_display();
	
	if (!m_disp) {
		al_set_new_display_flags(ALLEGRO_OPENGL);
		al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
		al_set_new_display_option(ALLEGRO_SAMPLES, 4, ALLEGRO_SUGGEST);

		ALLEGRO_MONITOR_INFO m_inf;
		al_get_monitor_info(0, &m_inf);

		const int mon_w = screen_scale * (m_inf.x2 - m_inf.x1);
		const int mon_h = mon_w * base_h / base_w;

		m_disp = al_create_display(mon_w, mon_h);
	}

	const int targ_w = base_w * resolution;
	const int targ_h = base_h * resolution;

	prog(0.06f);

	ALLEGRO_BITMAP* bmp = al_create_bitmap(targ_w, targ_h);

	prog(0.10f);

	al_set_target_bitmap(bmp);
	
	// make any resolution [0..base_*]
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_scale_transform(&t, targ_w * 1.0f / base_draw_w, targ_h * 1.0f / base_draw_h);
	al_use_transform(&t);

	al_clear_to_color(al_map_rgb(195,195,195));

	al_draw_filled_rectangle(
		0.0f,
		0.0f,
		base_draw_w,
		font_factor_16 * 1.4f + max_base_resolution_multiplier * 2.5f,
		al_map_rgb(160, 160, 160)
	);

	al_draw_textf(m_font_16, al_map_rgb(0, 0, 0),
		base_draw_w * 0.5f, max_base_resolution_multiplier * 2.5f, ALLEGRO_ALIGN_CENTER,
		u8"<---  Arquivo: \"%s\"  --->", save_path.c_str(), this->calc_data_size());

	prog(0.12f);

	constexpr int opt_len = 6;

	for (int opt = 0; opt < opt_len; ++opt) {
		prog(0.12f + opt * 0.08f / opt_len);

		const char* sel = nullptr;
		switch (opt) {
		case 0: sel = u8"Dados"; break;
		case 1: sel = u8"Mínimo"; break;
		case 2: sel = u8"Média"; break;
		case 3: sel = u8"Máximo"; break;
		case 4: sel = u8"Desvio"; break;
		case 5: sel = u8"Variância"; break;
		}

		const int offx = base_draw_w * (opt + 1) / (opt_len + 1);
		char buf_draw[64]{};

		switch (opt) {
		case 0: snprintf(buf_draw, sizeof(buf_draw), "%zu", data_size); break;
		case 1: snprintf(buf_draw, sizeof(buf_draw), "%.6lf", m_self_sum.min); break;
		case 2: snprintf(buf_draw, sizeof(buf_draw), "%.6lf", m_self_sum.avg); break;
		case 3: snprintf(buf_draw, sizeof(buf_draw), "%.6lf", m_self_sum.max); break;
		case 4: snprintf(buf_draw, sizeof(buf_draw), "%.6lf", m_self_sum.desvio_padrao); break;
		case 5: snprintf(buf_draw, sizeof(buf_draw), "%.6lf", m_self_sum.variancia_amostra); break;
		}

		al_draw_text(m_font_16, al_map_rgb(0, 0, 0),
			offx,
			font_factor_16 * 1.95f + max_base_resolution_multiplier * 1.5f,
			ALLEGRO_ALIGN_CENTER, sel);

		al_draw_text(m_font_16, al_map_rgb(0, 0, 0),
			offx,
			font_factor_16 * 3.3f + max_base_resolution_multiplier * 1.5f,
			ALLEGRO_ALIGN_CENTER, buf_draw);
	}

	al_draw_filled_rectangle(
		base_draw_w / (2 * opt_len + 1),
		font_factor_16 * 3.15f + max_base_resolution_multiplier * 1.5f,
		(base_draw_w * (2 * opt_len)) / (2 * opt_len + 1),
		font_factor_16 * 3.25f + max_base_resolution_multiplier * 1.5f,
		al_map_rgb(0, 0, 0)
	);

	prog(0.20f);

	// graph

	constexpr float begin_width_graph = max_base_resolution_multiplier * 70.0f;
	constexpr float begin_height_graph = base_draw_h * 0.14f;
	constexpr float end_width_graph = base_draw_w - max_base_resolution_multiplier * 30.0f;
	constexpr float end_height_graph = base_draw_h - max_base_resolution_multiplier * 60.0f;

	constexpr float max_width_graph = end_width_graph - begin_width_graph;
	constexpr float max_height_graph = end_height_graph - begin_height_graph;

	al_draw_filled_rectangle(
		begin_width_graph,
		begin_height_graph,
		end_width_graph,
		end_height_graph,
		al_map_rgb(235, 235, 235)
	);


	const long double values[] = {
		(abs(m_self_sum.max - m_self_sum.min) * 5.0L / 5) + m_self_sum.min, // 500 e 300 -> 500 - 300 + 300
		(abs(m_self_sum.max - m_self_sum.min) * 4.0L / 5) + m_self_sum.min,
		(abs(m_self_sum.max - m_self_sum.min) * 3.0L / 5) + m_self_sum.min,
		(abs(m_self_sum.max - m_self_sum.min) * 2.0L / 5) + m_self_sum.min,
		(abs(m_self_sum.max - m_self_sum.min) * 1.0L / 5) + m_self_sum.min,
		m_self_sum.min
	};

	const struct tm times[] = {
		utc_to_time(get_point(data_size * 0 / 5).m_time_pc),
		utc_to_time(get_point(data_size * 1 / 5).m_time_pc),
		utc_to_time(get_point(data_size * 2 / 5).m_time_pc),
		utc_to_time(get_point(data_size * 3 / 5).m_time_pc),
		utc_to_time(get_point(data_size * 4 / 5).m_time_pc),
		utc_to_time(get_point(data_size - 1).m_time_pc)
	};


	for (size_t p = 0; p < std::size(values); ++p) {
		const float py = begin_height_graph + max_height_graph * p / (std::size(values) - 1) - font_factor_10 * 0.5f;
		const float px = begin_width_graph - max_base_resolution_multiplier * 7.5f;

		const float rec_x1 = begin_width_graph;
		const float rec_x2 = end_width_graph;
		const float rec_y1 = begin_height_graph + max_height_graph * p / (std::size(values) - 1) - max_base_resolution_multiplier;
		const float rec_y2 = begin_height_graph + max_height_graph * p / (std::size(values) - 1) + max_base_resolution_multiplier;

		const auto& val = values[p];

		al_draw_textf(m_font_10, al_map_rgb(0, 0, 0),
			px,
			py + max_base_resolution_multiplier * 1.5f,
			ALLEGRO_ALIGN_RIGHT, "%.2lf %s", val, m_measurement.c_str());

		if (p == 0 || p == std::size(values)) continue;

		al_draw_filled_rectangle(
			rec_x1, rec_y1,
			rec_x2, rec_y2,
			al_map_rgb(200, 200, 200)
		);
	}



	// make any resolution [0..base_*] but vertical
	ALLEGRO_TRANSFORM vf{};
	al_identity_transform(&vf);
	al_scale_transform(&vf, targ_w * 1.0f / base_draw_w, targ_h * 1.0f / base_draw_h);
	al_rotate_transform(&vf, -ALLEGRO_PI * 0.5f); // rotated counter clock! x is y and x > 0 is up the screen. y is x, y > 0 is x > 0.
	al_use_transform(&vf);


	for (size_t p = 0; p < std::size(times); ++p) {
		const float py = begin_width_graph + max_width_graph * p / (std::size(times) - 1) - font_factor_10;
		const float px = -(end_height_graph + max_base_resolution_multiplier * 1.5f);

		const float rec_x1 = -end_height_graph;
		const float rec_x2 = -begin_height_graph;
		const float rec_y1 = begin_width_graph + max_width_graph * p / (std::size(times) - 1) - max_base_resolution_multiplier;
		const float rec_y2 = begin_width_graph + max_width_graph * p / (std::size(times) - 1) + max_base_resolution_multiplier;

		const auto& tn = times[p];

		al_draw_textf(m_font_10, al_map_rgb(0, 0, 0),
			px,
			py + max_base_resolution_multiplier * 1.5f,
			ALLEGRO_ALIGN_RIGHT, "%04i/%02i/%02i",
			tn.tm_year + 1900, tn.tm_mon + 1, tn.tm_mday);
		al_draw_textf(m_font_10, al_map_rgb(0, 0, 0),
			px,
			py + font_factor_10 + max_base_resolution_multiplier * 1.5f,
			ALLEGRO_ALIGN_RIGHT, "%02i:%02i:%02i",
			tn.tm_hour, tn.tm_min, tn.tm_sec);

		if (p == 0 || p == std::size(times)) continue;

		al_draw_filled_rectangle(
			rec_x1, rec_y1,
			rec_x2, rec_y2,
			al_map_rgb(200, 200, 200)
		);
	}

	const long double each_step_w = static_cast<long double>(max_width_graph) / static_cast<long double>(data_size - 1);

	const long double& min = m_self_sum.min;
	const long double& max = m_self_sum.max;

	const auto calc_height_graph = [&](const long double& val) {
		return static_cast<float>(((max - val) * 1.0L / (max - min)) * max_height_graph);
	};

	al_use_transform(&t);

	for (size_t p = 0; p < data_size - 1; ++p) {
		prog(0.20f + (p + 1) * 0.70f / data_size);

		const auto& it = { get_point(p), get_point(p + 1) };

		const float pos_x1 = begin_width_graph + static_cast<float>(each_step_w * static_cast<long double>(p));
		const float pos_x2 = begin_width_graph + static_cast<float>(each_step_w * static_cast<long double>(p + 1));
		const float pos_y1 = begin_height_graph + calc_height_graph(it.begin()->m_value);
		const float pos_y2 = begin_height_graph + calc_height_graph(std::next(it.begin(), 1)->m_value);

		al_draw_line(pos_x1, pos_y1, pos_x2, pos_y2, al_map_rgb(16,16,16), 2.0f * max_base_resolution_multiplier);
		
		if (p == 0)
			al_draw_filled_circle(pos_x1, pos_y1, 1.0f * max_base_resolution_multiplier, al_map_rgb(16, 16, 16));

		al_draw_filled_circle(pos_x2, pos_y2, 1.0f * max_base_resolution_multiplier, al_map_rgb(16, 16, 16));
	}



	prog(0.90f);

	// draw back to user
	al_set_target_backbuffer(m_disp);
	al_draw_scaled_bitmap(bmp, 0, 0, targ_w, targ_h, 0, 0, al_get_display_width(m_disp), al_get_display_height(m_disp), 0);

	prog(0.95f);

	// save
	if (!al_save_bitmap(save_path.c_str(), bmp))
		throw std::runtime_error("Cannot save png file at " + save_path);

	prog(1.00f);
	
	al_flip_display();

	al_destroy_bitmap(bmp);
	//while (1);
}
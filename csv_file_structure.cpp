#include "csv_file_structure.h"

#include <stdexcept>
#include <fstream>

bool CSV_Item::each_block::has_data() const
{
	return m_data_len > 0;
}

bool CSV_Item::each_block::push_get_can_push_more(each&& e)
{
	if (m_data_len >= each_amount_slice) 
		throw std::runtime_error("Tried to push when cannot do it anymore. Failed on code.");

	m_data[m_data_len++] = std::move(e);
	if (m_data_len >= each_amount_slice) return false;
	return true;
}

void CSV_Item::each_block::calculate()
{
	if (!has_data()) throw std::runtime_error("Empty dataset!");

	max = min = m_data[0].m_value;
	avg = 0.0;
	variancia_amostra = 0.0;

	for (size_t p = 0; p < m_data_len; ++p) {
		const auto& it = m_data[p];

		// avg part: GOOD
		avg += it.m_value;

		// min/max part: GOOD
		if (it.m_value > max) max = it.m_value;
		if (it.m_value < min) min = it.m_value;
	}
	// from now on: MIN & MAX done!

	// avg DONE!
	avg /= static_cast<long double>(m_data_len);

	// desvio padrao part
	for (size_t p = 0; p < m_data_len; ++p) {
		const auto& it = m_data[p];

		const long double diff = (it.m_value - avg);
		const long double sq_div = (diff * diff) / static_cast<long double>(m_data_len);

		variancia_amostra += sq_div;
	}
	// desvio padrao DONE
	desvio_padrao = sqrtl(variancia_amostra);

	// variancia_amostra is DONE already too!
}

CSV_Item::each& CSV_Item::get_point(const size_t v) const
{
	const size_t total_len = calc_data_size();
	if (v >= total_len) throw std::out_of_range("Value was out of range (was " + std::to_string(v) + ", size was " + std::to_string(total_len) + ")");

	const size_t point_off = v / each_amount_slice;
	const size_t index = v % each_amount_slice;

	return m_data[point_off].m_data[index];
}

size_t CSV_Item::calc_data_size() const
{
	size_t sum = 0;
	for (const auto& i : m_data) sum += i.m_data_len;
	return sum;
}

size_t CSV_Item::load_from(const std::string& file_name)
{
	std::ifstream in(file_name, std::ios::binary);

	if (!in || in.bad()) 
		throw std::invalid_argument("Path is invalid, cannot open file " + file_name);

	m_data.clear();
	m_data.push_back(each_block()); // has at least one
	m_self_sum = {};

	size_t skips = 0;

	while (!in.eof()) {
		std::string buf;
		std::getline(in, buf);

		each one;

		const auto got = sscanf_s(buf.c_str(), "%llu;%llu;%lf", &one.m_time_device, &one.m_time_pc, &one.m_value);
		if (got != 3) {
			continue;
		}

		if (!m_data.back().push_get_can_push_more(std::move(one)))
			m_data.push_back(each_block());
	}

	// if added one, but did not use, remove it.
	if (!m_data.back().has_data()) m_data.pop_back();

	if (m_data.size() == 0)
		throw std::invalid_argument("This file generated no data to work with!");

	// CALCULATING FULL DATA AVERAGES AND STUFF:

	for (auto& i : m_data) i.calculate();

	m_self_sum.min = m_data[0].min;
	m_self_sum.max = m_data[0].max;
	m_self_sum.avg = 0.0;
	m_self_sum.variancia_amostra = 0.0;

	for (const auto& i : m_data) {

		m_self_sum.avg += i.avg;
		m_self_sum.variancia_amostra += i.variancia_amostra;

		if (i.max > m_self_sum.max) m_self_sum.max = i.max;
		if (i.min < m_self_sum.min) m_self_sum.min = i.min;
	}

	m_self_sum.avg /= static_cast<long double>(m_data.size());
	m_self_sum.variancia_amostra /= static_cast<long double>(m_data.size());

	// desvio padrao DONE
	m_self_sum.desvio_padrao = sqrtl(m_self_sum.variancia_amostra);

	return skips - 1; // if -1, did not read a thing!
}

const CSV_Item::math_props& CSV_Item::get_calcd() const
{
	return m_self_sum;
}

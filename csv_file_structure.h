#pragma once

#include <vector>
#include <chrono>
#include <string>
#include <memory>

class CSV_Item {
protected:
	static constexpr size_t each_amount_slice = 1 << 10;
public:
	struct each {
		uint64_t m_time_pc{};
		uint64_t m_time_device{};
		long double m_value{};
	};
	struct math_props {
		long double min{}, max{}, avg{}, desvio_padrao{}, variancia_amostra{};
	};
	struct each_block : public math_props {
		std::unique_ptr<each[]> m_data = std::unique_ptr<each[]>(new each[each_amount_slice]);
		size_t m_data_len = 0;

		bool has_data() const;
		bool push_get_can_push_more(each&&);
		void calculate();
	};
protected:
	std::vector<each_block> m_data;
	math_props m_self_sum{};
	std::string m_measurement;

	void average_all(const int depth);
	each& get_point(const size_t) const;
	size_t calc_data_size() const;
public:
	// Load csv pre-formatted file. Cleans up first. Returns lines failed or (-1) if no lines read.
	size_t load_from(const std::string& file_name);

	const math_props& get_calcd() const;
};
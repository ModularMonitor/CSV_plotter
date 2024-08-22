#include <iostream>
#include <string>
#include <filesystem>
#include <thread>

#include "csv_drawer.h"

int main()
{
	CSV_drawer item;

	std::cout << "Reading CSV files under folder '/csv'..." << std::endl;

	for (const auto& el : std::filesystem::directory_iterator{ ".\\csv" }) {
		std::cout << "# " << el.path() << std::endl;

		const auto targ_path = el.path().string();
		const auto format_test = targ_path.substr(targ_path.size() - 4);
		if (!el.is_regular_file() || strncmp(".csv", format_test.c_str(), 4) != 0) continue;

		item.load_from(targ_path);

		float track = 0.0f;

		std::thread thr([&track] {
			while (track < 1.0f) {
				std::cout << static_cast<int>(100.0f * track) << "%  \r";
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
			std::cout << static_cast<int>(100.0f * track) << "%  \r";
		});

		try {
			item.plot_to(targ_path + "_.png", 16, &track);
		}
		catch (const std::exception& e) {
			std::cout << "Failed on " << targ_path << "'s plot because of exception: " << e.what() << std::endl;
			track = 2.0f;
		}
		thr.join();

		if (track == 1.0f) {
			std::cout << "Done: " << targ_path << ". Resting for 2 seconds and doing next..." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}
}
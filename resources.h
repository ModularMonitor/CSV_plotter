#pragma once

#include <Windows.h>
#include <vector>
#include <string>

class Resources {
	struct _ {
		std::string id;
		HRSRC myResource = nullptr;
		DWORD myResourceSize = 0;
		HGLOBAL myResourceData = nullptr;
		void* pMyBinaryData = nullptr;


		_(const _&) = delete;
		void operator=(const _&) = delete;
		_(_&& o) noexcept;
		_(const char* res_name, const char* res_type);
		~_();
	};

	std::vector<_> m_mem;
public:
	// exits on fail (app)
	void load(const char* res_name, const char* res_type);

	char* get_pointer(const char* res_name);
	size_t get_size(const char* res_name);
};

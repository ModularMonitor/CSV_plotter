// new code to make exe containing resource files easy

#include "resources.h"


Resources::_::_(_&& o) noexcept
	: id(std::move(o.id)), myResource(o.myResource), myResourceSize(o.myResourceSize), myResourceData(o.myResourceData), pMyBinaryData(o.pMyBinaryData)
{
	o.myResource = nullptr;
	o.myResourceSize = 0;
	o.myResourceData = nullptr;
	o.pMyBinaryData = nullptr;
}

Resources::_::_(const char* res_name, const char* res_type)
	: id(res_name)
{
	if (!(myResource = ::FindResourceA(GetModuleHandleA(NULL), res_name, res_type))) exit(1);
	myResourceSize = ::SizeofResource(NULL, myResource); // bytes
	if (!(myResourceData = ::LoadResource(NULL, myResource))) exit(2);
	pMyBinaryData = ::LockResource(myResourceData);
}

Resources::_::~_() {
	if (myResourceData) UnlockResource(myResourceData);
	myResourceData = nullptr;
}


void Resources::load(const char* res_name, const char* res_type)
{
	m_mem.push_back(_(res_name, res_type));
}

char* Resources::get_pointer(const char* res_name)
{
	for (size_t p = 0; p < m_mem.size(); ++p) {
		if (m_mem[p].id == res_name) return (char*)m_mem[p].pMyBinaryData;
	}
	return nullptr;
}

size_t Resources::get_size(const char* res_name)
{
	for (size_t p = 0; p < m_mem.size(); ++p) {
		if (m_mem[p].id == res_name) return (size_t)m_mem[p].myResourceSize;
	}
	return 0;
}



//static bool __extract_resource_file_at(const char* resource_name, const char* type_name, const char* target_path)
//{
//	HRSRC myResource = ::FindResourceA(GetModuleHandleA(NULL), resource_name, type_name);
//	if (!myResource) return false;
//
//	DWORD myResourceSize = ::SizeofResource(NULL, myResource); // bytes
//	HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
//	if (!myResourceData) return false;
//
//	void* pMyBinaryData = ::LockResource(myResourceData);
//
//	std::fstream out(target_path, std::ios::out, std::ios::binary);
//	if (!out || out.bad()) {
//		UnlockResource(myResourceData);
//		return false;
//	}
//
//	out.write((char*)pMyBinaryData, myResourceSize);
//	out.close();
//
//	UnlockResource(myResourceData);
//	return true;
//}
//
//void __extract_resources_auto()
//{
//	std::filesystem::create_directories("data");
//	if (!__extract_resource_file_at("IDR_DATA_BLOCK",  "BIN", "data/block.png"))   exit(1);
//	if (!__extract_resource_file_at("IDR_DATA_PLAYER", "BIN", "data/player.png"))  exit(2);
//	if (!__extract_resource_file_at("IDR_DATA_MUSIC",  "BIN", "data/music.mp3"))   exit(3);
//
//}
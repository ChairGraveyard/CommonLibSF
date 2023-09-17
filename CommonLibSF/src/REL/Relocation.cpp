#include "REL/Relocation.h"

namespace REL
{
	Module::Module(const std::uintptr_t a_base)
	{
		stl_assert(a_base, "failed to initializing module info with null module base");

		_base = a_base;

		const auto  dosHeader = reinterpret_cast<const WinAPI::IMAGE_DOS_HEADER*>(_base);
		const auto  ntHeader = stl::adjust_pointer<WinAPI::IMAGE_NT_HEADERS64>(dosHeader, dosHeader->lfanew);
		const auto* sections = WinAPI::IMAGE_FIRST_SECTION(ntHeader);
		const auto  size = std::min<std::size_t>(ntHeader->fileHeader.sectionCount, _segments.size());

		for (std::size_t i = 0; i < size; ++i) {
			const auto& section = sections[i];
			if (const auto it = std::ranges::find_if(SEGMENTS, [&](auto&& a_elem) {
					constexpr auto size_s = std::extent_v<decltype(section.name)>;
					const auto     len = std::min(a_elem.first.size(), size_s);
					return std::memcmp(a_elem.first.data(), section.name, len) == 0 && (section.characteristics & a_elem.second) == a_elem.second;
				});
				it != SEGMENTS.end()) {
				const auto idx = static_cast<std::size_t>(std::distance(SEGMENTS.begin(), it));
				_segments[idx] = Segment{ _base, _base + section.virtualAddress, section.virtualSize };
			}
		}
	}

	Module::Module(const std::string_view a_filePath)
	{
		const auto base = AsAddress(WinAPI::GetModuleHandle(a_filePath.data())) & ~3;

		stl_assert(base, "failed to initializing module info with file {}", a_filePath);

		*this = Module(base);
	}
}  // namespace REL

#include "memory.h"

#include <thread>
#include <array>

namespace offset
{
	// client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA964;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFFC4;
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;

	// player
	constexpr ::std::ptrdiff_t m_hMyWeapons = 0x2E08;

	// base attributable
	constexpr ::std::ptrdiff_t m_flFallbackWear = 0x31E0;
	constexpr ::std::ptrdiff_t m_nFallbackPaintKit = 0x31D8;
	constexpr ::std::ptrdiff_t m_iItemDefinitionIndex = 0x2FBA;
	constexpr ::std::ptrdiff_t m_iItemIDHigh = 0x2FD0;
	constexpr ::std::ptrdiff_t m_iEntityQuality = 0x2FBC;
}

// set skins to apply here
constexpr const int GetWeaponPaint(const short& itemDefiniton)
{
	switch (itemDefiniton)
	{
	case 1: return 711; // deagle
	case 4: return 38; // glock
	case 7: return 490; // ak
	case 9: return 344; // awp
	case 61: return 653; // usp
	default: return 0;
	}
}

int main()
{
	const auto memory = Memory{ "csgo.exe" };

	// get our module addresses
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	// cheat loop
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		const auto& localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		const auto& weapons = memory.Read<std::array<unsigned long, 8>>(localPlayer + offset::m_hMyWeapons);

		// local player weapon iteration
		for (const auto& handle : weapons)
		{
			const auto& weapon = memory.Read<std::uintptr_t>((client + offset::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

			// make sure weapon is valid
			if (!weapon)
				continue;

			// see if we want to apply a skin
			if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + offset::m_iItemDefinitionIndex)))
			{
				const bool shouldUpdate = memory.Read<std::int32_t>(weapon + offset::m_nFallbackPaintKit) != paint;

				// force weapon to use fallback values
				memory.Write<std::int32_t>(weapon + offset::m_iItemIDHigh, -1);

				memory.Write<std::int32_t>(weapon + offset::m_nFallbackPaintKit, paint);
				memory.Write<float>(weapon + offset::m_flFallbackWear, 0.1f);

				if (shouldUpdate)
					memory.Write<std::int32_t>(memory.Read<std::uintptr_t>(engine + offset::dwClientState) + 0x174, -1);
			}
		}
	}
	
	return 0;
}
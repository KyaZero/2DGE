#pragma once
#include "Core/Types.h"

namespace fw
{
    constexpr static u32 CURRENT_FRAME = 0;
    constexpr static u32 NEXT_FRAME = 1;
    constexpr static u32 FRAME_COUNT = 2;

    const static auto TEMP_DIRECTORY = std::filesystem::temp_directory_path() / "2DGE/";
}  // namespace fw
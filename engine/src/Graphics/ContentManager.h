#pragma once

#include <filesystem>

#include "Core/Singleton.h"
#include "Core/Types.h"

namespace fw
{
    using IDType = u64;
    static constexpr IDType NullId = (IDType)-1;

    class ContentManager : public Singleton<ContentManager>
    {
    public:
        IDType Register(const std::string& path);
        std::optional<std::filesystem::path> GetPath(IDType id);

    private:
        std::unordered_map<IDType, std::filesystem::path> m_Paths;
    };
}  // namespace fw
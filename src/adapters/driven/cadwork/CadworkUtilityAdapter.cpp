#include "CadworkUtilityAdapter.h"

#include <cwapi3d/ICwAPI3DString.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <cctype>
#include <exception>
#include <string_view>
#include <utility>

namespace cw_api3d::adapters::driven::cadwork
{

  namespace
  {

    [[nodiscard]] std::string_view trimWhitespace(std::string_view text) noexcept
    {
      constexpr auto isSpace = [](const char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
      };
      while (!text.empty() && isSpace(text.front()))
      {
        text.remove_prefix(1);
      }
      while (!text.empty() && isSpace(text.back()))
      {
        text.remove_suffix(1);
      }
      return text;
    }

  } // namespace

  CadworkUtilityAdapter::CadworkUtilityAdapter(
    CwAPI3D::Interfaces::ICwAPI3DUtilityController* utilityController,
    ports::interfaces::LoggerPtr logger) noexcept
    : mUtilityController(utilityController)
    , mLogger(std::move(logger))
  {
  }

  std::optional<std::filesystem::path> CadworkUtilityAdapter::getPluginPath() const noexcept
  {
    if (!mUtilityController)
    {
      if (mLogger)
      {
        mLogger->warn("CadworkUtilityAdapter: ICwAPI3DUtilityController is null");
      }
      return std::nullopt;
    }

    try
    {
      auto* cwStr = mUtilityController->getPluginPath();
      if (!cwStr)
      {
        if (mLogger)
        {
          mLogger->warn("CadworkUtilityAdapter: getPluginPath() returned null pointer");
        }
        return std::nullopt;
      }

      // Invariant: NEVER call cwStr->destroy() — the ICwAPI3DString is host-owned and
      // destroying it corrupts the heap. Read the narrow data and leave memory management to the host.
      const auto* narrow = cwStr->narrowData();
      if (!narrow)
      {
        if (mLogger)
        {
          mLogger->warn("CadworkUtilityAdapter: narrowData() returned null pointer");
        }
        return std::nullopt;
      }

      const auto trimmed = trimWhitespace(std::string_view(narrow));
      if (trimmed.empty())
      {
        if (mLogger)
        {
          mLogger->warn("CadworkUtilityAdapter: getPluginPath() returned empty string");
        }
        return std::nullopt;
      }

      return std::filesystem::path(trimmed);
    }
    catch (const std::exception& e)
    {
      if (mLogger)
      {
        mLogger->errorf("CadworkUtilityAdapter: Exception querying plugin path: {}", e.what());
      }
      return std::nullopt;
    }
    catch (...)
    {
      if (mLogger)
      {
        mLogger->error("CadworkUtilityAdapter: Unknown exception querying plugin path");
      }
      return std::nullopt;
    }
  }

} // namespace cw_api3d::adapters::driven::cadwork

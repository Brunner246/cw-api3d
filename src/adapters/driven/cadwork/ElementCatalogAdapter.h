#pragma once

#include "src/adapters/driven/cadwork/HostContracts.h"
#include "src/application/ElementSnapshot.h"
#include "src/ports/ElementCatalog.h"
#include "src/ports/Logger.h"

#include <cmath>
#include <cstdint>
#include <exception>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cw_api3d::adapters::driven::cadwork
{

  namespace detail
  {

    inline void appendUtf8CodePoint(std::string& out, const char32_t codePoint)
    {
      if (codePoint < 0x80)
      {
        out.push_back(static_cast<char>(codePoint));
        return;
      }
      if (codePoint < 0x800)
      {
        out.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        return;
      }
      if (codePoint < 0x10000)
      {
        out.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        return;
      }
      out.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
      out.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }

    [[nodiscard]] inline std::string utf8FromWide(const wchar_t* wide)
    {
      if (wide == nullptr)
      {
        return {};
      }

      std::string utf8;
      while (*wide != L'\0')
      {
        auto unit = static_cast<char32_t>(*wide++);
        if (unit >= 0xD800 && unit <= 0xDBFF)
        {
          if (*wide == L'\0')
          {
            break;
          }
          const auto low = static_cast<char32_t>(*wide);
          if (low < 0xDC00 || low > 0xDFFF)
          {
            continue;
          }
          ++wide;
          unit = 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
        }
        else if (unit >= 0xDC00 && unit <= 0xDFFF)
        {
          continue;
        }
        appendUtf8CodePoint(utf8, unit);
      }
      return utf8;
    }

    [[nodiscard]] inline std::string copyHostString(auto* hostString)
    {
      if (!hostString)
      {
        return {};
      }
      // narrowData() is the host ACP, not UTF-8. Domain strings and Qt 6 fromStdString are UTF-8.
      const auto* wide = hostString->data();
      if (!wide)
      {
        return {};
      }
      return std::string{trimWhitespace(utf8FromWide(wide))};
    }

    [[nodiscard]] inline std::optional<double> finiteOrNull(const double value) noexcept
    {
      if (!std::isfinite(value))
      {
        return std::nullopt;
      }
      return value;
    }

    template<concepts::HostElementType Type>
    [[nodiscard]] application::ElementKind mapElementKind(Type& type) noexcept
    {
      if (type.isFramedWall())
      {
        return application::ElementKind::FramedWall;
      }
      if (type.isSolidWoodWall())
      {
        return application::ElementKind::SolidWoodWall;
      }
      if (type.isLogWall())
      {
        return application::ElementKind::LogWall;
      }
      if (type.isWall())
      {
        return application::ElementKind::Wall;
      }
      if (type.isOpening())
      {
        return application::ElementKind::Opening;
      }
      if (type.isPanel())
      {
        return application::ElementKind::Panel;
      }
      if (type.isRectangularBeam())
      {
        return application::ElementKind::RectangularBeam;
      }
      if (type.isCircularBeam())
      {
        return application::ElementKind::CircularBeam;
      }
      return application::ElementKind::Other;
    }

  } // namespace detail

  template<concepts::ElementCatalogSource Host>
  class ElementCatalogAdapter final : public ports::IElementCatalog
  {
  public:
    explicit ElementCatalogAdapter(
      Host* host = nullptr,
      ports::LoggerPtr logger = nullptr) noexcept
      : mHost(host)
      , mLogger(std::move(logger))
    {
    }

    ~ElementCatalogAdapter() override = default;

    [[nodiscard]] std::expected<application::ElementSnapshot, std::string> fetch(
      const application::ElementUniverse universe) const override
    {
      if (!mHost)
      {
        constexpr std::string_view err = "ElementCatalogAdapter: host catalog is null";
        warn(err);
        return std::unexpected(std::string{err});
      }

      try
      {
        auto* ids = universe == application::ElementUniverse::Active
                      ? mHost->getActiveIdentifiableElementIDs()
                      : mHost->getAllIdentifiableElementIDs();
        if (!ids)
        {
          constexpr std::string_view err = "ElementCatalogAdapter: host returned null ID list";
          warn(err);
          return std::unexpected(std::string{err});
        }

        application::ElementSnapshot snapshot;
        const auto count = ids->count();
        snapshot.records.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i)
        {
          snapshot.records.push_back(readRecord(ids->at(i)));
        }
        return snapshot;
      }
      catch (const std::exception& e)
      {
        if (mLogger)
        {
          mLogger->errorf("ElementCatalogAdapter: Exception fetching snapshot: {}", e.what());
        }
        return std::unexpected(std::string{e.what()});
      }
      catch (...)
      {
        constexpr std::string_view err = "ElementCatalogAdapter: Unknown exception fetching snapshot";
        if (mLogger)
        {
          mLogger->error(err);
        }
        return std::unexpected(std::string{err});
      }
    }

  private:
    [[nodiscard]] application::ElementRecord readRecord(const application::ElementId id) const
    {
      application::ElementRecord record{.id = id};
      record.name = detail::copyHostString(mHost->getName(id));
      record.material = detail::copyHostString(mHost->getElementMaterialName(id));
      if (auto* type = mHost->getElementType(id))
      {
        record.kind = detail::mapElementKind(*type);
      }
      record.kindLabel = std::string{application::toString(record.kind)};
      record.length = detail::finiteOrNull(mHost->getLength(id));
      record.width = detail::finiteOrNull(mHost->getWidth(id));
      record.height = detail::finiteOrNull(mHost->getHeight(id));
      record.volume = detail::finiteOrNull(mHost->getVolume(id));
      return record;
    }

    void warn(const std::string_view message) const noexcept
    {
      if (mLogger)
      {
        mLogger->warn(message);
      }
    }

    Host* mHost{nullptr};
    ports::LoggerPtr mLogger;
  };

} // namespace cw_api3d::adapters::driven::cadwork

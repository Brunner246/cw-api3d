#pragma once

#include "src/application/ElementSnapshot.h"
#include "src/ports/ElementCatalog.h"
#include "src/ports/Logger.h"

#include <cmath>
#include <concepts>
#include <cstdint>
#include <exception>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace cw_api3d::adapters::driven::cadwork
{

  namespace concepts
  {

    template<typename T>
    concept HostString = requires(T& hostString) {
      { hostString.narrowData() } -> std::convertible_to<const char*>;
    };

    template<typename T>
    concept HostStringPointer = std::is_pointer_v<T> && HostString<std::remove_pointer_t<T>>;

    template<typename T>
    concept HostIdList = requires(T& list, std::uint32_t index) {
      { list.count() } -> std::convertible_to<std::uint32_t>;
      { list.at(index) } -> std::convertible_to<application::ElementId>;
    };

    template<typename T>
    concept HostIdListPointer = std::is_pointer_v<T> && HostIdList<std::remove_pointer_t<T>>;

    template<typename T>
    concept HostElementType = requires(T& type) {
      { type.isFramedWall() } -> std::convertible_to<bool>;
      { type.isSolidWoodWall() } -> std::convertible_to<bool>;
      { type.isLogWall() } -> std::convertible_to<bool>;
      { type.isWall() } -> std::convertible_to<bool>;
      { type.isRectangularBeam() } -> std::convertible_to<bool>;
      { type.isCircularBeam() } -> std::convertible_to<bool>;
      { type.isPanel() } -> std::convertible_to<bool>;
      { type.isOpening() } -> std::convertible_to<bool>;
    };

    template<typename T>
    concept HostElementTypePointer = std::is_pointer_v<T> && HostElementType<std::remove_pointer_t<T>>;

    template<typename T>
    concept ElementCatalogSource = requires(T& host, application::ElementId id) {
      { host.getActiveIdentifiableElementIDs() } -> HostIdListPointer;
      { host.getAllIdentifiableElementIDs() } -> HostIdListPointer;
      { host.getName(id) } -> HostStringPointer;
      { host.getElementMaterialName(id) } -> HostStringPointer;
      { host.getElementTypeDescription(id) } -> HostStringPointer;
      { host.getElementType(id) } -> HostElementTypePointer;
      { host.isBeam(id) } -> std::convertible_to<bool>;
      { host.getLength(id) } -> std::convertible_to<double>;
      { host.getWidth(id) } -> std::convertible_to<double>;
      { host.getHeight(id) } -> std::convertible_to<double>;
      { host.getVolume(id) } -> std::convertible_to<double>;
    };

  } // namespace concepts

  namespace detail
  {

    [[nodiscard]] constexpr std::string_view trimWhitespace(std::string_view text) noexcept
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

    [[nodiscard]] inline std::string copyHostString(auto* hostString)
    {
      if (!hostString)
      {
        return {};
      }
      const auto* narrow = hostString->narrowData();
      if (!narrow)
      {
        return {};
      }
      return std::string{trimWhitespace(std::string_view(narrow))};
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
    [[nodiscard]] application::ElementKind mapElementKind(Type& type, const bool isBeam) noexcept
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
      if (type.isRectangularBeam())
      {
        return application::ElementKind::RectangularBeam;
      }
      if (type.isCircularBeam())
      {
        return application::ElementKind::CircularBeam;
      }
      if (isBeam)
      {
        return application::ElementKind::Beam;
      }
      if (type.isPanel())
      {
        return application::ElementKind::Panel;
      }
      if (type.isOpening())
      {
        return application::ElementKind::Opening;
      }
      return application::ElementKind::Other;
    }

  } // namespace detail

  template<concepts::ElementCatalogSource Host>
  class ElementCatalogAdapter final : public ports::interfaces::IElementCatalog
  {
  public:
    explicit ElementCatalogAdapter(
      Host* host = nullptr,
      ports::interfaces::LoggerPtr logger = nullptr) noexcept
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
      record.kindLabel = detail::copyHostString(mHost->getElementTypeDescription(id));
      if (auto* type = mHost->getElementType(id))
      {
        record.kind = detail::mapElementKind(*type, mHost->isBeam(id));
      }
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
    ports::interfaces::LoggerPtr mLogger;
  };

} // namespace cw_api3d::adapters::driven::cadwork

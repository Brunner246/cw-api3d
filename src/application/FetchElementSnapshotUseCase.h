#pragma once

#include "src/application/ElementSnapshot.h"
#include "src/ports/ElementCatalog.h"
#include "src/ports/Logger.h"

#include <expected>
#include <format>
#include <memory>
#include <string>
#include <utility>

namespace cw_api3d::application
{

  struct IFetchElementSnapshotUseCase
  {
    virtual ~IFetchElementSnapshotUseCase() = default;

    [[nodiscard]] virtual std::expected<ElementSnapshot, std::string> execute(ElementUniverse universe) = 0;
  };

  using FetchElementSnapshotUseCasePtr = std::shared_ptr<IFetchElementSnapshotUseCase>;

  class FetchElementSnapshotUseCase : public IFetchElementSnapshotUseCase
  {
  public:
    explicit FetchElementSnapshotUseCase(
      ports::interfaces::IElementCatalog& catalog,
      ports::interfaces::ILogger& logger) noexcept
      : mCatalog(&catalog)
      , mLogger(&logger)
    {
    }

    explicit FetchElementSnapshotUseCase(
      ports::interfaces::ElementCatalogPtr catalog,
      ports::interfaces::LoggerPtr logger) noexcept
      : mCatalogPtr(std::move(catalog))
      , mLoggerPtr(std::move(logger))
      , mCatalog(mCatalogPtr.get())
      , mLogger(mLoggerPtr.get())
    {
    }

    [[nodiscard]] std::expected<ElementSnapshot, std::string> execute(const ElementUniverse universe) override
    {
      if (!mCatalog || !mLogger)
      {
        return std::unexpected("Catalog or logger dependency is uninitialized");
      }

      mLogger->trace("Fetching element snapshot from catalog...");

      auto result = mCatalog->fetch(universe);
      if (!result.has_value())
      {
        mLogger->warn(result.error());
        return result;
      }

      mLogger->info(std::format("Retrieved {} elements for universe {}", result->records.size(), toString(universe)));
      return result;
    }

  private:
    ports::interfaces::ElementCatalogPtr mCatalogPtr;
    ports::interfaces::LoggerPtr mLoggerPtr;
    ports::interfaces::IElementCatalog* mCatalog{nullptr};
    ports::interfaces::ILogger* mLogger{nullptr};
  };

} // namespace cw_api3d::application

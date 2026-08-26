#pragma once

#include "src/application/ElementSnapshot.h"
#include "src/ports/ElementActivation.h"
#include "src/ports/Logger.h"

#include <expected>
#include <format>
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace cw_api3d::application
{

  struct IActivateElementsUseCase
  {
    virtual ~IActivateElementsUseCase() = default;

    [[nodiscard]] virtual std::expected<void, std::string> execute(std::span<const ElementId> elementIds) = 0;
  };

  using ActivateElementsUseCasePtr = std::shared_ptr<IActivateElementsUseCase>;

  class ActivateElementsUseCase : public IActivateElementsUseCase
  {
  public:
    explicit ActivateElementsUseCase(
      ports::IElementActivation& activation,
      ports::ILogger& logger) noexcept
      : mActivation(&activation)
      , mLogger(&logger)
    {
    }

    explicit ActivateElementsUseCase(
      ports::ElementActivationPtr activation,
      ports::LoggerPtr logger) noexcept
      : mActivationPtr(std::move(activation))
      , mLoggerPtr(std::move(logger))
      , mActivation(mActivationPtr.get())
      , mLogger(mLoggerPtr.get())
    {
    }

    [[nodiscard]] std::expected<void, std::string> execute(const std::span<const ElementId> elementIds) override
    {
      if (!mActivation || !mLogger)
      {
        return std::unexpected("Activation or logger dependency is uninitialized");
      }

      if (elementIds.empty())
      {
        mLogger->info("Activating 0 elements");
        return {};
      }

      auto result = mActivation->activate(elementIds);
      if (!result.has_value())
      {
        mLogger->warn(result.error());
        return result;
      }

      mLogger->info(std::format("Activating {} elements", elementIds.size()));
      return result;
    }

  private:
    ports::ElementActivationPtr mActivationPtr;
    ports::LoggerPtr mLoggerPtr;
    ports::IElementActivation* mActivation{nullptr};
    ports::ILogger* mLogger{nullptr};
  };

} // namespace cw_api3d::application

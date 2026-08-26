#include "src/composition/Bootstrapping.h"

#include "src/adapters/driven/cadwork/CadworkUtilityAdapter.h"
#include "src/adapters/driven/logging/SpdLogLogger.h"
#include <cwapi3d/ICwAPI3DControllerFactory.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <exception>
#include <utility>

namespace cw_api3d::composition
{
  namespace
  {
    [[nodiscard]] constexpr ports::LogLevel defaultLogLevel() noexcept
    {
#if defined(CW_BUILD_RELWITHDEBINFO)
      return ports::LogLevel::Debug;
#elif defined(CW_BUILD_RELEASE)
      return ports::LogLevel::Info;
#elif defined(CW_BUILD_DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
      return ports::LogLevel::Trace;
#else
      return ports::LogLevel::Info;
#endif
    }

    [[nodiscard]] std::shared_ptr<adapters::driven::logging::SpdLogLogger> makeProductionLogger()
    {
      auto logger = std::make_shared<adapters::driven::logging::SpdLogLogger>();
      logger->setLevel(defaultLogLevel());
      return logger;
    }

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DUtilityController* resolveUtilityController(
      CwAPI3D::ControllerFactory* factory,
      ports::ILogger& logger) noexcept
    {
      if (factory == nullptr)
      {
        logger.warn("Null CwAPI3D ControllerFactory provided during bootstrapping");
        return nullptr;
      }

      try
      {
        return factory->getUtilityController();
      }
      catch (const std::exception& ex)
      {
        logger.errorf("Failed to retrieve utility controller from factory: {}", ex.what());
      }
      catch (...)
      {
        logger.error("Unknown exception while retrieving utility controller from factory");
      }
      return nullptr;
    }
  } // namespace

  PluginBootstrapper::PluginBootstrapper(
    ports::UtilityProviderPtr utilityProvider,
    ports::LoggerPtr logger) noexcept
    : mUtilityProvider(std::move(utilityProvider))
    , mLogger(std::move(logger))
    , mUseCase(mUtilityProvider, mLogger)
  {
  }

  std::unique_ptr<PluginBootstrapper> PluginBootstrapper::createProduction(
    CwAPI3D::ControllerFactory* factory) noexcept
  {
    try
    {
      auto logger = makeProductionLogger();

      auto utilityAdapter = std::make_shared<adapters::driven::cadwork::CadworkUtilityAdapter>(
        resolveUtilityController(factory, *logger),
        logger);

      return std::make_unique<PluginBootstrapper>(std::move(utilityAdapter), std::move(logger));
    }
    catch (...)
    {
      return nullptr;
    }
  }

  std::unique_ptr<PluginBootstrapper> PluginBootstrapper::createProductionForUtilityProvider(
    ports::UtilityProviderPtr utilityProvider) noexcept
  {
    try
    {
      return std::make_unique<PluginBootstrapper>(std::move(utilityProvider), makeProductionLogger());
    }
    catch (...)
    {
      return nullptr;
    }
  }

  std::expected<std::filesystem::path, std::string> PluginBootstrapper::run() noexcept
  {
    try
    {
      if (!mLogger)
      {
        return std::unexpected("Logger is not initialized in bootstrapper");
      }
      if (!mUtilityProvider)
      {
        mLogger->error("Utility provider is not initialized in bootstrapper");
        return std::unexpected("Utility provider is not initialized in bootstrapper");
      }

      return mUseCase.execute();
    }
    catch (const std::exception& ex)
    {
      if (mLogger)
      {
        mLogger->errorf("Unhandled exception during use case execution: {}", ex.what());
      }
      return std::unexpected(std::string("Exception: ") + ex.what());
    }
    catch (...)
    {
      if (mLogger)
      {
        mLogger->error("Unknown unhandled exception during use case execution");
      }
      return std::unexpected("Unknown exception during use case execution");
    }
  }

  bool bootstrapPlugin(CwAPI3D::ControllerFactory* factory) noexcept
  {
    try
    {
      const auto bootstrapper = PluginBootstrapper::createProduction(factory);
      if (!bootstrapper)
      {
        return false;
      }

      const auto result = bootstrapper->run();
      return result.has_value();
    }
    catch (...)
    {
      return false;
    }
  }
} // namespace cw_api3d::composition

#include "src/composition/Bootstrapping.h"

#include "src/adapters/driven/cadwork/CadworkUtilityAdapter.h"
#include "src/adapters/driven/logging/SpdLogLogger.h"
#include <cwapi3d/ICwAPI3DControllerFactory.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <exception>
#include <utility>

namespace cw_api3d::composition
{
  PluginBootstrapper::PluginBootstrapper(
    ports::interfaces::UtilityProviderPtr utilityProvider,
    ports::interfaces::LoggerPtr logger) noexcept
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
      auto logger = std::make_shared<adapters::driven::logging::SpdLogLogger>();

      CwAPI3D::Interfaces::ICwAPI3DUtilityController* utilityCtrl = nullptr;
      if (factory != nullptr)
      {
        try
        {
          utilityCtrl = factory->getUtilityController();
        }
        catch (const std::exception& ex)
        {
          logger->errorf("Failed to retrieve utility controller from factory: {}", ex.what());
        }
        catch (...)
        {
          logger->error("Unknown exception while retrieving utility controller from factory");
        }
      }
      else
      {
        logger->warn("Null CwAPI3D ControllerFactory provided during bootstrapping");
      }

      auto utilityAdapter = std::make_shared<adapters::driven::cadwork::CadworkUtilityAdapter>(
        utilityCtrl,
        logger);

      return std::make_unique<PluginBootstrapper>(std::move(utilityAdapter), std::move(logger));
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
      auto bootstrapper = PluginBootstrapper::createProduction(factory);
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

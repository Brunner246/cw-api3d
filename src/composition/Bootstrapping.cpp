#include "src/composition/Bootstrapping.h"

#include "src/adapters/driven/cadwork/CadworkUtilityAdapter.h"
#include "src/adapters/driven/logging/SpdLogLogger.h"
#include <cwapi3d/ICwAPI3DControllerFactory.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <exception>
#include <utility>

namespace cw_api3d::composition {

PluginBootstrapper::PluginBootstrapper(
    ports::interfaces::UtilityProviderPtr utility_provider,
    ports::interfaces::LoggerPtr logger) noexcept
    : utility_provider_(std::move(utility_provider)),
      logger_(std::move(logger)),
      use_case_(utility_provider_, logger_) {}

std::unique_ptr<PluginBootstrapper> PluginBootstrapper::create_production(
    CwAPI3D::ControllerFactory* factory) noexcept {
    try {
        auto logger = std::make_shared<adapters::driven::logging::SpdLogLogger>();

        CwAPI3D::Interfaces::ICwAPI3DUtilityController* utility_ctrl = nullptr;
        if (factory != nullptr) {
            try {
                utility_ctrl = factory->getUtilityController();
            } catch (const std::exception& ex) {
                logger->errorf("Failed to retrieve utility controller from factory: {}", ex.what());
            } catch (...) {
                logger->error("Unknown exception while retrieving utility controller from factory");
            }
        } else {
            logger->warn("Null CwAPI3D ControllerFactory provided during bootstrapping");
        }

        auto utility_adapter = std::make_shared<adapters::driven::cadwork::CadworkUtilityAdapter>(
            utility_ctrl, logger);

        return std::make_unique<PluginBootstrapper>(std::move(utility_adapter), std::move(logger));
    } catch (...) {
        return nullptr;
    }
}

std::expected<std::filesystem::path, std::string> PluginBootstrapper::run() noexcept {
    try {
        if (!logger_) {
            return std::unexpected("Logger is not initialized in bootstrapper");
        }
        if (!utility_provider_) {
            logger_->error("Utility provider is not initialized in bootstrapper");
            return std::unexpected("Utility provider is not initialized in bootstrapper");
        }

        return use_case_.execute();
    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->errorf("Unhandled exception during use case execution: {}", ex.what());
        }
        return std::unexpected(std::string("Exception: ") + ex.what());
    } catch (...) {
        if (logger_) {
            logger_->error("Unknown unhandled exception during use case execution");
        }
        return std::unexpected("Unknown exception during use case execution");
    }
}

bool bootstrap_plugin(CwAPI3D::ControllerFactory* factory) noexcept {
    try {
        auto bootstrapper = PluginBootstrapper::create_production(factory);
        if (!bootstrapper) {
            return false;
        }

        const auto result = bootstrapper->run();
        return result.has_value();
    } catch (...) {
        return false;
    }
}

} // namespace cw_api3d::composition

#pragma once

#include "src/application/ElementSnapshot.h"

#include <concepts>
#include <expected>
#include <memory>
#include <span>
#include <string>

namespace cw_api3d::ports
{

  namespace interfaces
  {

    struct IElementActivation
    {
      virtual ~IElementActivation() = default;

      [[nodiscard]] virtual std::expected<void, std::string> activate(
        std::span<const application::ElementId> elementIds) = 0;
    };

    using ElementActivationPtr = std::shared_ptr<IElementActivation>;

  } // namespace interfaces

  namespace concepts
  {

    template<typename T>
    concept ElementActivation = requires(T& activation, std::span<const application::ElementId> elementIds) {
      { activation.activate(elementIds) } -> std::same_as<std::expected<void, std::string>>;
    };

  } // namespace concepts

} // namespace cw_api3d::ports

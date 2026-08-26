#pragma once

#include "src/application/ElementSnapshot.h"

#include <expected>
#include <memory>
#include <span>
#include <string>

namespace cw_api3d::ports
{

  struct IElementActivation
  {
    virtual ~IElementActivation() = default;

    [[nodiscard]] virtual std::expected<void, std::string> activate(
      std::span<const application::ElementId> elementIds) = 0;
  };

  using ElementActivationPtr = std::shared_ptr<IElementActivation>;

} // namespace cw_api3d::ports

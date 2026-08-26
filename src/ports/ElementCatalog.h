#pragma once

#include "src/application/ElementSnapshot.h"

#include <expected>
#include <memory>
#include <string>

namespace cw_api3d::ports
{

  struct IElementCatalog
  {
    virtual ~IElementCatalog() = default;

    [[nodiscard]] virtual std::expected<application::ElementSnapshot, std::string> fetch(
      application::ElementUniverse universe) const = 0;
  };

  using ElementCatalogPtr = std::shared_ptr<IElementCatalog>;

} // namespace cw_api3d::ports

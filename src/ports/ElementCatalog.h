#pragma once

#include "src/application/ElementSnapshot.h"

#include <concepts>
#include <expected>
#include <memory>
#include <string>

namespace cw_api3d::ports
{

  namespace interfaces
  {

    struct IElementCatalog
    {
      virtual ~IElementCatalog() = default;

      [[nodiscard]] virtual std::expected<application::ElementSnapshot, std::string> fetch(
        application::ElementUniverse universe) const = 0;
    };

    using ElementCatalogPtr = std::shared_ptr<IElementCatalog>;

  } // namespace interfaces

  namespace concepts
  {

    template<typename T>
    concept ElementCatalog = requires(const T& catalog, application::ElementUniverse universe) {
      { catalog.fetch(universe) } -> std::same_as<std::expected<application::ElementSnapshot, std::string>>;
    };

  } // namespace concepts

} // namespace cw_api3d::ports

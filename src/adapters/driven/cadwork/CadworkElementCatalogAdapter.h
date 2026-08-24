#pragma once

#include "src/adapters/driven/cadwork/ElementCatalogAdapter.h"

#include <cwapi3d/ICwAPI3DAttributeController.h>
#include <cwapi3d/ICwAPI3DElementController.h>
#include <cwapi3d/ICwAPI3DGeometryController.h>

namespace cw_api3d::adapters::driven::cadwork
{

  struct CadworkElementCatalogHost
  {
    CwAPI3D::Interfaces::ICwAPI3DElementController* elements{nullptr};
    CwAPI3D::Interfaces::ICwAPI3DAttributeController* attributes{nullptr};
    CwAPI3D::Interfaces::ICwAPI3DGeometryController* geometry{nullptr};

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DElementIDList* getActiveIdentifiableElementIDs()
    {
      return elements ? elements->getActiveIdentifiableElementIDs() : nullptr;
    }

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DElementIDList* getAllIdentifiableElementIDs()
    {
      return elements ? elements->getAllIdentifiableElementIDs() : nullptr;
    }

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DString* getName(const CwAPI3D::elementID id)
    {
      return attributes ? attributes->getName(id) : nullptr;
    }

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DString* getElementMaterialName(const CwAPI3D::elementID id)
    {
      return attributes ? attributes->getElementMaterialName(id) : nullptr;
    }

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DString* getElementTypeDescription(const CwAPI3D::elementID id)
    {
      return elements ? elements->getElementTypeDescription(id) : nullptr;
    }

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DElementType* getElementType(const CwAPI3D::elementID id)
    {
      return attributes ? attributes->getElementType(id) : nullptr;
    }

    [[nodiscard]] bool isBeam(const CwAPI3D::elementID id)
    {
      return attributes && attributes->isBeam(id);
    }

    [[nodiscard]] double getLength(const CwAPI3D::elementID id)
    {
      return geometry ? geometry->getLength(id) : 0.0;
    }

    [[nodiscard]] double getWidth(const CwAPI3D::elementID id)
    {
      return geometry ? geometry->getWidth(id) : 0.0;
    }

    [[nodiscard]] double getHeight(const CwAPI3D::elementID id)
    {
      return geometry ? geometry->getHeight(id) : 0.0;
    }

    [[nodiscard]] double getVolume(const CwAPI3D::elementID id)
    {
      return geometry ? geometry->getVolume(id) : 0.0;
    }
  };

  using CadworkElementCatalogAdapter = ElementCatalogAdapter<CadworkElementCatalogHost>;

  extern template class ElementCatalogAdapter<CadworkElementCatalogHost>;

} // namespace cw_api3d::adapters::driven::cadwork

#pragma once

#include "src/adapters/driven/cadwork/ElementActivationAdapter.h"

#include <cwapi3d/ICwAPI3DControllerFactory.h>
#include <cwapi3d/ICwAPI3DVisualizationController.h>

namespace cw_api3d::adapters::driven::cadwork
{

  struct CadworkElementActivationHost
  {
    CwAPI3D::Interfaces::ICwAPI3DControllerFactory* factory{nullptr};
    CwAPI3D::Interfaces::ICwAPI3DVisualizationController* visualization{nullptr};

    [[nodiscard]] CwAPI3D::Interfaces::ICwAPI3DElementIDList* createEmptyElementIDList()
    {
      return factory ? factory->createEmptyElementIDList() : nullptr;
    }

    void setActive(CwAPI3D::Interfaces::ICwAPI3DElementIDList* list)
    {
      if (visualization)
      {
        visualization->setActive(list);
      }
    }
  };

  using CadworkElementActivationAdapter = ElementActivationAdapter<CadworkElementActivationHost>;

  extern template class ElementActivationAdapter<CadworkElementActivationHost>;

} // namespace cw_api3d::adapters::driven::cadwork

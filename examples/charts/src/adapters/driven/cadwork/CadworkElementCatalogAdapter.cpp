#include "CadworkElementCatalogAdapter.h"

namespace cw_api3d::adapters::driven::cadwork
{

  static_assert(concepts::ElementCatalogSource<CadworkElementCatalogHost>,
                "CadworkElementCatalogHost must satisfy concepts::ElementCatalogSource");
  static_assert(ports::concepts::ElementCatalog<CadworkElementCatalogAdapter>,
                "CadworkElementCatalogAdapter must satisfy ports::concepts::ElementCatalog");

  template class ElementCatalogAdapter<CadworkElementCatalogHost>;

} // namespace cw_api3d::adapters::driven::cadwork

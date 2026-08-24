#include "CadworkElementActivationAdapter.h"

namespace cw_api3d::adapters::driven::cadwork
{

  static_assert(concepts::ElementActivationSource<CadworkElementActivationHost>,
                "CadworkElementActivationHost must satisfy concepts::ElementActivationSource");
  static_assert(ports::concepts::ElementActivation<CadworkElementActivationAdapter>,
                "CadworkElementActivationAdapter must satisfy ports::concepts::ElementActivation");

  template class ElementActivationAdapter<CadworkElementActivationHost>;

} // namespace cw_api3d::adapters::driven::cadwork

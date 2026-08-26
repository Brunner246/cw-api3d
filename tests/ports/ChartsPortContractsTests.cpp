#include <gtest/gtest.h>

#include "src/ports/ElementActivation.h"
#include "src/ports/ElementCatalog.h"
#include "tests/doubles/FakeElementActivation.h"
#include "tests/doubles/FakeElementCatalog.h"

#include <concepts>
#include <vector>

using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;

static_assert(std::derived_from<FakeElementCatalog, IElementCatalog>, "FakeElementCatalog must derive from IElementCatalog");
static_assert(std::derived_from<FakeElementActivation, IElementActivation>, "FakeElementActivation must derive from IElementActivation");

TEST(ChartsPortContractsTests, FakeElementCatalogReturnsConfiguredSnapshot)
{
  FakeElementCatalog catalog;
  cw_api3d::application::ElementSnapshot snapshot;
  snapshot.records.push_back(cw_api3d::application::ElementRecord{.id = 7});
  catalog.setActive(snapshot);

  const auto result = catalog.fetch(cw_api3d::application::ElementUniverse::Active);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->records.size(), 1u);
  EXPECT_EQ(result->records.front().id, 7u);
}

TEST(ChartsPortContractsTests, FakeElementActivationRecordsIds)
{
  FakeElementActivation activation;
  const std::vector<cw_api3d::application::ElementId> ids{3, 5};

  const auto result = activation.activate(ids);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(activation.lastIds(), ids);
}

#pragma once

#include "tests/doubles/StubCwAPI3DUtilityController.h"
#include <cwapi3d/ICwAPI3DControllerFactory.h>

namespace cw_api3d::tests::doubles {

/// @brief Minimal ICwAPI3DControllerFactory test stub returning a configured StubCwAPI3DUtilityController.
class StubCwAPI3DControllerFactory : public CwAPI3D::Interfaces::ICwAPI3DControllerFactory {
public:
    explicit StubCwAPI3DControllerFactory(StubCwAPI3DUtilityController* utility_controller = nullptr) noexcept
        : utility_controller_(utility_controller) {}

    CwAPI3D::Interfaces::ICwAPI3DUtilityController* getUtilityController() override {
        return utility_controller_;
    }

    void set_utility_controller(StubCwAPI3DUtilityController* utility_controller) noexcept {
        utility_controller_ = utility_controller;
    }

    // --- Inert remainder of ICwAPI3DControllerFactory ---
    CwAPI3D::Interfaces::ICwAPI3DAttributeController* getAttributeController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DConnectorAxisController* getConnectorAxisController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementController* getElementController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DFileController* getFileController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DGeometryController* getGeometryController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DListController* getListController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DMachineController* getMachineController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DMaterialController* getMaterialController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DMenuController* getMenuController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DSceneController* getSceneController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DShopDrawingController* getShopDrawingController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DVisualizationController* getVisualizationController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementIDList* createEmptyElementIDList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DVertexList* createEmptyVertexList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementFilter* createElementFilter() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementMapQuery* createElementMapQuery() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DEndtypeController* getEndtypeController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementIDList* createElementIDListFromElement(CwAPI3D::elementID) override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementModuleProperties* createEmptyElementModuleProperties() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DRoofController* getRoofController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DBimController* getBimController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DLayerSettings* createEmptyLayerSettings() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DDisplayAttribute* createEmptyDisplayAttribute() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DStringList* createEmptyStringList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DPolygonList* createEmptyPolygonList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DDimensionController* getDimensionController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DGridController* getGridController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DExtendedSettings* createEmptyExtendedSettings() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DRhinoOptions* createRhinoOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DIfcOptions* createIfcOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DTextObjectOptions* createTextObjectOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DCameraData* createCameraData() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DMultiLayerCoverController* getMultiLayerCoverController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DImport3dcOptions* createImport3dcOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DEndtypeIDList* createEmptyEndtypeIDList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DElementType* createElementType() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DActivationState* createEmptyActivationState() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DAttributeDisplaySettings* createEmptyAttributeDisplaySettings() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DCoordinateSystemData* createEmptyCoordinateSystemData() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DEdgeList* createEmptyEdgeList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DFacetList* createEmptyFacetList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DIfc2x3ElementType* createEmptyIfc2x3ElementType() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DIfcPredefinedType* createEmptyIfcPredefinedType() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DMaterialIDList* createEmptyMaterialIDList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DProcessType* createEmptyProcessType() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DVisibilityState* createEmptyVisibilityState() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DShoulderOptions* createEmptyShoulderOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DHeelShoulderOptions* createEmptyHeelShoulderOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DDoubleShoulderOptions* createEmptyDoubleShoulderOptions() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DEventSubscriptionController* getEventSubscriptionController() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DHitResult* createEmptyHitResult() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DConnectorAxisItem* createConnectorAxisItem() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DConnectorAxisItemList* createEmptyConnectorAxisItemList() override { return nullptr; }
    CwAPI3D::Interfaces::ICwAPI3DPanelPrefabElementSettings* createPanelPrefabElementSettings() override { return nullptr; }

private:
    StubCwAPI3DUtilityController* utility_controller_{nullptr};
};

} // namespace cw_api3d::tests::doubles

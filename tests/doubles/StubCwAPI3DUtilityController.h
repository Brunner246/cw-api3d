#pragma once

#include <cwapi3d/ICwAPI3DString.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace cw_api3d::tests::doubles {

/// @brief Minimal ICwAPI3DString test stub whose destroy() sets a flag instead of freeing memory.
/// Used to assert that the adapter never calls destroy() on host-owned strings.
class StubCwAPI3DString final : public CwAPI3D::Interfaces::ICwAPI3DString {
public:
    explicit StubCwAPI3DString(std::string narrow = "") : mNarrow(std::move(narrow)) {}

    void destroy() override { mDestroyCalled = true; }
    uint32_t length() override { return static_cast<uint32_t>(mNarrow.size()); }
    const CwAPI3D::character* data() override { return nullptr; }

    const CwAPI3D::narrowCharacter* narrowData() override {
        return mReturnNullNarrowData ? nullptr : mNarrow.c_str();
    }

    void copyToBuffer(CwAPI3D::character*, uint32_t) override {}
    void copyToNarrowBuffer(CwAPI3D::narrowCharacter*, uint32_t) override {}

    void setReturnNullNarrowData(bool returnNull) noexcept {
        mReturnNullNarrowData = returnNull;
    }

    void setNarrow(std::string narrow) {
        mNarrow = std::move(narrow);
    }

    [[nodiscard]] bool destroyCalled() const noexcept { return mDestroyCalled; }

private:
    std::string mNarrow;
    bool mReturnNullNarrowData{false};
    bool mDestroyCalled{false};
};

/// @brief ICwAPI3DUtilityController test stub with configurable getPluginPath() behavior.
class StubCwAPI3DUtilityController final : public CwAPI3D::Interfaces::ICwAPI3DUtilityController {
public:
    enum class Behaviour {
        ReturnsString,
        ReturnsNullptr,
        Throws
    };

    StubCwAPI3DUtilityController() = default;
    explicit StubCwAPI3DUtilityController(std::string pluginPath)
        : mString(std::move(pluginPath)) {}

    CwAPI3D::Interfaces::ICwAPI3DString* getPluginPath() override {
        ++mCallCount;
        switch (mBehaviour) {
            case Behaviour::ReturnsNullptr:
                return nullptr;
            case Behaviour::Throws:
                throw std::runtime_error("Simulated CwAPI3D exception");
            case Behaviour::ReturnsString:
                break;
        }
        return &mString;
    }

    void setBehaviour(Behaviour behaviour) noexcept { mBehaviour = behaviour; }
    [[nodiscard]] StubCwAPI3DString& stringStub() noexcept { return mString; }
    [[nodiscard]] int callCount() const noexcept { return mCallCount; }

    // --- Inert remainder of ICwAPI3DUtilityController ---
    CwAPI3D::Interfaces::ICwAPI3DString* getLastError(int32_t*) override { return {}; }
    uint32_t get3DVersion() override { return {}; }
    uint32_t get3DBuild() override { return {}; }
    HWND get3DHWND() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DFileName() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DFilePath() override { return {}; }
    void setProjectData(const CwAPI3D::character*, const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectData(const CwAPI3D::character*) override { return {}; }
    void printError(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getLanguage() override { return {}; }
    void printMessage(const CwAPI3D::character*, uint32_t, uint32_t) override {}
    int32_t getUserInt(const CwAPI3D::character*) override { return {}; }
    double getUserDouble(const CwAPI3D::character*) override { return {}; }
    bool getUserBool(const CwAPI3D::character*, bool) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getUserString(const CwAPI3D::character*) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectName() override { return {}; }
    void setProjectName(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectNumber() override { return {}; }
    void setProjectNumber(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectPart() override { return {}; }
    void setProjectPart(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectArchitect() override { return {}; }
    void setProjectArchitect(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectCustomer() override { return {}; }
    void setProjectCustomer(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectDesigner() override { return {}; }
    void setProjectDesigner(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectDeadline() override { return {}; }
    void setProjectDeadline(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectUserAttribute(uint32_t) override { return {}; }
    void setProjectUserAttribute(uint32_t, const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectUserAttributeName(uint32_t) override { return {}; }
    void setProjectUserAttributeName(uint32_t, const CwAPI3D::character*) override {}
    double getProjectLatitude() override { return {}; }
    double getProjectLongitude() override { return {}; }
    void setProjectLatitude(double) override {}
    void setProjectLongitude(double) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectAddress() override { return {}; }
    void setProjectAddress(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectPostalCode() override { return {}; }
    void setProjectPostalCode(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectCity() override { return {}; }
    void setProjectCity(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectCountry() override { return {}; }
    void setProjectCountry(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* get3DUserprofilPath() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getUserFileFromDialog(const CwAPI3D::character*) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getClientNumber() override { return {}; }
    CwAPI3D::vector3D getUserPoint() override { return {}; }
    void disableAutoDisplayRefresh() override {}
    void enableAutoDisplayRefresh() override {}
    CwAPI3D::Interfaces::ICwAPI3DString* createNewGUID() override { return {}; }
    void printToConsole(const CwAPI3D::character*) override {}
    void exportScreenToImage(const CwAPI3D::character*, uint32_t) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getNewUserFileFromDialog(const CwAPI3D::character*) override { return {}; }
    uint32_t apiAutostart(const CwAPI3D::character*, uint32_t) override { return {}; }
    void enableAutostart(const CwAPI3D::character*) override {}
    void disableAutostart(const CwAPI3D::character*) override {}
    bool checkAutostart(const CwAPI3D::character*) override { return {}; }
    void deleteProjectData(const CwAPI3D::character*) override {}
    bool runExternalProgram(const CwAPI3D::character*) override { return {}; }
    void save3DFileSilently() override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectGUID() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getLicenceFirstPart() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getLicenceSecondPart() override { return {}; }
    void showProgressBar() override {}
    void updateProgressBar(int32_t) override {}
    void hideProgressBar() override {}
    uint32_t getUserColor(uint32_t) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DLinearUnits() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DLinearDisplayUnits() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DAngularUnits() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DAngularDisplayUnits() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DBuildDate() override { return {}; }
    double getProjectElevation() override { return {}; }
    void setProjectElevation(double) override {}
    void clearErrors() override {}
    void pushCheckAndQueryData() override {}
    void popCheckAndQueryData() override {}
    void changeCheckAndQueryDataToNoQueries() override {}
    void changeCheckAndQueryDataToQueries() override {}
    bool isDirectInfoEnabled() override { return {}; }
    void enableDirectInfo() override {}
    void disableDirectInfo() override {}
    void loadAttributeDisplaySettings(const CwAPI3D::character*) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getProjectDescription() override { return {}; }
    void setProjectDescription(const CwAPI3D::character*) override {}
    void startProjectDataDialog() override {}
    void initLxSDK() override {}
    void loadElementAttributeDisplaySettings(const CwAPI3D::character*, CwAPI3D::Interfaces::ICwAPI3DElementIDList*) override {}
    double getGlobalXOffset() override { return {}; }
    void setGlobalXOffset(double) override {}
    double getGlobalYOffset() override { return {}; }
    void setGlobalYOffset(double) override {}
    double getGlobalZOffset() override { return {}; }
    void setGlobalZOffset(double) override {}
    void showNorthArrow() override {}
    void hideNorthArrow() override {}
    bool isNorthArrowVisible() override { return {}; }
    double getNorthAngle() override { return {}; }
    void setNorthAngle(double) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* getUserFileFromDialogInPath(const CwAPI3D::character*, const CwAPI3D::character*) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getNewUserFileFromDialogInPath(const CwAPI3D::character*, const CwAPI3D::character*) override { return {}; }
    double getMillimetreFromImperialString(const CwAPI3D::character*) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getImperialStringFromMillimetre(double) override { return {}; }
    void enableUpdateVariant() override {}
    void disableUpdateVariant() override {}
    CwAPI3D::Interfaces::ICwAPI3DVertexList* getUserPoints() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DVertexList* getUserPointsWithCount(uint64_t) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getUserCatalogPath() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getUserPathFromDialog() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getUserPathFromDialogInPath(const CwAPI3D::character*) override { return {}; }
    void executeShortcut(CwAPI3D::shortcutKeyModifier, CwAPI3D::shortcutKey) override {}
    void closeCadworkDocumentSaved() override {}
    void closeCadworkDocumentUnsaved() override {}
    bool getUseOfGlobalCoordinates() override { return {}; }
    void setUseOfGlobalCoordinates(bool) override {}
    CwAPI3D::vector3D getGlobalOrigin() override { return {}; }
    void setGlobalOrigin(CwAPI3D::vector3D) override {}
    CwAPI3D::Interfaces::ICwAPI3DString* createSnapshot(const CwAPI3D::character*, int, bool) override { return {}; }
    bool runExternalProgramFromCustomDirectory(const CwAPI3D::character*) override { return {}; }
    CwAPI3D::windowGeometry get3dMainWindowGeometry() override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DStringList* getProjectDataKeys() override { return {}; }
    int32_t getUserIntWithDefaultValue(const CwAPI3D::character*, int32_t) override { return {}; }
    double getUserDoubleWithDefaultValue(const CwAPI3D::character*, double) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* getUserStringWithDefaultValue(const CwAPI3D::character*, const CwAPI3D::character*) override { return {}; }
    CwAPI3D::Interfaces::ICwAPI3DString* get3DVersionName() override { return {}; }
    void redirectPythonOutputToLogger() override {}
    void setLanguage(CwAPI3D::language) override {}
    CwAPI3D::language getLanguageEnum() override { return {}; }

private:
    StubCwAPI3DString mString{""};
    Behaviour mBehaviour{Behaviour::ReturnsString};
    int mCallCount{0};
};

} // namespace cw_api3d::tests::doubles

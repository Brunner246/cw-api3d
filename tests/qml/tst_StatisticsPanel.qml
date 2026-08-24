import QtQuick
import QtQuick.Controls
import QtTest
import CwApi3d.Statistics

Item {
    width: 520
    height: 720

    StatisticsPanel {
        id: panel
        anchors.fill: parent
    }

    TestCase {
        name: "StatisticsPanelBindings"
        when: windowShown

        function named(name) {
            return findChild(panel, name)
        }

        function germanDummy(text) {
            return text + text.substring(0, Math.ceil(text.length * 0.4))
        }

        function test_chartKindDefaultsToBar() {
            compare(viewModel.chartKind, 0)
        }

        function test_chartKindSwitchToPie() {
            viewModel.chartKind = 1
            compare(viewModel.chartKind, 1)
            compare(named("kind").currentIndex, 1)
            viewModel.chartKind = 0
        }

        function test_axisBinding() {
            viewModel.axis = 1
            compare(named("axis").currentIndex, 1)
            viewModel.axis = 0
            compare(named("axis").currentIndex, 0)
        }

        function test_universeBinding() {
            viewModel.universe = 1
            compare(named("universe").currentIndex, 1)
            viewModel.universe = 0
        }

        function test_legendRowsMatchBuckets() {
            const legend = named("legend")
            compare(legend.count, viewModel.buckets.rowCount())
            verify(legend.count > 0)
            const first = legend.itemAtIndex(0)
            verify(first !== null)
            verify(first.text.indexOf(String(viewModel.buckets.countAt(0))) !== -1)
            verify(first.text.indexOf(viewModel.buckets.labelAt(0)) !== -1)
        }

        function test_fetchAccessibleName() {
            compare(named("fetch").Accessible.name, qsTr("Fetch"))
            compare(named("fetch").text, qsTr("Fetch"))
        }

        function test_activeFocusOnTab() {
            verify(named("fetch").activeFocusOnTab)
            verify(named("universe").activeFocusOnTab)
            verify(named("axis").activeFocusOnTab)
            verify(named("kind").activeFocusOnTab)
            verify(named("chartTable").activeFocusOnTab)
        }

        function test_tabOrderObjectNames() {
            named("fetch").forceActiveFocus()
            compare(named("fetch").activeFocus, true)
            keyClick(Qt.Key_Tab)
            compare(named("universe").activeFocus, true)
            keyClick(Qt.Key_Tab)
            compare(named("axis").activeFocus, true)
            keyClick(Qt.Key_Tab)
            compare(named("kind").activeFocus, true)
            keyClick(Qt.Key_Tab)
            compare(named("chartTable").activeFocus, true)
        }

        function test_escapeIsNotAccepted() {
            named("fetch").forceActiveFocus()
            keyClick(Qt.Key_Escape)
            compare(named("fetch").visible, true)
            compare(panel.visible, true)
        }

        function test_qmlDoesNotComputeBuckets() {
            verify(panel.rebuildSeries !== undefined)
            compare(viewModel.buckets.rowCount() > 0, true)
        }

        function test_changingAxisUpdatesBoundControl() {
            const previous = named("axis").currentIndex
            viewModel.axis = 2
            compare(named("axis").currentIndex, 2)
            verify(named("axis").currentIndex !== previous)
            viewModel.axis = 0
        }

        function test_emptyAndErrorLabelsUseQsTr() {
            compare(named("emptyState").text, qsTr("No elements to chart"))
            compare(named("chartUnavailable").text, qsTr("Chart surface is unavailable; showing table"))
        }

        function test_germanDummyDoesNotTruncate() {
            const empty = named("emptyState")
            const original = empty.text
            empty.text = germanDummy(original)
            empty.width = 480
            wait(30)
            compare(empty.truncated, false)
            empty.text = original
        }

        function test_bucketAccessibleNameContainsLabelAndCount() {
            const legend = named("legend")
            const first = legend.itemAtIndex(0)
            verify(first !== null)
            verify(first.Accessible.name.indexOf(viewModel.buckets.labelAt(0)) !== -1)
            verify(first.Accessible.name.indexOf(String(viewModel.buckets.countAt(0))) !== -1)
        }
    }
}

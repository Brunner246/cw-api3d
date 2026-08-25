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

        function cleanup() {
            testSetup.restoreDefaultFixture()
            panel.rebuildSeries()
        }

        function germanDummy(text) {
            return text + text.substring(0, Math.ceil(text.length * 0.4))
        }

        function seriesListOfViews(chart) {
            const series = []
            const views = chart.children
            for (let v = 0; v < views.length; ++v) {
                const list = views[v].seriesList
                if (list === undefined)
                    continue
                for (let i = 0; i < list.length; ++i)
                    series.push(list[i])
            }
            return series
        }

        function pieSeriesOf(chart) {
            const series = seriesListOfViews(chart)
            for (let i = 0; i < series.length; ++i) {
                if (series[i].holeSize !== undefined)
                    return series[i]
            }
            return null
        }

        function barSeriesOf(chart) {
            const series = seriesListOfViews(chart)
            for (let i = 0; i < series.length; ++i) {
                if (series[i].barsType !== undefined)
                    return series[i]
            }
            return null
        }

        function isUnder(item, ancestor) {
            let current = item.parent
            while (current !== null && current !== undefined) {
                if (current === ancestor)
                    return true
                current = current.parent
            }
            return false
        }

        function chipAt(row) {
            const legend = named("legend")
            legend.positionViewAtIndex(row, ListView.Contain)
            wait(30)
            const rowItem = legend.itemAtIndex(row)
            verify(rowItem !== null)
            return findChild(rowItem, "legendChip")
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

        function test_chartLegendSplitIsVertical() {
            const split = named("chartLegendSplit")
            verify(split !== null)
            compare(split.orientation, Qt.Vertical)
            const chart = named("chartTable")
            compare(chart.SplitView.fillHeight, true)
            compare(chart.SplitView.minimumHeight, 96)
            const legend = named("legend")
            compare(legend.SplitView.preferredHeight, 160)
            compare(legend.SplitView.minimumHeight, 64)
        }

        function test_legendScrollBarIsAlwaysOn() {
            const legend = named("legend")
            const bar = named("legendScrollBar")
            verify(bar !== null)
            compare(bar.policy, ScrollBar.AlwaysOn)
            compare(legend.ScrollBar.vertical, bar)
        }

        function test_pieSliceColorsMatchColorAt() {
            panel.rebuildSeries()
            wait(30)
            const pie = pieSeriesOf(named("chartTable"))
            verify(pie !== null)
            const n = Math.min(viewModel.buckets.rowCount(), 12)
            verify(n > 0)
            for (let i = 0; i < n; ++i) {
                const slice = pie.at(i)
                verify(slice !== null)
                verify(Qt.colorEqual(slice.color, viewModel.buckets.colorAt(i)))
            }
        }

        function test_barSetColorsMatchColorAt() {
            panel.rebuildSeries()
            wait(30)
            const bars = barSeriesOf(named("chartTable"))
            verify(bars !== null)
            const n = Math.min(viewModel.buckets.rowCount(), 12)
            verify(n > 0)
            for (let i = 0; i < n; ++i) {
                const barSet = bars.at(i)
                verify(barSet !== null)
                verify(Qt.colorEqual(barSet.color, viewModel.buckets.colorAt(i)))
            }
        }

        function test_legendChipMatchesColorAt() {
            const n = Math.min(viewModel.buckets.rowCount(), 12)
            verify(n > 0)
            for (let i = 0; i < n; ++i) {
                const chip = chipAt(i)
                verify(chip !== null)
                compare(chip.width, 12)
                compare(chip.height, 12)
                verify(Qt.colorEqual(chip.color, viewModel.buckets.colorAt(i)))
                verify(Qt.colorEqual(chip.border.color, "#808080"))
            }
        }

        function test_emptyErrorAndControlsStayOutsideSplit() {
            const split = named("chartLegendSplit")
            const names = ["emptyState", "errorState", "chartUnavailable", "fetch", "universe", "axis", "kind"]
            for (let i = 0; i < names.length; ++i) {
                const item = named(names[i])
                verify(item !== null)
                compare(isUnder(item, split), false)
            }
        }

        function test_legendAndCategoriesShowUmlauts() {
            testSetup.loadUmlautFixture()
            panel.rebuildSeries()
            const legend = named("legend")
            tryCompare(legend, "count", 1)
            legend.positionViewAtIndex(0, ListView.Contain)
            wait(30)
            const row = legend.itemAtIndex(0)
            verify(row !== null)
            verify(row.text.indexOf("Öffnung") !== -1)
            compare(panel.categoryList.indexOf("Öffnung") !== -1, true)
            viewModel.axis = 1
            panel.rebuildSeries()
            tryCompare(legend, "count", 1)
            legend.positionViewAtIndex(0, ListView.Contain)
            wait(30)
            const materialRow = legend.itemAtIndex(0)
            verify(materialRow !== null)
            verify(materialRow.text.indexOf("Grün") !== -1)
            compare(panel.categoryList.indexOf("Grün") !== -1, true)
            viewModel.chartKind = 1
            panel.rebuildSeries()
            wait(30)
            const pie = pieSeriesOf(named("chartTable"))
            verify(pie !== null)
            const slice = pie.at(0)
            verify(slice !== null)
            compare(slice.label, "Grün")
        }
    }
}

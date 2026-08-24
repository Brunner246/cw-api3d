import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

Item {
    id: root
    anchors.fill: parent
    Accessible.ignored: true

    property var categoryList: []
    property var valueList: []
    property real maxValue: 1

    function rebuildSeries() {
        const n = viewModel.buckets.rowCount()
        const cats = []
        const vals = []
        let peak = 1
        for (let i = 0; i < n; ++i) {
            const label = viewModel.buckets.labelAt(i)
            const count = viewModel.buckets.countAt(i)
            cats.push(label)
            vals.push(count)
            if (count > peak)
                peak = count
        }
        categoryList = cats
        valueList = vals
        maxValue = peak
        pieSeries.clear()
        for (let i = 0; i < n; ++i)
            pieSeries.append(cats[i], vals[i])
    }

    Connections {
        target: viewModel
        function onTotalCountChanged() { root.rebuildSeries() }
        function onAxisChanged() { root.rebuildSeries() }
        function onEmptyChanged() { root.rebuildSeries() }
        function onErrorMessageChanged() { root.rebuildSeries() }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Button {
            id: fetch
            objectName: "fetch"
            text: qsTr("Fetch")
            Accessible.name: qsTr("Fetch")
            Accessible.role: Accessible.Button
            activeFocusOnTab: true
            enabled: !viewModel.busy
            Layout.fillWidth: true
            onClicked: viewModel.fetch()
        }

        ComboBox {
            id: universe
            objectName: "universe"
            model: [qsTr("Active"), qsTr("All")]
            currentIndex: viewModel.universe
            Accessible.name: qsTr("Universe")
            activeFocusOnTab: true
            Layout.fillWidth: true
            onActivated: viewModel.universe = currentIndex
        }

        ComboBox {
            id: axis
            objectName: "axis"
            model: [
                qsTr("Type"),
                qsTr("Material"),
                qsTr("Name"),
                qsTr("Length"),
                qsTr("Width"),
                qsTr("Height"),
                qsTr("Volume")
            ]
            currentIndex: viewModel.axis
            Accessible.name: qsTr("Axis")
            activeFocusOnTab: true
            Layout.fillWidth: true
            onActivated: viewModel.axis = currentIndex
        }

        ComboBox {
            id: kind
            objectName: "kind"
            model: [qsTr("Bar"), qsTr("Pie")]
            currentIndex: viewModel.chartKind
            Accessible.name: qsTr("Chart kind")
            activeFocusOnTab: true
            Layout.fillWidth: true
            onActivated: viewModel.chartKind = currentIndex
        }

        Label {
            id: emptyState
            objectName: "emptyState"
            visible: viewModel.empty
            text: qsTr("No elements to chart")
            wrapMode: Text.Wrap
            elide: Text.ElideNone
            Layout.fillWidth: true
        }

        Label {
            id: errorState
            objectName: "errorState"
            visible: viewModel.errorMessage.length > 0
            text: viewModel.errorMessage
            wrapMode: Text.Wrap
            elide: Text.ElideNone
            Layout.fillWidth: true
        }

        Label {
            id: chartUnavailable
            objectName: "chartUnavailable"
            visible: !viewModel.chartAvailable
            text: qsTr("Chart surface is unavailable; showing table")
            wrapMode: Text.Wrap
            elide: Text.ElideNone
            Layout.fillWidth: true
        }

        Label {
            visible: viewModel.axis === 6
            text: viewModel.volumeDisclaimer
            wrapMode: Text.Wrap
            elide: Text.ElideNone
            Layout.fillWidth: true
        }

        Item {
            id: chartTable
            objectName: "chartTable"
            Layout.fillWidth: true
            Layout.fillHeight: true
            activeFocusOnTab: true
            Accessible.name: qsTr("Chart")
            Accessible.role: Accessible.Chart

            GraphsView {
                id: barView
                anchors.fill: parent
                visible: viewModel.chartKind === 0 && viewModel.chartAvailable && !viewModel.empty
                theme: GraphsTheme {
                    colorScheme: GraphsTheme.ColorScheme.Light
                    seriesColors: ["#5a5a5a", "#808080", "#404040", "#707070"]
                }
                axisX: BarCategoryAxis {
                    categories: root.categoryList
                    lineVisible: false
                }
                axisY: ValueAxis {
                    min: 0
                    max: Math.max(1, root.maxValue)
                }
                BarSeries {
                    BarSet {
                        values: root.valueList
                    }
                }
            }

            GraphsView {
                id: pieView
                anchors.fill: parent
                visible: viewModel.chartKind === 1 && viewModel.chartAvailable && !viewModel.empty
                theme: GraphsTheme {
                    colorScheme: GraphsTheme.ColorScheme.Light
                    seriesColors: ["#5a5a5a", "#808080", "#404040", "#707070"]
                }
                PieSeries {
                    id: pieSeries
                    holeSize: 0
                }
            }
        }

        ListView {
            id: legend
            objectName: "legend"
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, 160)
            clip: true
            model: viewModel.buckets
            delegate: ItemDelegate {
                required property string label
                required property int count
                required property int index
                width: ListView.view.width
                text: label + " — " + count
                Accessible.name: label + " " + count
                activeFocusOnTab: true
                onClicked: viewModel.selectBucket(index)
            }
        }
    }

    Component.onCompleted: rebuildSeries()
}

import QtQuick 2.9
import QtLocation 5.11
import QtPositioning 5.11


Rectangle {
    id:rc
    visible: true
    width : wini.get_w() //880
    height: wini.get_h() //290

    property real min_zoom : 4.0
    property real max_zoom : 17.0

    property double lati: 0.0
    property double loni: 0.0

    function zooms()
    {
        if (lati > 0.0) {
            return max_zoom
        } else {
            return min_zoom
        }
    }
    //
    Map {
        id: itMap
        property real zm : zooms()
        MapQuickItem {
                id: pItem
                sourceItem: Image {
                    id: marker
                    source: "/png/marker.png"
                    Text{
                        id: txt
                        y: marker.height/10 - 15
                        x: marker.width + 30
                        width: marker.width
                        color: "red"
                        font.italic: true
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        text: lati + "," + loni
                    }
                }
                coordinate : QtPositioning.coordinate(lati, loni) // pos.coordinate
                opacity: 1.0
                anchorPoint: Qt.point( sourceItem.width/2, sourceItem.height/2 )// + 32 )
        }

        anchors.fill: parent
        plugin: Plugin {
            id: itPlug
            name: "osm"
        } //"osm","mapboxgl"
        center: QtPositioning.coordinate(lati, loni) // pos.coordinate //QtPositioning.coordinate(lati, loni)
        zoomLevel: zm

        Timer {
                id: itTimer
                interval: 1000
                repeat: true
                running: true
                triggeredOnStart: true
                onTriggered: {
                    var la = Math.floor(wini.get_lat() * 1000000)
                    var lo = Math.floor(wini.get_lon() * 1000000)
                    lati = la / 1000000
                    loni = lo / 1000000
                }
        }

    }

}



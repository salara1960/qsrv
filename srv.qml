/*
import QtQuick 2.9
import QtQuick.Window 2.3
import QtPositioning 5.8
import QtLocation 5.9
Window {
    width: 512
    height: 512
    visible: true
  //PositionSource {
  //  id: positionSource
  //  active: true
  //}
  Plugin {
    id: mapPlugin
    name: "osm" // "mapboxgl", "esri", ...
  }
  Map {
    id: map
    anchors.fill: paren
    plugin: mapPlugin
    center: positionSource.position.coordinate
    zoomLevel: 14
  }
}
*/

import QtQuick 2.9
import QtLocation 5.11
import QtPositioning 5.11
import CppToQml 1.0

Rectangle {
    width: 880
    height: 260
    visible: true

        //property variant sourceData: [64.10]
        //property variant cor_lat: [CNameQml.lat()]

        //CNameQml {
        //    id: cla
        //}
        //Text {
        //    anchors.centerIn: parent
        //    font.family: "Ravie"
        //    font.pointSize: 24
        //    color: "red"
        //    text: "My property is: " //+ cla.lat
        //}

        Plugin {
            id: mapPlugin
            name: "osm" // "mapboxgl", "esri", ...
        }
        Map {
            Location {
                id:cor
                coordinate {
                    latitude : 54.699680//33//54.699680
                    longitude: 20.514002//85//20.514001
                }
            }

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
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        Component.onCompleted: {
                            text = cor.coordinate.latitude + " , " + cor.coordinate.longitude
                        }
                    }
                }
                coordinate : cor.coordinate
                opacity: 1.0
                anchorPoint: Qt.point( sourceItem.width/2, (sourceItem.height/2) + 32 )
            }

            anchors.fill: parent
            plugin: mapPlugin
            center: cor.coordinate
            zoomLevel: 16.5
        }

}



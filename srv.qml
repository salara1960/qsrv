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
import QtPositioning 5.6

Rectangle  {
    width: 880
    height: 260
    visible: true

    Plugin {
        id: mapPlugin
        name: "osm" // "mapboxgl", "esri", ...
    }
    Map {
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(54.699650, 20.514000) // Klgd
        zoomLevel: 13.5
    }
}



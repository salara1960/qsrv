import QtQuick 2.9
import QtLocation 5.11
import QtPositioning 5.11
//import QtQuick.Window 2.2
//import QtQuick.Controls 1.4
import CppToQml 1.0


Rectangle {
    id:rc
    width: 880
    height: 260
    visible: true

    property double lati: 0.0//wini.lat //wini.lat //0.0 //wini.get_lat() //54.699680
    property double loni: 0.0//wini.lon //20.514002

    /*
    TheClass {
        id: itClass
        //onLat_Changed: {
        //    console.log("Latutude changed !!!!!!!!!!!!")
        //}
    }
    */
    /*
    Connections{
        target: wini
        //wini.onLat_Changed: {
        //    lati = wini.lat
        //    loni = wini.lon
        //}
        //Component.onCompleted: {
        //    lati = wini.lat
        //    loni = wini.lon
        //    console.log("Object wini : " + lati + " , " + loni)
        //}
        wini.onLat_Changed: {
            console.log("Latutude changed !!!!!!!!!!!!")
        }
    }
    */

/**/
    function zooms()
    {
        if (lati > 0.0) {
            return 16.5
        } else {
            return 4.0
        }
    }
    //
    function cheka(la, lo)
    {
        if ((lati < la) || (lati > la)) return 1
        if ((loni < lo) || (loni > lo)) return 1
        return 0
    }

    //
    Map {
        id: itMap
        property real zm : zooms()
        Location {
            id:pos
            coordinate {
                latitude : lati //lati //sourceData[0]//lat//54.699680//33//54.699680
                longitude: loni //sourceData[1]//lon//20.514002//85//20.514001
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
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        text: lati + "," + loni
                    }
                }
                coordinate : pos.coordinate
                opacity: 1.0
                anchorPoint: Qt.point( sourceItem.width/2, sourceItem.height/2 )// + 32 )
        }

        anchors.fill: parent
        plugin: Plugin {name: "osm"} //"osm","mapboxgl"
        center: pos.coordinate //QtPositioning.coordinate(lati, loni)
        //zoomLevel: zm


        Timer {
                id: itTimer
                interval: 1000
                repeat: true
                running: true
                triggeredOnStart: true
                onTriggered: {
                    var old_lat = lati
                    var old_lon = loni
                    lati = wini.get_lat()
                    loni = wini.get_lon()
                    var la = Math.floor(lati * 100000000)
                    var lo = Math.floor(loni * 100000000)
                    lati = la / 100000000
                    loni = lo / 100000000
                    //if ((old_lat !== lati ) || (old_lon !== loni)) {
                    //    console.log("!!! timer : " + lati + " , " + loni)
                    //}
                }
        }

    }
    //

    //
/**/

}



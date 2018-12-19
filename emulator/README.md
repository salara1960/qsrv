#############################################################################################
#
#               vdev - software emulator Teltonika FMB630 device (tcp client)
#
#############################################################################################

## Include files:

.vdev.conf - configuration file (with a scenario of behavior)

Makefile   - make file (for compile project)

version.c  - emulator version
vdev.c     - main thread (src)
vlib.c     - library functions file (src)
vlib.h     - header file (src)
vstart.sh  - start/stop/restart script

Binary file name - vdev


## Building and installing

make

Copy to <working_folder/> next files :

vdev       - elf binary file

vstart.sh  - start/stop/restart script

.vdev.conf - configuration file


start program :
```
$ ./vstart.sh start
or
$ ./vdev config=.vdev.conf &
```

stop program :
```
$ ./vstart.sh stop
```

restart program :
```
$ ./vstart.sh restart
```


## Support next command:
```
- "getgps",        //{"command":0}
- "deleterecords", //{"command":29}
- "getver",        //{"command":20}
- "getio",         //{"command":15}
- "SET_ALL",       //{"command":48,"param":"X1,Y1 X2,Y2 X3,Y3 X4,Y4 X5,Y5 X6,Y6 X7,Y7 X8,Y8"}
- "GET_STAT",      //{"command":35}
- "setdigout",     // 1XXX 0 0 0 0",//{"command":1, "relay":1, "time":0}
- "setdigout",     // 0XXX 0 0 0 0",//{"command":2, "relay":1, "time":0}
- "#DO REPORT",    //{"command":40}
- "SET_ON",        //{"command":33, "relay":X, "time":Y}
- "SET_OFF"        //{"command":34, "relay":X}
```


## P.S.
```
* Client try connect to server 127.0.0.1:9090 and with imei=351580051430040 (look to .vdev.conf file)
* Client write log files "vdev.log.txt" in current folder (max size - 32M)
* Client accept next Signals :
* SIGKILL, SIGTERM, SIGINT - interrupt program

* configuration file example:
server=localhost:9090                    | car-server's url
imei=351580051430040                     | imei device
mode=0                                   | work mode (0-parking, 1-trip)
period_park=30                           | in parking mode send period (sec)
period_move=5                            | in moveing mode send period (sec)
wait_ack=10                              | wait ack from car-server (sec)
wait_before_new_connect=3                | wait before try new connection to car-server
location=54.699680,20.514002             | first location of device
;
;направление движения:приращение координаты:количество замеров по сценарию
;
;остановка:длительность остановки в замерах (каждые 30 сек - 1 замер)
#Park:5
; latitude+0.001000, longitude-0.001000, angle=315      | Северо-Запад - 315^
#North-West:1000:50
; latitude+0.001000, longitude+0.001000, angle=45       | Северо-Восток угол = 45^
#North-East:1000:50
; latitude-0.001000, longitude+0.001000, angle=135      | Юго-Восток угол = 135^
#South-East:1000:50
; latitude-0.001000, longitude-0.001000, angle=225      | Юго-Запад угол = 225^
#South-West:1000:50
; longitude-0.001000, angle=270                         | Запад угол = 270^
#West:-1000:50
; latitude-0.001000 , angle=180                         | Юг угол = 180^
#South:-1000:50
; longitude+0.001000, angle=90                          | Восток угол = 90^
#East:1000:50
; latitude+0.001000 , angle=0                           | Север угол = 0^
#North:1000:50
;остановка:длительность остановки в замерах (каждые 30 сек - 1 замер)
#Stop:10
```


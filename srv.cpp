#include "srv.h"
#include "ui_srv.h"


//-----------------------------------------------------------------------

//char const *vers = "0.2";//15.11.2018
//char const *vers = "0.3";//17.11.2018
//char const *vers = "0.4";//18.11.2018
//char const *vers = "0.5";//19.11.2018
//char const *vers = "0.6";//20.11.2018
//char const *vers = "0.6.1";//20.11.2018
//char const *vers = "0.7";//21.11.2018
//char const *vers = "0.7.1";//21.11.2018
//char const *vers = "0.7.2";//21.11.2018
//char const *vers = "0.8";//22.11.2018
//char const *vers = "0.8.2";//22.11.2018
//char const *vers = "0.8.3";//22.11.2018
//char const *vers = "0.9";//23.11.2018
//char const *vers = "0.9.1";//23.11.2018
//char const *vers = "1.0";//24.11.2018
char const *vers = "1.1";//26.11.2018


const QString title = "GPS device (FM6320/FMB630) app.";
const QString LogFileName = "logs.txt";
uint8_t dbg = 2;
const int time_wait_answer = 10000;//10 sec.

char gradus = '^';
const char *prio_name[] = {"Low","High","Panic","Security"};
const char *unknown = "Unknown";
const char *lat_name[] = {"North","South"};     // широта Latitude
const char *lon_name[] = {"East","West"};       // долгота Longitude
const char *net_type_name[] = {"3G","2G"};

const QString srv_command = "command";
const QString srv_command_param = "param";
const QString srv_command_relay = "relay";
const QString srv_command_value = "value";
const QString srv_command_time = "time";

const QString mvolt = " mv";

//--------------------   Commands for device FMB630/FM6320   ---------------------------------------
const char *cmds[] = {
"getgps",       //0//Current GPS data and time | Yes//{"command":0}
"setdigout",    //1//set DOUTX=1 на Y sec.//{"command":1, "relay":X, "time":Y} - 'включить реле № X на время Y секунд'
"setdigout",    //2//set DOUTX=0 на Y sec.//{"command":2, "relay":X, "time":Y} - 'выключить реле № X на время Y секунд'
"setdigout 1XXX 0 0 0 0",//3//set DOUT1=1 | Yes - "разблокировать бензонасос"//{"command":3} - 'разблокировать бензонасоса'     ={"command":1, "relay":1, "time":0}
"setdigout 0XXX 0 0 0 0",//4//set DOUT1=0 | Yes - "заблокировать бензонасос"//{"command":4} - 'заблокировать бензонасоса'       ={"command":2, "relay":1, "time":0}
"setdigout X1XX 0 5 0 0",//5//set DOUT2=1 | Yes - "разблокировать двери" (ЦЗ) импульс 5 сек//{"command":5} - 'разблокировать двери'    ={"command":1, "relay":2, "time":5}
"setdigout XX1X 0 0",//6//set DOUT3=1 на 18 сек | Yes - "заблокировать двери" (ЦЗ) импульс 18 сек//{"command":6} - 'заблокировать двери' (18 сек) ={"command":1, "relay":3, "time":18}
"setdigout XX1X 0 0 2 0",//7//set DOUT3=1 на 2 сек | Yes - "заблокировать двери" (ЦЗ) импульс 2 сек//{"command":7} - 'заблокировать двери' (2 сек) ={"command":1, "relay":3, "time":2}
"readio",//{"command":8} - 'включить аврийку' = {"command":1, "relay":4, "time":0}
"getio",//{"command":9} - 'включить аварийку на № секунд' (в файле конфигурации сервера) = {"command":1, "relay":4, "time":Y}
"getio",//{"command":10} - 'выключить аварийку' = {"command":2, "relay":4, "time":0}
"setdigout XXX1 0 0 0 0",//11//set DOUT4=1 | Yes - "включить зажигание"//{"command":11} - 'включить зажигание' ={"command":1, "relay":5, "time":0} |
"setdigout XXX0 0 0 0 0",//12//set DOUT4=0 | Yes - "выключить зажигание"//{"command":12} - 'выключить зажигание' ={"command":2, "relay":5, "time":0} |
"getio",//{"command":13} - 'запустить стартер' = {"command":1, "relay":6, "time":2}
"getio",//{"command":14} - 'включить звуковой сигнал на 2 сек.' = {"command":1, "relay":7, "time":2}
"getio",          //15//Readout digital inputs and outputs | Yes //{"command":15}
"getstatus",      //16//Modem Status information | Yes //{"command":16}
"getweektime",    //17//Current device time, Day of Week and amount of minutes passed since start of week | Yes //{"command":17}
"getops",         //18//List of currently available GSM operator | Yes //{"command":18}
"getcfgtime",     //19//Date and Time of last successful configuration | Yes //{"command":19}
"getver",         //20//Device / Modem / Code version information | Yes //{"command":20}
"getinfo",        //21//Device runtime system information | Yes //{"command":21}
"lvcangetprog", //"getcanprog"  //22//CAN Program Number request | Yes //{"command":22}
"readio",//"lvcangetinfo",   //23//Get LV-CAN info | Yes //{"command":23}
"getparam",       //24// getparam paramID // "getparam 1540",//8//Min Period     1540    4       0..2592000 sec | Yes //{"command":24}
"setparam",       //25// setparam paramID // "getparam 1540 20",//8//Min Period  1540    4       0..2592000 sec | Yes //{"command":25}
"cpureset",       //26// | No//{"command":26}
"lvcansetprog", //"setcanprog"  //27//LVCANSETPROG     //"lvcansetprog 112" | Yes//{"command":27}
"readops",        //28//READ_OPS_LIST    //"readops 1","readops 2","readops 3" | Yes//{"command":28}
"deleterecords",  //29//Delete all records saved on FLASH | Yes//{"command":29}
"loadprofile",    //30//Load profile #   //"loadprofile 1", "loadprofile 2","loadprofile 3","loadprofile 4" | //{"command":30}
"getgnss",        //31//Current GNSS information//{"command":31}
"odinfo",         //32//Get odometer info//{"command":32}
"SET_ON",         //33//{"command":33}
"SET_OFF",        //34//{"command":34}
"GET_STAT",       //35//{"command":35}
"#GET VERSION",   //36//{"command":36} | Yes |//Receive firmware version
"#GET NETWORK",   //37//{"command":37} | Yes |//Get GSM operator to which device is connected
"#GET IMSI",      //38//{"command":38} | Yes |//Get IMSI of the device (SIM ?)
"#GET OUT",       //39//{"command":39} | Yes |//Get DOUT values
"#DO REPORT",     //40//{"command":40} | Yes |//Save a record and send to server (after 5 seconds done)
"#GET REMIP",     //41//{"command":41,"param":X}, X=1..4 | Yes |//Get IP and port number from the configuration of a certain profile, X – profile
"#GET AUP",       //42//{"command":42,"param":X}, X=1..2 | Yes |//Get APN, user login and password from the configuration of a certain profile, X – SIMSLOT
"#GET REPRT",     //43//{"command":43,"param":X}, X=1..4 | Yes |//Get MinPeriod from the configuration of a certain profile, X – profile
"#GET REPDIST",   //44//{"command":44,"param":X}, X=1..4 | Yes |//Get MinDistance from the configuration of a certain profile, X – profile
"#GET REPANG",    //45//{"command":45,"param":X}, X=1..4 | Yes |//Get MinAngle from the configuration of a certain profile, X – profile
"#GET SENDPERIOD",//46//{"command":46,"param":X}, X=1..4 | Yes |//Get SendPeriod from the configuration of a certain profile, X – profile
"#GET REPMR",     //47//{"command":47,"param":X}, X=1..4 | Yes |//Get MinRecords from the configuration of a certain profile, X – profile
"SET_ALL"         //48//{"command":48,"param":"X1,Y1 X2,Y2 X3,Y3 X4,Y4 X5,Y5 X6,Y6 X7,Y7 X8,Y8"} // set all relay to Xn,Yn / Xn=X,0,1 Yn=0...255
};

const uint16_t crc16tab[] = // CRC lookup table [B]polynomial 0xA001[/B]
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};
#define UPDC16(ch, crc) (crc16tab[((crc) ^ (ch)) & 0xff] ^ ((crc) >> 8))

//-----------------------------------------------------------------------
void MainWindow::LogSave(const char *func, QString st, bool pr)
{
    QFile fil(LogFileName);
    if (fil.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QString fw;
        if (pr) {
            time_t it_ct = QDateTime::currentDateTime().toTime_t();
            struct tm *ctimka = localtime(&it_ct);
            fw.sprintf("%02d.%02d.%02d %02d:%02d:%02d | ",
                        ctimka->tm_mday,
                        ctimka->tm_mon+1,
                        ctimka->tm_year+1900,
                        ctimka->tm_hour,
                        ctimka->tm_min,
                        ctimka->tm_sec);
        }
        if (func) {
            fw.append("[");
            fw.append(func);
            if (imei.length() > 0) fw.append(":"+imei);
            fw.append("] ");
        }
        fw.append(st);
        if (fw.at(fw.length() - 1) != '\n') fw.append("\n");
        fil.write(fw.toLocal8Bit(), fw.length());
        fil.close();
    }
}
//----------------------------------------------------------------------------------------------------------------------------
uint16_t MainWindow::ks(uint8_t *uk, int len)
{
uint16_t crc16 = 0, word;
int i;

    for (i = 0; i < len; i++) {
        word = *(uint16_t *)(uk + i);
        crc16 = UPDC16(word, crc16);
    }

    return crc16;
}
//----------------------------------------------------------------------------------------------------------------------------
int MainWindow::MakeAvlPacket(void *buf, int command_id, char *par)
{
int ret = -1;
uint8_t *uk = NULL, *tmp = NULL, *tp = NULL;
uint8_t cnt = 0x01, ct, type = 5;
uint32_t alen, len;
uint32_t tlen, crc = 0, dl = 0, cmd_len = 0;
s_avl_cmd *hdr = NULL;
char cds[max_cmd_len] = {0};

    if ((!buf) || (command_id < 0) || (command_id >= max_cmds)) return ret;

    sprintf(cds,"%s%s\r\n", &cmds[command_id][0], par);


    if (dbg > 2) {
        QString qcds; qcds.append(cds);
        LogSave(__func__, qcds+"\n", 1);
    }

    len = strlen(cds);

    alen = sizeof(s_avl_cmd) + ((len + 5) * cnt) + 5;

    tmp = (uint8_t *)calloc(1, alen + 1);
    if (tmp) {
        hdr = (s_avl_cmd *)tmp;
        hdr->codec_id = 0x0c;//CodecID=12
        hdr->cmd_cnt = cnt;
        uk = tmp + sizeof(s_avl_cmd);//uk to command_type
        ct = 0;
        while (ct < cnt) {
            *uk = type; uk++;
            dl = len;
            cmd_len = htonl(dl);
            memcpy(uk, &cmd_len, 4); uk += 4;
            memcpy(uk, (uint8_t *)cds, dl); uk += dl;
            ct++;
        }
        *uk = cnt;

        uk ++;//uk to crc
        tp = tmp + (sizeof(uint32_t) << 1);
        tlen = uk - tp; hdr->tlen = htonl(tlen);
        crc = htonl( ks(tp, uk - tp) );
        memcpy(uk, &crc, sizeof(uint32_t)); uk += sizeof(uint32_t);

        ret = uk - tmp;

        if (ret > 0) {
            memcpy((uint8_t *)buf, tmp, ret);
            memcpy(par, cds, len);
        }

        free(tmp);
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------------
char *MainWindow::ShowTime(time_t *ct, char *bf)
{
    asctime_r(gmtime(ct), bf);

    bf[strlen(bf)-1] = '\0';

    return &bf[0];
}
//----------------------------------------------------------------------------------------------------------------------------
char *MainWindow::io_name(uint8_t id, char *st)//for FMB630 or FM6320 only
{
    if (!st) return NULL;

    switch (id) {
        case 1  : sprintf(st,"DIN1"); break;
        case 2  : sprintf(st,"DIN2"); break;
        case 3  : sprintf(st,"DIN3"); break;
        case 4  : sprintf(st,"DIN4"); break;
        case 5  : sprintf(st,"DallasTempID5"); break;
        case 6  : sprintf(st,"DallasTemp5"); break;
        case 7  : sprintf(st,"DallasTempID6"); break;
        case 8  : sprintf(st,"DallasTemp6"); break;
        case 9  : sprintf(st,"AIN1"); break;
        case 10 : sprintf(st,"AIN2"); break;
        case 11 : sprintf(st,"AIN3"); break;
        case 12 : sprintf(st,"CAN_ProgramNumber"); break;
        case 13 : sprintf(st,"CAN_ModuleID"); break;
        case 14 : sprintf(st,"CAN_EngineWorkTime"); break;
        case 15 : sprintf(st,"CAN_EngineWorkTime(counted)"); break;
        case 16 : sprintf(st,"CAN_TotalMileage"); break;
        case 17 : sprintf(st,"CAN_FuelConsumed"); break;
        case 18 : sprintf(st,"CAN_FuelRate"); break;
        case 19 : sprintf(st,"CAN_AdBlueLevel(p)"); break;
        case 20 : sprintf(st,"CAN_AdBlueLevel(l)"); break;
        case 21 : sprintf(st,"GSM_slevel"); break;
        case 22 : sprintf(st,"Profile"); break;
        case 23 : sprintf(st,"CAN_EngineLoad"); break;
        case 24 : sprintf(st,"Speed"); break;
        case 25 : sprintf(st,"CAN_EngineTemp"); break;
        case 26 : sprintf(st,"CAN_Axle1Load"); break;
        case 27 : sprintf(st,"CAN_Axle2Load"); break;
        case 28 : sprintf(st,"CAN_Axle3Load"); break;
        case 29 : sprintf(st,"CAN_Axle4Load"); break;
        case 30 : sprintf(st,"CAN_Speed"); break;
        case 31 : sprintf(st,"CAN_Accelerator"); break;
        case 32 : sprintf(st,"CAN_Axle5Load"); break;
        case 33 : sprintf(st,"CAN_FuelConsumed"); break;
        case 34 : sprintf(st,"CAN_FuelLevel(l)"); break;
        case 35 : sprintf(st,"CAN_EngineRPM"); break;
        case 36 : sprintf(st,"CAN_TotalMileage"); break;
        case 37 : sprintf(st,"CAN_FuelLevel(p)"); break;
        case 38 : sprintf(st,"CAN_ControlStateFlags"); break;
        case 39 : sprintf(st,"CAN_AgriculturalMachineryFlags"); break;
        case 40 : sprintf(st,"CAN_HarvestingTime"); break;
        case 41 : sprintf(st,"CAN_AreaofHarvest"); break;
        case 42 : sprintf(st,"CAN_MowingEfficiency"); break;
        case 43 : sprintf(st,"CAN_GrainMownVolume"); break;
        case 44 : sprintf(st,"CAN_GrainMoisture"); break;
        case 45 : sprintf(st,"CAN_HarvestingDrumRPM"); break;
        case 46 : sprintf(st,"CAN_GapUnderHarvestingDrum"); break;
        case 47 : sprintf(st,"CAN_SecurityStateFlags"); break;
        case 48 : sprintf(st,"CAN_TachoTotalVehicleDistance"); break;
        case 49 : sprintf(st,"CAN_TripDistance"); break;
        case 50 : sprintf(st,"DOUT3"); break;
        case 51 : sprintf(st,"DOUT4"); break;
        case 52 : sprintf(st,"CAN_TachoVehicleSpeed"); break;
        case 53 : sprintf(st,"CAN_TachoDriverCardPresence"); break;
        case 54 : sprintf(st,"CAN_Driver1States"); break;
        case 55 : sprintf(st,"CAN_Driver2States"); break;
        case 56 : sprintf(st,"CAN_Driver1ContinuousDrivingTime"); break;
        case 57 : sprintf(st,"CAN_Driver2ContinuousDrivingTime"); break;
        case 58 : sprintf(st,"CAN_Driver1CumulativeBreakTime"); break;
        case 59 : sprintf(st,"CAN_Driver2CumulativeBreakTime"); break;
        case 60 : sprintf(st,"CAN_Driver1DurationOfSelectedActivity"); break;
        case 61 : sprintf(st,"CAN_Driver2DurationOfSelectedActivity"); break;
        case 62 : sprintf(st,"DallasTempID1"); break;
        case 63 : sprintf(st,"DallasTempID2"); break;
        case 64 : sprintf(st,"DallasTempID3"); break;
        case 65 : sprintf(st,"DallasTempID4"); break;
        case 66 : sprintf(st,"ExternalVoltage"); break;
        case 67 : sprintf(st,"BatteryVoltage"); break;
        case 68 : sprintf(st,"BatteryCurrent"); break;
        case 69 : sprintf(st,"GNSS_status"); break;
        case 70 : sprintf(st,"PCBTemp"); break;
        case 71 : sprintf(st,"GNSSStatus"); break;
        case 72 : sprintf(st,"DallasTemp1"); break;
        case 73 : sprintf(st,"DallasTemp2"); break;
        case 74 : sprintf(st,"DallasTemp3"); break;
        case 75 : sprintf(st,"DallasTemp4"); break;
        case 76 : sprintf(st,"FuelCounter"); break;
        case 77 : sprintf(st,"CAN_Driver2CumulativeDrivingTime"); break;
        case 78 : sprintf(st,"iButtonID"); break;

        case 108: sprintf(st,"CAN_Driver2IDHigh"); break;

        case 140: sprintf(st,"CAN_Driver2IDLow"); break;
        case 141: sprintf(st,"CAN_BatteryTemp"); break;
        case 142: sprintf(st,"CAN_BatteryLevel(p)"); break;
        case 143: sprintf(st,"CAN_DoorStatus"); break;
        case 144: sprintf(st,"SD_status"); break;
        case 145: sprintf(st,"Manual_1CAN1"); break;
        case 146: sprintf(st,"Manual_1CAN2"); break;
        case 147: sprintf(st,"Manual_1CAN3"); break;
        case 148: sprintf(st,"Manual_1CAN4"); break;
        case 149: sprintf(st,"Manual_1CAN5"); break;
        case 150: sprintf(st,"Manual_1CAN6"); break;
        case 151: sprintf(st,"Manual_1CAN7"); break;
        case 152: sprintf(st,"Manual_1CAN8"); break;
        case 153: sprintf(st,"Manual_1CAN9"); break;
        case 154: sprintf(st,"Manual_1CAN10"); break;
        case 155 : sprintf(st,"Geofence_zone_01"); break;
        case 156 : sprintf(st,"Geofence_zone_02"); break;
        case 157 : sprintf(st,"Geofence_zone_03"); break;
        case 158 : sprintf(st,"Geofence_zone_04"); break;
        case 159 : sprintf(st,"Geofence_zone_05"); break;
        case 160 : sprintf(st,"Geofence_zone_06"); break;
        case 161 : sprintf(st,"Geofence_zone_07"); break;
        case 162 : sprintf(st,"Geofence_zone_08"); break;
        case 163 : sprintf(st,"Geofence_zone_09"); break;
        case 164 : sprintf(st,"Geofence_zone_10"); break;
        case 165 : sprintf(st,"Geofence_zone_11"); break;
        case 166 : sprintf(st,"Geofence_zone_12"); break;
        case 167 : sprintf(st,"Geofence_zone_13"); break;
        case 168 : sprintf(st,"Geofence_zone_14"); break;
        case 169 : sprintf(st,"Geofence_zone_15"); break;
        case 170 : sprintf(st,"Geofence_zone_16"); break;
        case 171 : sprintf(st,"Geofence_zone_17"); break;
        case 172 : sprintf(st,"Geofence_zone_18"); break;
        case 173 : sprintf(st,"Geofence_zone_19"); break;
        case 174 : sprintf(st,"Geofence_zone_20"); break;
        case 175 : sprintf(st,"AutoGeofence"); break;
        case 176 : sprintf(st,"CAN_DTCErrors"); break;
        case 177 : sprintf(st,"CAN_DTCCodes"); break;
        case 178 : sprintf(st,"NetworkType"); break;
        case 179 : sprintf(st,"DOUT1"); break;
        case 180 : sprintf(st,"DOUT2"); break;
        case 181 : sprintf(st,"GNSS_PDOP"); break;
        case 182 : sprintf(st,"GNSS_HDOP"); break;

        case 199 : sprintf(st,"Odometer"); break;
        case 200 : sprintf(st,"SleepMode"); break;
        case 201 : sprintf(st,"LLS1_FuelLevel"); break;
        case 202 : sprintf(st,"LLS1_FuelTemp"); break;
        case 203 : sprintf(st,"LLS2_FuelLevel"); break;
        case 204 : sprintf(st,"LLS2_FuelTemp"); break;
        case 205 : sprintf(st,"CellID"); break;
        case 206 : sprintf(st,"LAC"); break;
        case 207 : sprintf(st,"RFID_ID_COM1"); break;
        case 208 : sprintf(st,"Acceleration"); break;
        case 209 : sprintf(st,"Decelaration"); break;
        case 210 : sprintf(st,"LLS3_FuelLevel"); break;
        case 211 : sprintf(st,"LLS3_FuelTemp"); break;
        case 212 : sprintf(st,"LLS4_FuelLevel"); break;
        case 213 : sprintf(st,"LLS4_FuelTemp"); break;
        case 214 : sprintf(st,"LLS5_FuelLevel"); break;
        case 215 : sprintf(st,"LLS5_FuelTemp"); break;
        case 216 : sprintf(st,"TotalOdometer"); break;//sprintf(st,"Manual_2CAN1"); break;
        case 217 : sprintf(st,"RFID_ID_COM2"); break;//sprintf(st,"Manual_2CAN2"); break;
        case 218 : sprintf(st,"IMSI"); break;//sprintf(st,"Manual_2CAN3"); break;
        case 219 : sprintf(st,"CC_ID_1"); break;//sprintf(st,"Manual_2CAN4"); break;
        case 220 : sprintf(st,"CC_ID_2"); break;//sprintf(st,"Manual_2CAN5"); break;
        case 221 : sprintf(st,"CC_ID_3"); break;//sprintf(st,"Manual_2CAN6"); break;
        case 222 : sprintf(st,"DriverCard1IssuingMemberState"); break;//sprintf(st,"Manual_2CAN7"); break;
        case 223 : sprintf(st,"DriverCard2IssuingMemberState"); break;//sprintf(st,"Manual_2CAN8"); break;

        case 226 : sprintf(st,"CAN_CNGStatus"); break;
        case 227 : sprintf(st,"CAN_CNGUsed"); break;
        case 228 : sprintf(st,"CAN_CNGLevel"); break;
        case 229 : sprintf(st,"CAN_Driver1IDHigh"); break;
        case 230 : sprintf(st,"CAN_Driver1IDLow"); break;
        case 231 : sprintf(st,"VehicleRegistrationNumberPart1"); break;
        case 232 : sprintf(st,"VehicleRegistrationNumberPart2"); break;
        case 233 : sprintf(st,"VinNumberPart1"); break;
        case 234 : sprintf(st,"VinNumberPart2"); break;
        case 235 : sprintf(st,"VinNumberPart3"); break;
        case 236 : sprintf(st,"AxisX"); break;
        case 237 : sprintf(st,"AxisY"); break;
        case 238 : sprintf(st,"AxisZ"); break;
        case 239 : sprintf(st,"Ignition"); break;
        case 240 : sprintf(st,"MovementSensor"); break;
        case 241 : sprintf(st,"GSMOperator"); break;
        case 242 : sprintf(st,"DataLimitHit"); break;
        case 243 : sprintf(st,"ExcessiveIdlingDetected"); break;
        case 244 : sprintf(st,"CameraImageGenerated"); break;

        case 246 : sprintf(st,"TowingDetection"); break;
        case 247 : sprintf(st,"CrashDetection"); break;
        case 248 : sprintf(st,"GeofenceZoneOverspeeding"); break;
        case 249 : sprintf(st,"JammingDetection"); break;
        case 250 : sprintf(st,"Trip"); break;
        case 251 : sprintf(st,"Immobilizer"); break;
        case 252 : sprintf(st,"AuthorizedDriving"); break;
        case 253 : sprintf(st,"ECODrivingType"); break;
        case 254 : sprintf(st,"ECODrivingValue"); break;
        case 255 : sprintf(st,"OverSpeeding"); break;
            default : sprintf(st,"%d",id);
    }

    return st;
}
//----------------------------------------------------------------------------------------------------------------------------
int MainWindow::ParseResp(QString *ack, char *out)
{
int ret = -1, param = -1, relay = -1, val = 0xffff0000;
QJsonParseError err;
QByteArray buf;
char tp[max_cmd_len] = {0};
int arr_len = 0, ct = 0;


    buf.append(*ack);
    QJsonDocument jdoc= QJsonDocument::fromJson(buf , &err);
    if (err.error != QJsonParseError::NoError) {
        if (dbg) {
            QString stx = "Command parser error : " + err.errorString() + "\n";
            LogSave(__func__, stx, 1);
            statusBar()->clearMessage(); statusBar()->showMessage(stx);
        }
        return ret;
    }
    QJsonObject *obj = new QJsonObject(jdoc.object());
    if (!obj) {
        if (dbg) {
            QString stx = "Command parser error : " + err.errorString() + "\n";
            LogSave(__func__, stx, 1);
            statusBar()->clearMessage(); statusBar()->showMessage(stx);
        }
        return ret;
    }

    QJsonValue tmp = obj->value(srv_command);
    if (!tmp.isUndefined()) {
        ret = tmp.toInt() & 0xff;//cmd_id
        tmp = obj->value(srv_command_param);//"param";
        if (tmp.isUndefined()) tmp = obj->value(srv_command_relay);//"relay";
        if (!tmp.isUndefined()) {
            if (tmp.isString()) {//is_string
                sprintf(tp+strlen(tp)," %s", (char *)tmp.toString().data());
            } else {//is_integer
                param = tmp.toInt();
                relay = param & 0xff;
                if (param > 0) {
                    if (param != 9999) sprintf(tp," %d", param);
                    tmp = obj->value(srv_command_value);//"value";
                    if (tmp.isUndefined()) tmp = obj->value(srv_command_time);//"time";
                    if (!tmp.isUndefined()) {
                        if (tmp.isString()) {
                            sprintf(tp+strlen(tp)," %s", (char *)tmp.toString().data());
                        } else if (tmp.isDouble()) {
                            sprintf(tp+strlen(tp)," %f", tmp.toDouble());
                        } else if (tmp.isArray()) {
                            arr_len = tmp.toArray().size();
                            if (arr_len <= 5) {
                                ct = 0;
                                sprintf(tp+strlen(tp)," ");
                                while (ct < arr_len) {
                                    sprintf(tp+strlen(tp), "%s", (char *)tmp.toArray().at(ct).toString().data());
                                    if (++ct != arr_len) sprintf(tp+strlen(tp), ",");
                                }
                            } else ret = -1;
                        } else {//is_integer
                            val = tmp.toInt();
                            sprintf(tp+strlen(tp)," %d", val);
                            val = (val & 0xffff) << 16;
                        }

                    }
                }
            }
        }
    }
    if (strlen(tp)) memcpy(out, tp, strlen(tp));

    if (obj) delete obj;

    if ((ret == 1) || (ret == 2)) {//for relay #5, 6, 7, 8
        if ((relay >= 5) && (relay <= 8)) {
            relay -= 4;// convert relay number
            ret += 32;//get command {"command":33} -"SET_ON", // set ON relay on extended module (4 relay #5,6,7,8)
        }
    }

    relay <<= 8;

    ret |= (val | relay);

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------------
int MainWindow::CalcFuel(uint16_t mv, uint8_t ign)
{
int ret = -1;
uint16_t adc = mv;


    if (ign) {
             if (adc <= 750) ret = 42;
        else if (adc <= 825) ret = 41;
        else if (adc <= 900) ret = 40;
        else if (adc <= 966) ret = 39;
        else if (adc <= 1032) ret = 38;
        else if (adc <= 1100) ret = 37;
        else if (adc <= 1162) ret = 36;
        else if (adc <= 1224) ret = 35;
        else if (adc <= 1286) ret = 34;
        else if (adc <= 1350) ret = 33;
        else if (adc <= 1450) ret = 32;
        else if (adc <= 1550) ret = 31;
        else if (adc <= 1650) ret = 30;
        else if (adc <= 1734) ret = 29;
        else if (adc <= 1817) ret = 28;
        else if (adc <= 1900) ret = 27;
        else if (adc <= 1966) ret = 26;
        else if (adc <= 2032) ret = 25;
        else if (adc <= 2100) ret = 24;
        else if (adc <= 2184) ret = 23;
        else if (adc <= 2267) ret = 22;
        else if (adc <= 2350) ret = 21;
        else if (adc <= 2416) ret = 20;
        else if (adc <= 2482) ret = 19;
        else if (adc <= 2550) ret = 18;
        else if (adc <= 2600) ret = 17;
        else if (adc <= 2650) ret = 16;
        else if (adc <= 2700) ret = 15;
        else if (adc <= 2733) ret = 14;
        else if (adc <= 2766) ret = 13;
        else if (adc <= 2800) ret = 12;
        else if (adc <= 2838) ret = 11;
        else if (adc <= 2875) ret = 10;
        else if (adc <= 2912) ret = 9;
        else if (adc <= 2950) ret = 8;
        else if (adc <= 3000) ret = 7;
        else if (adc <= 3050) ret = 6;
        else if (adc <= 3100) ret = 5;
        else if (adc <= 3150) ret = 4;
        else if (adc <= 3200) ret = 3;
        else if (adc <= 3250) ret = 2;
        else ret = 0;
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------------
QJsonObject *MainWindow::ConvertStrToJsonObject(char *st, int *kda)
{
const int max_str = 256;
int dl = 0, cmd, loop = 1, cnt = 0;
char tp[max_str], val[max_str];
uint8_t err = 0;
QJsonObject *ret = NULL;
char *uki = NULL, *uks = NULL, *uke = NULL, *ukend = NULL;

    if (!st) return ret;
    else {
        dl = strlen(st);
        if (!dl) return ret;
    }
    cmd = *kda;

    if (dbg >= 2) {
        QString stx;
        stx.sprintf("IN (%d):cmd=%d text=%s\n", dl, cmd, st);
        LogSave(__func__, stx, 1);
    }

    switch (cmd) {
        case 16://getstatus //Modem Status information
        case  0://getgps //Current GPS data and time
        case 20://getver // Device / Modem / Code version information
        case 15://getio // Readout digital inputs and outputs
        case 22://lvcangetprog // get program number
        case 23://lvcangetinfo //Get LV-CAN info
        case 24://getparam # // # - paramID
        case 25://setparam # @ // # - paramID, @ - value
        case 27://lvcansetprog # // # - program number (112 for Fiat Stilo)
        case 28://readops # //# - 1,2,3
            if (strstr(st,"WARNING")) { err=1; break; }
            uks = st; ukend = st + strlen(st) - 1;
            ret = new QJsonObject();
            while (loop) {
                cnt++;
                uke = strchr(uks, ':');
                if (uke) {
                    if (*(uke+1) == ' ') uke++;
                    memset(tp, 0, sizeof(tp));
                    memcpy(tp, uks, uke-uks);
                    dl = strlen(tp);
                    if (!dl) sprintf(tp,"UnknownParam_%d", cnt);
                    else {
                        if (tp[dl - 1] == ':') tp[dl - 1] = '\0';
                        dl = strlen(tp);
                        if (!dl) sprintf(tp,"UnknownParam_%d", cnt);
                    }
                    uks = uke + 1;
                    uke = strchr(uks,' ');
                    if (uke == NULL) uke = strchr(uks,'\0');
                    if (uke) {
                        memset(val, 0, sizeof(val));
                        dl = uke - uks; if (dl >= max_str) dl = max_str - 1;
                        memcpy(val, uks, dl);
                        ret->insert(tp, QJsonValue(val));
                        uks = uke + 1;
                    } else { err = 1; break; }
                } else { err = 1; break; }
                if (uks >= ukend) loop = 0;
            }
        break;
        case 33: case 34: case 35: case 48:
            ret = new QJsonObject();
            err = 1;
            uks = st;
            uke = strchr(uks, ':');
            if (uke) {
                memset(tp, 0, sizeof(tp));
                memcpy(tp, uks, uke - uks);
                if (strlen(tp)) {
                    uki = &tp[strlen(tp)-1]; if ( *uki == ' ') *uki = '\0';
                    ret->insert("Cmd", QJsonValue(tp));
                    uks = uke + 1; if (*uks == ' ') uks++;
                    uke = strchr(uks, '\0');
                    if (uke) {
                        memset(val, 0, sizeof(val));
                        memcpy(val, uks, uke - uks);
                        uki = strstr(val, "\r\n"); if (uki) *uki = '\0';
                        ret->insert("Result", QJsonValue(val));
                        err = 0;
                    }
                }
            }
        break;
    }//switch (cmd)

    if ((err) && (ret)) {
        delete ret;
        ret = NULL;
    }

    if (!err && ret && (cmd == 15)) {
        QJsonValue tmp;
        tmp = ret->value("DI1"); if (!tmp.isUndefined()) pins.din1 = tmp.toString().toInt();
        tmp = ret->value("DI2"); if (!tmp.isUndefined()) pins.din2 = tmp.toString().toInt();
        tmp = ret->value("DI3"); if (!tmp.isUndefined()) pins.din3 = tmp.toString().toInt();
        tmp = ret->value("DI4"); if (!tmp.isUndefined()) pins.din4 = tmp.toString().toInt();

        tmp = ret->value("DO1"); if (!tmp.isUndefined()) pins.dout1 = tmp.toString().toInt();
        tmp = ret->value("DO2"); if (!tmp.isUndefined()) pins.dout2 = tmp.toString().toInt();
        tmp = ret->value("DO3"); if (!tmp.isUndefined()) pins.dout3 = tmp.toString().toInt();
        tmp = ret->value("DO4"); if (!tmp.isUndefined()) pins.dout4 = tmp.toString().toInt();

        tmp = ret->value("AIN1"); if (!tmp.isUndefined()) pins.ain1 = tmp.toString().toInt();
        tmp = ret->value("AIN2"); if (!tmp.isUndefined()) pins.ain2 = tmp.toString().toInt();
        tmp = ret->value("AIN3"); if (!tmp.isUndefined()) pins.ain3 = tmp.toString().toInt();

        UpdatePins();
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------------
int MainWindow::parse_data_from_dev(char *lin, int dline, QJsonObject *js, int *kom)
{
int ret = 0, el_dl;
char stx[max_tmp_len] = {0};
char sta[max_tmp_len] = {0};
uint8_t *st = NULL;
char *stm = NULL;
s_hdr_pack_bin *h_bin = NULL;
s_pack_bin *p_bin = NULL;
s_gps_pack_bin *g_bin = NULL;
s_hdr_bin_ack *h_bin_ack = NULL;
float lat, lon;
int intm, delitel, more = 0, len = 0, i;
uint8_t *_end = NULL, *uk = NULL;
const char *ptr;
char stz[64] = {0};
time_t tim;
uint8_t io_elem, llen, ind, cmd_id, shi, dol, n_cnt = 0;
uint32_t dword = 0;
uint8_t byte;
uint16_t word;
uint64_t dint8;
float latit, longit, rdat;
QJsonObject *jpack = NULL, *jarr_io = NULL, *jans = NULL, *jsta = NULL;
QJsonArray *jarr = NULL;
uint8_t ign = 0, door = 1, trunk = 1, skp = 1, hood = 0, tah = 0;
uint16_t fuel = 0;
short temp_pcb;
char *uki = NULL;
QString qstx, qstz;



    if ((dline < (int)sizeof(s_hdr_pack_bin)) || (dline >= buf_size) || !js) {
        qstx.sprintf("Parse AVL error or ack for command -> len=%d\n", dline);
        LogSave(__func__, qstx, 1);
        return -1;
    } else {
        qstx.sprintf("Parse AVL with len=%d from dev_type FMB630/FM6320\n", dline);
        LogSave(__func__, qstx, 1);
    }

    if ((codec_id != 8) && (codec_id != 12) && (codec_id != 13)) {
        qstx.sprintf("Error codec_id = %u\n", codec_id);
        LogSave(__func__, qstx, 1);
        return -1;
    }

    g_bin = (s_gps_pack_bin *)calloc(1, sizeof(s_gps_pack_bin));
    if (!g_bin) return -1;

    st = (uint8_t *)calloc(1, dline + 1);
    if (!st) {
        if (g_bin) free(g_bin);
        return -1;
    }

    memcpy(st, lin, dline);
    uk = &st[0];

    h_bin = (s_hdr_pack_bin *)&st[0];//for codec.id=8
    h_bin_ack = (s_hdr_bin_ack *)&st[0];//for codec.id=12

    if (dbg >= 2) {
        memset(sta, 0, sizeof(sta));
        if (codec_id == 8) len = sizeof(s_hdr_pack_bin); else len = sizeof(s_hdr_bin_ack);
        for (i = 0; i < len; i++) sprintf(sta+strlen(sta),"%02X", *(uk+i));
        sprintf(stx,"HDR(%d):%s\nCodecID:\t%d\t%x\nTotalPackets:\t%d\t%x\n",
                        len,
                        sta,
                        h_bin->codec_id, h_bin->codec_id,
                        h_bin->numbers_pack, h_bin->numbers_pack);
        if ((codec_id == 12) || (codec_id == 13)) sprintf(stx+strlen(stx),"PacketType:\t%d\t%x\n", h_bin_ack->type, h_bin_ack->type);
        qstx.clear(); qstx.append(stx);
        LogSave(NULL, qstx, 0);
    }

    js->insert("CodecID",  QJsonValue(h_bin->codec_id));
    js->insert("TotalPackets", QJsonValue(h_bin->numbers_pack));
    jarr = new QJsonArray();

    /*---------------------------------------------------------------------------------------*/
    if ((codec_id == 12) || (codec_id == 13)) {//answer for command
        if (dbg >= 2) LogSave(NULL, "DATA:\n", 0);
        n_cnt = 0; i = 0;
        len = ntohl(h_bin_ack->len);
        uk += sizeof(s_hdr_bin_ack);
        if (codec_id == 13) {
            memcpy(&dword, uk, 4);
            dword = ntohl(dword);
            uk += 4; len -= 4;
        }
        while (n_cnt < h_bin_ack->numbers_pack) {
            n_cnt++;
            stm = (char *)calloc(1, len+1);
            if (stm) {
                memcpy(stm, uk, len);
                if ((strstr(stm,"Error")) || (strstr(stm,"ERROR"))) i = 1;
                jans = ConvertStrToJsonObject(stm, kom);
                if (jans) {
                    jarr->append(*jans);
                    delete jans; jans = NULL;
                } else {
                    uki = strstr(stm, "\r\n"); if (uki) { if (uki != stm) *uki = '\0'; }
                    jarr->append(QJsonValue(stm));
                }
                if (dbg >= 2) {
                    memset(sta, 0, sizeof(sta));
                    sprintf(sta,"[%d]:", len);
                    qstx.clear(); qstx.append(sta);
                    LogSave(NULL, qstx + "\n", 0);
                }
                if (n_cnt < h_bin_ack->numbers_pack) {
                    uk += len;
                    memcpy(&len, uk, sizeof(uint32_t));//get len for next pack
                    len = ntohl(len);
                }
                free(stm); stm = NULL;
            }
        }

        if (jarr) {
            if (codec_id == 13) js->insert("TimeStamp", QJsonValue((qint32)dword));
            js->insert("DATA", QJsonValue(*jarr));
            js->insert("Status", QJsonValue(i));
            delete jarr; jarr = NULL;
        }
        if (st) free(st);
        if (g_bin) free(g_bin);

        return 0;
    }
    /*-----------------------------------------------------------------------------------------*/

    len = 0;
    _end = st + dline;
    uk +=  sizeof(s_hdr_pack_bin);
    len += sizeof(s_hdr_pack_bin);

    more = 0;
    while (more < h_bin->numbers_pack) {
        len += sizeof(s_pack_bin);
        if (dline >= len) {
            //-------------------------------  HEADER PACK  ------------------------------------------------------
            p_bin = (s_pack_bin *)uk;
            dint8 = be64toh(p_bin->ts);
            tim = dint8 / 1000;
            if (p_bin->prio < total_prio) ptr = &prio_name[p_bin->prio][0]; else ptr = unknown;
            if (dbg >= 2) {
                memset(stz, 0, sizeof(stz));
                memset(sta, 0, sizeof(sta));
                for (i = 0; i < (int)sizeof(s_pack_bin); i++) sprintf(sta+strlen(sta),"%02X", *(uk+i));
                sprintf(stx,"[%d] PACK(%d):%s\nTimeStamp:\t%s\t0x%" PRIX64 "\t%" PRIu64 "\t%u\nPriority:\t%x\t%s\n",
                        more + 1,
                        (int)sizeof(s_pack_bin),
                        sta,
                        ShowTime(&tim, stz),
                        dint8, dint8, (uint32_t)tim,
                        p_bin->prio,
                        ptr);
                qstx.clear(); qstx.append(stx);
                LogSave(NULL, qstx, 0);
            }
            jpack = new QJsonObject(); //jpack = json_object();
            jpack->insert("TimeStamp", QJsonValue((qint32)tim));
            jpack->insert("Priority", QJsonValue((const char *)ptr));
            //-------------------------------   GPS  ------------------------------------------------------
            memset((uint8_t *)g_bin, 0, sizeof(s_gps_pack_bin));
            g_bin->latitude = ntohl(p_bin->latitude);   latit = g_bin->latitude;   latit /= 10000000;
            g_bin->longitude = ntohl(p_bin->longitude); longit = g_bin->longitude; longit /= 10000000;
            g_bin->altitude = ntohs(p_bin->altitude);
            g_bin->angle = ntohs(p_bin->angle);
            g_bin->sattelites = p_bin->sattelites;
            g_bin->speed = ntohs(p_bin->speed);

            if (g_bin->latitude & 0x80000000) shi = 1;     // южная широта
                                         else shi = 0;     // северная широта
            if (g_bin->longitude & 0x80000000) dol = 1;    // западная долгота
                                          else dol = 0;    // восточная долгота
            delitel = 10000000;
            lat = g_bin->latitude; lat /= delitel;    g_bin->latitude_grad = lat;//grad
            intm = (lat - g_bin->latitude_grad) * delitel; lat = intm; lat /= delitel; lat *= 60; g_bin->latitude_min = lat;//min
            delitel /= 10;
            intm = (lat - g_bin->latitude_min) * delitel; lat = intm; lat /= delitel; lat *= 60; g_bin->latitude_sec = lat;//sec

            delitel = 10000000;
            lon = g_bin->longitude; lon /= delitel;    g_bin->longitude_grad = lon;//grad
            intm = (lon - g_bin->longitude_grad) * delitel; lon = intm; lon /= delitel; lon *= 60; g_bin->longitude_min = lon;//min
            delitel /=10;
            intm = (lon - g_bin->longitude_min) * delitel; lon = intm; lon /= delitel; lon *= 60; g_bin->longitude_sec = lon;//sec

            if (dbg >= 2) {
                memset(sta, 0, sizeof(sta));
                     if (g_bin->angle ==   0) sprintf(sta," North");
                else if (g_bin->angle == 270) sprintf(sta," West");
                else if (g_bin->angle ==  90) sprintf(sta," East");
                else if (g_bin->angle == 180) sprintf(sta," South");
                else if (g_bin->angle == 315) sprintf(sta," North-West");
                else if (g_bin->angle ==  45) sprintf(sta," North-East");
                else if (g_bin->angle == 225) sprintf(sta," South-West");
                else if (g_bin->angle == 135) sprintf(sta," South-East");
                sprintf(stx, "GPS:\n%s Latitude:\t%d%c%d\"%d'\t%f\n%s Longitude:\t%d%c%d\"%d'\t%f\nAltitude:\t%d meters\nAngel:\t\t%d%c\t%s\nSattelites:\t%d\nSpeed:\t\t%d km/h\n",
                             lat_name[shi], g_bin->latitude_grad, gradus, g_bin->latitude_min, g_bin->latitude_sec, latit,
                             lon_name[dol], g_bin->longitude_grad, gradus, g_bin->longitude_min, g_bin->longitude_sec, longit,
                             g_bin->altitude,
                             g_bin->angle, gradus, sta,
                             g_bin->sattelites,
                             g_bin->speed);
                qstx.clear(); qstx.append(stx);
                LogSave(NULL, qstx, 0);
            }
            jpack->insert("Latitude",  QJsonValue(latit));
            jpack->insert("Longitude", QJsonValue(longit));
            jpack->insert("Altitude",  QJsonValue(g_bin->altitude));
            jpack->insert("Angle",     QJsonValue(g_bin->angle));
            jpack->insert("Sattelites",QJsonValue(g_bin->sattelites));
            jpack->insert("Speed",     QJsonValue(g_bin->speed));
            //-----------------------  show gps data  --------------------------------------------
            qstx.clear(); ui->latitude->setText(qstx.setNum(latit, 'f', 8));
            qstx.clear(); ui->longitude->setText(qstx.setNum(longit, 'f', 8));
            ui->altitude->setText(QString::number(g_bin->altitude, 10));
            ui->angle->setText(QString::number(g_bin->angle, 10));
            ui->sat->setText(QString::number(g_bin->sattelites, 10));
            ui->speed->setText(QString::number(g_bin->speed, 10));
            //-----------------------   IO  ------------------------------------------------------
            if (dbg >= 2) {
                sprintf(stx, "IO: (ID_event:\t%u, Total_elem:\t%u)\n", p_bin->id_event, p_bin->total_elem);
                qstx.clear(); qstx.append(stx);
                LogSave(NULL, qstx, 0);
            }

            jpack->insert("EventID",      QJsonValue(p_bin->id_event));
            jpack->insert("TotalElements",QJsonValue(p_bin->total_elem));

            uk += sizeof(s_pack_bin);

            io_elem = p_bin->total_elem;
            el_dl = 1;

            if (io_elem > 0) jarr_io = new QJsonObject();
            for (i = 0; i < 4; i++) {
                if (uk >= _end) break;
                llen = *(uk); uk++;
                delitel = el_dl;
                if (dbg >= 2) sprintf(stx, "IO elem [%d/%u]\n", delitel, llen);
                for (ind = 0; ind < llen; ind++) {
                    cmd_id = *(uk); uk++;
                    switch (el_dl) {
                        case 1:
                            byte = *(uk);
                            dint8 = byte;
                            switch (cmd_id) {
                                case 1://DIN1 - Ign
                                    pins.din1 = byte;
                                    ign = byte;
                                break;
                                case 2://DIN2 - Door
                                    pins.din2 = byte;
                                    door = ~byte; door &= 1;
                                break;
                                case 3://DIN3 - Trunk
                                    pins.din3 = byte;
                                    trunk = ~byte; trunk &= 1;
                                break;
                                case 4://DIN4 - Skp
                                    pins.din4 = byte;
                                    skp = byte;
                                break;
                                case 179://'DOUT1'
                                    pins.dout1 = byte;
                                break;
                                case 180://'DOUT2'
                                    pins.dout2 = byte;
                                break;
                                case 050://'DOUT3'
                                    pins.dout3 = byte;
                                break;
                                case 051://'DOUT4'
                                    pins.dout4 = byte;
                                break;
                                case 240://move
                                    pins.msensor = byte;
                                break;
                            }
                        break;
                        case 2:
                            memcpy(&word, uk, el_dl);
                            dint8 = (uint16_t)htons(word);
                            switch (cmd_id) {
                                case 9://AIN1 - fuel (ДУТ)
                                    fuel = dint8;
                                    pins.ain1 = (uint16_t)dint8;
                                break;
                                case 10://AIN2 - tach (Тахометр)
                                    pins.ain2 = (uint16_t)dint8;
                                    if (dint8 > 1000) tah = 1;//двигатель заведен
                                                 else tah = 0;//двигатель заглушен
                                break;
                                case 11://AIN3 - hood (Капот)
                                    pins.ain3 = (uint16_t)dint8;
                                    if (dint8 < 1000) hood = 1;//открыт
                                                 else hood = 0;//закрыт
                                break;
                            }
                        break;
                        case 4:
                            memcpy(&dword, uk, el_dl);
                            dint8 = (uint32_t)htonl(dword);
                        break;
                        case 8:
                            memcpy(&dint8, uk, el_dl);
                            dint8 = be64toh(dint8);
                        break;
                    }
                    uk += el_dl;
                    memset(stz, 0, sizeof(stz));
                    io_name(cmd_id, stz); qstz.clear(); qstz.append(stz);
                    switch (cmd_id) {
                        case 178: //"NetworkType"
                            jarr_io->insert(qstz, QJsonValue(net_type_name[dint8&1]));
                            if (dbg >= 2) sprintf(stx+strlen(stx),"\t[%u]:\tio(%03u)=%" PRIu64 "\t'%s'\t(%s)\n", ind + 1, cmd_id, dint8, stz, net_type_name[dint8&1]);
                        break;
                        case 70: //"PCBTemp"
                            temp_pcb = dint8; rdat = temp_pcb; rdat /= 10.0; rdat = round(rdat * 10.0); rdat /= 10.0;
                            jarr_io->insert(qstz, QJsonValue(rdat));
                            if (dbg >= 2) sprintf(stx+strlen(stx),"\t[%u]:\tio(%03u)=%" PRIu64 "\t'%s'\t(%.1f%c)\n", ind + 1, cmd_id, dint8, stz, rdat, gradus);
                        break;
                        case 9://AIN1 - fuel (ДУТ)
                            jarr_io->insert(qstz, QJsonValue((qint64)dint8));
                            if (dbg >= 2) {
                                if (ign)
                                    sprintf(stx+strlen(stx),"\t[%u]:\tio(%03u)=%" PRIu64 "\t'%s'\t%d liters\n", ind + 1, cmd_id, dint8, stz, CalcFuel(fuel, ign));
                                else
                                    sprintf(stx+strlen(stx),"\t[%u]:\tio(%03u)=%" PRIu64 "\t'%s'\n", ind + 1, cmd_id, dint8, stz);
                            }
                        break;
                        default: {
                            jarr_io->insert(qstz, QJsonValue((qint64)dint8));
                            if (dbg >= 2) sprintf(stx+strlen(stx),"\t[%u]:\tio(%03u)=%" PRIu64 "\t'%s'\n", ind + 1, cmd_id, dint8, stz);
                        }
                    }
                }
                el_dl <<= 1;
                if (dbg >= 2) {
                    qstx.clear(); qstx.append(stx);
                    LogSave(NULL, qstx, 0);
                }
                if (uk >= _end) break;
            }

            //UpdatePins();

            if (dbg >= 2) LogSave(NULL, "\n", 0);
            //--------------------   ADD STAT FLAGS   -----------------------------
            if (jsta) { delete jsta; jsta = NULL; }
            jsta = new QJsonObject();
            jsta->insert("Door",       QJsonValue(door));
            jsta->insert("Ignition",   QJsonValue(ign));
            jsta->insert("Trunk",      QJsonValue(trunk));
            jsta->insert("Skp",        QJsonValue(skp));
            jsta->insert("Hood",       QJsonValue(hood));
            jsta->insert("Tachometer", QJsonValue(tah));

            if (jarr_io) {
                jarr_io->insert("Stat", QJsonValue(*jsta));
                ////json_object_set_new(jarr_io, "Fuel", json_integer(CalcFuel(fuel, ign)));
                jpack->insert("ELEMENTS", QJsonValue(*jarr_io));
                delete jarr_io; jarr_io = NULL;
            }
            delete jsta; jsta = NULL;
            if (jpack) {
                jarr->append(*jpack);
                delete jpack; jpack = NULL;
            }
            if (uk >= _end) break;
        } else break;
        more++;
    }

    UpdatePins();

    js->insert("PACKETS", QJsonValue(*jarr));
    delete jarr; jarr = NULL;

    if (st) free(st);
    if (g_bin) free(g_bin);

    return ret;
}

//----------------------------------------------------------------------------------------------------------------------------

MainWindow::TheError::TheError(int err) { code = err; }

//----------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent, int p) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port = p;
    tcpServer = NULL;
    MyError = 0;
    client = auth = false;
    imei.clear(); CliUrl.clear();
    seq_number = cmd_seq_number = 0;
    fd = -1;
    tmr_ack = 0;
    dev_wait_answer = 0;
    total_pack = total_cmd = 0;
    setWindowTitle(title + " ver. " + vers);
    ui->sending->setEnabled(false);
    ui->avto->hide();
    memset((uint8_t *)&pins, 0, sizeof(s_pins));
    tmr_sec = startTimer(1000);// 1 sec.
}
//-----------------------------------------------------------------------
MainWindow::~MainWindow()
{
    killTimer(tmr_sec);
    if (tmr_ack) killTimer(tmr_ack);
    this->disconnect();
    delete ui;
}
//-----------------------------------------------------------------------
void MainWindow::UpdatePins()
{
    if (pins.msensor) ui->avto->show(); else ui->avto->hide();

    if (pins.din1) ui->din1->setCheckState(Qt::Checked); else ui->din1->setCheckState(Qt::Unchecked);
    if (pins.din2) ui->din2->setCheckState(Qt::Checked); else ui->din2->setCheckState(Qt::Unchecked);
    if (pins.din3) ui->din3->setCheckState(Qt::Checked); else ui->din3->setCheckState(Qt::Unchecked);
    if (pins.din4) ui->din4->setCheckState(Qt::Checked); else ui->din4->setCheckState(Qt::Unchecked);

    if (pins.dout1) ui->dout1->setCheckState(Qt::Checked); else ui->dout1->setCheckState(Qt::Unchecked);
    if (pins.dout2) ui->dout2->setCheckState(Qt::Checked); else ui->dout2->setCheckState(Qt::Unchecked);
    if (pins.dout3) ui->dout3->setCheckState(Qt::Checked); else ui->dout3->setCheckState(Qt::Unchecked);
    if (pins.dout4) ui->dout4->setCheckState(Qt::Checked); else ui->dout4->setCheckState(Qt::Unchecked);

    ui->ain1->setText(QString::number(pins.ain1, 10) + mvolt);
    ui->ain2->setText(QString::number(pins.ain2, 10) + mvolt);
    ui->ain3->setText(QString::number(pins.ain3, 10) + mvolt);
}
//-----------------------------------------------------------------------
void MainWindow::ShowHideData(bool flg)
{
    if (!flg) ui->avto->hide();
    ui->l_imei->setEnabled(flg); ui->imei->setEnabled(flg);
    ui->l_cmd->setEnabled(flg); ui->cmd->setEnabled(flg);
    ui->latitude->setEnabled(flg); ui->label->setEnabled(flg);
    ui->longitude->setEnabled(flg); ui->label_2->setEnabled(flg);
    ui->altitude->setEnabled(flg); ui->label_3->setEnabled(flg);
    ui->angle->setEnabled(flg); ui->label_4->setEnabled(flg);
    ui->sat->setEnabled(flg); ui->label_5->setEnabled(flg);
    ui->speed->setEnabled(flg); ui->label_6->setEnabled(flg);
    ui->din1->setEnabled(flg);
    ui->din2->setEnabled(flg);
    ui->din3->setEnabled(flg);
    ui->din4->setEnabled(flg);
    ui->dout1->setEnabled(flg);
    ui->dout2->setEnabled(flg);
    ui->dout3->setEnabled(flg);
    ui->dout4->setEnabled(flg);
    ui->ain1->setEnabled(flg); ui->l_ain1->setEnabled(flg);
    ui->ain2->setEnabled(flg); ui->l_ain2->setEnabled(flg);
    ui->ain3->setEnabled(flg); ui->l_ain3->setEnabled(flg);
}
//-----------------------------------------------------------------------
void MainWindow::clearParam(int len)
{
    memset(to_cli, 0, sizeof(to_cli)); txdata = 0;
    memset(from_cli, 0, sizeof(from_cli)); rxdata = len; lenrecv = 0;
}
//-----------------------------------------------------------------------
void MainWindow::on_starting_clicked()
{

    clearParam(sizeof(s_imei)); faza = 0; dev_wait_answer = 0;

    tcpServer = new QTcpServer(this);

    if (tcpServer == NULL) {
        MyError |= 1;//create server object error - no memory
        throw TheError(MyError);
    }

    QString stx = "Server start, listen port " + QString::number(port, 10);

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newuser()));

    if (!tcpServer->listen(QHostAddress("0.0.0.0"), port) && !server_status) {
        stx.clear();
        stx.append("Unable to start the server : " + tcpServer->errorString() + "\n");
    } else {
        server_status = 1;
        total_pack = total_cmd = 0;
    }
    statusBar()->clearMessage();
    statusBar()->showMessage(stx);
    LogSave(__func__, stx, true);
    ui->sending->setEnabled(false);
    memset((uint8_t *)&pins, 0, sizeof(s_pins));
}
//-----------------------------------------------------------------------
void MainWindow::on_stoping_clicked()
{
    if (server_status){
        foreach(int i, SClients.keys()) {
            SClients[i]->close();
            SClients.remove(i);
        }
        tcpServer->disconnect();
        tcpServer->close();
        statusBar()->clearMessage();
        statusBar()->showMessage("Server stop.");
        LogSave(__func__, "Server stop.", true);
        server_status = 0;
        client = auth = rdy = false;
        fd = -1;
        dev_wait_answer = 0;
        ui->sending->setEnabled(false);
        ShowHideData(false);
    }
}
//-----------------------------------------------------------------------
void MainWindow::newuser()
{
    if (server_status) {
        QTcpSocket *cliSocket = tcpServer->nextPendingConnection();
        fd = cliSocket->socketDescriptor();
        QString ssock; ssock.sprintf("%d", fd);
        QString stx;
        if (!client) {
            clearParam(sizeof(s_imei)); faza = 0; dev_wait_answer = 0;
            imei.clear();
            ui->imei->clear();
            seq_number = cmd_seq_number = 0;
            total_pack = total_cmd = 0;
            cmd_id = 0;
            CliUrl.clear(); CliUrl.append(cliSocket->peerAddress().toString() + ":" + QString::number(cliSocket->peerPort(), 10));
            client = true; rdy = false;
            stx.append("New client '" + CliUrl + "' online, socket " + ssock);
            SClients[fd] = cliSocket;
            connect(SClients[fd], SIGNAL(readyRead()), this, SLOT(slotReadClient()));
            connect(this, SIGNAL(sigCliDone(QTcpSocket *, int)), this, SLOT(slotCliDone(QTcpSocket *, int)));
            connect(SClients[fd], SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotErrorClient(QAbstractSocket::SocketError)));
            connect(this, SIGNAL(sigRdyPack(int)), this, SLOT(slotRdyPack(int)));
            statusBar()->clearMessage(); statusBar()->showMessage(stx);
            ui->sending->setEnabled(true);//unblock send button
            memset((uint8_t *)&pins, 0, sizeof(s_pins));
        } else {
            stx.append("New client '" + CliUrl + "' online, socket " + ssock + ", but client already present !\n");
            cliSocket->close();
        }
        ui->textinfo->append(stx);
        LogSave(__func__, stx, true);
    }
}
//-----------------------------------------------------------------------
void MainWindow::slotReadClient()
{
QTcpSocket *cliSocket = (QTcpSocket *)sender();
QString out;
qint64 dl = 0;
QString stx;

    switch (faza) {
        case 0://wait imei from device
            if (!auth) {
                dl = cliSocket->read(from_cli + lenrecv, rxdata - lenrecv);
                lenrecv += dl;
                if (lenrecv == sizeof(s_imei)) {
                    s_imei *im = (s_imei *)from_cli;
                    imei.clear(); imei.append(im->imei);
                    ui->imei->setText(imei);
                    to_cli[0] = 0x01; cliSocket->write(to_cli, 1);
                    auth = true;
                    lenrecv = 0; memset(from_cli, 0, sizeof(from_cli)); rxdata = sizeof(s_avl_hdr);
                    faza = 1;
                    ShowHideData(auth);//true
                }
            }
        break;
        case 1://wait header pack from device
            if (auth) {
                dl = cliSocket->read(from_cli + lenrecv, rxdata - lenrecv);
                lenrecv += dl;
                if (lenrecv == sizeof(s_avl_hdr)) {
                    s_avl_hdr *hdr = (s_avl_hdr *)from_cli;
                    rxdata = ntohl(hdr->len) + 4;
                    if (rxdata > max_buf) {
                        rxdata = max_buf;
                        MyError |= 0x10;//avl_pack len > max_buf
                    }
                    lenrecv = 0; memset(from_cli, 0, sizeof(from_cli));
                    rdy = true;
                }
            } else MyError |= 4;//auth OFF
        break;
    }

    if (auth && rdy) {
        dl = cliSocket->read(from_cli + lenrecv, rxdata - lenrecv);
        lenrecv += dl;
        if (lenrecv == rxdata) {
            codec_id = from_cli[0];
            cnt_pack = from_cli[1];
            switch (codec_id) {
                case 8: total_pack++; break;
                case 12: case 13: total_cmd++; break;
            }
            uint16_t ks_in = *(uint16_t *)(from_cli + lenrecv - 2); ks_in = ntohs(ks_in);
            uint16_t ks_calc = ks((uint8_t *)from_cli, lenrecv - 4);
            QString stx = "Adr='" + CliUrl +
                          "' CS=0x" + QString::number(ks_in, 16) + "/0x" + QString::number(ks_calc, 16) +
                          " Len=" + QString::number(lenrecv, 10) +
                          " bytes, packets/commands=" + QString::number(total_pack, 10) + "/" + QString::number(total_cmd, 10);
            LogSave(__func__, stx, true);
            statusBar()->clearMessage(); statusBar()->showMessage(stx);

            //send ack : cnt_pack
            memset(to_cli, 0, 4); to_cli[3] = cnt_pack; cliSocket->write(to_cli, 4);

            emit sigRdyPack(lenrecv);

        }
    }

    if (MyError > 0) {
        slotCliDone(cliSocket, 1);
        throw TheError(MyError);
    }

}
//-----------------------------------------------------------------------
void MainWindow::slotCliDone(QTcpSocket *cli, int prn)
{
    if (cli->isOpen()) {
        if (prn) {
                QString stx = "Close client, socket " + QString::number(fd, 10) + ".\n";
                ui->textinfo->append(stx);
                LogSave(__func__, stx, true);
                statusBar()->clearMessage(); statusBar()->showMessage(stx);
        }
        disconnect(cli);
        cli->close();
    }
    SClients.remove(fd); fd = -1;
    client = auth = rdy = false;
    ui->sending->setEnabled(false);
    ShowHideData(auth);//false
}
//-----------------------------------------------------------------------
void MainWindow::slotErrorClient(QAbstractSocket::SocketError SErr)
{
QTcpSocket *cliSocket = (QTcpSocket *)sender();
QString qs = "Socket ERROR (" + QString::number((int)SErr, 10) + ") : " + cliSocket->errorString();

    QString stx = "Close client, socket " + QString::number(cliSocket->socketDescriptor(), 10) + ". " + qs + "\n";
    ui->textinfo->append(stx);
    LogSave(__func__, stx, true);
    statusBar()->clearMessage(); statusBar()->showMessage(stx);

    emit sigCliDone(cliSocket, 0);
}
//-----------------------------------------------------------------------
void MainWindow::slotRdyPack(int ilen)
{

    if (ilen > 0) {
        QJsonObject *jobj = new QJsonObject();//json_object();
        if (jobj) {
            bool yes = true;
            jobj->insert("DevName", QJsonValue("FMB630"));
            jobj->insert("DevID",   QJsonValue(imei));
            jobj->insert("ServerTime",QJsonValue((qint32)time(NULL)));
            jobj->insert("FromAddr", QJsonValue(CliUrl));
            switch (codec_id) {
                case 8:
                    seq_number++;
                    jobj->insert("SeqNumber", QJsonValue(seq_number));
                break;
                case 12:
                case 13:
                    cmd_seq_number++;
                    jobj->insert("SeqNumber",QJsonValue(cmd_seq_number));
                    jobj->insert("Command", QJsonValue(cmd_id));
                    statusBar()->clearMessage();
                    statusBar()->showMessage("For command " + ui->cmd->text() + " response received");
                    slotWaitDone();//enable next command sending
                break;
                default : yes = false;
            }

            if (yes) {
                if (!parse_data_from_dev(from_cli, ilen, jobj, &cmd_id)) {
                    QString qstx = QJsonDocument(*jobj).toJson(QJsonDocument::Compact) + "\n";
                    //
                    time_t ict = QDateTime::currentDateTime().toTime_t();
                    struct tm *ct = localtime(&ict);
                    QString dt; dt.sprintf("%02d:%02d:%02d  ", ct->tm_hour, ct->tm_min, ct->tm_sec);
                    //
                    ui->textinfo->append(dt + qstx);
                    LogSave(NULL, qstx, 0);
                    codec_id = 0;
                }
            }
            delete jobj;
        }
    }

    clearParam(sizeof(s_avl_hdr));
    rdy = false;

}
//-----------------------------------------------------------------------
void MainWindow::on_sending_clicked()
{
QString stx = ui->cmd->text();
int res, relz, timz, i, cid, result = 0;
char cmd_par[max_cmd_len] = {0};
char chab;
char st[4] = {0};

    if (client && stx.length() && (fd > 0)) {
        ui->textinfo->append(stx);
        LogSave(__func__, stx, 1);

        res = ParseResp(&stx, cmd_par);
        relz = (res >> 8) & 0xff;
        timz = res >> 16;
        res &= 0xff;
        cmd_id = res;
        if ((cmd_id >= 0) && (cmd_id < max_cmds)) {
            cid = cmd_id;
            switch (cmd_id) {
                case 1://SET ON DOUT
                    if ((relz > 0) && (relz <= max_rel)) {
                        sprintf(cmd_par, " ");
                        for (i = 0; i < 4; i++) {
                            if ((relz - 1) == i) chab = '1'; else chab = 'X';
                            sprintf(cmd_par+strlen(cmd_par),"%c", chab);
                        }
                        if (timz > 0) {
                            for (i = 0; i < 4; i++) {
                                if ((relz - 1) == i) sprintf(cmd_par+strlen(cmd_par)," %d", timz);
                                                else sprintf(cmd_par+strlen(cmd_par)," 0");
                            }
                        } else sprintf(cmd_par+strlen(cmd_par)," 0 0 0 0");
                    }
                break;
                case 2://SET OFF DOUT
                    if ((relz > 0) && (relz <= max_rel)) {
                        sprintf(cmd_par, " ");
                        for (i = 0; i < 4; i++) {
                            if ((relz - 1) == i) chab = '0'; else chab = 'X';
                            sprintf(cmd_par+strlen(cmd_par),"%c", chab);
                        }
                        sprintf(cmd_par+strlen(cmd_par)," 0 0 0 0");
                    }
                break;
                case 3://{"command":3} - 'разблокировать бензонасос' -> "SET_ON 1 0"  //{"command":33, "relay":1,"time":0}
                    if (cmd_id == 3) cid = 33;//"SET_ON 1 0"
                                else cid = 34;//"SET_OFF 1 0"
                    sprintf(cmd_par, " 1 0");
                break;
                case 4://{"command":4} - 'заблокировать бензонасоса' -> "SET_OFF 1 0"  //{"command":34, "relay":1,"time":0}
                    if (cmd_id == 3) cid = 33;//"SET_ON 1 0"
                                else cid = 34;//"SET_OFF 1 0"
                    sprintf(cmd_par, " 1 0");
                break;
                case 5://{"command":5} - 'разблокировать двери' -> "SET_ON 2 2" //{"command":33,"relay":2, "time":2}
                    cid = 33;
                    sprintf(cmd_par, " 2 2");//"SET_ON 2 2"
                break;
                case 6://{"command":6} - 'заблокировать двери' (18 сек) -> "SET_ON 3 18" //{"command":33,"relay":3, "time":18}
                case 7://{"command":7} - 'заблокировать двери' (2 сек) -> "SET_ON 3 2" //{"command":33,"relay":3, "time":2}
                    cid = 33;
                    if (cmd_id == 6) sprintf(cmd_par, " 3 18");//"SET_ON 3 18"
                                else sprintf(cmd_par, " 3 2");//"SET_ON 3 2"
                break;
                case 8://{"command":8} - 'включить аврийку' -> "SET_ON 4 0" //{"command":33,"relay":4,"time":0}
                case 10://{"command":10} - 'выключить аварийку' -> "SET_OFF 4 0" //{"command":34,"relay":4,"time":0}
                    if (cmd_id == 8) cid = 33;//"SET_ON 4 0"
                                else cid = 34;//"SET_OFF 4 0"
                    sprintf(cmd_par, " 4 0");
                break;
                case 9://{"command":9} - 'включить аврийку' -> "SET_ON 4 Y" //{"command":33,"relay":4,"time":Y}
                    cid = 33;
                    sprintf(cmd_par, " 4 %d", 5);//oc_time);//"SET_ON 4 Y"
                break;
                case 11://{"command":11} - 'включить зажигание' -> "SET_ON 5 0" //{"command":33,"relay":5,"time":0}
                case 12://{"command":12} - 'выключить зажигание' -> "SET_OFF 5 0" //{"command":34,"relay":5,"time":0}
                    if (cmd_id == 11)  cid = 33;//"SET_ON 5 0"
                                  else cid = 34;//"SET_OFF 5 0"
                    sprintf(cmd_par, " 5 0");
                break;
                case 13://{"command":13} - 'запустить стартер' -> "SET_ON 6 2" //{"command":33,"relay":6,"time":2}
                case 14://{"command":14} - 'включить звуковой сигнал на 1 сек.' "SET_ON 7 2" //{"command":33,"relay":7,"time":2}
                    cid = 33;
                    if (cmd_id == 13) sprintf(cmd_par, " 6 2");//"SET_ON 6 2"
                                 else sprintf(cmd_par, " 7 2");//"SET_ON 7 2"
                break;
                case 41://"#GET REMIPX" X=1..4 - profile
                case 42://"#GET AUPX" X=1..2 - sim slot
                case 43://"#GET REPRTX" X=1..4
                case 44://"#GET REPDISTX" X=1..4
                case 45://"#GET REPANGX" X=1..4
                case 46://"#GET SENDPERIODX" X=1..4
                case 47://"#GET REPMRX" X=1..4
                        sprintf(cmd_par,"%d", relz);
                break;
                case 48://"SET_ALL X1,Y1 X2,Y2 X3,Y3 X4,Y4 X5,Y5 X6,Y6 X7,Y7 X8,Y8"
                break;
            }//switch(cmd_id)
            memset(to_cli, 0, sizeof(to_cli));
            result = MakeAvlPacket((uint8_t *)to_cli, cid, cmd_par);//make avl-packet for device
        } else result = -1;
        //
        if (result > 0) {
            if (dbg) {
                stx.clear(); stx.append("Command #" + QString::number(cmd_id,10) + " to device '" + imei + "' ready ("+QString::number(result,10) +") : ");
                for (i = 0; i < result; i++) {
                        sprintf(st, "%02X", (uint8_t)to_cli[i]);
                        stx.append(st);
                }
                ui->textinfo->append(stx);
                LogSave(__func__, stx + "\n", 1);

            }
            QTcpSocket *cliS = SClients[fd];
            cliS->write(to_cli, result);
            dev_wait_answer = 1;
            tmr_ack = startTimer(time_wait_answer);//wait ack from device until 10 sec
            if (tmr_ack <= 0) {
                MyError |= 0x20;//start_timer_error
                throw TheError(MyError);
            }
            ui->sending->setEnabled(false);//block send button
        }
        //
    }
}
//-----------------------------------------------------------------------
void MainWindow::slotWaitDone()
{
    if (tmr_ack) {
        killTimer(tmr_ack);
        tmr_ack = 0;
    }
    ui->sending->setEnabled(true);//unblock send button
    dev_wait_answer = 0;
}
//-----------------------------------------------------------------------
void MainWindow::timerEvent(QTimerEvent *event)
{
    if (tmr_ack == event->timerId()) {
        if (dev_wait_answer) {
            statusBar()->clearMessage(); statusBar()->showMessage("!!! TimeOut wait answer from device !!!");
            emit sigWaitDone();
        }
    } else if (tmr_sec == event->timerId()) {
        time_t it_ct = QDateTime::currentDateTime().toTime_t();
        struct tm *ctimka = localtime(&it_ct);
        QString dt;
        dt.sprintf(" | %02d.%02d.%02d %02d:%02d:%02d",
                    ctimka->tm_mday,
                    ctimka->tm_mon+1,
                    ctimka->tm_year+1900,
                    ctimka->tm_hour,
                    ctimka->tm_min,
                    ctimka->tm_sec);
        setWindowTitle(title + " ver. " + vers + dt);
    }
}
//-----------------------------------------------------------------------

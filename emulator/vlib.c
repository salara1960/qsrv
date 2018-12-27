#include "vlib.h"

#include "version.c"

//**********************************************************************************************************************

int QuitAll = 0;

uint8_t SIGHUPs  = 1;
uint8_t SIGTERMs = 1;
uint8_t SIGINTs  = 1;
uint8_t SIGKILLs = 1;
uint8_t SIGSEGVs = 1;
uint8_t SIGABRTs = 1;
uint8_t SIGSYSs  = 1;
uint8_t SIGTRAPs = 1;

char time_str[TIME_STR_LEN] = {0};

const char *im_def = "351580051430040";
char im[size_imei + 1] = {0};
s_imei avl_imei;
s_avl_hdr avl_hdr = {0, 0};

/*
//направление движения:приращение координаты:количество замеров по сценарию
West:1000:10  ; mirror.gsm.longitude-0.001000, mirror.gsm.angle=270 | Запад угол = 270^
North:1000:10 ; mirror.gsm.latitude+0.001000 , mirror.gsm.angle=0   | Север угол = 0^
East:1000:10  ; mirror.gsm.longitude+0.001000, mirror.gsm.angle=90  | Восток угол = 90^
South:1000:10 ; mirror.gsm.latitude-0.001000 , mirror.gsm.angle=180 | Юг угол = 180^
;остановка:длительность остановки в замерах (каждые 20 сек - 1 замер)
Stop:30
*/
const char *scena[] = {
"Park",
"North",
"East",
"South",
"West",
"North-East",
"North-West",
"South-East",
"South-West",
"Stop"
};


s_hdr_rec hdr_rec = {NULL, 0};

pthread_mutex_t rec_mutex = PTHREAD_MUTEX_INITIALIZER;


s_hdr_pack hdr_pack = {
    8,//codec_id
    1//total packets
};
s_mir mirror = {
    {
        0x16493094298,//TimeStamp with millisec.
        0,//Priority:       0       Low
        0x0C3A4548,//0x013904D0,//East Longitude: 20.514000
        0x209A9758,//0x0342A682,//North Latitude: 54.699650
        8,//Altitude:       8 meters
        0,//Angel:          0^
        3,//Sattelites:     3
        0,//Speed:          0 km/h
        0,//ID_event:     0
        23//Total_elem:  23
    },//gsm
    {
        13,//total 1-byte_len elements
        1,0, //DIN1 - Ign : 1-вкл.  0-выкл.
        2,1, //DIN2 - Door : 0-отрк.  1-закр.
        3,1, //DIN3 - Trunk : 0-откр.  1-закр.
        4,1, //DIN4 - Skp : 1-парковка
        179,0,//DOUT1 - бензонасос : 0-заблокирован  1-разблокирован
        180,0,//DOUT2
        50,0,//DOUT3
        51,0,//DOUT4
        22,3,//Profile
        71,3,//GNSSStatus
        240,0,//MovementSensor
        21,4,//GSM_slevel
        239,0//Ignition (aka DIN1)
    },//io1
    {
        7,//total 2-bytes_len elements
        9,max_fuel_tank,//AIN1 - fuel (ДУТ)
        10,21,//AIN2 - tach (Тахометр) : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
        11,1500,//AIN3 - hood (Капот) : if (val < 1000) hood = 1;//открыт else hood = 0;//закрыт
        67,9925,//BatteryVoltage
        68,50,//BatteryCurrent
        66,12044,//ExternalVoltage
        24,0//Speed
    },//io2
    {
        3,//total 4-bytes_len elements
        199,0,//Odometer
        241,25099,//GSMOperator
        216,0//TotalOdometer
    },//io4
    {
        0//total 8-bytes_len elements
    }//,//io8
};
s_tail_pack tail_pack = {
    1,//total packets
    0//16bit zero
};


s_box box[max_relay] = {{0,0}};
uint32_t tmr_box[max_relay] = {0};

s_box pin[max_pin] = {{0,0}};
uint32_t tmr_pin[max_pin] = {0};

uint32_t tmr_send;
uint32_t tmr_ack;

uint32_t tmr_two;

uint32_t tmr_evt;
uint32_t TMR_EVENT = TMR_EVT_DEF;

uint8_t Prio = 0;

pid_t pid_main;
FILE *pid_fp = NULL;
//const char *pid_name = "virt.pid";

int fd_log = -1;
int max_size_log = 1024 * 32000;
const char *fm_log_tail = ".log.txt";
char fm_log[max_log_name] = {0};


const char *cfg_file_def = ".virt.conf";
uint8_t work_mode = 0;

int scena_kols = 0, scena_delta = 0, scena_index = 1, scena_total = 0;

uint32_t wait_ack_sec = wait_ack_sec_def;

uint32_t sp_park = send_period_park;
uint32_t sp_move = send_period_move;
uint32_t send_period = send_period_park;
uint32_t wait_before_new_connect = next_try;

uint32_t last_send;

uint8_t move_flag = 0;
uint8_t stop_flag = 0;

pthread_mutex_t mirror_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conf_mutex = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------

static const char *cmds[] = {
"getgps",        //{"command":0}
"deleterecords", //{"command":29}
"getver",        //{"command":20}
"getio",         //{"command":15}
"SET_ALL",       //{"command":48,"param":"X1,Y1 X2,Y2 X3,Y3 X4,Y4 X5,Y5 X6,Y6 X7,Y7 X8,Y8"}
"GET_STAT",      //{"command":35}
"setdigout",     // 1XXX 0 0 0 0",//{"command":1, "relay":1, "time":0}
"setdigout",     // 0XXX 0 0 0 0",//{"command":2, "relay":1, "time":0}
"#DO REPORT",    //{"command":40}
"SET_ON",        //{"command":33, "relay":X, "time":Y}
"SET_OFF"        //{"command":34, "relay":X}
};

static const char *acks[] = {
"Error command getgps",//0
"All Records Deleted",//1 - cmd29 - deleterecords
"Code Ver:00.02.87 Device IMEI:351580051430040 Device ID:000014 BL Ver:06.45 Modem Ver:Teltonika TM25Q TM25Q_R_01.00.00.00_001, 2016/12/20 12:46 Hw:K-Line TM25Q ",//2 - cmd20 - getver
"DI1:1 DI2:0 DI3:0 DI4:0 AIN1:0 AIN2:0 AIN3:0 DO1:0 DO2:0 DO3:0 DO4:0",//3 - cmd15 - getio
"Error command SET_ALL",//4 = cmd48 - SET_ALL X,0 X,0 X,0 X,0 X,0 X,0 1,0 1,0
"Error command GET_STAT",//5 = cmd35 - "GET_STAT : 0x00\r\n" or "GET_STAT : ERROR\r\n"
"DOUTS are set to: 1XXX TMOs are: 0 0 0 0",//6 = cmd1 - enable gasoline pump
"DOUTS are set to: 0XXX TMOs are: 0 0 0 0",//7 = cmd2 - disable gasoline pump
"#OK",//8 = cmd40 - "#DO REPORT"
"Error command SET_ON",//9 = cmd33 - SET_ON X Y
"Error command SET_OFF",//10 = cmd34 - SET_OFF X
};

static const char *cname[] = {
"server=", //127.0.0.1:9090
"imei=",   //351580051430037
"mode=",    //0
"period_park=",//30
"period_move=",//10
"wait_ack=",//15
"wait_before_new_connect=",//3
"location=",//54.700218,20.514542
"#"
};

s_conf conf;


static const uint16_t itcrc[] = // 16 bit CRC lookup table (polynomial 0xA001)
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
#define Bit16(ch, crc) (itcrc[((crc) ^ (ch)) & 0xff] ^ ((crc) >> 8))
//----------------------------------------------------------------------------------------------------------------------------
static uint16_t ksum(uint8_t *uk, int len)
{
uint16_t crc = 0, slovo = 0;
int i;

    for (i = 0; i < len; i++) {
        slovo = *(uint16_t *)(uk + i);
        crc = Bit16(slovo, crc);
    }

    return crc;
}
//----------------------------------------------------------------------------------------------------------------------------
uint8_t rnd(const uint8_t mx)
{
uint8_t ret = rand() % mx;

    if (ret < 3) ret = 3;

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------------
void TheS(int sig)
{
int out = 0;

    switch (sig) {
        case SIGUSR1:
            printf("\tSIGNAL USR1 (%d)\n", sig);
        break;
        case SIGUSR2:
            printf("\tSIGNAL USR2 (%d)\n", sig);
        break;
        case SIGHUP:
            printf("\tSIGNAL SIGHUP (%d)\n", sig);
        break;
        case SIGKILL:
            if (SIGKILLs) {
                SIGKILLs = 0;
                printf("\tSIGNAL SIGKILL (%d)\n", sig);
                out = 1;
            }
        break;
        case SIGPIPE:
            printf("\tSIGNAL SIGPIPE (%d)\n", sig);
        break;
        case SIGTERM:
            if (SIGTERMs) {
                SIGTERMs = 0;
                printf("\tSIGNAL SIGTERM (%d)\n", sig);
                out = 1;
            }
        break;
        case SIGINT:
            if (SIGINTs) {
                SIGINTs = 0;
                printf("\tSIGNAL SIGINT (%d)\n", sig);
                out = 1;
            }
        break;
        case SIGSEGV:
            if (SIGSEGVs) {
                SIGSEGVs = 0;
                printf("\tSIGNAL SIGSEGV (%d)\n", sig);
                out = 1;
            }
        break;
        case SIGABRT:
            if (SIGABRTs) {
                SIGABRTs = 0;
                printf("\tSIGNAL SIGABRT (%d)\n", sig);
                out = 1;
            }
        break;
        case SIGSYS:
            if (SIGSYSs) {
                SIGSYSs = 0;
                printf("\tSIGNAL SIGSYS (%d)\n", sig);
                out = 1;
            }
        break;
        case SIGTRAP:
            if (SIGTRAPs) {
                SIGTRAPs = 0;
                printf("\tSIGNAL SIGTRAP (%d)\n", sig);
                out = 1;
            }
        break;
            default : printf("\tUNKNOWN signal %d",sig);
    }

    if (out) QuitAll = out;
}
//---------------------------------------------------------------------------------------------------------------------
char *TimeNowPrn(char *ts)
{
struct tm *ctimka;
time_t it_ct;
int i_hour, i_min, i_sec, i_day, i_mes, mil;
struct timeval tvl;

    gettimeofday(&tvl,NULL);
    it_ct = tvl.tv_sec;
    ctimka = localtime(&it_ct);
    i_hour = ctimka->tm_hour;   i_min = ctimka->tm_min;         i_sec = ctimka->tm_sec;
    i_day  = ctimka->tm_mday;   i_mes = ctimka->tm_mon+1;       mil   = (int)(tvl.tv_usec / 1000);
    memset(ts, 0, TIME_STR_LEN);
    sprintf(ts,"%02d.%02d %02d:%02d:%02d.%03d | ", i_day, i_mes, i_hour, i_min, i_sec, mil);

    return ts;
}
//---------------------------------------------------------------------------------------------------------------------
void prints(const char *func, char *st, uint8_t with)
{
struct stat sb;
int res = 0;
char stx[256] = {0};

    if (fd_log > 0) {
        if (with)  res = write(fd_log, TimeNowPrn(time_str), strlen(time_str));
        if (func) {
            res = strlen(func); if (res > (sizeof(stx) - 5)) res = sizeof(stx) - 5;
            sprintf(stx,"["); memcpy(stx + 1, func, res); sprintf(stx+strlen(stx),"] ");
            res = write(fd_log, stx, strlen(stx));
        }
        if (res >= 0) res = write(fd_log, st, strlen(st));

        if (!fstat(fd_log, &sb)) {
            if (sb.st_size > max_size_log) {
                close(fd_log); fd_log = -1; unlink(fm_log);                 //remove old file
                fd_log = open(fm_log, O_WRONLY | O_APPEND | O_CREAT, 0664); //create new file
            }
        }
    }
}
//-----------------------------------------------------------------------
int read_cfg(const char *fn, uint8_t prn)
{
char buf[max_tmp_len] = {0}, chap[max_tmp_len] = {0} ,tmp[max_tmp_len] = {0};
int cnt = 0, dl, i, j, ret = -1, delta = 1000, kols = 10;
s_conf cnf;
FILE *fp = NULL;
char *uki = NULL, *uks = NULL, *uke = NULL, *us = NULL;
uint8_t md = 0, dir = 0;
s_ones ones = {STOP, 0, 0};
float latf, lonf;



    if ((fp = fopen(fn, "r"))) {
        if (prn) {
            sprintf(chap,"Configuration file '%s' present:\n", fn);
            prints(__func__, chap, 1);
        }
        memset(buf, 0, max_tmp_len);
        memset((uint8_t *)&cnf, 0, sizeof(s_conf));

        while (fgets(buf, max_tmp_len - 1, fp) != NULL) {
            if (prn) prints(NULL, buf, 1);
            dl = strlen(buf);
            if (dl > 5) {
                if (buf[0] != ';') {
                    uki = strchr(buf, '\n'); if (uki) *uki = '\0';
                    //-----------------------------------------
                    for (i = 0; i < max_cname; i++) {
                        uks = strstr(buf, cname[i]);
                        if (uks) {
                            uks += strlen(cname[i]);
                            switch (i) {
                                case 0 ://"server="
                                    sprintf(chap, "%s", uks);
                                    us = strchr(chap, ':');
                                    if (us) {
                                        cnf.port = (uint16_t)atoi(us + 1);
                                        *us = '\0';
                                        dl = strlen(chap); if (dl > max_url_len - 1) dl = max_url_len - 1;
                                        memcpy((char *)cnf.srv, chap, dl); cnt++;//1
                                    }
                                break;
                                case 1://"imei="
                                    dl = strlen(uks);
                                    if (dl > size_imei) dl = size_imei;
                                    memcpy(cnf.imei, uks, dl); cnt++;//2
                                break;
                                case 2://"mode="
                                    cnf.mode = atoi(uks); cnt++;//3
                                break;
                                case 3://"period_park="//30
                                    cnf.snd_park = atoi(uks); cnt++;//4
                                break;
                                case 4://"period_move="//1
                                    cnf.snd_move = atoi(uks); cnt++;//5
                                break;
                                case 5://"wait_ack="//15
                                    cnf.wait_ack = atoi(uks); cnt++;//6
                                break;
                                case 6://"wait_before_next_connect="//3
                                    cnf.wait_before_new_connect = atoi(uks); cnt++;//7
                                break;
                                case 7://"location="//54.700218,20.514542
                                    sprintf(tmp, "%s", uks);
                                    us = strchr(tmp, ',');
                                    if (us) {
                                        lonf = (float)atof(us + 1) * 10000000; *us = '\0'; cnf.longitude = lonf;
                                        latf = (float)atof(tmp) * 10000000;                cnf.latitude  = latf;
                                    }
                                    cnt++;//8
                                break;
                                case 8://"#"
                                    md = 0;
                                    for (j = 0; j < max_scena; j++) {
                                        uke = strchr(uks, ':');
                                        if (uke) {
                                            dl = uke - uks; if (dl >= max_tmp_len - 1) dl = max_tmp_len - 1;
                                            memset(tmp, 0, sizeof(tmp));
                                            memcpy(tmp, uks, dl);
                                            if (!strncmp(tmp, scena[j], dl)) {
                                                dir = j;
                                                md = 1;
                                                uke = uks + strlen(scena[j]) + 1;
                                                sprintf(chap, "%s", uke);
                                                if ((j == (int)STOP) || (j == (int)PARK)) {
                                                    delta = 0;
                                                    kols = atoi(chap);
                                                    break;
                                                }
                                                uks = strchr(chap, ':');
                                                if (uks) {
                                                    kols = atoi(uks+1);
                                                    *uks = '\0';
                                                    delta = atoi(chap);
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if (md) {
                                        ones.dir = dir; ones.delta = delta; ones.kols = kols;
                                        add_ones(&ones, prn);
                                    }
                                break;
                            }
                        }
                    }//for
                    //-----------------------------------------
                }//if (buf[0] != ';')
            }//if (dl>5)
            memset(buf, 0, max_tmp_len);
        }//while
        fclose(fp);
    }


    if (cnt >= 7) {
        ret = 0;
        pthread_mutex_lock(&conf_mutex);
            memcpy((uint8_t *)&conf, (uint8_t *)&cnf, sizeof(s_conf));
        pthread_mutex_unlock(&conf_mutex);
    } else {
        if (prn) {
            sprintf(chap,"[%s] Error configuration file '%s'\n", __func__, fn);
            prints(__func__, chap, 1);
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
void init_mirror(uint32_t *lat, uint32_t *lon)
{

    mirror.gsm.ts = htobe64(mirror.gsm.ts);
    if (lon) mirror.gsm.longitude = htonl(*lon);
        else mirror.gsm.longitude = htonl(mirror.gsm.longitude);
    if (lat) mirror.gsm.latitude  = htonl(*lat);
        else mirror.gsm.latitude  = htonl(mirror.gsm.latitude);
    mirror.gsm.altitude = htons(mirror.gsm.altitude);
    mirror.gsm.angle    = htons(mirror.gsm.angle);
    mirror.gsm.speed    = htons(mirror.gsm.speed);
    //io2 - 7
    mirror.io2.val_ain1  = htons(mirror.io2.val_ain1);
    mirror.io2.val_ain2  = htons(mirror.io2.val_ain2);
    mirror.io2.val_ain3  = htons(mirror.io2.val_ain3);
    mirror.io2.val_bvolt = htons(mirror.io2.val_bvolt);
    mirror.io2.val_bcur  = htons(mirror.io2.val_bcur);
    mirror.io2.val_evolt = htons(mirror.io2.val_evolt);
    mirror.io2.val_speed = htons(mirror.io2.val_speed);
    //io4 - 3
    mirror.io4.val_odo  = htonl(mirror.io4.val_odo);
    mirror.io4.val_oper = htonl(mirror.io4.val_oper);
    mirror.io4.val_todo = htonl(mirror.io4.val_todo);
    //io8 - 0

}
//-----------------------------------------------------------------------
int make_record(uint8_t *buf, uint8_t kolpack, uint8_t wmode)
{
int ret = 0;
uint8_t i, pk = kolpack;
uint64_t ts64;
s_mir mirs;
s_ones one;
char chap[max_tmp_len] = {0};
uint32_t bit32, way;
uint16_t bit16;
short bit15;
float fway;
static uint8_t cdut = 3;


    if (!pk) pk = 1; else if (pk > max_pk) pk = max_pk;//max_pk=2 -> 2 samples in packet


    uint8_t *tmp = (uint8_t *)calloc(1, buf_size);
    if (tmp) {
        uint8_t *uk = tmp;

        hdr_pack.numbers_pack = pk; hdr_pack.codec_id = 8; tail_pack.numbers_pack = pk;

        pthread_mutex_lock(&mirror_mutex);
            memcpy((uint8_t *)&mirs.gsm.ts, (uint8_t *)&mirror.gsm.ts, sizeof(s_mir));
        pthread_mutex_unlock(&mirror_mutex);


        if (wmode == 1) {

            if (get_ones(scena_index, &one, 1)) {
		scena_kols++;
		switch ((int)one.dir) {
			case 0://Park:10
			    move_flag = 0;
			    send_period = sp_park;
			    //mirs.gsm.angle = 0;
			    mirs.gsm.speed = mirs.io2.val_speed = 0;
			    scena_kols = 1;
			    mirs.io1.val_din2 = 1;//двери закрыты
			    mirs.io1.val_din3 = 1;//DIN3 - Trunk : 0-откр.  1-закр.
			    mirs.io1.val_din4 = 1;//skp - парковка
			    bit16 = 1200; mirs.io2.val_ain3 = htons(bit16);//AIN3 - hood (Капот) : if (val < 1000) hood = 1;//открыт else hood = 0;//закрыт
			    bit16 = 326;  mirs.io2.val_ain2 = htons(bit16);//AIN2 - Тахометр : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
			    mirs.io4.val_odo = 0;
			    if (mirs.io1.val_dout1) {//бибика поехала <- разблокирован бензонасос
				mirs.io1.val_din1 = mirs.io1.val_ignition = 1;
				bit16 = ntohs(mirs.io2.val_ain1);
				if (bit16 > (min_fuel_tank - max_fuel_tank) >> 1) {//надо заправиться
				    bit16 -= (min_fuel_tank - max_fuel_tank) >> 1;
				    if (bit16 < max_fuel_tank) bit16 = max_fuel_tank;
				    mirs.io2.val_ain1 = htons(bit16);
				} else {//если больше половины бака -> можно ехать
				    mirs.io1.val_din4 = 0;//skp - не парковка
				    send_period = 2;
				    scena_kols = 255;
				    bit16 = 1536; mirs.io2.val_ain2 = htons(bit16);//AIN2 - Тахометр : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
				    //bit16 = max_fuel_tank; mirs.io2.val_ain1 = htons(bit16);//AIN1 : 47 - полный бак
				    prints(__func__, "Бибика поехала....\n", 1);
				    move_flag = 1;
				    bit16 = 12950; mirs.io2.val_evolt = htons(bit16);//ExternalVoltage up
				}
			    }
			break;
			case 1://North
			case 3://South
			    if (one.delta < 0) one.delta *= -1;
			    bit16 = ntohs(mirs.io2.val_ain2) - 28;
			    mirs.io2.val_ain2 = htons(bit16);//AIN2 - Тахометр : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
			    bit16 = ntohs(mirs.gsm.speed); bit16 = scena_kols + 8; mirs.gsm.speed = mirs.io2.val_speed = htons(bit16);
			    fway = ((bit16 * 1000) / 3600) * ((uint32_t)time(NULL) - last_send); way = fway;
			    mirs.io4.val_odo = htonl(way);
			    way += ntohl(mirs.io4.val_todo); mirs.io4.val_todo = htonl(way);
			    bit15 = ntohs(mirs.gsm.altitude);
			    if (one.dir == NORTH) {
				bit32 = ntohl(mirs.gsm.latitude); bit32 += one.delta; mirs.gsm.latitude = htonl(bit32);
				bit16 = 0; bit15--;
			    } else if (one.dir == SOUTH) {
				bit32 = ntohl(mirs.gsm.latitude); bit32 -= one.delta; mirs.gsm.latitude = htonl(bit32);
				bit16 = 180; bit15++;
			    }
			    mirs.gsm.angle = htons(bit16);
			    mirs.gsm.altitude = (short)htons(bit15);
			    send_period = sp_move;
			    cdut--; cdut &= 3;
			    if (!cdut) {
				bit16 = ntohs(mirs.io2.val_ain1) + 1;
				mirs.io2.val_ain1 = htons(bit16);//AIN1 - ДУТ
				if (bit16 >= (min_fuel_tank - min_fuel_tank/10) ) stop_flag = 1;//если в баке <= 10% -> stop
			    }
			    move_flag = 1;
			break;
			case 2://East
			case 4://West
			    if (one.delta < 0) one.delta *= -1;
			    bit16 = ntohs(mirs.io2.val_ain2) + 56;
			    mirs.io2.val_ain2 = htons(bit16);//AIN2 - Тахометр : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
			    bit16 = ntohs(mirs.gsm.speed); bit16 = scena_kols + 5; mirs.gsm.speed = mirs.io2.val_speed = htons(bit16);
			    fway = ((bit16 * 1000) / 3600) * ((uint32_t)time(NULL) - last_send); way = fway;
			    mirs.io4.val_odo = htonl(way);//odometer
			    way += ntohl(mirs.io4.val_todo); mirs.io4.val_todo = htonl(way);//total odometer
			    bit15 = ntohs(mirs.gsm.altitude);
			    if (one.dir == WEST) {
				bit32 = ntohl(mirs.gsm.longitude); bit32 -= one.delta; mirs.gsm.longitude = htonl(bit32);
				bit16 = 270; bit15--;
			    } else if (one.dir == EAST) {
				bit32 = ntohl(mirs.gsm.longitude); bit32 += one.delta; mirs.gsm.longitude = htonl(bit32);
				bit16 = 90; bit15++;
			    }
			    mirs.gsm.angle = htons(bit16);
			    mirs.gsm.altitude = (short)htons(bit15);
			    send_period = sp_move;
			    cdut--; cdut &= 3;
			    if (!cdut) {
				bit16 = ntohs(mirs.io2.val_ain1) + 1;
				mirs.io2.val_ain1 = htons(bit16);//AIN1 - ДУТ
				if (bit16 >= (min_fuel_tank - min_fuel_tank/10) ) stop_flag = 1;//если в баке <= 10% -> stop
			    }
			    move_flag = 1;
			break;
			case 5://"North-East"// - 45 degrees
			case 6://"North-West"// - 315 degrees
			case 7://"South-East"// - 135 degrees
			case 8://"South-West"// - 225 degrees
			    if (one.delta < 0) one.delta *= -1;
			    if ((one.dir == NORTH_WEST) || (one.dir == NORTH_EAST)) {
				bit32 = ntohl(mirs.gsm.latitude); bit32 += one.delta; mirs.gsm.latitude = htonl(bit32);
				bit16 = ntohs(mirs.io2.val_ain2) - 18;
			    } else if ((one.dir == SOUTH_WEST) || (one.dir == SOUTH_EAST)) {
				bit32 = ntohl(mirs.gsm.latitude); bit32 -= one.delta; mirs.gsm.latitude = htonl(bit32);
				bit16 = ntohs(mirs.io2.val_ain2) - 32;
			    }
			    mirs.io2.val_ain2 = htons(bit16);//AIN2 - Тахометр : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
			    bit16 = ntohs(mirs.gsm.speed); bit16 = scena_kols + 8; mirs.gsm.speed = mirs.io2.val_speed = htons(bit16);
			    fway = ((bit16 * 1000) / 3600) * ((uint32_t)time(NULL) - last_send); way = fway;
			    mirs.io4.val_odo = htonl(way);
			    way += ntohl(mirs.io4.val_todo); mirs.io4.val_todo = htonl(way);
			    bit15 = ntohs(mirs.gsm.altitude);
			    bit32 = ntohl(mirs.gsm.longitude);
			         if (one.dir  == NORTH_WEST) bit16 = 315;
			    else if (one.dir  == SOUTH_WEST) bit16 = 225;
			    else if (one.dir  == NORTH_EAST) bit16 =  45;
			    else if  (one.dir == SOUTH_EAST) bit16 = 135;
			    if ((one.dir == NORTH_WEST) || (one.dir == SOUTH_WEST))  {
				bit32 -= one.delta;
				bit15--;
			    } else if ((one.dir == NORTH_EAST) || (one.dir == SOUTH_EAST)) {
				bit32 += one.delta;
				bit15++;
			    }
			    mirs.gsm.longitude = htonl(bit32);
			    mirs.gsm.angle = htons(bit16);
			    mirs.gsm.altitude = (short)htons(bit15);
			    send_period = sp_move;
			    cdut--; cdut &= 3;
			    if (!cdut) {
				bit16 = ntohs(mirs.io2.val_ain1) + 1;
				mirs.io2.val_ain1 = htons(bit16);//AIN1 - ДУТ
				if (bit16 >= (min_fuel_tank - min_fuel_tank/10) ) stop_flag = 1;//если в баке <= 10% -> stop
			    }
			    move_flag = 1;
			break;
			case 9://Stop:10
			    move_flag = 0;
			    if (scena_kols == 1) {
				mirs.io1.val_din1 = mirs.io1.val_ignition = 0;
				mirs.io1.val_din2 = 0;//Door : 0-открыты   1-закрыты
				mirs.io1.val_din3 = 0;//Trunk : 0-открыт  1-закрыт
				mirs.io1.val_din4 = 0;//skp : 0-не парковка  1-парковка
				bit16 = 234; mirs.io2.val_ain3 = htons(bit16);//AIN3 - hood (Капот) : if (val < 1000) hood = 1;//открыт else hood = 0;//закрыт
			    } else {
				mirs.io1.val_din2 = 1;//Door : 0-открыты   1-закрыты
				mirs.io1.val_din3 = 1;//Trunk : 0-открыт  1-закрыт
				mirs.io1.val_din4 = 1;//skp : 0-не парковка  1-парковка
				bit16 = 1250; mirs.io2.val_ain3 = htons(bit16);//AIN3 - hood (Капот) : if (val < 1000) hood = 1;//открыт else hood = 0;//закрыт
			    }
			    if (scena_kols == 3) scena_kols--;
			    mirs.gsm.speed = mirs.io2.val_speed = 0;
			    send_period = sp_park;
			    bit16 = 21;  mirs.io2.val_ain2 = htons(bit16);//AIN2 - Тахометр : if (val > 1000) tah = 1;//двигатель заведен  else tah = 0;//двигатель заглушен
			    bit16 = 12050; mirs.io2.val_evolt = htons(bit16);//ExternalVoltage down
			    mirs.io4.val_odo = 0;
			    if (!mirs.io1.val_dout1) scena_kols = 255;
                        break;
                }
                sprintf(chap, "[%d] msg #%d from %d\n", scena_index, scena_kols, one.kols);
                prints(__func__, chap, 1);
                if (scena_kols >= one.kols) {
                    scena_kols = 0;
                    scena_index++;
                   if (scena_index > scena_total) scena_index = 1;
                   send_period = sp_move;
                }

            }

        } else send_period = sp_park;



        if (check_delay_sec(tmr_evt)) {
            //edit sattelites
            mirs.gsm.sattelites = rnd(18);//pseudo_random number from 3 to 18
            tmr_evt = get_timer_sec(TMR_EVENT);
        }
        //for move mode
        if (mirs.gsm.speed || mirs.io2.val_speed) mirs.io1.val_moves = 1;
        else
        if (!mirs.gsm.speed || !mirs.io2.val_speed) mirs.io1.val_moves = 0;

        //edit timestamp
        ts64 = time(NULL); ts64 *= 1000; mirs.gsm.ts = htobe64(ts64);//be64toh(ts64);
        mirs.gsm.prio = Prio; Prio = 0;


        //calc len of packet and put to struct avl_hdr
        uint32_t lens = sizeof(s_hdr_pack) + (sizeof(s_mir) * pk) + sizeof(s_tail_pack);
        avl_hdr.len = ntohl(lens-2);

        //copy struct avl_hdr to buffer
        memcpy(uk, (uint8_t *)&avl_hdr.zero, sizeof(s_avl_hdr));
        uk += sizeof(s_avl_hdr);

        //cpoy struct hdr_pack
        memcpy(uk, (uint8_t *)&hdr_pack.codec_id, sizeof(s_hdr_pack));
        uk += sizeof(s_hdr_pack);

        for (i = 0; i < pk; i++) {
            //copy struct mir to buffer
            memcpy(uk, (uint8_t *)&mirs.gsm.ts, sizeof(s_mir));
            uk += sizeof(s_mir);
            ts64 = time(NULL); ts64 *= 1000; mirs.gsm.ts = htobe64(ts64);//be64toh(ts64);
        }

        //cpoy struct tail_pack
        memcpy(uk, (uint8_t *)&tail_pack.numbers_pack, sizeof(s_tail_pack));
        uk += sizeof(s_tail_pack);

        //calc control sum of packet
        uint16_t ks = ksum(tmp + sizeof(s_avl_hdr), lens-2); ks = htons(ks);
        //copy ksum of packet to buffer
        memcpy(uk, (uint8_t *)&ks, sizeof(uint16_t));

        //calc total len of buffer for sending to server
        ret = lens + sizeof(s_avl_hdr) + 2;

        pthread_mutex_lock(&mirror_mutex);
            memcpy((uint8_t *)&mirror.gsm.ts, (uint8_t *)&mirs.gsm.ts, sizeof(s_mir));
        pthread_mutex_unlock(&mirror_mutex);

        memcpy(buf, tmp, buf_size);

        free(tmp);

        last_send = (uint32_t)time(NULL);

    }

    tmr_send = get_timer_sec(send_period);


    return ret;
}
//-----------------------------------------------------------------------
int parse_cmd(uint8_t ct,   // command_type
              char *com,    // command string
              uint8_t *out, // output buffer
              int *ci)      // index of command
{
int len, i, j, k, ret = -1, ind = -1, bit, tim;
char ack_body[256] = {0}, param[256] = {0}, tmp[32], chap[128] = {0};
char *uk = NULL, *uks = NULL, *uke = NULL, *us = NULL;
uint8_t *uki = out;
uint8_t cid = 12; // codec.id
uint8_t tpk = 1;  // total packets
uint8_t erc = 0;  // error code
s_avl_cmd avl_cmd = {0, 0, cid, tpk, ct + 1, 0};
uint8_t tail[] = {tpk, 0, 0};
const char *err = "Error command (unknown)";
uint32_t ts4;
s_box bx[max_relay];
s_box pn[max_pin];
s_mir mirs;


    pthread_mutex_lock(&mirror_mutex);
        memcpy((uint8_t *)&mirs.gsm.ts, (uint8_t *)&mirror.gsm.ts, sizeof(s_mir));// get mirror struct
    pthread_mutex_unlock(&mirror_mutex);

    memcpy((uint8_t *)&bx[0].stat, (uint8_t *)&box[0].stat, sizeof(s_box) * max_relay);
    memcpy((uint8_t *)&pn[0].stat, (uint8_t *)&pin[0].stat, sizeof(s_box) * max_pin);

    memset(out, 0, buf_size);

    for (i = 0; i < max_cmds; i++) {
        if (strstr(com, cmds[i])) {
            ind = i;
            break;
        }
    }
    if (ind == -1) sprintf(ack_body,"%s", err);
              else sprintf(ack_body,"%s", acks[ind]);

    switch (ind) {
        case 0://getgps
        {
            //GPS:1 Sat:5 Lat:54.694922 Long:20.516853 Alt:-353 Speed:0 Dir:104 Date: 2018/7/24 Time: 6:44:47
            int i_hour, i_min, i_sec, i_year, i_day, i_mes;
            time_t tct = time(NULL);
            struct tm *ct = localtime(&tct);
            i_hour = ct->tm_hour;        i_min = ct->tm_min;  i_sec = ct->tm_sec;
            i_year = ct->tm_year + 1900; i_day = ct->tm_mday; i_mes = ct->tm_mon + 1;
            if (mirs.gsm.sattelites) sprintf(ack_body,"GPS:1");
                                else sprintf(ack_body,"GPS:0");
            sprintf(ack_body+strlen(ack_body)," Sat:%u Lat:%.6f Long:%.6f Alt:%d Speed:%u Dir:%u Date: %04d/%02d/%02d Time: %02d:%02d:%02d",
                        mirs.gsm.sattelites,
                        (float)(ntohl(mirs.gsm.latitude)) / 10000000,
                        (float)(ntohl(mirs.gsm.longitude)) / 10000000,
                        (short)ntohs(mirs.gsm.altitude),
                        ntohs(mirs.gsm.speed),
                        ntohs(mirs.gsm.angle),
                        i_year, i_mes, i_day, i_hour, i_min, i_sec);
        }
        break;
        case 2://getver
            uk = strstr(ack_body, "IMEI:");
            if (uk) {
                uk += 5;
                memcpy(ack_body, im, size_imei);
            }
        break;
        case 3://getio
            sprintf(ack_body,"DI1:%u DI2:%u DI3:%u DI4:%u AIN1:%u AIN2:%u AIN3:%u DO1:%u DO2:%u DO3:%u DO4:%u",
                        mirs.io1.val_din1, mirs.io1.val_din2, mirs.io1.val_din3, mirs.io1.val_din3,
                        ntohs(mirs.io2.val_ain1), ntohs(mirs.io2.val_ain2), ntohs(mirs.io2.val_ain3),
                        mirs.io1.val_dout1, mirs.io1.val_dout2, mirs.io1.val_dout3, mirs.io1.val_dout4);
        break;
        case 4://SET_ALL
            erc = 0;
            cid = 13;
            memset(ack_body, 0, sizeof(ack_body));
            strcpy(param, com);
            uks = strstr(param, cmds[ind]);
            if (uks) {
                uks += strlen((char *)cmds[ind]);
                if (*uks == ' ') uks++;
                //prints(__func__, "SET_ALL parse :\n", 1);
                for (k = 0; k < max_relay; k++) {
                    uke = strchr(uks, ' '); if (!uke) uke = strchr(uks, '\0');
                    if (uke) {
                        j = (uke - uks) & 0x1f;// <= 31
                        us = uke + 1;
                        memset(tmp,0,sizeof(tmp));
                        memcpy(tmp, uks, j);
                        //sprintf(chap,"\t%d - %s\n", k, tmp); prints(NULL, chap, 0);
                        uk = strchr(tmp, ',');
                        if (uk) {
                            tim = atoi(uk + 1);
                            *uk = '\0';
                            if (!strchr(tmp, 'X')) {
                                bit = atoi(tmp);
                                if (!bit) tim = 0;
                                bx[k].time = tim;
                                bx[k].stat = bit;
                            }
                        } else erc = 1;
                    } else erc = 1;
                    if (erc) break;
                    uks = us;
                }//for
            } else erc = 1;
            if (erc) sprintf(ack_body, "%s : ERROR\r\n", com);
            else {// all parse OK
                for (k = 0; k < max_relay; k++) {
                    if (bx[k].stat) {//set relay to 1
                        if (bx[k].time > 0) tmr_box[k] = get_timer_sec(bx[k].time);
                                       else tmr_box[k] = 0;
                    } else tmr_box[k] = 0;//set relay to 0
                }
                if (bx[6].stat && bx[7].stat) {//Ignition ON
                    mirs.io1.val_din1 = mirs.io1.val_ignition = 1;
                } else if (!bx[6].stat && !bx[7].stat) {//Ignition OFF
                    mirs.io1.val_din1 = mirs.io1.val_ignition = 0;
                }
                memcpy((uint8_t *)&box[0].stat, (uint8_t *)&bx[0].stat, sizeof(s_box) * max_relay);
                sprintf(ack_body, "%s : OK\r\n", com);
            }
        break;
        case 5://GET_STAT
            erc = 1;
            cid = 13;
            memset(param, 0, sizeof(param));
            strcpy(param, com);
            uks = strstr(param, cmds[ind]);
            if (uks) {
                uks += strlen((char *)cmds[ind]);
                uke = strchr(uks, ' ');
                if (uke) {
                    k = atoi(uke + 1); k--;
                    if ((k >= 0) && (k < max_relay)) {
                        erc = 0;
                        sprintf(ack_body, "%s : %d\r\n", com, bx[k].stat);//" : 1\r\n"
                    }
                } else {
                    uint8_t bos = 0;
                    for (k = 0; k < max_relay; k++) bos |= (bx[k].stat << k);
                    sprintf(ack_body, "%s : %02X\r\n", com, bos);//" : 0x01\r\n"
                    erc = 0;
                }
            }
            if (erc) sprintf(ack_body, "%s : ERROR\r\n", com);
        break;
        case 6://"setdigout 1XXX 0 0 0 0",//{"command":1, "relay":1, "time":0}
        case 7://"setdigout 0XXX 0 0 0 0",//{"command":2, "relay":1, "time":0}
            erc = 0;
            memset(param, 0, sizeof(param));
            strcpy(param, com);
            uks = strstr(param, cmds[ind]);
            if (uks) {
                uks += strlen((char *)cmds[ind]);
                if (*uks == ' ') uks++;
                uke = strchr(uks, ' ');
                if (uke) {
                    j = (uke - uks);// == 4
                    if (j > 4) j = 4;
                    us = uke + 1;//save uk to time1
                    memset(tmp, 0, sizeof(tmp));
                    memcpy(tmp, uks, j);
                    for (k = 0; k < max_pin; k++) {
                        if (tmp[k] != 'X') pn[k].stat = tmp[k] - 0x30;
                    }
                    uks = us;
                    k = 0;
                    do {
                        uke = strchr(uks, ' '); if (!uke) uke = strchr(uks, '\0');
                        if (uke) {
                            memset(tmp, 0, sizeof(tmp));
                            memcpy(tmp, uks, uke - uks);
                            if (pn[k].stat) pn[k].time = atoi(tmp); else pn[k].time = 0;
                            k++;
                            uks = uke + 1;
                        } else { erc = 1; break; }
                    } while (k < max_pin);
                } else erc = 1;
            } else erc = 1;
            if (erc) sprintf(ack_body, "%s : %s", com, err);
            else {//command parse OK | "DOUTS are set to: 1XXX TMOs are: 0 0 0 0",//6 = cmd1 - enable gasoline pump
                sprintf(ack_body, "DOUTS are set to: ");
                sprintf(param," TMOs are:");
                for (k = 0; k < max_pin; k++) {
                    switch (k) {
                        case 0: mirs.io1.val_dout1 = pn[k].stat; break;
                        case 1: mirs.io1.val_dout2 = pn[k].stat; break;
                        case 2: mirs.io1.val_dout3 = pn[k].stat; break;
                        case 3: mirs.io1.val_dout4 = pn[k].stat; break;
                    }
                    if (pn[k].stat) {
                        if (pn[k].time > 0) tmr_pin[k] = get_timer_sec(pn[k].time);
                                       else tmr_pin[k] = 0;
                    } else tmr_pin[k] = 0;//set pin to 0
                    sprintf(ack_body+strlen(ack_body),"%u", pn[k].stat);
                    sprintf(param+strlen(param)," %d", pn[k].time);
                }//for
                sprintf(ack_body+strlen(ack_body),"%s",param);
                memcpy((uint8_t *)&pin[0].stat, (uint8_t *)&pn[0].stat, sizeof(s_box) * max_pin);
            }
        break;
        case 8://#DO REPORT
            Prio = 1;
        break;
        case 9://"Error command SET_ON",//9 = cmd33 - SET_ON X Y
        case 10://"Error command SET_OFF",//10 = cmd34 - SET_OFF X
            erc = 1;
            cid = 13;
            memset(param, 0, sizeof(param));
            strcpy(param, com);
            uks = strstr(param, cmds[ind]);
            if (uks) {
                uks += strlen((char *)cmds[ind]);
                if (*uks == ' ') uks++;
                uke = strchr(uks, ' '); tim = -1;
                if (!uke) { uke = strchr(uks, '\0'); tim = 0; }
                if (uke) {
                    memset(tmp, 0, sizeof(tmp));
                    j = (uke - uks) & 1;
                    memcpy(tmp, uks, j);
                    k = atoi(tmp); k--;//relay number (0..7)
                    if ((k >= 0) && (k < max_relay)) {
                        if (ind == 9) bit = 1; else bit = 0;
                        if (tim != 0) tim = atoi(uke + 1);
                        if (!bit) tim = 0;
                        bx[k].time = tim;
                        bx[k].stat = bit;
                        if (bx[k].stat) {//set relay to 1
                            if (bx[k].time > 0) tmr_box[k] = get_timer_sec(bx[k].time);
                                           else tmr_box[k] = 0;
                        } else tmr_box[k] = 0;//set relay to 0
                        erc = 0;
                    }
                }
            }
            if (erc) sprintf(ack_body, "%s : ERROR\r\n", com);
            else {
                if ((k == 6) || (k == 7)) {
                    if (bx[6].stat && bx[7].stat) {//Ignition ON
                        mirs.io1.val_din1 = mirs.io1.val_ignition = 1;
                    } else if (!bx[6].stat && !bx[7].stat) {//Ignition OFF
                        mirs.io1.val_din1 = mirs.io1.val_ignition = 0;
                    }
                }
                sprintf(ack_body, "%s : OK\r\n", com);
                memcpy((uint8_t *)&box[0].stat, (uint8_t *)&bx[0].stat, sizeof(s_box) * max_relay);
                sprintf(chap,"cmd=%d (%s) : rel=%d bit=%d tim=%u\n", ind, cmds[ind], k, bit, tim); prints(__func__, chap, 1);
            }
        break;
    }

    pthread_mutex_lock(&mirror_mutex);
        memcpy((uint8_t *)&mirror.gsm.ts, (uint8_t *)&mirs.gsm.ts, sizeof(s_mir));// put mirror struct
    pthread_mutex_unlock(&mirror_mutex);


    avl_cmd.codec_id = cid;

    i = strlen(ack_body);
    if (cid == 13) i += 4;
    avl_cmd.blen = htonl(i);
    len = sizeof(s_avl_cmd) - 8 + i + 1;
    if (cid == 13) {
        len += 4;//add timestamp
        ts4 = htonl((uint32_t)time(NULL));
    }
    avl_cmd.tlen = htonl(len);

    //copy struct avl_cmd to out
    memcpy(uki, (uint8_t *)&avl_cmd.zero, sizeof(s_avl_cmd));
    uki += sizeof(s_avl_cmd);


    if (cid == 13) {//if codec.id == 13 -> add timestamp before ack_body
        memcpy(uki, (uint8_t *)&ts4, 4);
        uki += 4;
    }

    //copy ack_body to out
    memcpy(uki, (uint8_t *)ack_body, i);
    uki += i;

    //cpoy tail to out
    memcpy(uki, &tail[0], 3);
    uki += 3;

    //calc control sum of packet
    uint16_t ks = ksum(out + 8, len); ks = htons(ks);
    //copy ksum of packet to buffer
    memcpy(uki, (uint8_t *)&ks, sizeof(uint16_t));

    ret = sizeof(s_avl_cmd) + i + sizeof(tail) + 2;
    if (cid == 13) ret += 4;

    char stx[256] = {0};
    if (cid == 13) sprintf(stx,"ret=%d len_ks=%u timestamp=%u ack:%s\n", ret, len, (uint32_t)ntohl(ts4), ack_body);
              else sprintf(stx,"ret=%d len_ks=%u ack:%s\n", ret, len, ack_body);
    prints(__func__, stx, 1);

    *ci = ind;//return command index to upper level

    return ret;
}
//---------------------------------------------------------------------------------------------------------------------
int add_ones(s_ones *data, uint8_t flag)
{
s_rec *tmp = NULL, *rec = NULL, *next = NULL;
int ret = -1, yes = 0, dl = 0;
char stx[max_url_len] = {0};

    if (!data) return ret;

    pthread_mutex_lock(&rec_mutex);//lock list

        rec = (s_rec *)calloc(1, sizeof(s_rec));
        if (rec) {
            rec->next = NULL;
            tmp = hdr_rec.first;
            if (!tmp) {//first record
                hdr_rec.first = rec;
                yes = 1;
            } else {//add to tail
                while (tmp) {
                    next = tmp->next;
                    if (next) tmp = next;
                    else {
                        tmp->next = rec;
                        yes = 1;
                        break;
                    }
                }
            }
            if (yes) {
                memcpy((uint8_t *)&rec->ones.dir, (uint8_t *)&data->dir, sizeof(s_ones));
                hdr_rec.counter++;
                rec->index = hdr_rec.counter;
                ret = hdr_rec.counter;
                if (flag)
                    dl = sprintf(stx,"Add record [%u] '%s'(%d):%d:%d\n",
                                     ret, scena[(int)rec->ones.dir], (int)rec->ones.dir, rec->ones.delta, rec->ones.kols);
            }
        }

    pthread_mutex_unlock(&rec_mutex);//unlock list

    if (flag && dl) prints(__func__, stx, 1);

    return ret;
}
//---------------------------------------------------------------------------------------------------------------------
s_rec *get_ones(int ind, s_ones *data, uint8_t flag)
{
s_rec *rec = NULL, *next = NULL;
char stx[max_url_len];
int dl = 0;

    if (ind > 0) {

        pthread_mutex_lock(&rec_mutex);
            rec = hdr_rec.first;
            while (rec) {
                if (rec->index == ind) {
                    if (flag) dl = sprintf(stx,"Get record [%u] '%s'(%d):%d:%d\n",
                                               rec->index, scena[(int)rec->ones.dir], (int)rec->ones.dir, rec->ones.delta, rec->ones.kols);
                    if (data) memcpy((uint8_t *)&data->dir, (uint8_t *)&rec->ones.dir, sizeof(s_ones));
                    break;
                } else {
                    next = rec->next;
                    rec = next;
                }
            }
        pthread_mutex_unlock(&rec_mutex);

        if (flag && dl) prints(__func__, stx, 1);

    }

    return rec;
}
//---------------------------------------------------------------------------------------------------------------------
int find_ind(rumb_t rumb, uint8_t flag)
{
s_rec *rec = NULL, *next = NULL;
char stx[max_url_len];
int dl = 0;
int ret = 0;

    if (rumb <= STOP) {

        pthread_mutex_lock(&rec_mutex);
            rec = hdr_rec.first;
            while (rec) {
                if (rec->ones.dir == rumb) {
                    ret = rec->index;
                    if (flag) dl = sprintf(stx,"Find_ind_by_dir %d : %d:'%s'(%d):%d:%d\n",
                                               (int)rumb,
                                               ret,
                                               scena[(int)rec->ones.dir],
                                               (int)rec->ones.dir,
                                               rec->ones.delta,
                                               rec->ones.kols);
                    break;
                } else {
                    next = rec->next;
                    rec = next;
                }
            }
        pthread_mutex_unlock(&rec_mutex);

        if (flag && dl) prints(__func__, stx, 1);

    }

    return ret;
}
//---------------------------------------------------------------------------------------------------------------------
void del_all_ones(uint8_t flag)
{
s_rec *rec = NULL, *next = NULL, *del = NULL;
char stx[max_url_len] = {0};

    pthread_mutex_lock(&rec_mutex);

        if (flag) sprintf(stx,"Delete %u records from list\n", hdr_rec.counter);

        rec = hdr_rec.first;

        while (rec) {
            del = rec;
            next = del->next;
            if (next) rec = next;
                 else rec = NULL;
            if (hdr_rec.counter > 0) hdr_rec.counter--;
            free(del);
        }
        hdr_rec.first = NULL;

    pthread_mutex_unlock(&rec_mutex);

    if (flag) prints(__func__, stx, 1);

}
//---------------------------------------------------------------------------------------------------------------------
void prn_all_ones(uint8_t flag)
{
s_rec *rec = NULL, *next = NULL;
char stx[len1k] = {0};


    pthread_mutex_lock(&rec_mutex);

        if (flag) sprintf(stx,"Total records in list %u :\n", hdr_rec.counter);

        rec = hdr_rec.first;

        while (rec) {
            if (flag) sprintf(stx+strlen(stx),"\t[%u] '%s'(%d):%d:%d\n",
                                              rec->index, scena[(int)rec->ones.dir], (int)rec->ones.dir, rec->ones.delta, rec->ones.kols);
            next = rec->next;
            if (next) rec = next;
                 else rec = NULL;
        }

    pthread_mutex_unlock(&rec_mutex);

    if (flag) prints(__func__, stx, 1);

}

//*************************************************************************************************


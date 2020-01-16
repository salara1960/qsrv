#ifndef SRV_H
#define SRV_H

#include <inttypes.h>
#include <iostream>
#include <arpa/inet.h>
#include <time.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QtNetwork>
#include <QTcpSocket>
#include <QTcpServer>
#include <QtNetwork/QAbstractSocket>
#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMovie>
#include <QMessageBox>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QListWidget>

#include <QtQuickWidgets/QQuickWidget>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QtQml>

#include <QGeoPositionInfoSource>

#include <QGeoAddress>
#include <QGeoCodingManager>
#include <QGeoCoordinate>
#include <QGeoLocation>
#include <QGeoServiceProvider>
#include <QGeoCodingManagerEngine>


//********************************************************************************

#undef SET_DEBUG

#define size_imei 15
#define size_num 16
#define max_buf 4096
#define max_tmp_len 1024
#define buf_size 8192
#define total_prio 4
#define max_cmd_len 128
#define max_cmds0 23// for FM1110
#define max_cmds1 49// for FM5300, FM6320, FMB630
#define max_rel 8
#define max_dev_type 5
#define max_rows 4
#define def_oc_time 2
#define CodecID8 8
#define CodecID12 12
#define CodecID13 13
//********************************************************************************

typedef enum {
    DEV_FM1110 = 0,
    DEV_FM5300,
    DEV_FMB630,
    DEV_FM6320,
    DEV_UNKNOWN
} dev_type_t;

#pragma pack(push,1)
typedef struct
{
    uint16_t len;              // 2 bytes
    char imei[size_imei];      // 14+1 bytes
} s_imei;
typedef struct
{
    uint32_t zero;
    uint32_t len;
} s_avl_hdr;
#pragma pack(pop)

//----------------  AVL packet with codec.id = 12 (command to device)  ------------------
#pragma pack(push,1)
typedef struct
{
    unsigned int zero;      //0x00000000
    unsigned int tlen;      //data len without crc
    unsigned char codec_id; //0x0c
    unsigned char cmd_cnt;  //command quantity
} s_avl_cmd;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t codec_id;
    uint8_t numbers_pack;
    uint8_t type;
    uint32_t len;
} s_hdr_bin_ack;
typedef struct {
    uint8_t codec_id;
    uint8_t numbers_pack;
} s_hdr_pack_bin;
typedef struct {
    uint64_t ts;
    uint8_t prio;
    uint32_t longitude;
    uint32_t latitude;
    short altitude;
    uint16_t angle;
    uint8_t sattelites;
    uint16_t speed;
    uint8_t id_event;
    uint8_t total_elem;
} s_pack_bin;
typedef struct {
    uint32_t longitude;
    uint16_t longitude_grad;
    uint16_t longitude_min;
    uint16_t longitude_sec;
    uint32_t latitude;
    uint16_t latitude_grad;
    uint16_t latitude_min;
    uint16_t latitude_sec;
    short altitude;
    uint16_t angle;
    uint8_t sattelites;
    uint16_t speed;
} s_gps_pack_bin;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t din1;
    uint8_t din2;
    uint8_t din3;
    uint8_t din4;
    uint8_t dout1;
    uint8_t dout2;
    uint8_t dout3;
    uint8_t dout4;
    uint16_t ain1;
    uint16_t ain2;
    uint16_t ain3;
    uint8_t msensor;
} s_pins;
#pragma pack(pop)

typedef struct
{
    int index;
    QString imei;
    QString sim;
    uint8_t type;
} s_car;

typedef struct
{
    double latitude;
    double longitude;
} s_cord;
//********************************************************************************

extern int srv_port;
extern QString sdnm;
extern char const *vers;
extern uint8_t dbg;
extern char gradus;
extern const char *prio_name[];
extern const char *unknown;
extern const char *lat_name[];       // широта Latitude
extern const char *lon_name[];       // долгота Longitude
extern const char *net_type_name[];

extern const char *form;
extern QString dev_type_name[];
extern const char *mk_table;
extern const char *cmds0[];
extern const char *cmds1[];

extern QQuickWidget *wids;

//extern s_cord cord;

extern void parse_param_start(char *param);

//********************************************************************************
//********************************************************************************
//********************************************************************************

namespace Ui {
    class MainWindow;
}
//********************************************************************************
//********************************************************************************
//********************************************************************************
class CordClass : public QObject
{
    Q_OBJECT
public:
    explicit CordClass(QObject *parent = nullptr, s_cord *loc = nullptr, QRect r = {0,0,0,0}) : QObject(parent)
    {
        if (loc) {
            m_lat = loc->latitude;
            m_lon = loc->longitude;
        }
        if (parent) {
            w = r.width();
            h = r.height();
        }
    }
    ~CordClass() {}

    Q_INVOKABLE int get_w() const { return w; }
    Q_INVOKABLE int get_h() const { return h; }

    Q_INVOKABLE double get_lat() const { return m_lat; }
    Q_INVOKABLE double get_lon() const { return m_lon; }

    void set_pos(s_cord *loc)
    {
        m_lat = loc->latitude;
        m_lon = loc->longitude;
        QString dts(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        QString lin;
        dts += lin.sprintf(" New location : %.6g , %.6g", m_lat, m_lon);
#ifdef SET_DEBUG
        qDebug() << dts;
#endif
    }

private:
    double m_lat = 0.0, m_lon = 0.0;
    int w = 800, h = 260;
};

//********************************************************************************
//********************************************************************************
//********************************************************************************

class QTcpServer;

//********************************************************************************

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    class TheError {
        public :
            int code;
            TheError(int);
    };

    explicit MainWindow(QWidget *parent = nullptr, int p = 9090, QString *dnm = nullptr);
    ~MainWindow();
    void timerEvent(QTimerEvent *event);

public slots:
    void PrnTextInfo(QString);
    void LogSave(const char *, QString, bool);
    uint16_t ks(uint8_t *, int);
    int MakeAvlPacket(void *, int, char *);
    char *ShowTime(time_t *, char *);
    char *io_name(uint8_t, char *, uint8_t);
    int ParseResp(QString, char *);
    int CalcFuel(uint16_t, uint8_t);
    QJsonObject *ConvertStrToJsonObject(char *, int *);
    int parse_data_from_dev(char *, int, QJsonObject *, int *, s_cord *);
    void slotErrorClient(QAbstractSocket::SocketError);
    void clearParam(int);
    void UpdatePins();

private slots:
    void ShowHideData(bool);
    void on_starting_clicked();
    void on_stoping_clicked();
    void on_sending_clicked();
    void newuser();
    void slotReadClient();
    void slotCliDone(QTcpSocket *, int);
    void slotRdyPack(int);
    void slotWaitDone();
    bool check_dev(s_car *car);
    void sql_err_msg(QSqlError &er);
    void ShowMap();
    void HideMap();

signals:
    void sigCliDone(QTcpSocket *, int);
    void sigRdyPack(int);
    void sigSending();
    void sigWaitDone();

private:
    bool client, auth, rdy;
    int MyError, port, rxdata, txdata, lenrecv;
    int cmd_id, fd, tmr_ack, tmr_sec, server_status;
    char to_cli[max_buf], from_cli[max_buf];
    uint8_t faza, dev_wait_answer, codec_id, cnt_pack;
    qint32 seq_number, cmd_seq_number, total_pack, total_cmd;
    QString imei;
    QString CliUrl;
    s_pins pins;
    Ui::MainWindow *ui;
    QTcpServer *tcpServer;
    QMap <int, QTcpSocket *> SClients;
    QMovie *movie;
    QSqlDatabase *db;
    QSqlQuery *query;
    QString *db_name;
    bool openok, good;
    QSqlError sql_err;
    s_car thecar;

    QQuickWidget *wid;
    double latitude, longitude;
    s_cord cord;
    CordClass *Coro;
    QObject *RootObj;

};


#endif // SRV_H

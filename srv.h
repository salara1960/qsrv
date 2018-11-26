#ifndef SRV_H
#define SRV_H

#include <inttypes.h>
#include <iostream>
#include <arpa/inet.h>
#include <time.h>
#include <endian.h>
#include <QApplication>
#include <QMainWindow>
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

//********************************************************************************
#define size_imei 15
#define size_num 16
#define max_buf 4096
#define max_tmp_len 1024
#define buf_size 8192
#define total_prio 4
#define max_cmd_len 128
#define max_cmds 49
#define max_rel 8
//********************************************************************************

#pragma pack(push,1)
typedef struct
{
    uint16_t len;                 // 2 bytes
    char imei[size_imei];      // 15 bytes
} s_imei;
typedef struct
{
    uint32_t zero;
    uint32_t len;
} s_avl_hdr;
#pragma pack(pop)

//----------------  AVL  packet with codec.id = 12 (command to device)  ------------------
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

//********************************************************************************

extern QString imei;
extern void LogSave(const char *func, QString st, bool pr);

//********************************************************************************

namespace Ui {
    class MainWindow;
}

//********************************************************************************

class QTcpServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    class TheError {
        public :
            int code;
            TheError(int);
    };

    explicit MainWindow(QWidget *parent = 0, int p = 9090);
    ~MainWindow();
    void timerEvent(QTimerEvent *event);

public slots:

    void LogSave(const char *, QString, bool);
    uint16_t ks(uint8_t *, int);
    int MakeAvlPacket(void *, int, char *);
    char *ShowTime(time_t *, char *);
    char *io_name(uint8_t, char *);
    int ParseResp(QString *, char *);
    int CalcFuel(uint16_t, uint8_t);
    QJsonObject *ConvertStrToJsonObject(char *, int *);
    int parse_data_from_dev(char *, int, QJsonObject *, int *);
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
};

#endif // SRV_H

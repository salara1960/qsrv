/*
                            "Для резиновой калоши
                             Настоящая беда,
                             Если день — сухой, хороший,
                             Если высохла вода.
                             Ей всего на свете хуже
                             В чистой комнате стоять:
                             То ли дело шлепать в луже,
                             Через улицу шагать!"
                                       О.Э. Мандельштам
*/

#include "srv.h"

//map type : "osm","mapboxgl","esri"

int main(int argc, char *argv[])
{
int port = 9090, cerr = 0;
QString errStr = "", cerrStr;
QString dnm("cars.s3db");
QString nm("osm");

    setlocale(LC_ALL,"UTF8");

    if (argc > 1) parse_param_start(argv[1]);
    if (argc > 2) parse_param_start(argv[2]);
    if (argc > 3) parse_param_start(argv[3]);

    if (srv_port > 0) port = srv_port;
    if (sdnm.length() > 0) dnm = sdnm;
    if (win_map.length() > 0) nm = win_map;

    try {
        QApplication srv(argc, argv);

        MainWindow wnd(nullptr, port, &dnm, nm);

        wnd.show();

        srv.exec();
    }

    catch (MainWindow::TheError(er)) {
        cerr = er.code;
        cerrStr.sprintf("%d", cerr);
        if (cerr > 0) {
            if (cerr & 1) errStr.append("Error create server object (" + cerrStr + ")\n");
            if (cerr & 2) errStr.append("Error auth OFF (" + cerrStr + ")\n");
            if (cerr & 4) errStr.append("Error avl_pack len: too big (" + cerrStr + ")\n");
            if (cerr & 8) errStr.append("Error starting timer_wait_ack_for_command (" + cerrStr + ")\n");
            if (cerr & 0x10) errStr.append("Error create database object (" + cerrStr + ")\n");
            if (cerr & 0x20) errStr.append("Error open database (" + cerrStr + ")\n");
            if (cerr & 0x40) errStr.append("Error create query object (" + cerrStr + ")\n");
        } else errStr.append("Unknown Error (" + cerrStr + ")\n");
        if (errStr.length() > 0) {
            perror((char *)cerrStr.data());
        }
        return cerr;
    }
    catch (std::bad_alloc) {
        perror("Error while alloc memory via call function new\n");
        return -1;
    }

    return 0;
}

#include "srv.h"

int main(int argc, char *argv[])
{
int port = 9090;
int cerr = 0;
QString errStr = "";
QString cerrStr;
QString dnm("cars.s3db");

    setlocale(LC_ALL,"UTF8");

    if (argc > 1) port = atoi(argv[1]);

    try {
        QApplication srv(argc, argv);
        MainWindow wnd(NULL, port, &dnm);
        wnd.show();
        srv.exec();
    }

    catch (MainWindow::TheError(er)) {
        cerr = er.code;
        cerrStr.sprintf("%d", cerr);
        if (cerr > 0) {
            if (cerr & 1) errStr.append("Error create server object (" + cerrStr + ")\n");
            if (cerr & 2) errStr.append("Error faza 0: auth ON (" + cerrStr + ")\n");
            if (cerr & 4) errStr.append("Error faza 1: auth OFF (" + cerrStr + ")\n");
            if (cerr & 8) errStr.append("Error faza 2: auth OFF (" + cerrStr + ")\n");
            if (cerr & 0x10) errStr.append("Error avl_pack len: too big (" + cerrStr + ")\n");
            if (cerr & 0x20) errStr.append("Error starting tmr0 (" + cerrStr + ")\n");
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

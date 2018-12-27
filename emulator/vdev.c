#include "vlib.h"

//*************************************************************************************************
int main (int argc, char *argv[])
{
char *uk, *body_uk, *uks = NULL;
static uint8_t to_server[buf_size];
static uint8_t from_server[buf_size];
char scmd[256];
char tmp[buf_size << 1];
char chap[2048] = {0};
char stx[256] = {0}, line[256] = {0};
uint16_t tcp_port;
struct sockaddr_in srv_conn;
socklen_t srvlen;
struct hostent *hostPtr = NULL;
int connsocket = -1, i = 0, j, ik, lenr = 0, lenr_tmp = 0, ind = 0, lenr_wait = 4, body_len = 0, lens = 0, icmd = -1;
size_t resa;
uint32_t cnt_send = 0;
uint8_t grant = 0, err = 255, first = 1, rdy = 0, Vixod = 0, wait_ack = 0, kol_pack = 1, ctype = 5, one = 1;
uint8_t faza = 0;
int com_id = -1;

struct sigaction Ata, OAta;
struct timeval cli_tv;
fd_set read_Fds;
s_avl_hdr *avl_hdr_ack = NULL;
s_avl_cmd *avl_cmd = NULL;
s_conf cfg;
uint64_t bit64;

time_t start_ses = time(NULL);
time_t stop_ses = time(NULL);
char pid_name[128] = {0};

uint32_t *first_lat = NULL, *first_lon = NULL;
float flat = 0.0, flon = 0.0;


    if (argc < 2) return 1;

    sprintf(stx,"%s", argv[1]);
    uks = strchr(stx, '\n'); if (uks) *uks = '\0';
    uks = strstr(stx, "config=");
    if (uks) {
        uks += 7;
        sprintf(line,"%s", uks);
    }
    ik = sprintf(pid_name,"%s", line);
    if (pid_name[0] == '.') {
        ik--;
        memcpy(pid_name, pid_name + 1, ik);
        pid_name[ik] = '\0';
    }
    uks = strstr(pid_name, ".conf"); if (uks) *uks = '\0';
    sprintf(fm_log,"%s%s", pid_name, fm_log_tail);
    sprintf(pid_name+strlen(pid_name), ".pid");

    fd_log = open(fm_log, O_WRONLY | O_APPEND | O_CREAT, 0664);
    if (fd_log < 0) {
        printf(stx, "[Ver.%s] Can't open %s file (%d)\n", Version, fm_log, fd_log);
        return 1;
    }

//------------  set signals route function ------------------
    memset((uint8_t *)&Ata, 0, sizeof(struct sigaction));
    memset((uint8_t *)&OAta, 0, sizeof(struct sigaction));
    Ata.sa_handler = &TheS;
    Ata.sa_flags   = 0;
    sigaction(SIGPIPE, &Ata, &OAta);
    sigaction(SIGHUP,  &Ata, &OAta);
    sigaction(SIGSEGV, &Ata, &OAta);
    sigaction(SIGTERM, &Ata, &OAta);
    sigaction(SIGABRT, &Ata, &OAta);
    sigaction(SIGINT,  &Ata, &OAta);
    sigaction(SIGSYS,  &Ata, &OAta);
    sigaction(SIGKILL, &Ata, &OAta);
    sigaction(SIGTRAP, &Ata, &OAta);
    sigaction(SIGUSR1, &Ata, &OAta);
    sigaction(SIGUSR2, &Ata, &OAta);
//------------------------------------------------------------------------------------------------

    memset(im, 0, size_imei + 1);

    if (read_cfg(line, 0) != 0) {
        if (fd_log > 0) close(fd_log);
        return 1; //config file not present or error -> get param from start line
    }

    pthread_mutex_lock(&conf_mutex);
            memcpy((char *)&cfg, (char *)&conf, sizeof(s_conf));
    pthread_mutex_unlock(&conf_mutex);
    memset(line, 0, sizeof(line));
    strcpy(line, (char *)cfg.srv);
    tcp_port = cfg.port;
    memcpy(im, (char *)cfg.imei, size_imei);
    work_mode = cfg.mode;
    if (cfg.snd_park) sp_park = cfg.snd_park;
    if (cfg.snd_move) sp_move = cfg.snd_move;
    if (cfg.wait_ack) wait_ack_sec = cfg.wait_ack;
    if (cfg.wait_before_new_connect) wait_before_new_connect = cfg.wait_before_new_connect;
    if (cfg.latitude) { first_lat = &cfg.latitude; flat = cfg.latitude; flat /= 10000000; }
    if (cfg.longitude) { first_lon = &cfg.longitude; flon = cfg.longitude; flon /= 10000000; }


    if (work_mode) {
        send_period = sp_move;
        prn_all_ones(1);
    } else {
        send_period = sp_park;
    }

    pid_main = getpid();// create pid file for start|stop script
    pid_fp = fopen(pid_name, "w");
    if (!pid_fp) {
        sprintf(stx, "Unable to create pid file %s: %s\n", pid_name, strerror(errno));
        prints(__func__, stx, 1);
        goto stop_job;
    } else {
        fprintf(pid_fp, "%i\n", (int)pid_main);
        fclose(pid_fp);
    }

    sprintf(chap,"[%s] MAIN START :\n\tserver=%s:%d\n\timei=%s\n\tmode=%u\n\tsend_period=%d/%d\n\twait_ack=%d\n\twait_before_new_connect=%d\n\tlocation=%f,%f\n",
                 Version, line, tcp_port, im, work_mode, sp_park, sp_move, wait_ack_sec, wait_before_new_connect, flat, flon);
    if (work_mode) {
        pthread_mutex_lock(&rec_mutex);
            scena_total = hdr_rec.counter;
        pthread_mutex_unlock(&rec_mutex);
        sprintf(chap+strlen(chap),"\ttotal_scena=%u\n", scena_total);
    }
    prints(NULL, chap, 1);


    while (!QuitAll) {

        start_ses = time(NULL);

        connsocket = socket(AF_INET,SOCK_STREAM,6);
        if (connsocket < 0) {
            sprintf (chap, "FATAL ERROR: open socket (%d)\n", connsocket);
            prints(__func__, chap, 1);
            if (pid_main) unlink(pid_name);
            return 1;
        }

        hostPtr = gethostbyname(line);
        if (!hostPtr) {
            hostPtr = gethostbyaddr(line, strlen(line), AF_INET);
            if (!hostPtr) {
                sprintf(chap,"ERROR resolving server address '%s'\n", line);
                prints(__func__, chap, 1);
                goto out_of_job;
            }
        }

        start_ses = time(NULL);

        srvlen = sizeof(struct sockaddr);
        memset(&srv_conn,0,srvlen);
        srv_conn.sin_family = AF_INET;
        srv_conn.sin_port = htons(tcp_port);
        (void) memcpy(&srv_conn.sin_addr, hostPtr->h_addr, hostPtr->h_length);


        if (connect(connsocket, (struct sockaddr *)&srv_conn, srvlen) == -1) {
            sprintf(chap, "ERROR: connect to %s:%d\n", line, tcp_port);
            prints(__func__, chap, 1);
            goto out_of_job;
        } else {
            sprintf(chap, "Connect to %s:%d OK\n", line, tcp_port);
            prints(__func__, chap, 1);
        }
        fcntl(connsocket, F_SETFL, (fcntl(connsocket, F_GETFL)) | O_NONBLOCK);


        Vixod = 0;
        memset(to_server,   0, buf_size); resa = 0;
        memset(from_server, 0, buf_size); lenr = 0;
        grant = 0; faza = 0; first = 1; err = 0; wait_ack = 0;
        kol_pack = 1;
        stop_flag = 0;
        avl_imei.len = 0x0f00;
        memcpy((char *)&avl_imei.imei, im, size_imei);

        tmr_ack = get_timer_sec(wait_ack_sec);

        if (one) {
            one = 0;
            init_mirror(first_lat, first_lon);
            if (work_mode) {
                scena_kols = scena_delta = 0;
                scena_index = 1;
            }

        }

        tmr_two = get_timer_sec(TMR_TWO);
        tmr_evt = get_timer_sec(TMR_EVENT);

        while (!Vixod && !QuitAll) {

            switch (faza) {
                case 0 :
                    if (first) {
                        first = 0;
                        memset(to_server, 0, buf_size);
                        bit64 = sizeof(s_imei);
                        memcpy(to_server, (uint8_t *)&avl_imei.len, bit64);
                        sprintf(chap,"data to server (%"PRIu64"):", bit64);
                        for (i = 0; i < sizeof(s_imei); i++) sprintf(chap+strlen(chap), "%02X ",(uint8_t)to_server[i]);
                        sprintf(chap+strlen(chap), "\n");
                        prints(__func__, chap, 1);
                        resa = send(connsocket, to_server, sizeof(s_imei), MSG_DONTWAIT);
                        tmr_ack = get_timer_sec(wait_ack_sec);
                        wait_ack = 1;
                        if (resa != sizeof(s_imei)) { err |= 1; Vixod = 1; break; }
                    }
                    if (wait_ack) {
                        lenr = recv(connsocket, from_server, 1, MSG_DONTWAIT);
                        if (lenr == 0) { err |= 2; Vixod = 1; break; }// server closed connection
                        else if (lenr > 0) {
                            wait_ack = 0; rdy = 1;
                            if (from_server[0] == 1) { grant = 1; faza = 1; }// goto send packet
                                                else { Vixod = 1; err |= 4; }// server reject connection (imei unknown)
                        }
                        if (rdy) {
                            rdy = 0;
                            sprintf(chap, "data from server (%d):", lenr);
                            if (lenr > 0) {
                                for (i = 0; i < lenr; i++) sprintf(chap+strlen(chap), "%02X ",(uint8_t)from_server[i]);
                            }
                            sprintf(chap+strlen(chap), "\n");
                            prints(__func__, chap, 1);
                            memset(from_server, 0, buf_size); lenr = 0;
                            break;
                        }
                    }
                break;
                case 1://init
                    if (grant) {
                        start_ses = time(NULL);
                        prints(__func__, "Access ganted !!!\n", 1);
                        tmr_send = get_timer_sec(0);
                        faza = 2;
                        body_uk = (char *)&from_server[0];
                        body_len = 0;
                        lenr_wait = 4;
                        lenr = lenr_tmp = 0; ind = 0;
                        memset(to_server, 0, buf_size);
                        memset(from_server, 0, buf_size);
                        last_send = (uint32_t)time(NULL);
                        stop_flag = 0;
                        wait_ack = 0;
                    } else { err |= 0x10; Vixod = 1; }
                break;
                case 2:// send/recv. data to/from server
                    if (!wait_ack) {
                        if (check_delay_sec(tmr_send)) {//send packet to server
                            tmr_send = get_timer_sec(send_period);
                            if (check_delay_sec(tmr_two)) {
                                kol_pack++; if (kol_pack > max_pk) kol_pack = 1;
                                tmr_two = get_timer_sec(TMR_TWO);
                            }
                            lens = make_record(to_server, kol_pack, work_mode);
                            if (lens > 0) {
                                resa = send(connsocket, to_server, lens, MSG_DONTWAIT);
                                cnt_send++;
                                sprintf(tmp,"Send packet #%u with len=%d to server:\n", cnt_send, lens);
                                for (i = 0; i < lens; i++) sprintf(tmp+strlen(tmp),"%02X",(uint8_t)to_server[i]);
                                sprintf(tmp+strlen(tmp),"\n"); prints(__func__, tmp, 1);
                                if (resa != lens) { Vixod = 1; err |= 20;/* write error */ break; }
                                else {//goto recv. ack from server
                                    wait_ack = 1; lenr = lenr_tmp = 0; ind = 0; lenr_wait = 4;
                                    tmr_ack = get_timer_sec(wait_ack_sec);//start wait ack timer
                                }
                            }
                        }
                    }
                    cli_tv.tv_sec = 0; cli_tv.tv_usec = 50000;
                    FD_ZERO(&read_Fds); FD_SET(connsocket, &read_Fds);

                    if (select(connsocket+1, &read_Fds, NULL, NULL, &cli_tv) > 0) {// watch sockets : device and broker
                        if (FD_ISSET(connsocket, &read_Fds)) {// event from device socket

                            lenr_tmp = recv(connsocket, &from_server[ind], 1, MSG_DONTWAIT);
                            if (lenr_tmp == 0) { err |= 2; Vixod = 1; break; }
                            else if (lenr_tmp > 0) {
                                lenr += lenr_tmp; ind += lenr_tmp;
                                if (lenr == lenr_wait) {//4 or X bytes must be recv. from server
                                    switch (from_server[3]) {
                                        case 0 : //!!!   recv. command from server   !!!
                                            if (lenr == 4) {
                                                lenr_wait += 4;
                                            } else if (lenr == 8) {
                                                avl_hdr_ack = (s_avl_hdr *)from_server;
                                                lenr_wait += ntohl(avl_hdr_ack->len) + 4;
                                            } else if (lenr > 8) {
                                                memset(scmd, 0, sizeof(scmd));
                                                avl_cmd = (s_avl_cmd *)from_server;
                                                body_len = ntohl(avl_cmd->blen);
                                                ctype = avl_cmd->cmd_type;
                                                body_uk = (char *)&from_server[sizeof(s_avl_cmd)];
                                                sprintf(chap, "data from server (%d) :", lenr);
                                                for (i = 0; i < lenr; i++) sprintf(chap+strlen(chap), "%02X",(uint8_t)from_server[i]);
                                                if (body_len > 0) {
                                                    memcpy(scmd, body_uk, body_len&0xff);
                                                    uk = strstr(scmd, "\r\n"); if (uk) *uk = '\0';
                                                    sprintf(chap+strlen(chap), "\n\tcmd='%s'", scmd);
                                                }
                                                sprintf(chap+strlen(chap), "\n");
                                                prints(__func__, chap, 1);
                                                // ----------   clear all   --------------
                                                memset(from_server, 0, buf_size);
                                                lenr = lenr_tmp = 0; ind = 0; lenr_wait = 4;
                                                body_uk = (char *)&from_server[0];
                                                body_len = 0;
                                                // ----------------------------------------
                                                icmd = parse_cmd(ctype, scmd, to_server, &com_id);
                                                if (icmd > 0) {
                                                    resa = send(connsocket, to_server, icmd, MSG_DONTWAIT);//send to server ack for command
                                                    sprintf(tmp,"Send ack for cmd(%d)='%s' :", com_id, scmd);
                                                    for (i = 0; i < icmd; i++) sprintf(tmp+strlen(tmp),"%02X",(uint8_t)to_server[i]);
                                                    sprintf(tmp+strlen(tmp),"\n");
                                                    prints(__func__, tmp, 1);
                                                    if (resa != icmd) { Vixod = 1; err |= 20; break; }// write error
                                                    else
                                                    if (com_id == 8) tmr_send = get_timer_sec(2);//high prio packet send
                                                }
                                            }
                                        break;
                                        default : {// byte > 0 | == kol_pack) // =1  server recv. packet OK
                                            wait_ack = 0;
                                            sprintf(chap, "data from server (%d) :", lenr);
                                            if (lenr > 0) { for (i = 0; i < lenr; i++) sprintf(chap+strlen(chap), "%02X",(uint8_t)from_server[i]); }
                                            sprintf(chap+strlen(chap), "\n");
                                            prints(__func__, chap, 1);
                                            memset(from_server, 0, buf_size); lenr = lenr_tmp = 0; ind = 0; lenr_wait = 4;
                                        }
                                    }//switch (from_server[3])
                                    if (Vixod) break;
                                }// recv. 4 bytes

                            }//else if (lenr_tmp > 0)

                        }//if (FD_ISSET(connsocket, &read_Fds))
                    }//if (select(connsocket + 1, &read_Fds, NULL, NULL, &cli_tv) > 0)
                break;

            }//switch(faza)

            if (wait_ack) {
                if (check_delay_sec(tmr_ack)) { err |= 8; Vixod = 1; }
            }

            //for box_relay
            for (j = 0; j < max_relay; j++) {//check timer[i] is done
                if (tmr_box[j] > 0) {//if timer was setup for box[i]
                    if (check_delay_sec(tmr_box[j])) {//if timeer is done -> relay OFF
                        tmr_box[j] = 0;
                        box[j].stat = 0;
                        sprintf(chap, "MAIN : relay_box %d set to 0 by timer (%u sec.) => relay new status :", j + 1, box[j].time);
                        box[j].time = 0;
                        for (ik = max_relay - 1; ik >= 0; ik--) sprintf(chap+strlen(chap), " %u", box[ik].stat);
                        sprintf(chap+strlen(chap), "\n");
                        prints(__func__, chap, 1);
                    }
                }
            }

            //for pin_relay
            for (j = 0; j < max_pin; j++) {//check timer[i] is done
                if (tmr_pin[j] > 0) {//if timer was setup for pin[i]
                    if (check_delay_sec(tmr_pin[j])) {//if timeer is done -> relay OFF
                        tmr_pin[j] = 0;
                        pin[j].stat = 0;
                        pthread_mutex_lock(&mirror_mutex);
                            switch (j) {
                                case 0: mirror.io1.val_dout1 = 0; break;
                                case 1: mirror.io1.val_dout2 = 0; break;
                                case 2: mirror.io1.val_dout3 = 0; break;
                                case 3: mirror.io1.val_dout4 = 0; break;
                            }
                        pthread_mutex_unlock(&mirror_mutex);
                        sprintf(chap, "MAIN : relay_pin %d set to 0 by timer (%u sec.) => pin new status :", j + 1, pin[j].time);
                        pin[j].time = 0;
                        for (ik = max_pin - 1; ik >= 0; ik--) sprintf(chap+strlen(chap), " %u", pin[ik].stat);
                        sprintf(chap+strlen(chap), "\n");
                        prints(__func__, chap, 1);
                    }
                }
            }

            usleep(10000);

        }/*while (!QuitAll && !Vixod)*/

        if (err) {
            sprintf(chap,"Error code 0x%02X :\n", err);
            if (err&1) sprintf(chap+strlen(chap),"\tSend imei to server error.\n");
            if (err&2) sprintf(chap+strlen(chap),"\tServer closed connection without answer.\n");
            if (err&4) sprintf(chap+strlen(chap),"\tServer reject connection (imei unknown).\n");
            if (err&8) sprintf(chap+strlen(chap),"\tTimeout recv. data from server.\n");
            if (err&0x10) sprintf(chap+strlen(chap),"\tInternal error.\n");
            if (err&0x20) sprintf(chap+strlen(chap),"\tSend packet to server error.\n");
            prints(__func__, chap, 1);
        }


out_of_job:

        if (err == 0xff) prints(__func__, "!!!   Jump error   !!!\n", 1);

        if (connsocket > 0) {
            shutdown(connsocket, SHUT_RDWR);
            close(connsocket);
            connsocket = -1;
        }

        if (!QuitAll) {
            stop_ses = time(NULL);
            sprintf(chap, "Sleep %u sec before try next connection...(ses %u sec.)\n\n", wait_before_new_connect, (uint32_t)stop_ses - (uint32_t)start_ses);
            prints(__func__, chap, 1);
            sleep(wait_before_new_connect);
        }

    }//while(!QuitAll)

stop_job:

    del_all_ones(1);

    sprintf(chap,"[%s] MAIN STOP server=%s:%d imei=%s mode=%u\n================================================================\n",
                 Version, line, tcp_port, im, work_mode);
    prints(NULL, chap, 1);

    if (fd_log > 0) close(fd_log);

    if (pid_main) unlink(pid_name);


    return 0;
}


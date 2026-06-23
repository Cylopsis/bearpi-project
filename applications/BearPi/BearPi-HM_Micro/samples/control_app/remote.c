#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "control.h"

#define SERVER_PORT     5000
#define RECV_BUFSZ      256
#define SEND_BUFSZ      700
#define MAX_ARGS        16
#define LISTEN_BACKLOG  5

static pthread_t g_server_thread;
static int g_server_running = 0;

/*
 * Serve a single accepted connection until the client disconnects.
 * One of these runs per client (detached thread), so multiple front-ends
 * (e.g. local HAP + a remote dashboard) can be connected at the same time.
 */
static void serve_client(int connected)
{
    char recv_buf[RECV_BUFSZ];
    char send_buf[SEND_BUFSZ];
    char *argv[MAX_ARGS];
    int argc;

    while (1) {
        int bytes_received = recv(connected, recv_buf, RECV_BUFSZ - 1, 0);
        if (bytes_received <= 0) {
            printf("[Remote] Client disconnected or recv error.\n");
            break;
        }

        recv_buf[bytes_received] = '\0';
        char *p = strpbrk(recv_buf, "\r\n");
        if (p) *p = '\0';

        argc = 0;
        char *saveptr;
        char *ptr = strtok_r(recv_buf, " ", &saveptr);
        while (ptr != NULL && argc < MAX_ARGS) {
            argv[argc++] = ptr;
            ptr = strtok_r(NULL, " ", &saveptr);
        }
        if (argc == 0) {
            continue;
        }

        if (strcmp(argv[0], "get_status") == 0) {
            const char *ptc_state_str = (ptc_state == HEAT) ? "ON" : "OFF";
            const char *ctrl_state_str = control_state_to_string(control_state);

            int len = snprintf(send_buf, sizeof(send_buf), "{"
                "\"current_ptc_temperature\":%.2f,"
                "\"current_temperature\":%.2f,"
                "\"box_pressure\":%.2f,"
                "\"box_humidity\":%.2f,"
                "\"target_temperature\":%.2f,"
                "\"ptc_target_temperature\":%.2f,"
                "\"ptc_state\":\"%s\","
                "\"control_state\":\"%s\","
                "\"current_pwm\":%.2f,"
                "\"heat_kp\":%.2f,\"heat_ki\":%.2f,\"heat_kd\":%.2f,"
                "\"box_kp\":%.2f,\"box_ki\":%.2f,\"box_kd\":%.2f,"
                "\"cool_kp\":%.2f,\"cool_ki\":%.2f,"
                "\"warming_bias\":%.2f,\"heating_bias\":%.2f,"
                "\"warming_threshold\":%.2f,\"hysteresis_band\":%.2f"
                "}\r\n",
                ptc_temperature, current_temperature, box_pressure, box_humidity,
                target_temperature, ptc_target_temp,
                ptc_state_str, ctrl_state_str, final_pwm_duty,
                pid_ptc.kp, pid_ptc.ki, pid_ptc.kd,
                pid_box.kp, pid_box.ki, pid_box.kd,
                pid_cool.kp, pid_cool.ki,
                warming_bias, heating_bias, warming_threshold, hysteresis_band);

            if (len < 0 || len >= SEND_BUFSZ) {
                printf("[Remote] JSON buffer overflow detected\n");
                continue;
            }
            if (send(connected, send_buf, (size_t)len, 0) < 0) {
                printf("[Remote] Send response failed.\n");
                break;
            }
        } else if (strcmp(argv[0], "tune") == 0) {
            tune(argc, argv);
            const char ok_msg[] = "OK\r\n";
            send(connected, ok_msg, sizeof(ok_msg) - 1, 0);
        } else {
            snprintf(send_buf, sizeof(send_buf), "ERROR: Unknown command '%s'.\r\n", argv[0]);
            send(connected, send_buf, strlen(send_buf), 0);
        }

        usleep(30 * 1000);
    }

    close(connected);
}

/* Per-client thread entry: takes ownership of the connected fd. */
static void *client_thread_entry(void *arg)
{
    int connected = (int)(intptr_t)arg;
    serve_client(connected);
    return NULL;
}

static void *remote_server_thread_entry(void *parameter)
{
    (void)parameter;
    int sock, connected;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("[Remote] Socket error\n");
        return NULL;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        printf("[Remote] Unable to bind\n");
        close(sock);
        return NULL;
    }

    if (listen(sock, LISTEN_BACKLOG) == -1) {
        printf("[Remote] Listen error\n");
        close(sock);
        return NULL;
    }

    printf("[Remote] TCP Server waiting for client on port %d...\n", SERVER_PORT);

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        if (connected < 0) {
            printf("[Remote] Accept connection failed! errno = %d\n", errno);
            continue;
        }
        printf("[Remote] Got a connection from (%s, %d)\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* Hand the connection off to a dedicated thread so the accept loop
         * stays free to serve additional clients concurrently. */
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread_entry,
                           (void *)(intptr_t)connected) != 0) {
            printf("[Remote] Failed to create client thread.\n");
            close(connected);
            continue;
        }
        pthread_detach(tid);
    }

    close(sock);
    return NULL;
}

void remote_start(void)
{
    if (g_server_running) {
        printf("[Remote] Server is already running.\n");
        return;
    }
    if (pthread_create(&g_server_thread, NULL, remote_server_thread_entry, NULL) != 0) {
        printf("[Remote] Failed to create TCP server thread.\n");
        return;
    }
    g_server_running = 1;
    printf("[Remote] TCP server started successfully.\n");
}

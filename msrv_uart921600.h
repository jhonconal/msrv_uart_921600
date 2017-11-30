#ifndef MSRV_UART921600_H
#define MSRV_UART921600_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include <termios.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <ctime>
#include <pthread.h>
#include <iostream>
using namespace std;

#if 1
//#define DEFAULT_DEVICE     "/dev/ttyS1"
#define DEFAULT_DEVICE   "/dev/ttyUSB0"
#define DEFAULT_BAUD_RATE   921600
#else
#define DEFAULT_DEVICE     "/dev/ttyS1"
#define DEFAULT_BAUD_RATE   115200
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)

class MSRV_UART921600
{
public:
    MSRV_UART921600();
    ~MSRV_UART921600();
    static  MSRV_UART921600* GetInstance();
    static  void DestoryInstance();

    void sleep_ms(unsigned int msec);
    //termios操作
    void show_termios(const struct termios *s);

    int get_termios(int fd, struct termios *s);
    int set_termios(int fd, const struct termios *s);

    int baud_to_speed(int baud);
    int get_speed(const struct termios *s);
    int get_ispeed(const struct termios *s);
    int get_ospeed(const struct termios *s);
    int get_bsize(const struct termios *s);
    char get_parity(const struct termios *s);
    int get_stop(const struct termios *s);

    int speed_to_baud(int speed);
    int set_speed(struct termios *s, int speed);
    int set_ispeed(struct termios *s, int speed);
    int set_ospeed(struct termios *s, int speed);
    int set_bsize(struct termios *s, int bsize);
    int set_parity(struct termios *s, char parity);
    int set_stop(struct termios *s, int stop);
    int enable_read(struct termios *s);
    int disable_read(struct termios *s);
    int enable_flow_control(struct termios *s);
    int disable_flow_control(struct termios *s);

    //串口操作
    int uart_open(const char *path);
    int uart_config(int fd, int baud, char parity, int bsize, int stop);
    int uart_read(int fd, char *buf, int len);
    int uart_write(int fd, const char *data, int len);
    int uart_close(int fd);

    void start();
private:
    static  MSRV_UART921600 *m_pInstance;
    static  void *Run(void*arg);
    pthread_t m_pthread;
};

#endif // MSRV_UART921600_H

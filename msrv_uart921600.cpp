#include "msrv_uart921600.h"

#define UART_SERVER
enum{
    U_MODE_FULL_422 = 0,
    U_MODE_HALF_485 = 1,
    U_MODE_FULL_232 = 2,
};
#define TIOCSERMODE	0x5460

static int uart_tty_fd = -1;

MSRV_UART921600*MSRV_UART921600::m_pInstance=NULL;

MSRV_UART921600::MSRV_UART921600()
{
    memset(&m_pthread, 0, sizeof(pthread_t));
}

MSRV_UART921600::~MSRV_UART921600()
{
    uart_close(uart_tty_fd);
}

MSRV_UART921600 *MSRV_UART921600::GetInstance()
{
    if(m_pInstance ==NULL)
    {
        m_pInstance = new MSRV_UART921600 ();
        assert(m_pInstance);
    }
    return m_pInstance;
}

void MSRV_UART921600::DestoryInstance()
{
    if(m_pInstance !=NULL)
    {
        delete m_pInstance;
        m_pInstance =NULL;
    }
}

void MSRV_UART921600::start()
{
    string  default_device = DEFAULT_DEVICE;
    const char *uart_tty_device = default_device.data();

    printf("=====>device:[%s]\n",uart_tty_device);
    struct termios old_termios,new_termios;

    uart_tty_fd = uart_open(uart_tty_device);
    if(uart_tty_device <0)
    {
        perror("uart_open() failed.\n");
        return ;
    }
    else
    {
        printf("uart_open fd:[%d]\n",uart_tty_fd);
    }

    ioctl(uart_tty_fd,TIOCSERMODE,U_MODE_FULL_232);
    get_termios(uart_tty_fd,&old_termios);
    printf("old termios:\n");
    show_termios(&old_termios);

    sleep_ms(100);
    uart_config(uart_tty_fd,DEFAULT_BAUD_RATE,'n',8,1);//配置tty信息

    get_termios(uart_tty_fd,&new_termios);
    printf("new termios:\n");
    show_termios(&new_termios);

#ifdef UART_SERVER  //主板服务
    while (1) {
        char rvbuf[1024]={0};
        char sndbuf[2048]={0};

        if(uart_read(uart_tty_fd,rvbuf,sizeof(rvbuf))>0)
        {
            //读取接收到的信息，再回显
            printf("----------->rvbuf[%d]: %s\n",(int)strlen(rvbuf),rvbuf);
            printf("----------->echo rvbuf to sender...\n");
            sprintf(sndbuf,"echo-->%s",rvbuf);
            uart_write(uart_tty_fd,sndbuf,strlen(sndbuf));
            printf("-------------------------------------------------------\n");
        }
        else
        {
            //未读取到串口信息，主动向串口写入数据
            memset(sndbuf,0,sizeof(sndbuf));
            sprintf(sndbuf,"uart read null data try to write something [%d]",rand()%100);
            uart_write(uart_tty_fd,sndbuf,strlen(sndbuf));
            printf("mstar board uart write data to ttsS1 with baund 921600 buf: %s.\n",sndbuf);
        }
        sleep_ms(1000);
    }
#else //PC服务
    int result= pthread_create(&m_pthread,NULL,Run,(void*)this);
    if(result==0)
    {
          printf("pthread_create() success.\n");
    }
    else
    {
         printf("pthread_create failed.\n");
    }
    while (1)
    {
        char sndbuf[1024]={0};
        srand((unsigned)time(NULL)); //用于保证是随机数
        sprintf(sndbuf,"write data from client with baund rate[921600]:[%d]",rand()%100);//用rand产生随机数并设定范围
        if(uart_write(uart_tty_fd,sndbuf,sizeof(sndbuf))>0)
        {
            printf("--->sndbuf[%d]: %s\n",(int)strlen(sndbuf),sndbuf);
        }
        else
        {
            printf("uart write null.\n");
        }
        tcflush(uart_tty_fd,TCIOFLUSH);
        sleep_ms(1000);
    }
    pthread_join(m_pthread,NULL);
#endif
}

void *MSRV_UART921600::Run(void *arg)
{
    MSRV_UART921600 *msrv = (MSRV_UART921600*)arg;
    char rvbuf[1024]={0};
    while (1)
    {
        if(msrv->uart_read(uart_tty_fd,rvbuf,sizeof(rvbuf))>0)
        {
            printf("====>read data from server whith baund rate[921600]:%s\n",rvbuf);
        }
        msrv->sleep_ms(100);
    }
    return ((void*)0);
}


void MSRV_UART921600::sleep_ms(unsigned int msec)
{
    struct timeval tval;
    tval.tv_sec = msec/1000;
    tval.tv_usec = (msec*1000)%1000000;
    select(0,NULL,NULL,NULL,&tval);
}

void MSRV_UART921600::show_termios(const termios *s)
{
    if( s )
    {
#if 0
        printf("speed=%d ", get_speed(s));
        printf("parity=%c ", get_parity(s));
        printf("bsize=%d ", get_bsize(s));
        printf("stop=%d\n", get_stop(s));
#endif
        printf("termios settings: speed=[%d] parity=[%c] bsize=[%d] stop=[%d]\n\n",get_speed(s),get_parity(s),get_bsize(s),get_stop(s));
    }
}

int MSRV_UART921600::get_termios(int fd, termios *s)
{
    if( -1 == fd || 0 == s )
    {
        return -1;
    }
    return tcgetattr(fd, s);
}

int MSRV_UART921600::set_termios(int fd, const termios *s)
{
    if( -1 == fd || 0 == s )
    {
        return -1;
    }
    return tcsetattr(fd, TCSANOW, s);
}

int MSRV_UART921600::baud_to_speed(int baud)
{
    switch( baud )
    {
    case B1200:
        return 1200;
    case B2400:
        return 2400;
    case B4800:
        return 4800;
    case B9600:
        return 9600;
    case B19200:
        return 19200;
    case B38400:
        return 38400;
    case B57600:
        return 57600;
    case B115200:
        return 115200;
    case B921600:
        return 921600;
    }
    return 0;
}

int MSRV_UART921600::get_speed(const termios *s)
{
    if( s )
    {
        return baud_to_speed(s->c_cflag & CBAUD);
    }
    return -1;
}

int MSRV_UART921600::get_ispeed(const termios *s)
{
    if( s )
    {
        return baud_to_speed(s->c_iflag & CBAUD);
    }
    return -1;
}

int MSRV_UART921600::get_ospeed(const termios *s)
{
    if( s )
    {
        return baud_to_speed(s->c_oflag & CBAUD);
    }
    return 0;
}

int MSRV_UART921600::get_bsize(const termios *s)
{
    if( s )
    {
        switch(s->c_cflag & CSIZE)
        {
        case CS5:
            return 5;
        case CS6:
            return 6;
        case CS7:
            return 7;
        case CS8:
            return 8;
        }
    }
    return -1;
}

char MSRV_UART921600::get_parity(const termios *s)
{
    if( s )
    {
        if( s->c_cflag & PARENB )
        {
            if( s->c_cflag & PARODD )
            {
                return 'O';
            }
            else
            {
                return 'E';
            }
        }
        return 'N';
    }
    return -1;
}

int MSRV_UART921600::get_stop(const termios *s)
{
    if( s )
    {
        if( s->c_cflag & CSTOPB )
        {
            return 2;
        }
        return 1;
    }
    return -1;
}

int MSRV_UART921600::speed_to_baud(int speed)
{
    switch(speed)
    {
    case 1200:
        return B1200;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 921600:
        return B921600;
    }
    return B9600;
}

int MSRV_UART921600::set_speed(termios *s, int speed)
{
    if( s )
    {
        s->c_cflag &= ~CBAUD;
        s->c_cflag |= speed_to_baud(speed);
        return 0;
    }
    return -1;
}

int MSRV_UART921600::set_ispeed(termios *s, int speed)
{
    if( s )
    {
        return cfsetispeed(s, speed_to_baud(speed) );
    }
    return -1;
}

int MSRV_UART921600::set_ospeed(termios *s, int speed)
{
    if( s )
    {
        return cfsetospeed(s, speed_to_baud(speed) );
    }
    return -1;
}

int MSRV_UART921600::set_bsize(termios *s, int bsize)
{
    if( s )
    {
        s->c_cflag &= ~CSIZE;
        switch( bsize )
        {
        case 5:
            s->c_cflag |= CS5;
            break;
        case 6:
            s->c_cflag |= CS6;
            break;
        case 7:
            s->c_cflag |= CS7;
            break;
        case 8:
            s->c_cflag |= CS8;
            break;
        }
        return 0;
    }
    return -1;
}

int MSRV_UART921600::set_parity(termios *s, char parity)
{
    if( s )
    {
        switch(parity)
        {
        case 'n':
        case 'N':
            s->c_cflag &= ~PARENB;
            break;
        case 'o':
        case 'O':
            s->c_cflag |= PARENB;
            s->c_cflag |= PARODD;
            break;
        case 'e':
        case 'E':
            s->c_cflag |= PARENB;
            s->c_cflag &= ~PARODD;
            break;
        }
        return 0;
    }
    return -1;
}

int MSRV_UART921600::set_stop(termios *s, int stop)
{
    if( s )
    {
        if( 1 == stop )
        {
            s->c_cflag &= ~CSTOPB;
        }
        else
        {
            s->c_cflag |= CSTOPB;
        }
        return 0;
    }
    return -1;
}

int MSRV_UART921600::enable_read(termios *s)
{
    if( s )
    {
        s->c_cflag |= CREAD;
        return 0;
    }
    return -1;
}

int MSRV_UART921600::disable_read(termios *s)
{
    if( s )
    {
        s->c_cflag &= ~CREAD;
        return 0;
    }
    return -1;
}

int MSRV_UART921600::enable_flow_control(termios *s)
{
    if( s )
    {
        s->c_cflag |= CRTSCTS;
        return 0;
    }
    return -1;
}

int MSRV_UART921600::disable_flow_control(termios *s)
{
    if( s )
    {
        s->c_cflag &= ~CRTSCTS;
        return 0;
    }
    return -1;
}

int MSRV_UART921600::uart_open(const char *path)
{
    if( path )
    {
        return open(path, O_RDWR);
    }
    return -1;
}

int MSRV_UART921600::uart_config(int fd, int baud, char parity, int bsize, int stop)
{
    if( fd < 0 )
    {
        return -1;
    }
    else
    {
        struct termios new_termios = {0};

        bzero(&new_termios, sizeof(new_termios));
        if( set_speed(&new_termios, baud))
        {
            return -2;
        }
        if( set_parity(&new_termios, parity))
        {
            return -3;
        }
        if( set_bsize(&new_termios, bsize))
        {
            return -4;
        }
        if( set_stop(&new_termios, stop))
        {
            return -5;
        }
        enable_read(&new_termios);
        disable_flow_control(&new_termios);
        if( set_termios(fd, &new_termios))
        {
            return -6;
        }
    }
    return 0;
}

int MSRV_UART921600::uart_read(int fd, char *buf, int len)
{
    if( fd < 0 || 0 == buf || len < 1 )
    {
        return -1;
    }
    return read(fd, buf, len);
}

int MSRV_UART921600::uart_write(int fd, const char *data, int len)
{
    if( fd < 0 || 0 == data || len < 1 )
    {
        return -1;
    }
    return write(fd, data, len);
}

int MSRV_UART921600::uart_close(int fd)
{
    return close(fd);
}



SOURCES= main.cpp msrv_uart921600.cpp 

LINUX_SRC = $(SOURCES)
LINUX_APP = MSRV_UART921600
CC=g++
#CC=aarch64-linux-gnu-g++

all: $(LINUX_APP)

$(LINUX_APP): $(LINUX_SRC)
	$(CC) -pthread $^ -o $@ $(LDFLAGS) -static

install: all

clean:
	@rm -f $(LINUX_APP)

check:

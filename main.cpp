#include "msrv_uart921600.h"
#include <iostream>
using namespace std;

int main()
{
    MSRV_UART921600::GetInstance()->start();
    return 0;
}




#include "eth.hpp"
#include "socket.hpp"
#include "iocp.hpp"

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::setw;
using std::internal;
using std::fixed;
using std::setfill;

using namespace std;


int main(void)
{
    auto iocp = IOCP();
    auto sock = Socket {PF_INET, SOCK_STREAM, IPPROTO_TCP};
    auto handleData = PerHandleData(HandleType::HANDLE_SOCKET, sock.getHandle());
    iocp.registerHandle(handleData);
}


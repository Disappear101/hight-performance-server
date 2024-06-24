#include "../src/fdmanager.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <iostream>
//#include "../src/hook.h"

int main(int argc, char** argv) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    // bool m_isSocket = false;
    // struct stat fd_stat;
    // if(-1 == fstat(fd, &fd_stat)) {
    //     m_isSocket = false;
    // } else {
    //     //m_isInit = true;
    //     m_isSocket = S_ISSOCK(fd_stat.st_mode);
    // }

    // if (m_isSocket) {
    //     std::cout << "yes" << std::endl;
    // }

    tao::FdMgr::GetInstance()->get(fd, true);
    
    return 0;
}
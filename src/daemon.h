#ifndef __TAO_DAEMON_H__
#define __TAO_DAEMON_H__

#include <functional>
#include <string>
#include "singleton.h"

namespace tao {

struct  ProcessInfo
{
    /// parent process id
    pid_t parent_id = 0;
    /// main process id
    pid_t main_id = 0;
    /// start time of parent process
    uint64_t parent_start_time = 0;
    /// start time of main process
    uint64_t main_start_time = 0;
    /// restart count of main process
    uint32_t restart_count = 0;

    std::string toString() const;
};

using ProcessInfoMgr = tao::Singleton<ProcessInfo>;

int start_daemon(int argc, char** argv
                , std::function<int(int argc, char** argv)> main_cb
                , bool is_daemon);

}

#endif
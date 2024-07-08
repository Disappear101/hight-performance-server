#ifndef __TAO_FD_MANAGER_H__
#define __TAO_FD_MANAGER_H__

#include <memory>
#include <vector>
#include "iomanager.h"
#include "mutex.h"
#include "singleton.h"

namespace tao {
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    using ptr = std::shared_ptr<FdCtx>;

    FdCtx(int fd);
    ~FdCtx();

    bool init();
    bool isInit() const { return m_isInit;}
    bool isSocket() const { return m_isSocket;}
    bool isClosed() const { return m_isClosed;}
    bool close();

    void setUserNonBlock(bool v) { m_userNonblock = v;}
    bool getUserNonBlock() const { return m_userNonblock;}

    void getSysNonBlock(bool v) { m_sysNonblock = v;}
    bool getSysNonBlock() const { return m_sysNonblock;}

    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);

private:
    bool m_isInit = true;
    bool m_isSocket = true;
    bool m_isNonblock = true;
    bool m_sysNonblock = true;
    bool m_userNonblock = true;
    bool m_isClosed = true;
    int m_fd;
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
    tao::IOManager* m_iomanager;
};

class FdManager {
public:
    using RWMutexType = RWMutex;

    FdManager();

    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);

private:
    RWMutexType m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

using FdMgr = Singleton<FdManager>;

}

#endif
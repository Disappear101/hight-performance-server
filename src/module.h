#ifndef __TAO_MODULE_H__
#define __TAO_MODULE_H__

#include "src/stream.h"
#include "src/singleton.h"
#include "src/mutex.h"
#include "src/protocol.h"
#include <map>
#include <unordered_map>


namespace tao {

class Module {
public:
    enum Type {
        MODULE = 0,
        ROCK = 1
    };
    using ptr = std::shared_ptr<Module>;
    Module(const std::string& name
        ,const std::string& version
        ,const std::string& filename
        ,uint32_t type = Type::MODULE);
    virtual ~Module() {}

    virtual void onBeforeArgsParse(int argc, char** argv);
    virtual void onAfterArgsParse(int argc, char** argv);

    virtual bool onLoad();
    virtual bool onUnload();

    virtual bool onConnect(tao::Stream::ptr stream);
    virtual bool onDisconnect(tao::Stream::ptr stream);
    
    virtual bool onServerReady();
    virtual bool onServerUp();

    virtual bool handleRequest(tao::Message::ptr req
                               ,tao::Message::ptr rsp
                               ,tao::Stream::ptr stream);
    virtual bool handleNotify(tao::Message::ptr notify
                              ,tao::Stream::ptr stream);

    virtual std::string statusString();

    const std::string& getName() const { return m_name;}
    const std::string& getVersion() const { return m_version;}
    const std::string& getFilename() const { return m_filename;}
    const std::string& getId() const { return m_id;}

    void setFilename(const std::string& v) { m_filename = v;}

    uint32_t getType() const { return m_type;}

    void registerService(const std::string& server_type,
            const std::string& domain, const std::string& service);
protected:
    std::string m_name;
    std::string m_version;
    std::string m_filename;
    std::string m_id;
    uint32_t m_type;
};
}

#endif
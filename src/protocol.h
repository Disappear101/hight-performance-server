#ifndef __TAO_PROTOCOL_H__
#define __TAO_PROTOCOL_H__

#include <functional>
#include <memory>
#include "bytearray.h"

namespace tao {

class Message {
public:
    using ptr = std::shared_ptr<Message>;

    enum MessageType {
        REQUEST = 1,
        RESPONSE = 2,
        NOTIFY = 3
    };
    virtual ~Message() {}

    virtual ByteArray::ptr toByteArray();
    virtual bool serializeToByteArray(ByteArray::ptr bytearray) = 0;
    virtual bool parseFromByteArray(ByteArray::ptr bytearray) = 0;

    virtual std::string toString() const = 0;
    //virtual const std::string& getName() const = 0;

    int32_t getType() const { return m_type;}
    void setType(MessageType type) { m_type = type;}

protected:
    uint8_t m_type;
};

class Request : public Message {
public:
    using ptr = std::shared_ptr<Request>;
    Request();

    uint32_t getSn() const { return m_sn;}
    uint32_t getCmd() const { return m_cmd;}

    void setSn(uint32_t v) { m_sn = v;}
    void setCmd(uint32_t v) { m_cmd = v;}

    virtual bool serializeToByteArray(ByteArray::ptr bytearray) override;
    virtual bool parseFromByteArray(ByteArray::ptr bytearray) override;
protected:
    uint32_t m_sn;
    uint32_t m_cmd;

};

class Response : public Message {
public:
    using ptr = std::shared_ptr<Response>;

    Response();

    uint32_t getSn() const { return m_sn;}
    uint32_t getCmd() const { return m_cmd;}
    uint32_t getResult() const { return m_result;}
    const std::string& getResultStr() const { return m_resultStr;}

    void setSn(uint32_t v) { m_sn = v;}
    void setCmd(uint32_t v) { m_cmd = v;}
    void setResult(uint32_t v) { m_result = v;}
    void setResultStr(const std::string& v) { m_resultStr = v;}

    virtual bool serializeToByteArray(ByteArray::ptr bytearray) override;
    virtual bool parseFromByteArray(ByteArray::ptr bytearray) override;
protected:
    uint32_t m_sn;
    uint32_t m_cmd;
    uint32_t m_result;
    std::string m_resultStr;
};

class Notify : public Message {
public:
    using ptr = std::shared_ptr<Notify>;

    Notify();

    uint32_t getNotify() const { return m_notify;}
    void setNotify(uint32_t v) { m_notify = v;}

    virtual bool serializeToByteArray(ByteArray::ptr bytearray) override;
    virtual bool parseFromByteArray(ByteArray::ptr bytearray) override;
protected:
    uint32_t m_notify;
};

}

#endif
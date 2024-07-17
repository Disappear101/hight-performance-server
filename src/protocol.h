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
    virtual const std::string& getName() const = 0;
    virtual int32_t getType() const = 0;

};

class Request : public Message {
public:
    using ptr = std::shared_ptr<Request>;
    Request();

private:


};

}

#endif
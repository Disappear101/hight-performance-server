#ifndef __TAO_STREAM_H__
#define __TAO_STREAM_H__

#include <memory>
#include "bytearray.h"

namespace tao {

class Stream {
public:
    using ptr = std::shared_ptr<Stream>;
    virtual ~Stream() {}

    virtual int read(void* buffer, size_t length) = 0;
    virtual int read(tao::ByteArray::ptr ba, size_t length) = 0;
    virtual int readFixSize(void* buffer, size_t length);
    virtual int readFixSize(ByteArray::ptr ba, size_t length);
    virtual int write(const void* buffer, size_t length) = 0;
    virtual int write(tao::ByteArray::ptr ba, size_t length) = 0;
    virtual int writeFixSize(const void* buffer, size_t length);
    virtual int writeFixSize(ByteArray::ptr ba, size_t length);
    virtual bool close() = 0;

private:
};

}


#endif
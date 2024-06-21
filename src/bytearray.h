#ifndef __TAO_BYTEARRAY_H__
#define __TAO_BYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>

namespace tao {

class ByteArray {
public:
    using ptr = std::shared_ptr<ByteArray>;

    struct Node
    {
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;
        Node* next;
        size_t size;
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    /*------------------------write-------------------------*/
    //fixed length interger
    void writeFint8(int8_t value);
    void writeFuint8 (uint8_t value);
    void writeFint16 (int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32 (int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64 (int64_t value);
    void writeFuint64(uint64_t value);

    //
    //void writeInt8();
    //void writeUint8 (uint8_t value);
    //void writeInt16 (int16_t value);
    //void writeUint16(uint16_t value);
    void writeInt32 (int32_t value);
    void writeUint32(uint32_t value);
    void writeInt64 (int64_t value);
    void writeUint64(uint64_t value);

    void writeFloat  (float value);
    void writeDouble (double value);

    //fixed lenghth string
    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);
    void writeStringVint(const std::string& value);

    void writeStringWithoutLength(const std::string& value);

    /*------------------------read-------------------------*/
    //fix length
    int8_t  readFint8();
    uint8_t readFuint8();
    int16_t  readFint16();
    uint16_t readFuint16();
    int32_t  readFint32();
    uint32_t readFuint32();
    int64_t  readFint64();
    uint64_t readFuint64();

    //7-bit Compression method
    int32_t  readInt32();
    uint32_t readUint32();
    int64_t  readInt64();
    uint64_t readUint64();

    float    readFloat();
    double   readDouble();

    //fixed lenghth string
    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();

    void clear();

    /*
    buf: write buffer
    size: number of bytes
    */
    void write(const void* buf, size_t size);
    void read(void* buf, size_t size);
    void read(void* buf, size_t size, size_t position) const;
    size_t getPosition() {return m_position;}
    void setPosition(size_t v);

    //FILE DEBUG 
    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);

    std::string toString() const;
    std::string toHexString() const;

private:
    void addCapacity(size_t size);
    //get current available memory
    size_t getCapacity() {return m_capacity - m_position;};
    //get size of single memory block
    int getBaseSize() const { return m_baseSize;}
    //get readable size of entire memory pool
    size_t getReadableSize() const { return m_size - m_position;};
private:
    size_t m_baseSize;  //size of single memory block
    size_t m_position;  //global current operating positiom across memory blocks
    size_t m_capacity;  //total capacity
    size_t m_size;      //total memory size
    int8_t m_endian;    //byte order
    
    Node* m_root;       //root memory block
    Node* m_cur;        //current memory block
};

}

#endif
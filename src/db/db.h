#ifndef __TAO_DB_DB_H__
#define __TAO_DB_DB_H__

#include <memory>
#include <string>

namespace tao {

/// @brief Represents the result of a SQL query.
class ISQLData {
public:
    using ptr = std::shared_ptr<ISQLData>;
    virtual ~ISQLData() {}

    virtual int getErrno() const = 0;
    virtual const std::string& getErrStr() const = 0;

    virtual int getDataCount() = 0;
    virtual int getColumnCount() = 0;
    virtual int getColumnBytes(int idx) = 0;
    virtual int getColumnType(int idx) = 0;
    virtual std::string getColumnName(int idx) = 0;

    virtual bool isNull(int idx) = 0;
    virtual int8_t getInt8(int idx) = 0;
    virtual uint8_t getUint8(int idx) = 0;
    virtual int16_t getInt16(int idx) = 0;
    virtual uint16_t getUint16(int idx) = 0;
    virtual int32_t getInt32(int idx) = 0;
    virtual uint32_t getUint32(int idx) = 0;
    virtual int64_t getInt64(int idx) = 0;
    virtual uint64_t getUint64(int idx) = 0;
    virtual float getFloat(int idx) = 0;
    virtual double getDouble(int idx) = 0;
    virtual std::string getString(int idx) = 0;
    virtual std::string getBlob(int idx) = 0;
    virtual time_t getTime(int idx) = 0;
    virtual bool next() = 0;
};

/// @brief Represents SQL update operations.
class ISQLUpdate {
public:
    virtual ~ISQLUpdate() {}
    virtual int execute(const char* format, ...) = 0;
    virtual int execute(const std::string& sql) = 0;

    /// @brief Retrieve the ID of the last inserted row.
    virtual int64_t getLastInsertId() = 0;
};

/// @brief Represents SQL query operations.
class ISQLQuery {
public:
    virtual ~ISQLQuery() {}
    virtual ISQLData::ptr query(const char* format, ...) = 0;
    virtual ISQLData::ptr query(const std::string& sql) = 0;
};


/// @brief Represents a prepared SQL statement.
class IStmt {
public:
    using ptr = std::shared_ptr<IStmt>;

    virtual ~IStmt(){}
    virtual int bindInt8(int idx, const int8_t& value) = 0;
    virtual int bindUint8(int idx, const uint8_t& value) = 0;
    virtual int bindInt16(int idx, const int16_t& value) = 0;
    virtual int bindUint16(int idx, const uint16_t& value) = 0;
    virtual int bindInt32(int idx, const int32_t& value) = 0;
    virtual int bindUint32(int idx, const uint32_t& value) = 0;
    virtual int bindInt64(int idx, const int64_t& value) = 0;
    virtual int bindUint64(int idx, const uint64_t& value) = 0;
    virtual int bindFloat(int idx, const float& value) = 0;
    virtual int bindDouble(int idx, const double& value) = 0;
    virtual int bindString(int idx, const char* value) = 0;
    virtual int bindString(int idx, const std::string& value) = 0;
    virtual int bindBlob(int idx, const void* value, int64_t size) = 0;
    virtual int bindBlob(int idx, const std::string& value) = 0;
    virtual int bindTime(int idx, const time_t& value) = 0;
    virtual int bindNull(int idx) = 0;

    virtual int execute() = 0;
    virtual int64_t getLastInsertId() = 0;
    virtual ISQLData::ptr query() = 0;

    virtual int getErrno() = 0;
    virtual std::string getErrStr() = 0;
};

/// @brief Represents a database transaction.
class ITransaction : public ISQLUpdate {
public:
    typedef std::shared_ptr<ITransaction> ptr;
    virtual ~ITransaction() {};
    virtual bool begin() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};

/// @brief Represents the main database interface.
class IDB : public ISQLUpdate
            ,public ISQLQuery {
public:
    using ptr = std::shared_ptr<IDB>;
    virtual ~IDB() {}

    /// @brief Prepare a SQL statement.
    /// @param stmt SQL statement in string
    /// @return IStmt
    virtual IStmt::ptr prepare(const std::string& stmt) = 0;
    virtual int getErrno() = 0;
    virtual std::string getErrStr() = 0;

    /// @brief Open a database transaction.
    /// @param auto_commit auto commit
    /// @return transaction
    virtual ITransaction::ptr openTransaction(bool auto_commit = false) = 0;
};

}

#endif
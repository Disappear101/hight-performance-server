#ifndef __TAO__ADDRESS_H__
#define __TAO__ADDRESS_H__

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

namespace tao {

class IPAddress;

class Address {
public:
    using ptr = std::shared_ptr<Address>;
    virtual ~Address() {};

    //create from struct sockaddr
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);

    //get all matched Address by host(domain name, IP, server name) 
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    //return any matched Address
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    //return any matched IPAddress
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    //get all network cards<network name, <address, subnet preix_len>>
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);

    //get <netwwork card address, subnet prefix len> according to specified network card name
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);

    int getFamily() const;

    virtual sockaddr* getAddr() const = 0;
    virtual socklen_t getAddrLen() const = 0;

    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString();

    bool operator<(const Address& r) const;
    bool operator==(const Address& r) const;
    bool operator!=(const Address& r) const;
};

class IPAddress : public Address {
public:
    using ptr = std::shared_ptr<IPAddress>;
    virtual ~IPAddress() {};
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;

    //create IPAddress rom domain name, IP and server name.
    static IPAddress::ptr Create(const char* address, uint16_t port = 0);
};

class IPv4Address : public IPAddress {
public:
    using ptr = std::shared_ptr<IPv4Address>;
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    IPv4Address(const sockaddr_in& address);
    ~IPv4Address();

    //use decimal-dot format to create IPv4Address(192.128.1.1)
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

    sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    using ptr = std::shared_ptr<IPv6Address>;
    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    ~IPv6Address();

    //create IPv6Address from IPv6 string
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);

    sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
    using ptr = std::shared_ptr<UnixAddress>;
    UnixAddress();
    UnixAddress(const std::string& path);
    ~UnixAddress();

    sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::ostream& insert(std::ostream& os) const override;
private:
    struct sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknowAddress : public Address {
public:
    using ptr = std::shared_ptr<UnknowAddress>;
    UnknowAddress(int family);
    UnknowAddress(const sockaddr& addr);
    ~UnknowAddress();

    sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

private:
    sockaddr m_addr;
};

}

#endif
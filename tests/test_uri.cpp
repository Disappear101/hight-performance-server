#include "../src/http/uri.h"
#include <iostream>

int main(int argc, char** argv) {
    //tao::Uri::ptr uri = tao::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
    tao::Uri::ptr uri = tao::Uri::Create("http://admin@www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    //tao::Uri::ptr uri = tao::Uri::Create("http://admin@www.sylar.top");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
    return 0;
}

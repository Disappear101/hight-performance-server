#ifndef __TAO_NONCOPYABLE_H__
#define __TAO_NONCOPYABLE_H__

namespace tao {

class Noncopyable {
public:
    Noncopyable() = default;

    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;

    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif
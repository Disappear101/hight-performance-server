#ifndef __TAO_LIBRARY_H__
#define __TAO_LIBRARY_H__

#include <memory>
#include "module.h"

namespace tao {

class Library {
public:
    static Module::ptr GetModule(const std::string& path);
};

}

#endif

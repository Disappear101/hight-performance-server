#include "src/application.h"

int main(int argc, char** argv) {
    tao::Application app;
    if(app.init(argc, argv)) {
        return app.run();
    }
    return 0;
}

#include "app.hpp"

int main() {

    int window_width  = 1900;
    int window_height = 1080;

    App app(window_width, window_height);
    app.run();

    return 0;
}

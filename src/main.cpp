#include "core/Application.hpp"

int main() {
    Zhenzhu::Application app;
    app.Init();
    app.Run();
    app.Shutdown();
    return 0;
}

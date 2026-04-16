#include "main.hpp"
#include "panel.hpp"
#include "wnd_proc.hpp"

main::main() {
    hookMainloop_.install(std::make_tuple(this, &main::hookedMainloop));
}

main::~main() {
    hookMainloop_.remove();
}

void main::hookedMainloop(oMain orig) {
    static bool initialized = false;
    if (!initialized) {
        static const panel panel_;
        static const wndProc wndProc_;
        initialized = true;
    }
    return orig();
}

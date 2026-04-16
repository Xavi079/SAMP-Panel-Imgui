#include "main.hpp"
#include "samp_base.hpp"

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        // Wait for SAMP to be loaded
        if (!sampBase::getSampBase()) return FALSE;
        static const main main_;
    }
    return TRUE;
}

#pragma once
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#include <memory>
#include "lemon/detour.hpp"

class panel {
public:
    panel();
    ~panel();

    static inline bool menuOpen{ false };

private:
    using oReset   = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    using oPresent = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);

    std::unique_ptr<lemon::detour<oReset>>   hookReset_;
    std::unique_ptr<lemon::detour<oPresent>> hookPresent_;

    HRESULT hookedReset  (oReset   orig, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPP);
    HRESULT hookedPresent(oPresent orig, IDirect3DDevice9* pDevice,
                          const RECT* pSrc, const RECT* pDest, HWND hDest, const RGNDATA* pDirty);

    [[nodiscard]] IDirect3DDevice9* getDevice() const noexcept;
    [[nodiscard]] HWND              getHwnd()   const noexcept;

    void imguiCreate (IDirect3DDevice9* pDevice) const;
    void imguiDestroy() const;
    void imguiRender () const;
    void imguiStyle  () const;

    void tabMain () const;
    void tabAbout() const;
};

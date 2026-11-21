#pragma once

#include <windows.h>

#include <memory>

#include <QRect>

namespace mf {

inline QRect windowRect(HWND hwnd) {
    if(!hwnd)
        return {};
    RECT rect{};
    if(!GetWindowRect(hwnd, &rect))
        return {};
    return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
}

template<typename T>
struct ComRelease {
    void operator()(T* pointer) const {
        if(pointer)
            pointer->Release();
    }
};

template<typename T>
using ComPtr = std::unique_ptr<T, ComRelease<T>>;

class ScopedHandle {
    public:
        explicit ScopedHandle(HANDLE handle = INVALID_HANDLE_VALUE) : m_handle(handle) {}
        ~ScopedHandle() {
            if(m_handle != INVALID_HANDLE_VALUE)
                CloseHandle(m_handle);
        }

        ScopedHandle(const ScopedHandle&) = delete;
        ScopedHandle& operator=(const ScopedHandle&) = delete;

        HANDLE get() const {
            return m_handle;
        }
        bool valid() const {
            return m_handle != INVALID_HANDLE_VALUE;
        }

    private:
        HANDLE m_handle;
};

} // namespace mf

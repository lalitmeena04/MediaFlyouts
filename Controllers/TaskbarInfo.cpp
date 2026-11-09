#include "TaskbarInfo.h"

#include <cstdint>
#include <memory>

#include <QWindow>
#include <windows.h>

// WIN32_LEAN_AND_MEAN drops the COM headers UIAutomation.h relies on.
#include <objbase.h>
#include <oleauto.h>
#include <UIAutomation.h>

#include "Logging.h"
#include "MonitorMapper.h"
#include "TaskbarLayout.h"

namespace {

QRect nativeWindowRect(HWND hwnd) {
    if(!hwnd)
        return QRect();
    RECT r{};
    if(!GetWindowRect(hwnd, &r))
        return QRect();
    return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

// Each monitor draws its own taskbar. Secondary ones use a different class.
HWND taskbarWindowForMonitor(HMONITOR monitor) {
    HWND primary = FindWindowW(L"Shell_TrayWnd", nullptr);
    if(!monitor)
        return primary;
    if(primary && MonitorFromWindow(primary, MONITOR_DEFAULTTONULL) == monitor)
        return primary;

    HWND secondary = nullptr;
    while((secondary = FindWindowExW(nullptr, secondary, L"Shell_SecondaryTrayWnd", nullptr)) != nullptr) {
        if(MonitorFromWindow(secondary, MONITOR_DEFAULTTONULL) == monitor)
            return secondary;
    }
    return nullptr;
}

// Secondary taskbars carry a clock instead of the full tray on Windows 10.
HWND trayChildOf(HWND taskbar) {
    if(!taskbar)
        return nullptr;
    if(HWND notify = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr))
        return notify;
    return FindWindowExW(taskbar, nullptr, L"ClockButton", nullptr);
}

template <typename T> struct ComRelease {
        void operator()(T* p) const {
            if(p)
                p->Release();
        }
};
template <typename T> using ComPtr = std::unique_ptr<T, ComRelease<T>>;

// Windows 11 secondary taskbars have no Win32 tray child. The clock and
// notification button are XAML controls. Only UI Automation exposes their bounds.
// Every tray control shares the SystemTrayIcon automation id.
QRect trayRectViaAutomation(HWND taskbar) {
    if(!taskbar)
        return QRect();

    IUIAutomation* rawAutomation = nullptr;
    if(FAILED(CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation),
                               reinterpret_cast<void**>(&rawAutomation))) ||
       !rawAutomation) {
        return QRect();
    }
    ComPtr<IUIAutomation> automation(rawAutomation);

    IUIAutomationElement* rawRoot = nullptr;
    if(FAILED(automation->ElementFromHandle(taskbar, &rawRoot)) || !rawRoot)
        return QRect();
    ComPtr<IUIAutomationElement> root(rawRoot);

    VARIANT idValue;
    VariantInit(&idValue);
    idValue.vt = VT_BSTR;
    idValue.bstrVal = SysAllocString(L"SystemTrayIcon");
    IUIAutomationCondition* rawCondition = nullptr;
    const HRESULT condHr = automation->CreatePropertyCondition(UIA_AutomationIdPropertyId, idValue, &rawCondition);
    VariantClear(&idValue);
    if(FAILED(condHr) || !rawCondition)
        return QRect();
    ComPtr<IUIAutomationCondition> condition(rawCondition);

    IUIAutomationElementArray* rawFound = nullptr;
    if(FAILED(root->FindAll(TreeScope_Descendants, condition.get(), &rawFound)) || !rawFound)
        return QRect();
    ComPtr<IUIAutomationElementArray> found(rawFound);

    int count = 0;
    found->get_Length(&count);
    QRect unionRect;
    for(int i = 0; i < count; ++i) {
        IUIAutomationElement* rawItem = nullptr;
        if(FAILED(found->GetElement(i, &rawItem)) || !rawItem)
            continue;
        ComPtr<IUIAutomationElement> item(rawItem);
        RECT r{};
        if(FAILED(item->get_CurrentBoundingRectangle(&r)))
            continue;
        const QRect bounds(r.left, r.top, r.right - r.left, r.bottom - r.top);
        if(bounds.isEmpty())
            continue;
        unionRect = unionRect.isNull() ? bounds : unionRect.united(bounds);
    }
    return unionRect;
}

QRect trayRectOf(HWND taskbar) {
    const QRect win32 = nativeWindowRect(trayChildOf(taskbar));
    if(!win32.isEmpty())
        return win32;
    return trayRectViaAutomation(taskbar);
}

TaskbarInfo* g_taskbarInfo = nullptr;
HWINEVENTHOOK g_winEventHook = nullptr;

void pinWindowTopmost(HWND hwnd) {
    if(!hwnd)
        return;
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

// Shell can move taskbar above app windows. Re-pin tracked windows.
void CALLBACK winEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    TRC_DEBUG("winEventProc: event={:#x} hwnd={:#x}", event, reinterpret_cast<std::uintptr_t>(hwnd));
    if(g_taskbarInfo)
        g_taskbarInfo->handleShellEvent(event);
}
} // namespace

TaskbarInfo::TaskbarInfo(const MonitorMapper* monitors, QObject* parent) : QObject(parent), m_monitors(monitors) {
    g_taskbarInfo = this;
    // The taskbar can move above widgets more than once during the minimize animation.
    m_repinBurst.setInterval(30);
    connect(&m_repinBurst, &QTimer::timeout, this, [this] {
        repinTracked();
        if(--m_repinBurstTicks <= 0)
            m_repinBurst.stop();
    });
    // OUTOFCONTEXT runs the callback on this GUI thread.
    g_winEventHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_MINIMIZEEND, nullptr, winEventProc, 0, 0,
                                     WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    TRC_INFO("TaskbarInfo: WinEvent hook installed={}", g_winEventHook != nullptr);
}

TaskbarInfo::~TaskbarInfo() {
    if(g_winEventHook) {
        UnhookWinEvent(g_winEventHook);
        g_winEventHook = nullptr;
    }
    g_taskbarInfo = nullptr;
}

bool TaskbarInfo::cacheIsFresh(const CachedReserve& entry, const MonitorEntry& monitor) const {
    constexpr qint64 kCacheMs = 2000;
    if(!entry.age.isValid() || entry.age.elapsed() >= kCacheMs)
        return false;
    if(entry.availableRect != monitor.availableRect || !qFuzzyCompare(entry.devicePixelRatio, monitor.devicePixelRatio)) {
        TRC_DEBUG("taskbar reserve: monitor {} layout changed, cache dropped", monitor.index);
        return false;
    }
    return true;
}

mf::TaskbarReserve TaskbarInfo::reserveFor(const MonitorEntry& monitor) const {
    mf::TaskbarReserve reserve;
    HWND taskbar = taskbarWindowForMonitor(static_cast<HMONITOR>(monitor.nativeHandle));
    if(!taskbar)
        return reserve;

    auto cached = m_reserveCache.find(taskbar);
    if(cached != m_reserveCache.end() && cacheIsFresh(*cached, monitor))
        return cached->reserve;

    reserve.trailing = MonitorMapper::toLogical(monitor, trayRectOf(taskbar));
    reserve.leading =
        MonitorMapper::toLogical(monitor, nativeWindowRect(FindWindowExW(taskbar, nullptr, L"Start", nullptr)));
    // A null monitor handle falls back to the primary taskbar. Its chrome may lie off this screen.
    reserve = mf::reserveOnScreen(monitor.logicalRect, reserve);
    TRC_DEBUG("taskbar reserve: monitor={} leading=({},{} {}x{}) trailing=({},{} {}x{})", monitor.index,
              reserve.leading.x(), reserve.leading.y(), reserve.leading.width(), reserve.leading.height(),
              reserve.trailing.x(), reserve.trailing.y(), reserve.trailing.width(), reserve.trailing.height());

    CachedReserve& slot = m_reserveCache[taskbar];
    slot.reserve = reserve;
    slot.availableRect = monitor.availableRect;
    slot.devicePixelRatio = monitor.devicePixelRatio;
    slot.age.start();
    return reserve;
}

mf::TaskbarEdge TaskbarInfo::edgeFor(const MonitorEntry& monitor) const {
    const HWND taskbar = taskbarWindowForMonitor(static_cast<HMONITOR>(monitor.nativeHandle));
    const QRect window = MonitorMapper::toLogical(monitor, nativeWindowRect(taskbar));
    const mf::TaskbarEdge edge = mf::detectTaskbarEdge(monitor.logicalRect, monitor.availableRect, window);
    auto& last = m_lastEdge[monitor.index];
    if(!last.has_value() || *last != edge) {
        TRC_INFO("taskbar edge: monitor={} edge={} available=({},{} {}x{}) window=({},{} {}x{})", monitor.index,
                 static_cast<int>(edge), monitor.availableRect.x(), monitor.availableRect.y(),
                 monitor.availableRect.width(), monitor.availableRect.height(), window.x(), window.y(),
                 window.width(), window.height());
        last = edge;
    }
    return edge;
}

int TaskbarInfo::taskbarEdge(int monitor) const {
    const MonitorEntry* entry = m_monitors ? m_monitors->entry(monitor) : nullptr;
    return static_cast<int>(entry ? edgeFor(*entry) : mf::TaskbarEdge::Bottom);
}

bool TaskbarInfo::widgetFits(int monitor) const {
    return mf::isHorizontalEdge(static_cast<mf::TaskbarEdge>(taskbarEdge(monitor)));
}

QPoint TaskbarInfo::widgetPosition(int monitor, int side, int offset, int width, int height) const {
    const MonitorEntry* entry = m_monitors ? m_monitors->entry(monitor) : nullptr;
    if(!entry)
        return QPoint(0, 0);

    const QRect full = entry->logicalRect;
    const int y = mf::taskbarWidgetY(full, entry->availableRect, edgeFor(*entry), height);
    const int x = mf::taskbarWidgetX(full, reserveFor(*entry), static_cast<mf::TaskbarSide>(side), offset, width, 8);
    return QPoint(x, y);
}

void TaskbarInfo::pinTopmost(QWindow* window) const {
    if(!window)
        return;
    if(!m_pinned.contains(window))
        m_pinned.append(window);
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    pinWindowTopmost(hwnd);
}

void TaskbarInfo::pinAboveTaskbar(QWindow* window) const {
    pinTopmost(window);
}

void TaskbarInfo::repinTracked() const {
    int repinned = 0;
    for(auto it = m_pinned.begin(); it != m_pinned.end();) {
        QWindow* window = *it;
        if(!window) {
            it = m_pinned.erase(it);
            continue;
        }
        if(window->isVisible()) {
            if(HWND hwnd = reinterpret_cast<HWND>(window->winId())) {
                pinWindowTopmost(hwnd);
                ++repinned;
            }
        }
        ++it;
    }
    TRC_DEBUG("repinTracked: tracked={} repinned={}", m_pinned.size(), repinned);
}

void TaskbarInfo::handleShellEvent(unsigned long event) {
    repinTracked();
    if(event == EVENT_SYSTEM_MINIMIZESTART || event == EVENT_SYSTEM_MINIMIZEEND || event == EVENT_SYSTEM_FOREGROUND) {
        m_repinBurstTicks = 15; // About 450 ms.
        m_repinBurst.start();
    }
}

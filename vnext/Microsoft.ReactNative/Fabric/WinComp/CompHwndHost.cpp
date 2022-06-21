// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "CompHwndHost.h"
#include "CompHwndHost.g.cpp"

#include <QuirkSettings.h>
#include <ReactHost/MsoUtils.h>
#include <Utils/Helpers.h>
#include <dispatchQueue/dispatchQueue.h>
#include <windowsx.h>
#include <winrt/Windows.UI.Core.h>
#include "CompositionContextHelper.h"
#include "ReactNativeHost.h"

WINUSERAPI UINT WINAPI GetDpiForWindow(_In_ HWND hwnd);

namespace winrt::Microsoft::ReactNative::implementation {

winrt::Windows::UI::Composition::Visual CompHwndHost::RootVisual() const noexcept {
  return m_target.Root();
}

winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CompHwndHost::Target() const noexcept {
  assert(m_target);
  return m_target;
}

void CompHwndHost::CreateDesktopWindowTarget(HWND window) {
  namespace abi = ABI::Windows::UI::Composition::Desktop;

  auto interop = m_compositor.as<abi::ICompositorDesktopInterop>();
  winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target{nullptr};
  check_hresult(interop->CreateDesktopWindowTarget(
      window, false, reinterpret_cast<abi::IDesktopWindowTarget **>(put_abi(target))));
  m_target = target;
}

void CompHwndHost::CreateCompositionRoot() {
  auto root = m_compositor.CreateContainerVisual();
  root.RelativeSizeAdjustment({1.0f, 1.0f});
  root.Offset({0, 0, 0});
  m_target.Root(root);
}

CompHwndHost::CompHwndHost() noexcept {}

void CompHwndHost::Initialize(uint64_t hwnd) noexcept {
  m_hwnd = (HWND)hwnd;

  m_compRootView = winrt::Microsoft::ReactNative::CompRootView();
  m_compRootView.CompositionContext(
      winrt::Microsoft::ReactNative::Composition::implementation::CompositionContextHelper::CreateContext(
          m_compositor));

  CreateDesktopWindowTarget(m_hwnd);
  CreateCompositionRoot();

  EnsureTarget();

  m_compRootView.InitialProps(std::move(m_initialPropsWriter));
  m_compRootView.ComponentName(std::move(m_componentName));
  m_compRootView.ReactNativeHost(std::move(m_reactNativeHost));

  m_compRootView.ScaleFactor(ScaleFactor());
  m_compRootView.RootVisual(
      winrt::Microsoft::ReactNative::Composition::implementation::CompositionContextHelper::CreateVisual(RootVisual()));

  UpdateSize();
}

double CompHwndHost::ScaleFactor() noexcept {
  return GetDpiForWindow(m_hwnd) / 96.0;
}

void CompHwndHost::UpdateSize() noexcept {
  RECT rc;
  if (GetClientRect(m_hwnd, &rc)) {
    winrt::Windows::Foundation::Size size{
        static_cast<float>((rc.right - rc.left) / ScaleFactor()),
        static_cast<float>((rc.bottom - rc.top) / ScaleFactor())};
    m_compRootView.Size(size);
    m_compRootView.Measure(size);
    m_compRootView.Arrange(size);
  }
}

LRESULT CompHwndHost::TranslateMessage(int msg, WPARAM wParam, LPARAM lParam) noexcept {
  if (!m_hwnd || !m_compRootView)
    return 0;

  switch (msg) {
    case WM_MOUSEWHEEL: {
      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      ::ScreenToClient(m_hwnd, &pt);
      int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
      m_compRootView.OnScrollWheel({static_cast<float>(pt.x), static_cast<float>(pt.y)}, delta);
      return 0;
    }
    /*
    case WM_POINTERDOWN: {
      m_compRootView.OnPointerPressed({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
      return 0;
    }
    case WM_LBUTTONUP: {
      m_compRootView.OnMouseUp({static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam))});
      return 0;
    }
    */
    case WM_WINDOWPOSCHANGED: {
      UpdateSize();
      /*
      winrt::Windows::Foundation::Size size{
          static_cast<float>(reinterpret_cast<const WINDOWPOS *>(lParam)->cx),
          static_cast<float>(reinterpret_cast<const WINDOWPOS *>(lParam)->cy)};
      m_compRootView.Size(size);
      m_compRootView.Measure(size);
      m_compRootView.Arrange(size);
      */
      return 0;
    }
  }

  if (m_compRootView) {
    return m_compRootView.SendMessage(msg, wParam, lParam);
  }
  return 0;
}

ReactNative::ReactNativeHost CompHwndHost::ReactNativeHost() noexcept {
  return m_compRootView ? m_compRootView.ReactNativeHost() : m_reactNativeHost;
}

void CompHwndHost::ReactNativeHost(ReactNative::ReactNativeHost const &value) noexcept {
  if (m_compRootView) {
    m_compRootView.ReactNativeHost(value);
  } else {
    m_reactNativeHost = value;
  }
  EnsureTarget();
}

winrt::Windows::UI::Composition::Compositor CompHwndHost::Compositor() const noexcept {
  return m_compositor;
}

void CompHwndHost::Compositor(winrt::Windows::UI::Composition::Compositor const &value) noexcept {
  m_compositor = value;
}

void CompHwndHost::EnsureTarget() noexcept {
  if (!ReactNativeHost() || !m_hwnd) {
    return;
  }

  winrt::Microsoft::ReactNative::ReactPropertyBag propBag(ReactNativeHost().InstanceSettings().Properties());
  propBag.Set(
      winrt::Microsoft::ReactNative::ReactPropertyId<winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget>(
          L"CompCoreDispatcher"),
      Target());
}

winrt::hstring CompHwndHost::ComponentName() noexcept {
  return m_compRootView ? m_compRootView.ComponentName() : m_componentName;
}

void CompHwndHost::ComponentName(winrt::hstring const &value) noexcept {
  if (m_compRootView)
    m_compRootView.ComponentName(value);
  else
    m_componentName = value;
}

ReactNative::JSValueArgWriter CompHwndHost::InitialProps() noexcept {
  return m_compRootView ? m_compRootView.InitialProps() : m_initialPropsWriter;
}

void CompHwndHost::InitialProps(ReactNative::JSValueArgWriter const &value) noexcept {
  if (m_compRootView)
    m_compRootView.InitialProps(value);
  else
    m_initialPropsWriter = value;
}

} // namespace winrt::Microsoft::ReactNative::implementation

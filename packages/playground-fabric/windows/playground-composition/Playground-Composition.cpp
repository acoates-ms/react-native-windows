#include "pch.h"
#include "resource.h"

#include <windows.h>
#include <windowsx.h>

// Disabled until we have a 3rd party story for custom components
// #include "AutolinkedNativeModules.g.h"

#include <DispatcherQueue.h>
#include <UIAutomation.h>
#include <windows.ui.composition.interop.h>

#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.h>

#include "NativeModules.h"
#include "ReactPropertyBag.h"

#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.h>

// TODO - Should this header be copied automatically?
#include <E:\repos\react-native-windows\packages\playground-fabric\windows\packages\Microsoft.WindowsAppSDK.1.5.230906000-experimental\include\Microsoft.UI.Dispatching.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/microsoft.ui.interop.h>

winrt::Microsoft::UI::Dispatching::DispatcherQueueController g_liftedDispatcherQueueController{nullptr};
winrt::Microsoft::UI::Composition::Compositor g_liftedCompositor{nullptr};
HWND g_hwnd;

struct CustomProps : winrt::implements<CustomProps, winrt::Microsoft::ReactNative::IComponentProps> {
  CustomProps(winrt::Microsoft::ReactNative::ViewProps props) : m_props(props) {}

  void SetProp(uint32_t hash, winrt::hstring propName, winrt::Microsoft::ReactNative::IJSValueReader value) noexcept {
    if (propName == L"label") {
      if (!value) {
        label.clear();
      } else {
        label = winrt::to_string(value.GetString());
      }
    }
  }

  std::string label;
  winrt::Microsoft::ReactNative::ViewProps m_props;
};

struct CustomComponent : winrt::implements<CustomComponent, winrt::IInspectable> {
  CustomComponent(winrt::Microsoft::ReactNative::Composition::ICompositionContext compContext)
      : m_compContext(compContext) {}

  void UpdateProps(winrt::Microsoft::ReactNative::IComponentProps props) noexcept {
    auto customProps = props.as<CustomProps>();
  }

  void UpdateLayoutMetrics(winrt::Microsoft::ReactNative::Composition::LayoutMetrics metrics) noexcept {
    m_visual.Size({metrics.Frame.Width * metrics.PointScaleFactor, metrics.Frame.Height * metrics.PointScaleFactor});

    m_systemBridgeRootVisual.Size(
        {metrics.Frame.Width * metrics.PointScaleFactor, metrics.Frame.Height * metrics.PointScaleFactor});

    auto site = m_systemVisualSiteBridge.Site();
    auto siteWindow = site.Environment();
    auto displayScale = siteWindow.DisplayScale();

    site.ParentScale(displayScale);
    site.ActualSize({metrics.Frame.Width / 2, metrics.Frame.Height / 2});
    site.ClientSize(
        {static_cast<int32_t>(metrics.Frame.Width / 2 * metrics.PointScaleFactor),
         static_cast<int32_t>(metrics.Frame.Height / 2 * metrics.PointScaleFactor)});
  }

  winrt::Microsoft::ReactNative::Composition::IVisual CreateVisual() noexcept {
    m_visual = m_compContext.CreateSpriteVisual();

    m_lightVisual = m_compContext.CreateSpriteVisual();
    m_lightVisual.Brush(m_compContext.CreateColorBrush(winrt::Windows::UI::Colors::Green()));
    m_lightVisual.RelativeSizeWithOffset({0, 0}, {1, 1});
    m_lightVisual.Opacity(0.3f);
    m_visual.InsertAt(m_lightVisual, 0);

    auto compositor =
        winrt::Microsoft::ReactNative::Composition::WindowsCompositionContextHelper::InnerCompositor(m_compContext);

    auto sbrv = compositor.CreateSpriteVisual();
    sbrv.Brush(compositor.CreateColorBrush(winrt::Windows::UI::Colors::Blue()));

    m_systemBridgeRootVisual = sbrv;

    winrt::Microsoft::ReactNative::Composition::WindowsCompositionContextHelper::InnerVisual(m_visual)
        .as<winrt::Windows::UI::Composition::ContainerVisual>()
        .Children()
        .InsertAtTop(m_systemBridgeRootVisual);

    // SystemVisual Bridge
    m_systemVisualSiteBridge = winrt::Microsoft::UI::Content::SystemVisualSiteBridge::Create(
        g_liftedCompositor, m_systemBridgeRootVisual, winrt::Microsoft::UI::GetWindowIdFromWindow(g_hwnd));
    m_systemVisualSiteBridge.Site().ShouldApplyRasterizationScale(false);

    // auto window =
    // winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(winrt::Microsoft::UI::GetWindowIdFromWindow(g_hwnd))

    return m_visual;
  }

  void FinalizeUpdates() {
    if (!m_connected) {
      auto liftedBrush = g_liftedCompositor.CreateColorBrush(winrt::Windows::UI::Colors::Red());
      auto liftedVisual = g_liftedCompositor.CreateSpriteVisual();
      liftedVisual.RelativeSizeAdjustment({1, 1});
      liftedVisual.Brush(liftedBrush);

      // Create the App's content and connect to the DesktopChildSiteBridge.
      auto appContent{winrt::Microsoft::UI::Content::ContentIsland::Create(liftedVisual)};
      m_systemVisualSiteBridge.Connect(appContent);

      m_pointerSource = winrt::Microsoft::UI::Input::InputPointerSource::GetForIsland(appContent);

      m_pointerSource.PointerPressed([this](
                                         winrt::Microsoft::UI::Input::InputPointerSource const &,
                                         winrt::Microsoft::UI::Input::PointerEventArgs const &args) {
        auto currentPoint = args.CurrentPoint();
        auto properties = currentPoint.Properties();

        if (properties.IsLeftButtonPressed()) {
          Island_OnLeftButtonPressed(args);
        } else if (properties.IsRightButtonPressed()) {
          // Input_OnRightButtonPressed(args);
        }
      });

      m_inputFocusController = winrt::Microsoft::UI::Input::InputFocusController::GetForIsland(appContent);

      m_inputFocusController.GotFocus(
          [this](
              winrt::Microsoft::UI::Input::InputFocusController const &,
              winrt::Microsoft::UI::Input::FocusChangedEventArgs const &args) { Island_FocusChanged(true); });
      m_inputFocusController.LostFocus(
          [this](
              winrt::Microsoft::UI::Input::InputFocusController const &,
              winrt::Microsoft::UI::Input::FocusChangedEventArgs const &args) { Island_FocusChanged(false); });

      m_connected = true;
    }
  }

  void Island_OnLeftButtonPressed(const winrt::Microsoft::UI::Input::PointerEventArgs &args) {
    // Since this is marked as not being handled, I'd expect
    args.Handled(false);

    m_inputFocusController.TrySetFocus();
  }

  void Island_FocusChanged(bool focus) {
    ;
  }

  // TODO - Once we get more complete native eventing we can move spotlight based on pointer position
  int64_t SendMessage(uint32_t msg, uint64_t wParam, int64_t lParam) noexcept {
    if (msg == WM_MOUSEMOVE) {
      auto x = GET_X_LPARAM(lParam);
      auto y = GET_Y_LPARAM(lParam);
    }

    if (msg == WM_LBUTTONDOWN) {
      m_systemToggle = !m_systemToggle;
      auto compositor =
          winrt::Microsoft::ReactNative::Composition::WindowsCompositionContextHelper::InnerCompositor(m_compContext);

      m_systemBridgeRootVisual.as<winrt::Windows::UI::Composition::SpriteVisual>().Brush(compositor.CreateColorBrush(
          m_systemToggle ? winrt::Windows::UI::Colors::LightBlue() : winrt::Windows::UI::Colors::Blue()));
    }

    return 0;
  }

  static void RegisterViewComponent(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) {
    packageBuilder.as<winrt::Microsoft::ReactNative::IReactPackageBuilderFabric>().AddViewComponent(
        L"MyCustomComponent", [](winrt::Microsoft::ReactNative::IReactViewComponentBuilder const &builder) noexcept {
          builder.SetCreateProps(
              [](winrt::Microsoft::ReactNative::ViewProps props) noexcept { return winrt::make<CustomProps>(props); });
          auto compBuilder =
              builder.as<winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder>();
          compBuilder.SetCreateView(
              [](winrt::Microsoft::ReactNative::Composition::ICompositionContext context) noexcept {
                return winrt::make<CustomComponent>(context);
              });
          compBuilder.SetPropsUpdater([](winrt::Windows::Foundation::IInspectable handle,
                                         winrt::Microsoft::ReactNative::IComponentProps props) noexcept {
            handle.as<CustomComponent>()->UpdateProps(props);
          });
          compBuilder.SetLayoutMetricsUpdater(
              [](winrt::Windows::Foundation::IInspectable handle,
                 winrt::Microsoft::ReactNative::Composition::LayoutMetrics metrics) noexcept {
                handle.as<CustomComponent>()->UpdateLayoutMetrics(metrics);
              });
          compBuilder.SetVisualCreator([](winrt::Windows::Foundation::IInspectable handle) noexcept {
            return handle.as<CustomComponent>()->CreateVisual();
          });
          compBuilder.SetUpdateFinalizer([](winrt::Windows::Foundation::IInspectable handle) noexcept {
            return handle.as<CustomComponent>()->FinalizeUpdates();
          });
          compBuilder.SetMessageHandler(
              [](winrt::Windows::Foundation::IInspectable handle,
                 uint32_t Msg,
                 uint64_t WParam,
                 int64_t LParam) noexcept { return handle.as<CustomComponent>()->SendMessage(Msg, WParam, LParam); });
        });
  }

 private:
  bool m_connected{false};
  winrt::Windows::UI::Composition::ContainerVisual m_systemBridgeRootVisual{nullptr};
  bool m_systemToggle{false};
  winrt::Microsoft::UI::Content::SystemVisualSiteBridge m_systemVisualSiteBridge{nullptr};
  winrt::Microsoft::UI::Input::InputPointerSource m_pointerSource{nullptr};
  winrt::Microsoft::UI::Input::InputFocusController m_inputFocusController{nullptr};

  bool m_islandToggle{false};
  winrt::Microsoft::ReactNative::Composition::ISpriteVisual m_visual{nullptr};
  winrt::Microsoft::ReactNative::Composition::ISpriteVisual m_lightVisual{nullptr};
  winrt::Microsoft::ReactNative::Composition::ICompositionContext m_compContext;
};

struct CompReactPackageProvider
    : winrt::implements<CompReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
 public: // IReactPackageProvider
  void CreatePackage(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
    CustomComponent::RegisterViewComponent(packageBuilder);
  }
};

winrt::Windows::System::DispatcherQueueController g_dispatcherQueueController{nullptr};
winrt::Windows::UI::Composition::Compositor g_compositor{nullptr};

constexpr auto WindowDataProperty = L"WindowData";

int RunPlayground(int showCmd, bool useWebDebugger);
winrt::Microsoft::ReactNative::IReactPackageProvider CreateStubDeviceInfoPackageProvider() noexcept;


struct WindowData {
  static HINSTANCE s_instance;
  static constexpr uint16_t defaultDebuggerPort = 9229;

  std::wstring m_bundleFile;
  bool m_windowInited{false};
  winrt::Microsoft::ReactNative::CompositionHwndHost m_CompositionHwndHost{nullptr};
  winrt::Microsoft::ReactNative::ReactNativeHost m_host{nullptr};
  winrt::Microsoft::ReactNative::ReactInstanceSettings m_instanceSettings{nullptr};

  bool m_useWebDebugger{false};
  bool m_fastRefreshEnabled{true};
  bool m_useDirectDebugger{false};
  bool m_breakOnNextLine{false};
  uint16_t m_debuggerPort{defaultDebuggerPort};
  xaml::ElementTheme m_theme{xaml::ElementTheme::Default};

  WindowData(const winrt::Microsoft::ReactNative::CompositionHwndHost &compHost) : m_CompositionHwndHost(compHost) {
    winrt::Microsoft::ReactNative::Composition::CompositionUIService::SetCompositionContext(
        InstanceSettings().Properties(),
        winrt::Microsoft::ReactNative::Composition::WindowsCompositionContextHelper::CreateContext(g_compositor));
  }

  static WindowData *GetFromWindow(HWND hwnd) {
    auto data = reinterpret_cast<WindowData *>(GetProp(hwnd, WindowDataProperty));
    return data;
  }

  winrt::Microsoft::ReactNative::ReactNativeHost Host() noexcept {
    if (!m_host) {
      m_host = winrt::Microsoft::ReactNative::ReactNativeHost();
      m_host.InstanceSettings(InstanceSettings());
    }

    return m_host;
  }
  winrt::Microsoft::ReactNative::ReactInstanceSettings InstanceSettings() noexcept {
    if (!m_instanceSettings) {
      m_instanceSettings = winrt::Microsoft::ReactNative::ReactInstanceSettings();
    }

    return m_instanceSettings;
  }

  LRESULT OnCommand(HWND hwnd, int id, HWND /* hwndCtl*/, UINT) {
    switch (id) {
      case IDM_OPENJSFILE: {
        DialogBox(s_instance, MAKEINTRESOURCE(IDD_OPENJSBUNDLEBOX), hwnd, &Bundle);

        if (!m_bundleFile.empty()) {
          PCWSTR appName = (m_bundleFile == LR"(Samples\rntester)") ? L"RNTesterApp" : L"Bootstrap";

          WCHAR workingDir[MAX_PATH];
          GetCurrentDirectory(MAX_PATH, workingDir);

          auto host = Host();
          // Disable until we have a 3rd party story for custom components
          // RegisterAutolinkedNativeModulePackages(host.PackageProviders()); // Includes any autolinked modules

          host.InstanceSettings().JavaScriptBundleFile(m_bundleFile);

          host.InstanceSettings().UseWebDebugger(m_useWebDebugger);
          host.InstanceSettings().UseDirectDebugger(m_useDirectDebugger);
          host.InstanceSettings().BundleRootPath(
              std::wstring(L"file:").append(workingDir).append(L"\\Bundle\\").c_str());
          host.InstanceSettings().DebuggerBreakOnNextLine(m_breakOnNextLine);
          host.InstanceSettings().UseFastRefresh(m_fastRefreshEnabled);
          host.InstanceSettings().DebuggerPort(m_debuggerPort);
          host.InstanceSettings().UseDeveloperSupport(true);

          host.PackageProviders().Append(CreateStubDeviceInfoPackageProvider());
          host.PackageProviders().Append(winrt::make<CompReactPackageProvider>());
          winrt::Microsoft::ReactNative::ReactCoreInjection::SetTopLevelWindowId(
              host.InstanceSettings().Properties(), reinterpret_cast<uint64_t>(hwnd));

          // Nudge the ReactNativeHost to create the instance and wrapping context
          host.ReloadInstance();

          winrt::Microsoft::ReactNative::ReactViewOptions viewOptions;
          viewOptions.ComponentName(appName);
          m_CompositionHwndHost.ReactViewHost(
              winrt::Microsoft::ReactNative::ReactCoreInjection::MakeViewHost(host, viewOptions));

          auto windowData = WindowData::GetFromWindow(hwnd);
          if (!windowData->m_windowInited) {
            m_CompositionHwndHost.Initialize((uint64_t)hwnd);
            windowData->m_windowInited = true;
          }
        }

        break;
      }
      case IDM_NEWWINDOW: {
        std::thread playgroundThread{([]() {
          // For subsequent RN windows do not use the web debugger by default,
          // since one instance can be connected to it at a time.
          RunPlayground(SW_SHOW, false);
        })};
        playgroundThread.detach();
        break;
      }
      case IDM_ABOUT:
        DialogBox(s_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, &About);
        break;
      case IDM_EXIT:
        PostQuitMessage(0);
        break;
      case IDM_REFRESH:
        Host().ReloadInstance();
        break;
      case IDM_SETTINGS:
        DialogBoxParam(s_instance, MAKEINTRESOURCE(IDD_SETTINGSBOX), hwnd, &Settings, reinterpret_cast<INT_PTR>(this));
        break;
    }

    return 0;
  }

  LRESULT TranslateMessage(UINT message, WPARAM wparam, LPARAM lparam) noexcept {
    if (m_CompositionHwndHost) {
      return static_cast<LRESULT>(m_CompositionHwndHost.TranslateMessage(message, wparam, lparam));
    }
    return 0;
  }

  /// Message handler for about box.
  static INT_PTR CALLBACK About(HWND hwnd, UINT message, WPARAM wparam, LPARAM /* lparam */) noexcept {
    switch (message) {
      case WM_INITDIALOG:
        return TRUE;

      case WM_COMMAND:
        if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL) {
          EndDialog(hwnd, LOWORD(wparam));
          return TRUE;
        }
        break;
    }

    return FALSE;
  }

  static constexpr std::wstring_view g_bundleFiles[] = {
      LR"(Samples\rntester)",
      LR"(Samples\accessible)",
      LR"(Samples\callbackTest)",
      LR"(Samples\calculator)",
      LR"(Samples\click)",
      LR"(Samples\customViewManager)",
      LR"(Samples\control)",
      LR"(Samples\flexbox)",
      LR"(Samples\focusTest)",
      LR"(Samples\geosample)",
      LR"(Samples\image)",
      LR"(Samples\index)",
      LR"(Samples\nativeFabricComponent)",
      LR"(Samples\mouse)",
      LR"(Samples\scrollViewSnapSample)",
      LR"(Samples\simple)",
      LR"(Samples\text)",
      LR"(Samples\textinput)",
      LR"(Samples\ticTacToe)",
      LR"(Samples\view)",
      LR"(Samples\debugTest01)"};

  static INT_PTR CALLBACK Bundle(HWND hwnd, UINT message, WPARAM wparam, LPARAM /*lparam*/) noexcept {
    switch (message) {
      case WM_INITDIALOG: {
        HWND hwndListBox = GetDlgItem(hwnd, IDC_JSBUNDLELIST);
        for (int i = 0; i < _countof(g_bundleFiles); i++) {
          SendMessage(hwndListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(g_bundleFiles[i].data()));
        }
        return TRUE;
      }
      case WM_COMMAND:
        switch (LOWORD(wparam)) {
          case IDOK: {
            HWND hwndListBox = GetDlgItem(hwnd, IDC_JSBUNDLELIST);
            int selectedItem = static_cast<int>(SendMessage(hwndListBox, LB_GETCURSEL, 0, 0));
            if (0 <= selectedItem && selectedItem < _countof(g_bundleFiles)) {
              auto self = GetFromWindow(GetParent(hwnd));
              self->m_bundleFile = g_bundleFiles[selectedItem];
            }
          }
            [[fallthrough]];
          case IDCANCEL:
            EndDialog(hwnd, LOWORD(wparam));
            return TRUE;
        }
        break;
    }

    return FALSE;
  }

  static INT_PTR CALLBACK Settings(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
    switch (message) {
      case WM_INITDIALOG: {
        auto boolToCheck = [](bool b) { return b ? BST_CHECKED : BST_UNCHECKED; };
        auto self = reinterpret_cast<WindowData *>(lparam);
        CheckDlgButton(hwnd, IDC_WEBDEBUGGER, boolToCheck(self->m_useWebDebugger));
        CheckDlgButton(hwnd, IDC_FASTREFRESH, boolToCheck(self->m_fastRefreshEnabled));
        CheckDlgButton(hwnd, IDC_DIRECTDEBUGGER, boolToCheck(self->m_useDirectDebugger));
        CheckDlgButton(hwnd, IDC_BREAKONNEXTLINE, boolToCheck(self->m_breakOnNextLine));

        auto portEditControl = GetDlgItem(hwnd, IDC_DEBUGGERPORT);
        SetWindowTextW(portEditControl, std::to_wstring(self->m_debuggerPort).c_str());
        SendMessageW(portEditControl, (UINT)EM_SETLIMITTEXT, (WPARAM)5, (LPARAM)0);

        auto cmbEngines = GetDlgItem(hwnd, IDC_JSENGINE);
        SendMessageW(cmbEngines, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("Chakra"));
        SendMessageW(cmbEngines, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("Hermes"));
        SendMessageW(cmbEngines, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("V8"));
        // SendMessageW(cmbEngines, CB_SETCURSEL, (WPARAM) static_cast<int32_t>(self->m_jsEngine), (LPARAM)0);

        auto cmbTheme = GetDlgItem(hwnd, IDC_THEME);
        SendMessageW(cmbTheme, CB_ADDSTRING, 0, (LPARAM)L"Default");
        SendMessageW(cmbTheme, CB_ADDSTRING, 0, (LPARAM)L"Light");
        SendMessageW(cmbTheme, CB_ADDSTRING, 0, (LPARAM)L"Dark");
        ComboBox_SetCurSel(cmbTheme, static_cast<int>(self->m_theme));

        return TRUE;
      }
      case WM_COMMAND: {
        switch (LOWORD(wparam)) {
          case IDOK: {
            auto self = GetFromWindow(GetParent(hwnd));
            self->m_useWebDebugger = IsDlgButtonChecked(hwnd, IDC_WEBDEBUGGER) == BST_CHECKED;
            self->m_fastRefreshEnabled = IsDlgButtonChecked(hwnd, IDC_FASTREFRESH) == BST_CHECKED;
            self->m_useDirectDebugger = IsDlgButtonChecked(hwnd, IDC_DIRECTDEBUGGER) == BST_CHECKED;
            self->m_breakOnNextLine = IsDlgButtonChecked(hwnd, IDC_BREAKONNEXTLINE) == BST_CHECKED;

            auto themeComboBox = GetDlgItem(hwnd, IDC_THEME);

            WCHAR buffer[6] = {};
            auto portEditControl = GetDlgItem(hwnd, IDC_DEBUGGERPORT);
            GetWindowTextW(portEditControl, buffer, ARRAYSIZE(buffer));

            try {
              auto port = std::stoi(buffer);
              if (port > UINT16_MAX)
                port = defaultDebuggerPort;
              self->m_debuggerPort = static_cast<uint16_t>(port);
            } catch (const std::out_of_range &) {
              self->m_debuggerPort = defaultDebuggerPort;
            } catch (const std::invalid_argument &) {
              // Don't update the debugger port if the new value can't be parsed
              // (E.g. includes letters or symbols).
            }

            // auto cmbEngines = GetDlgItem(hwnd, IDC_JSENGINE);
            // int itemIndex = (int)SendMessageW(cmbEngines, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            // self->m_jsEngine = static_cast<Microsoft::ReactNative::JSIEngine>(itemIndex);
          }
            [[fallthrough]];
          case IDCANCEL:
            EndDialog(hwnd, LOWORD(wparam));
            return true;
        }
        break;
      }
    }

    return FALSE;
  }
};

extern "C" IMAGE_DOS_HEADER __ImageBase;
HINSTANCE WindowData::s_instance = reinterpret_cast<HINSTANCE>(&__ImageBase);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
  auto windowData = WindowData::GetFromWindow(hwnd);
  if (windowData) {
    auto result = WindowData::GetFromWindow(hwnd)->TranslateMessage(message, wparam, lparam);
    if (result)
      return result;
  }

  switch (message) {
    case WM_COMMAND: {
      return WindowData::GetFromWindow(hwnd)->OnCommand(
          hwnd, LOWORD(wparam), reinterpret_cast<HWND>(lparam), HIWORD(wparam));
    }
    case WM_DESTROY: {
      delete WindowData::GetFromWindow(hwnd);
      SetProp(hwnd, WindowDataProperty, 0);
      PostQuitMessage(0);
      return 0;
    }
    case WM_NCCREATE: {
      auto cs = reinterpret_cast<CREATESTRUCT *>(lparam);
      auto windowData = static_cast<WindowData *>(cs->lpCreateParams);
      WINRT_ASSERT(windowData);
      SetProp(hwnd, WindowDataProperty, reinterpret_cast<HANDLE>(windowData));
      break;
    }
    case WM_GETOBJECT: {
      if (lparam == UiaRootObjectId) {
        auto windowData = WindowData::GetFromWindow(hwnd);
        if (windowData == nullptr || !windowData->m_windowInited)
          break;

        auto hwndHost = windowData->m_CompositionHwndHost;
        winrt::com_ptr<IRawElementProviderSimple> spReps;
        if (!hwndHost.UiaProvider().try_as(spReps)) {
          break;
        }
        LRESULT lResult = UiaReturnRawElementProvider(hwnd, wparam, lparam, spReps.get());
        return lResult;
      }
    }
  }

  return DefWindowProc(hwnd, message, wparam, lparam);
}

constexpr PCWSTR c_windowClassName = L"MS_REACTNATIVE_PLAYGROUND_COMPOSITION";

int RunPlayground(int showCmd, bool useWebDebugger) {
  constexpr PCWSTR appName = L"React Native Playground (Composition)";

  auto windowData = std::make_unique<WindowData>(winrt::Microsoft::ReactNative::CompositionHwndHost());
  HWND hwnd = CreateWindow(
      c_windowClassName,
      appName,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      nullptr,
      nullptr,
      WindowData::s_instance,
      windowData.get());

  WINRT_VERIFY(hwnd);

  g_hwnd = hwnd; // Temporary for prototyping

  windowData.release();

  ShowWindow(hwnd, showCmd);
  UpdateWindow(hwnd);
  SetFocus(hwnd);

  HACCEL hAccelTable = LoadAccelerators(WindowData::s_instance, MAKEINTRESOURCE(IDC_PLAYGROUND_COMPOSITION));

  MSG msg = {};

  // This would be the same as the loop below.
  // g_liftedDispatcherQueueController.DispatcherQueue().RunEventLoop();

  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!ContentPreTranslateMessage(&msg)) {
      if (!TranslateAccelerator(hwnd, hAccelTable, &msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  return static_cast<int>(msg.wParam);
}

_Use_decl_annotations_ int CALLBACK WinMain(HINSTANCE instance, HINSTANCE, PSTR /* commandLine */, int showCmd) {
  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = &WndProc;
  wcex.cbClsExtra = DLGWINDOWEXTRA;
  wcex.cbWndExtra = sizeof(WindowData *);
  wcex.hInstance = WindowData::s_instance;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PLAYGROUND_COMPOSITION);
  wcex.lpszClassName = c_windowClassName;
  wcex.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_ICON1));
  ATOM classId = RegisterClassEx(&wcex);
  WINRT_VERIFY(classId);
  winrt::check_win32(!classId);

  DispatcherQueueOptions options{
      sizeof(DispatcherQueueOptions), /* dwSize */
      DQTYPE_THREAD_CURRENT, /* threadType */
      DQTAT_COM_ASTA /* apartmentType */
  };

  // Need to have a Dispatcher on the current thread to be able to create a Compositor
  winrt::check_hresult(CreateDispatcherQueueController(
      options,
      reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(
          winrt::put_abi(g_dispatcherQueueController))));

  // Create a Lifted (WinAppSDK) DispatcherQueue for this thread.  This is needed for
  // Microsoft.UI.Composition, Content, and Input APIs.
  g_liftedDispatcherQueueController =
      winrt::Microsoft::UI::Dispatching::DispatcherQueueController::CreateOnCurrentThread();
  g_liftedCompositor = winrt::Microsoft::UI::Composition::Compositor();

  g_compositor = winrt::Windows::UI::Composition::Compositor();
  return RunPlayground(showCmd, false);
}

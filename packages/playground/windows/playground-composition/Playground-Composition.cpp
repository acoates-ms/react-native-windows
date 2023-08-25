#include "pch.h"
#include "resource.h"

#include <windows.h>
#include <windowsx.h>

#include <memory>

// Disabled until we have a 3rd party story for custom components
// #include "AutolinkedNativeModules.g.h"

#include <winrt/Windows.Foundation.Collections.h>

#include "../../../../vnext/codegen/NativeDeviceInfoSpec.g.h"

#include <DispatcherQueue.h>
#include <UIAutomation.h>
#include <windows.ui.composition.interop.h>

#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.h>

#include "NativeModules.h"
#include "ReactPropertyBag.h"
#include "d2d1helper.h"
#include <d2d1_1.h>
#include <d3d11.h>
#include <d3d11_4.h>

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

winrt::Windows::UI::Composition::SpotLight AttachSpotlightToVisual(winrt::Windows::UI::Composition::SpriteVisual visual)
{
  auto compositor = visual.Compositor();
  visual.Brush(compositor.CreateColorBrush(winrt::Windows::UI::Colors::White()));

  auto spotlight = compositor.CreateSpotLight();
  spotlight.InnerConeAngleInDegrees(50.0f);
  spotlight.InnerConeColor(winrt::Windows::UI::Colors::FloralWhite());
  spotlight.InnerConeIntensity(5.0f);
  spotlight.OuterConeAngleInDegrees(0.0f);
  spotlight.ConstantAttenuation(1.0f);
  spotlight.LinearAttenuation(0.253f);
  spotlight.QuadraticAttenuation(0.58f);
  spotlight.CoordinateSpace(visual);
  spotlight.Targets().Add(visual);

  auto animation = compositor.CreateVector3KeyFrameAnimation();
  auto easeIn = compositor.CreateCubicBezierEasingFunction({0.5f, 0.0f}, {1.0f, 1.0f});
  animation.InsertKeyFrame(0.00f, {100.0f, 100.0f, 35.0f});
  animation.InsertKeyFrame(0.25f, {300.0f, 200.0f, 75.0f}, easeIn);
  animation.InsertKeyFrame(0.50f, {050.0f, 300.0f, 15.0f}, easeIn);
  animation.InsertKeyFrame(0.75f, {300.0f, 050.0f, 75.0f}, easeIn);
  animation.InsertKeyFrame(1.00f, {100.0f, 100.0f, 35.0f}, easeIn);
  animation.Duration(std::chrono::milliseconds(4000));
  animation.IterationBehavior(winrt::Windows::UI::Composition::AnimationIterationBehavior::Forever);

  spotlight.StartAnimation(L"Offset", animation);

  return spotlight;
}

struct CustomComponent : winrt::implements<CustomComponent, winrt::IInspectable> {
  CustomComponent(winrt::Microsoft::ReactNative::Composition::ICompositionContext compContext)
      : m_compContext(compContext) {}

  void UpdateProps(winrt::Microsoft::ReactNative::IComponentProps props) noexcept {
    auto customProps = props.as<CustomProps>();
  }

  void UpdateLayoutMetrics(winrt::Microsoft::ReactNative::Composition::LayoutMetrics metrics) noexcept {
    m_visual.Size({metrics.Frame.Width, metrics.Frame.Height});
  }

  winrt::Microsoft::ReactNative::Composition::IVisual CreateVisual() noexcept {
    m_visual = m_compContext.CreateSpriteVisual();

    m_spotlight = AttachSpotlightToVisual(winrt::Microsoft::ReactNative::Composition::CompositionContextHelper::InnerVisual(m_visual)
                                .as<winrt::Windows::UI::Composition::SpriteVisual>());
    return m_visual;
  }

  // TODO - Once we get more complete native eventing we can move spotlight based on pointer position
  int64_t SendMessage(uint32_t msg, uint64_t wParam, int64_t lParam) noexcept {
    if (msg == WM_MOUSEMOVE) {
      auto x = GET_X_LPARAM(lParam);
      auto y = GET_Y_LPARAM(lParam);

      m_spotlight.Offset({(float)x, (float)y, 15.0f});

      // m_propSet.InsertVector2(L"Position", {x, y});
      //  TODO expose coordinate translation methods
      //  TODO convert x/y into local coordinates
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
          compBuilder.SetMessageHandler(
              [](winrt::Windows::Foundation::IInspectable handle,
                 uint32_t Msg,
                 uint64_t WParam,
                 int64_t LParam) noexcept { return handle.as<CustomComponent>()->SendMessage(Msg, WParam, LParam); });
        });
  }

 private:
  winrt::Windows::UI::Composition::SpotLight m_spotlight{nullptr};

  winrt::Microsoft::ReactNative::Composition::SpriteVisual m_visual{nullptr};
  winrt::Microsoft::ReactNative::Composition::ICompositionContext m_compContext;
};

// Work around crash in DeviceInfo when running outside of XAML environment
// TODO rework built-in DeviceInfo to allow it to be driven without use of HWNDs or XamlApps
REACT_MODULE(DeviceInfo)
struct DeviceInfo {
  using ModuleSpec = Microsoft::ReactNativeSpecs::DeviceInfoSpec;

  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const &reactContext) noexcept {
    m_context = reactContext;
  }

  REACT_GET_CONSTANTS(GetConstants)
  Microsoft::ReactNativeSpecs::DeviceInfoSpec_Constants GetConstants() noexcept {
    Microsoft::ReactNativeSpecs::DeviceInfoSpec_Constants constants;
    Microsoft::ReactNativeSpecs::DeviceInfoSpec_DisplayMetrics screenDisplayMetrics;
    screenDisplayMetrics.fontScale = 1;
    screenDisplayMetrics.height = 1024;
    screenDisplayMetrics.width = 1024;
    screenDisplayMetrics.scale = 1;
    constants.Dimensions.screen = screenDisplayMetrics;
    constants.Dimensions.window = screenDisplayMetrics;
    return constants;
  }

 private:
  winrt::Microsoft::ReactNative::ReactContext m_context;
};

// Have to use TurboModules to override built in modules.. so the standard attributed package provider doesn't work.
struct CompReactPackageProvider
    : winrt::implements<CompReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
 public: // IReactPackageProvider
  void CreatePackage(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
    AddAttributedModules(packageBuilder, true);

    CustomComponent::RegisterViewComponent(packageBuilder);
  }
};

constexpr auto WindowDataProperty = L"WindowData";

int RunPlayground(int showCmd, bool useWebDebugger);

struct WindowData {
  static HINSTANCE s_instance;
  static constexpr uint16_t defaultDebuggerPort = 9229;

  std::wstring m_bundleFile;
  bool m_windowInited{false};
  winrt::Microsoft::ReactNative::Composition::ICompositionContext m_compContext;
  winrt::Microsoft::ReactNative::CompositionHwndHost m_CompositionHwndHost{nullptr};
  winrt::Microsoft::ReactNative::ReactNativeHost m_host{nullptr};
  winrt::Microsoft::ReactNative::ReactInstanceSettings m_instanceSettings{nullptr};
  winrt::Windows::System::DispatcherQueueController m_compositorDispatcherQueueController{nullptr};
  winrt::Windows::System::DispatcherQueueController m_uiDispatcherQueueController{nullptr};

  winrt::com_ptr<ID2D1Factory1> m_d2dFactory;
  winrt::com_ptr<ID3D11Device> m_d3dDevice;
  winrt::com_ptr<ID2D1Device> m_d2dDevice;
  bool m_backgroundCompositor{false};
  bool m_useWebDebugger{false};
  bool m_fastRefreshEnabled{true};
  bool m_useDirectDebugger{false};
  bool m_breakOnNextLine{false};
  uint16_t m_debuggerPort{defaultDebuggerPort};
  xaml::ElementTheme m_theme{xaml::ElementTheme::Default};

  WindowData(const winrt::Microsoft::ReactNative::CompositionHwndHost &compHost, bool background)
      : m_CompositionHwndHost(compHost), m_backgroundCompositor(background) {
    CreateCompositor(background);

    auto uiDispatcher = winrt::Microsoft::ReactNative::ReactDispatcherHelper::UIThreadDispatcher();
    assert(uiDispatcher);
    InstanceSettings().UIDispatcher(uiDispatcher);
  }

// The compositor could be running on a background or foreground thread.
  // Office runs it on a background thread, so its useful to be able to test that case
  void CreateCompositor(bool background) {
    // Create the UI thread
    DispatcherQueueOptions options{
        sizeof(DispatcherQueueOptions), /* dwSize */
        DQTYPE_THREAD_CURRENT, /* threadType */
        DQTAT_COM_STA /* apartmentType */
    };

    // Need to have a Dispatcher on the current thread to be able to create a Compositor
    winrt::check_hresult(CreateDispatcherQueueController(
        options,
        reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(
            winrt::put_abi(m_uiDispatcherQueueController))));

    if (background) {
      DispatcherQueueOptions optionsBackground{
          sizeof(DispatcherQueueOptions), /* dwSize */
          DQTYPE_THREAD_DEDICATED, /* threadType */
          DQTAT_COM_STA /* apartmentType */
      };

      // Need to have a Dispatcher on the current thread to be able to create a Compositor
      winrt::check_hresult(CreateDispatcherQueueController(
          optionsBackground,
          reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(
              winrt::put_abi(m_compositorDispatcherQueueController))));

      m_compositorDispatcherQueueController.DispatcherQueue().TryEnqueue([windowData = this]() {
        windowData->m_compContext = winrt::Microsoft::ReactNative::Composition::CompositionContextHelper::CreateContext(
            winrt::Windows::UI::Composition::Compositor());
        winrt::Microsoft::ReactNative::Composition::CompositionUIService::SetCompositionContext(
            windowData->InstanceSettings().Properties(), windowData->m_compContext);
      });
    } else {
      // Compositor uses UI queue
      m_compositorDispatcherQueueController = m_uiDispatcherQueueController;
      m_compContext = winrt::Microsoft::ReactNative::Composition::CompositionContextHelper::CreateContext(
          winrt::Windows::UI::Composition::Compositor());
      winrt::Microsoft::ReactNative::Composition::CompositionUIService::SetCompositionContext(
          InstanceSettings().Properties(), m_compContext);
    }
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

      if (message == WM_MOUSEMOVE) {
        QueueWorkOnCompositor();
      }
      return static_cast<LRESULT>(m_CompositionHwndHost.TranslateMessage(message, wparam, lparam));
    }
    return 0;
  }

  // Used to stress the compositor by causing additional commits to happen on the background compositor
  void QueueWorkOnCompositor()
  {
    if (m_CompositionHwndHost.CompositionRootView()) {
      auto rootVisual = m_CompositionHwndHost.CompositionRootView().RootVisual();

      m_compositorDispatcherQueueController.DispatcherQueue().TryEnqueue([&, compContext = m_compContext, rootVisual]() {

        auto compositor =
            winrt::Microsoft::ReactNative::Composition::CompositionContextHelper::InnerCompositor(compContext);

        auto visual = compositor.CreateSpriteVisual();

        winrt::Windows::UI::Composition::CompositionGraphicsDevice cgd{nullptr};

        winrt::com_ptr<ABI::Windows::UI::Composition::ICompositorInterop> compositorInterop{compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>()};

        winrt::Windows::Foundation::Size size{
            static_cast<float>(std::rand() % 100 + 10), static_cast<float>(std::rand() % 100 + 10)};

        // Create a graphics device backed by our D3D device
        winrt::com_ptr<ABI::Windows::UI::Composition::ICompositionGraphicsDevice> compositionGraphicsDeviceIface;
        winrt::check_hresult(
            compositorInterop->CreateGraphicsDevice(D2DDevice().get(), compositionGraphicsDeviceIface.put()));

        compositionGraphicsDeviceIface.as(cgd);

       auto drawingSurface = cgd.CreateDrawingSurface(size, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        winrt::Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied);

       // auto spotlight = AttachSpotlightToVisual(visual);
        // Leak!
        //reinterpret_cast<ABI::Windows::UI::Composition::ISpotLight*>(winrt::get_abi(spotlight))->AddRef();


        winrt::com_ptr<ID2D1DeviceContext> d2dDeviceContext;
       POINT offset;

       winrt::com_ptr<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop> drawingSurfaceInterop;
       drawingSurface.as(drawingSurfaceInterop);

       winrt::check_hresult(drawingSurfaceInterop->BeginDraw(
           nullptr, __uuidof(ID2D1DeviceContext), (void **)d2dDeviceContext.put(), &offset));
         d2dDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Black, 1.0f));
         assert(d2dDeviceContext->GetUnitMode() == D2D1_UNIT_MODE_DIPS);
         d2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
         // TODO get real DPI
         //const auto dpi = 1.5f * 96.0f;
         //float oldDpiX, oldDpiY;
         //d2dDeviceContext->GetDpi(&oldDpiX, &oldDpiY);
         //d2dDeviceContext->SetDpi(dpi, dpi);

         D2D1_RECT_F rc{
             static_cast<float>(offset.x),
             static_cast<float>(offset.y),
             static_cast<float>(offset.x) + static_cast<float>(size.Width),
             static_cast<float>(offset.y) + static_cast<float>(size.Height)};


           winrt::com_ptr<ID2D1SolidColorBrush> spBrush;
         d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), spBrush.put());

           d2dDeviceContext->DrawRectangle(&rc, spBrush.get(), 13.0f); //, strokeStyle);

         //winrt::check_hresult(hrDraw);

         // restore dpi state
         //d2dDeviceContext->SetDpi(oldDpiX, oldDpiY);
         d2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_DIPS);

         // Our update is done. EndDraw never indicates rendering device removed, so any
         // failure here is unexpected and, therefore, fatal.
         auto hrEndDraw = drawingSurfaceInterop->EndDraw();
         winrt::check_hresult(hrEndDraw);

      auto surfaceBrush = compositor.CreateSurfaceBrush(drawingSurface);
       surfaceBrush.HorizontalAlignmentRatio(0.f);
       surfaceBrush.VerticalAlignmentRatio(0.f);
       surfaceBrush.Stretch(winrt::Windows::UI::Composition::CompositionStretch::Fill);
       visual.Brush(surfaceBrush);
        
        //visual.Brush(compositor.CreateColorBrush(winrt::Windows::UI::Colors::Red()));
        visual.Size(size);
        visual.Offset({static_cast<float>(std::rand() % 300 + 10),static_cast<float>(std::rand() % 300 + 10), 0.0f});
        winrt::Microsoft::ReactNative::Composition::CompositionContextHelper::InnerVisual(rootVisual)
            .as<winrt::Windows::UI::Composition::ContainerVisual>()
            .Children()
            .InsertAtBottom(visual);
      });
    }
  }

  
  winrt::com_ptr<ID3D11Device> D3DDevice() noexcept {
    // This flag adds support for surfaces with a different color channel ordering than the API default.
    // You need it for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // #if defined(_DEBUG)
    //   creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    // #endif

    // This array defines the set of DirectX hardware feature levels this app  supports.
    // The ordering is important and you should  preserve it.
    // Don't forget to declare your app's minimum required feature level in its
    // description.  All apps are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1};
    if (!m_d3dDevice) {
      D3D11CreateDevice(
          nullptr, // specify null to use the default adapter
          D3D_DRIVER_TYPE_HARDWARE,
          0,
          creationFlags, // optionally set debug and Direct2D compatibility flags
          featureLevels, // list of feature levels this app can support
          ARRAYSIZE(featureLevels), // number of possible feature levels
          D3D11_SDK_VERSION,
          m_d3dDevice.put(), // returns the Direct3D device created
          nullptr /*&m_featureLevel*/, // returns feature level of device created
          nullptr /*&context*/ // returns the device immediate context
      );
    }
    return m_d3dDevice;
  }


  winrt::com_ptr<ID2D1Device> D2DDevice() noexcept {
    if (!m_d2dDevice) {
      winrt::com_ptr<IDXGIDevice> dxgiDevice;
      // Obtain the underlying DXGI device of the Direct3D11 device.
      D3DDevice().as(dxgiDevice);

      // Obtain the Direct2D device for 2-D rendering.
      winrt::check_hresult(D2DFactory()->CreateDevice(dxgiDevice.get(), m_d2dDevice.put()));
    }
    return m_d2dDevice;
  }

  winrt::com_ptr<ID2D1Factory1> D2DFactory() noexcept {
    if (!m_d2dFactory) {
      // Initialize Direct2D resources.
      // #if defined(_DEBUG)
      //     D2D1_FACTORY_OPTIONS d2d1FactoryOptions{D2D1_DEBUG_LEVEL_INFORMATION};
      // #else
      D2D1_FACTORY_OPTIONS d2d1FactoryOptions{D2D1_DEBUG_LEVEL_NONE};
      // #endif

      D2D1CreateFactory(
          D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &d2d1FactoryOptions, m_d2dFactory.put_void());
    }
    return m_d2dFactory;
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
      if (windowData->m_backgroundCompositor)
         windowData->m_compositorDispatcherQueueController.ShutdownQueueAsync();
      windowData->m_uiDispatcherQueueController.ShutdownQueueAsync();
      delete windowData;
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

  auto windowData = std::make_unique<WindowData>(winrt::Microsoft::ReactNative::CompositionHwndHost(), true /*background compositor*/);

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

  windowData.release();

  ShowWindow(hwnd, showCmd);
  UpdateWindow(hwnd);
  SetFocus(hwnd);

  HACCEL hAccelTable = LoadAccelerators(WindowData::s_instance, MAKEINTRESOURCE(IDC_PLAYGROUND_COMPOSITION));

  MSG msg = {};
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
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

  return RunPlayground(showCmd, false);
}

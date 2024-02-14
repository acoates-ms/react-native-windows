#include "pch.h"

#include "RNSVG.SvgView.h"
#if __has_include("SvgView.g.cpp")
#include "SvgView.g.cpp"
#endif

#include <windows.ui.xaml.media.dxinterop.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Graphics.Display.h>

#include "RNSVG.D2DDevice.h"
#include "RNSVG.D2DDeviceContext.h"
#include "RNSVG.GroupView.h"

#include <CompositionSwitcher.interop.h>
#include <AutoDraw.h>

#include <d3d11_4.h>

using namespace winrt;
using namespace winrt::Microsoft::ReactNative;

namespace winrt::RNSVG::implementation {

    SvgViewProps::SvgViewProps(const winrt::Microsoft::ReactNative::ViewProps& props) : m_props(props)
    {
    }

    void SvgViewProps::SetProp(uint32_t hash, winrt::hstring propName, winrt::Microsoft::ReactNative::IJSValueReader value) noexcept
    {
        winrt::Microsoft::ReactNative::ReadProp(hash, propName, value, *this);
    }



SvgView::SvgView(const winrt::Microsoft::ReactNative::Composition::CreateCompositionComponentViewArgs& args) : base_type(args), m_reactContext(args.ReactContext()), m_compContext(args.CompositionContext()) {
  uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1};

  // Create the Direct3D device.
  com_ptr<ID3D11Device> d3dDevice;
  D3D_FEATURE_LEVEL supportedFeatureLevel;
  check_hresult(D3D11CreateDevice(
      nullptr, // default adapter
      D3D_DRIVER_TYPE_HARDWARE,
      0,
      creationFlags,
      featureLevels,
      ARRAYSIZE(featureLevels),
      D3D11_SDK_VERSION,
      d3dDevice.put(),
      &supportedFeatureLevel,
      nullptr));

  com_ptr<IDXGIDevice> dxgiDevice{d3dDevice.as<IDXGIDevice>()};

  // Create the Direct2D device and a corresponding context.
  com_ptr<ID2D1Device> device;
  check_hresult(D2D1CreateDevice(dxgiDevice.get(), nullptr, device.put()));
  m_device = make<RNSVG::implementation::D2DDevice>(device);

  com_ptr<ID2D1DeviceContext> deviceContext;
  check_hresult(device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, deviceContext.put()));
  m_deviceContext = make<RNSVG::implementation::D2DDeviceContext>(deviceContext);
}

winrt::Microsoft::ReactNative::Composition::IVisual SvgView::CreateVisual() noexcept
{
    m_visual = m_compContext.CreateSpriteVisual();
    return m_visual;
}

void SvgView::MountChildComponentView(
    const winrt::Microsoft::ReactNative::ComponentView& childComponentView,
    uint32_t index) noexcept
{
    auto const& group { childComponentView.try_as<RNSVG::GroupView>() };

    if (group)
    {
        // Every SvgView has exactly one child - a Group that gets
        // all of Svg's children piped through.
        Group(group);
    }

    base_type::MountChildComponentView(childComponentView, index);
}

void SvgView::UnmountChildComponentView(
    const winrt::Microsoft::ReactNative::ComponentView& childComponentView,
    uint32_t index) noexcept
{
    if (Group())
    {
        Group().Unload();
    }
    Group(nullptr);

    base_type::UnmountChildComponentView(childComponentView, index);
}


void SvgView::UpdateProps(const winrt::Microsoft::ReactNative::IComponentProps& props, const winrt::Microsoft::ReactNative::IComponentProps& oldProps) noexcept
{
    UpdateProperties(props, oldProps);
}

void SvgView::UpdateProperties(const winrt::Microsoft::ReactNative::IComponentProps& props, const winrt::Microsoft::ReactNative::IComponentProps& oldProps, bool forceUpdate, bool invalidate)
{
    auto svgProps = props.as<SvgViewProps>();
    auto oldSvgProps = oldProps ? oldProps.as<SvgViewProps>() : nullptr;
    /*
 else if (propertyName == "width")
 {
     m_width = SVGLength::From(propertyValue);
      }
 else if (propertyName == "height")
 {
     m_height = SVGLength::From(propertyValue);
      }
      */

    if (!oldSvgProps || svgProps->bbWidth != oldSvgProps->bbWidth)
    {
        m_bbWidth = svgProps->bbWidth;
        assert(false);
        //Width(m_bbWidth.Value());
    }
    if (!oldSvgProps || svgProps->bbHeight != oldSvgProps->bbHeight)
    {
        m_bbHeight = svgProps->bbHeight;
        assert(false);
        //Height(m_bbHeight.Value());
    }
    if (!oldSvgProps || svgProps->vbWidth != oldSvgProps->vbWidth)
    {
        m_vbWidth = svgProps->vbWidth;
    }
    if (!oldSvgProps || svgProps->vbHeight != oldSvgProps->vbHeight)
    {
        m_vbHeight = svgProps->vbHeight;
    }
    if (!oldSvgProps || svgProps->minX != oldSvgProps->minX)
    {
        m_minX = svgProps->minX;
    }
    if (!oldSvgProps || svgProps->minY != oldSvgProps->minY)
    {
        m_minY = svgProps->minY;
    }
    if (!oldSvgProps || svgProps->align != oldSvgProps->align)
    {
        m_align = svgProps->align;
    }
    if (!oldSvgProps || svgProps->meetOrSlice != oldSvgProps->meetOrSlice)
    {
        m_meetOrSlice = svgProps->meetOrSlice;
    }
    if (!oldSvgProps || svgProps->color != oldSvgProps->color)
    {
        m_currentColor = svgProps->color;
    }
    /*
 else if (propertyName == "responsible")
 {
     m_isResponsible = propertyValue.AsBoolean();
     }
     */

    Invalidate();

}

void SvgView::SaveDefinition() {
  if (m_id != L"" && m_group) {
    m_group.SvgRoot().Templates().Insert(m_id, *this);
    m_group.SaveDefinition();
  }
}

void SvgView::MergeProperties(RNSVG::RenderableView const &other) {
  if (m_group) {
    m_group.MergeProperties(other);
  }
}

/*
Size SvgView::MeasureOverride(Size const &availableSize) {
  for (auto const &child : Children()) {
    child.Measure(availableSize);
  }
  return availableSize;
}

Size SvgView::ArrangeOverride(Size const &finalSize) {
  for (auto const &child : Children()) {
    child.Arrange({0, 0, finalSize.Width, finalSize.Height});
  }
  return finalSize;
}
*/

void SvgView::Draw(RNSVG::D2DDeviceContext const &context, Size const &size) {
  com_ptr<ID2D1DeviceContext> deviceContext{get_self<D2DDeviceContext>(context)->Get()};

  D2D1_MATRIX_3X2_F transform{D2DHelpers::GetTransform(deviceContext.get())};

  if (m_align != "") {
    Rect vbRect{m_minX, m_minY, m_vbWidth, m_vbHeight};
    float width{size.Width};
    float height{size.Height};

    if (Parent()) {
      width = Utils::GetAbsoluteLength(m_bbWidth, width);
      height = Utils::GetAbsoluteLength(m_bbHeight, height);
    }

    Rect elRect{0, 0, width, height};
    deviceContext->SetTransform(Utils::GetViewBoxTransformD2D(vbRect, elRect, m_align, m_meetOrSlice) * transform);
  }

  if (m_group) {
    m_group.SaveDefinition();
    m_group.Draw(context, size);
  }

  if (!m_hasRendered) {
    m_hasRendered = true;
  }

  deviceContext->SetTransform(transform);
}

void SvgView::UpdateLayoutMetrics(const LayoutMetrics& metrics, const LayoutMetrics& oldMetrics)
{
    m_layoutMetrics = metrics;
    base_type::UpdateLayoutMetrics(metrics, oldMetrics);
}

winrt::Windows::Foundation::Size SvgView::ActualSize() noexcept {
    return winrt::Windows::Foundation::Size { m_layoutMetrics.Frame.Width, m_layoutMetrics.Frame.Height };
}

void SvgView::CreateGeometry() {
  if (m_group) {
    m_group.CreateGeometry();
  }
}

void SvgView::CreateResources() {
  if (m_group) {
    m_group.CreateResources();
  }

  Invalidate();

  assert(false);
  /*
  m_image.Width(ActualWidth());
  m_image.Height(ActualHeight());
  m_image.Stretch(xaml::Media::Stretch::UniformToFill);

  Children().Append(m_image);
  */

}

/*
void SvgView::Panel_Loaded(IInspectable const &sender, xaml::RoutedEventArgs const & ) {
  if (auto const &svgView{sender.try_as<RNSVG::SvgView>()}) {
    if (!m_loaded) {
      m_loaded = true;
      svgView.CreateResources();
    }
  }
}

void SvgView::Panel_Unloaded(IInspectable const &sender, xaml::RoutedEventArgs const & ) {
  if (auto const &svgView{sender.try_as<RNSVG::SvgView>()}) {
    svgView.Unload();
  }
}
*/

void SvgView::Unload() {
  m_reactContext = nullptr;
  m_templates.Clear();
  m_brushes.Clear();

  if (m_group) {
    m_group.Unload();
  }
}

void SvgView::Invalidate() {
  m_brushes.Clear();
  m_templates.Clear();

  Size size = ActualSize();

  auto drawingSurface = m_compContext.CreateDrawingSurfaceBrush(
      size,
      winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
      winrt::Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied);

  POINT offset;
  {
      ::Microsoft::ReactNative::Composition::AutoDrawDrawingSurface autoDraw(drawingSurface, &offset);
      if (auto deviceContext = autoDraw.GetRenderTarget())
      {
          auto transform = Numerics::make_float3x2_translation({ static_cast<float>(offset.x), static_cast<float>(offset.y) });
          deviceContext->SetTransform(D2DHelpers::AsD2DTransform(transform));

          deviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));

          Draw(DeviceContext(), size);
      }
  }

  assert(false);
  //m_image.Source(surfaceImageSource);
}

void SvgView::RegisterComponent(const winrt::Microsoft::ReactNative::IReactPackageBuilderFabric& builder) noexcept
{
    builder.AddViewComponent(
        L"RNSVGSvgView",
        [](winrt::Microsoft::ReactNative::IReactViewComponentBuilder const& builder) noexcept
    {
        builder.SetCreateProps([](winrt::Microsoft::ReactNative::ViewProps props) noexcept
        {
            return winrt::make<winrt::RNSVG::implementation::SvgViewProps>(props);
        });
        auto compBuilder =
            builder.as<winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder>();
        compBuilder.SetCreateViewComponentView(
            [](const winrt::Microsoft::ReactNative::Composition::CreateCompositionComponentViewArgs& args) noexcept
        {
            return winrt::make<winrt::RNSVG::implementation::SvgView>(args);
        });
    });
}

} // namespace winrt::RNSVG::implementation

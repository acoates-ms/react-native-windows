#include "pch.h"
#include "RNSVG.RectView.h"
#if __has_include("RNSVG.RectView.g.cpp")
#include "RectView.g.cpp"
#endif

#include "JSValueXaml.h"
#include "RNSVG.Utils.h"

using namespace winrt;
using namespace Microsoft::ReactNative;

namespace winrt::RNSVG::implementation {


    RectProps::RectProps(const winrt::Microsoft::ReactNative::ViewProps& props) : base_type(props)
    {
    }

    void RectProps::SetProp(uint32_t hash, winrt::hstring propName, winrt::Microsoft::ReactNative::IJSValueReader value) noexcept
    {
        winrt::Microsoft::ReactNative::ReadProp(hash, propName, value, *this);
    }


RectView::RectView(const winrt::Microsoft::ReactNative::CreateComponentViewArgs& args) : base_type(args) {}

void RectView::UpdateProperties(const winrt::Microsoft::ReactNative::IComponentProps& props, const winrt::Microsoft::ReactNative::IComponentProps& oldProps, bool forceUpdate, bool invalidate) noexcept
{
    m_props = props.as<RectProps>();

    /*
  const JSValueObject &propertyMap{JSValue::ReadObjectFrom(reader)};

  for (auto const &pair : propertyMap) {
    auto const &propertyName{pair.first};
    auto const &propertyValue{pair.second};

    assert(false);
    if (propertyName == "width") {
      m_width = SVGLength::From(propertyValue);
    } else if (propertyName == "height") {
      m_height = SVGLength::From(propertyValue);
    } else if (propertyName == "x") {
      m_x = SVGLength::From(propertyValue);
    } else if (propertyName == "y") {
      m_y = SVGLength::From(propertyValue);
    } else if (propertyName == "rx") {
      m_rx = SVGLength::From(propertyValue);
    } else if (propertyName == "ry") {
      m_ry = SVGLength::From(propertyValue);
    }
  }
  */
  base_type::UpdateProperties(props, oldProps, forceUpdate, invalidate);
}

void RectView::CreateGeometry() {
  auto const &root{SvgRoot()};

  float x{Utils::GetAbsoluteLength(m_props->x, root.ActualSize().Width)};
  float y{Utils::GetAbsoluteLength(m_props->y, root.ActualSize().Height)};
  float width{Utils::GetAbsoluteLength(m_props->width, root.ActualSize().Width)};
  float height{Utils::GetAbsoluteLength(m_props->height, root.ActualSize().Height)};

  auto const rxLength{m_props->rx.Unit == RNSVG::LengthType::Unknown ? m_props->ry : m_props->rx};
  auto const ryLength{m_props->ry.Unit == RNSVG::LengthType::Unknown ? m_props->rx : m_props->ry};
  float rx{Utils::GetAbsoluteLength(rxLength, root.ActualSize().Width)};
  float ry{Utils::GetAbsoluteLength(ryLength, root.ActualSize().Height)};

  com_ptr<ID2D1DeviceContext> deviceContext{get_self<D2DDeviceContext>(root.DeviceContext())->Get()};

  com_ptr<ID2D1Factory> factory;
  deviceContext->GetFactory(factory.put());

  com_ptr<ID2D1RoundedRectangleGeometry> geometry;
  check_hresult(factory->CreateRoundedRectangleGeometry(
      D2D1::RoundedRect(D2D1::RectF(x, y, width + x, height + y), rx, ry), geometry.put()));

  Geometry(make<RNSVG::implementation::D2DGeometry>(geometry.as<ID2D1Geometry>()));
}


void RectView::RegisterComponent(const winrt::Microsoft::ReactNative::IReactPackageBuilderFabric& builder) noexcept
{
    builder.AddViewComponent(
        L"RNSVGRect",
        [](winrt::Microsoft::ReactNative::IReactViewComponentBuilder const& builder) noexcept
    {
        builder.SetCreateProps([](winrt::Microsoft::ReactNative::ViewProps props) noexcept
        {
            return winrt::make<winrt::RNSVG::implementation::RectProps>(props);
        });
        builder.SetCreateComponentView(
            [](const winrt::Microsoft::ReactNative::CreateComponentViewArgs& args) noexcept
        {
            return winrt::make<winrt::RNSVG::implementation::RectView>(args);
        });
    });
}

} // namespace winrt::RNSVG::implementation

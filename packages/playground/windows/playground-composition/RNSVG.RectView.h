#pragma once

#include "RNSVG.RectProps.g.h"
#include "RNSVG.RectView.g.h"
#include "RNSVG.RenderableView.h"

namespace winrt::RNSVG::implementation {

REACT_STRUCT(RectProps)
struct RectProps : RectPropsT<RectProps, SvgRenderableCommonProps> {
  RectProps(const winrt::Microsoft::ReactNative::ViewProps &props);

  void SetProp(uint32_t hash, winrt::hstring propName, winrt::Microsoft::ReactNative::IJSValueReader value) noexcept
      override;

  REACT_SVG_NODE_COMMON_PROPS;
  REACT_SVG_RENDERABLE_COMMON_PROPS;

  REACT_FIELD(x)
  RNSVG::SVGLength x{0, winrt::RNSVG::LengthType::Unknown};
  REACT_FIELD(y)
  RNSVG::SVGLength y{0, winrt::RNSVG::LengthType::Unknown};
  REACT_FIELD(height)
  RNSVG::SVGLength height{0, winrt::RNSVG::LengthType::Unknown};
  REACT_FIELD(width)
  RNSVG::SVGLength width{0, winrt::RNSVG::LengthType::Unknown};
  REACT_FIELD(rx)
  RNSVG::SVGLength rx{0, winrt::RNSVG::LengthType::Unknown};
  REACT_FIELD(ry)
  RNSVG::SVGLength ry{0, winrt::RNSVG::LengthType::Unknown};
};

struct RectView : RectViewT<RectView, RNSVG::implementation::RenderableView> {
 public:
  RectView(const winrt::Microsoft::ReactNative::CreateComponentViewArgs &args);

  void UpdateProperties(
      const winrt::Microsoft::ReactNative::IComponentProps &props,
      const winrt::Microsoft::ReactNative::IComponentProps &oldProps,
      bool forceUpdate = true,
      bool invalidate = true) noexcept override;
  void CreateGeometry(RNSVG::D2DDeviceContext const &context);

  static void RegisterComponent(const winrt::Microsoft::ReactNative::IReactPackageBuilderFabric &builder) noexcept;

 private:
  com_ptr<RectProps> m_props;
};
} // namespace winrt::RNSVG::implementation

namespace winrt::RNSVG::factory_implementation {
struct RectView : RectViewT<RectView, implementation::RectView> {};
} // namespace winrt::RNSVG::factory_implementation

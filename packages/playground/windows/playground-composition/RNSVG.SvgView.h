#pragma once

#include "RNSVG.SvgView.g.h"

#include "RNSVG.SvgViewProps.g.h"

#include <JSValueComposition.h>
#include "NativeModules.h"
#include "RNSVG.SVGLength.h"
#include "RNSVG.Utils.h"

namespace winrt::RNSVG::implementation {
REACT_STRUCT(SvgViewProps)
struct SvgViewProps : SvgViewPropsT<SvgViewProps> {
  SvgViewProps(const winrt::Microsoft::ReactNative::ViewProps &props);

  void SetProp(uint32_t hash, winrt::hstring propName, winrt::Microsoft::ReactNative::IJSValueReader value) noexcept;

  REACT_FIELD(bbWidth)
  winrt::RNSVG::SVGLength bbWidth{0.0, winrt::RNSVG::LengthType::Unknown};

  REACT_FIELD(bbHeight)
  winrt::RNSVG::SVGLength bbHeight{0.0, winrt::RNSVG::LengthType::Unknown};

  REACT_FIELD(minX)
  float minX;
  REACT_FIELD(minY)
  float minY;
  REACT_FIELD(vbWidth)
  float vbWidth;
  REACT_FIELD(vbHeight)
  float vbHeight;
  REACT_FIELD(align)
  std::string align;
  REACT_FIELD(meetOrSlice)
  winrt::RNSVG::MeetOrSlice meetOrSlice;
  REACT_FIELD(tintColor)
  winrt::Microsoft::ReactNative::Color tintColor{nullptr};
  REACT_FIELD(color)
  winrt::Microsoft::ReactNative::Color color{nullptr};

 private:
  winrt::Microsoft::ReactNative::ViewProps m_props{nullptr};
};

struct SvgView : SvgViewT<SvgView> {
 public:
  SvgView() = default;

  SvgView(const winrt::Microsoft::ReactNative::Composition::CreateCompositionComponentViewArgs &args);

  RNSVG::GroupView Group() {
    return m_group;
  }
  void Group(RNSVG::GroupView const &value) {
    m_group = value;
  }

  winrt::Windows::Foundation::Size ActualSize() noexcept;

  RNSVG::D2DDevice Device() {
    return m_device;
  }

  RNSVG::D2DDeviceContext DeviceContext() {
    return m_deviceContext;
  }

  RNSVG::D2DGeometry Geometry() {
    return m_group ? m_group.Geometry() : nullptr;
  }
  void Geometry(RNSVG::D2DGeometry const & /*value*/) {}

  winrt::Microsoft::ReactNative::Color CurrentColor() {
    return m_currentColor;
  }

  bool IsResponsible() {
    return m_isResponsible;
  }
  void IsResponsible(bool isResponsible) {
    m_isResponsible = isResponsible;
  }

  Windows::Foundation::Collections::IMap<hstring, RNSVG::IRenderable> Templates() {
    return m_templates;
  }
  Windows::Foundation::Collections::IMap<hstring, RNSVG::BrushView> Brushes() {
    return m_brushes;
  }

  // IRenderable
  void UpdateProperties(
      const winrt::Microsoft::ReactNative::IComponentProps &props,
      const winrt::Microsoft::ReactNative::IComponentProps &oldProps,
      bool forceUpdate = true,
      bool invalidate = true);
  void MergeProperties(RNSVG::RenderableView const &other);
  void SaveDefinition();
  void Unload();
  void Draw(RNSVG::D2DDeviceContext const &deviceContext, Windows::Foundation::Size const &size);
  void CreateResources();
  void CreateGeometry();
  RNSVG::IRenderable HitTest(Windows::Foundation::Point const & /*point*/) {
    return nullptr;
  }

  // Overrides
  winrt::Microsoft::ReactNative::Composition::IVisual CreateVisual() noexcept;
  void UpdateProps(
      const winrt::Microsoft::ReactNative::IComponentProps &props,
      const winrt::Microsoft::ReactNative::IComponentProps &oldProps) noexcept;
  void UpdateLayoutMetrics(
      const winrt::Microsoft::ReactNative::LayoutMetrics &metrics,
      const winrt::Microsoft::ReactNative::LayoutMetrics &oldMetrics);
  // Windows::Foundation::Size MeasureOverride(Windows::Foundation::Size const &availableSize);
  // Windows::Foundation::Size ArrangeOverride(Windows::Foundation::Size const &finalSize);
  void MountChildComponentView(
      const winrt::Microsoft::ReactNative::ComponentView &childComponentView,
      uint32_t index) noexcept;
  void UnmountChildComponentView(
      const winrt::Microsoft::ReactNative::ComponentView &childComponentView,
      uint32_t index) noexcept;

  void Invalidate();

  static void RegisterComponent(const winrt::Microsoft::ReactNative::IReactPackageBuilderFabric &builder) noexcept;

 private:
  bool m_hasRendered{false};
  bool m_isResponsible{false};
  Microsoft::ReactNative::IReactContext m_reactContext{nullptr};
  RNSVG::D2DDevice m_device;
  RNSVG::D2DDeviceContext m_deviceContext;
  // Windows::UI::Xaml::Controls::Image m_image;
  RNSVG::GroupView m_group{nullptr};
  hstring m_id{L""};
  float m_minX{0.0f};
  float m_minY{0.0f};
  float m_vbWidth{0.0f};
  float m_vbHeight{0.0f};
  RNSVG::SVGLength m_bbWidth{};
  RNSVG::SVGLength m_bbHeight{};
  RNSVG::SVGLength m_width{};
  RNSVG::SVGLength m_height{};
  std::string m_align{""};
  winrt::Microsoft::ReactNative::Composition::ISpriteVisual m_visual{nullptr};
  RNSVG::MeetOrSlice m_meetOrSlice{RNSVG::MeetOrSlice::Meet};
  winrt::Microsoft::ReactNative::Color m_currentColor{nullptr};
  winrt::Microsoft::ReactNative::LayoutMetrics m_layoutMetrics;
  winrt::Microsoft::ReactNative::Composition::ICompositionContext m_compContext{nullptr};

  Windows::Foundation::Collections::IMap<hstring, RNSVG::IRenderable> m_templates{
      winrt::single_threaded_map<hstring, RNSVG::IRenderable>()};
  Windows::Foundation::Collections::IMap<hstring, RNSVG::BrushView> m_brushes{
      winrt::single_threaded_map<hstring, RNSVG::BrushView>()};
};
} // namespace winrt::RNSVG::implementation

namespace winrt::RNSVG::factory_implementation {
struct SvgView : SvgViewT<SvgView, implementation::SvgView> {};
} // namespace winrt::RNSVG::factory_implementation

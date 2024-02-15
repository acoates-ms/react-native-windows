#include "pch.h"

#include "JSValueXaml.h"

#include "RNSVG.GroupView.h"
#if __has_include("RNSVG.GroupView.g.cpp")
#include "RNSVG.GroupView.g.cpp"
#endif

#include "RNSVG.SVGLength.h"
#include "RNSVG.Utils.h"

using namespace winrt;
using namespace Microsoft::ReactNative;

namespace winrt::RNSVG::implementation {

SvgGroupCommonProps::SvgGroupCommonProps(const winrt::Microsoft::ReactNative::ViewProps &props) : base_type(props) {}

void SvgGroupCommonProps::SetProp(
    uint32_t hash,
    winrt::hstring propName,
    winrt::Microsoft::ReactNative::IJSValueReader value) noexcept {
  winrt::Microsoft::ReactNative::ReadProp(hash, propName, value, *this);
}

GroupView::GroupView(const winrt::Microsoft::ReactNative::CreateComponentViewArgs &args)
    : base_type(args), m_reactContext(args.ReactContext()) {}

void GroupView::MountChildComponentView(
    const winrt::Microsoft::ReactNative::ComponentView &childComponentView,
    uint32_t index) noexcept {
  RenderableView::MountChildComponentView(childComponentView, index);

  if (auto const &childView{childComponentView.try_as<IRenderable>()}) {
    childView.MergeProperties(*this);

    if (childView.IsResponsible() && !IsResponsible()) {
      IsResponsible(true);
    }

    if (auto const &root{SvgRoot()}) {
      root.Invalidate();
    }
  }
}

void GroupView::UnmountChildComponentView(
    const winrt::Microsoft::ReactNative::ComponentView &childComponentView,
    uint32_t index) noexcept {
  RenderableView::UnmountChildComponentView(childComponentView, index);

  if (auto child = childComponentView.as<IRenderable>()) {
    if (!IsUnloaded()) {
      child.Unload();
    }

    if (auto const &root{SvgRoot()}) {
      root.Invalidate();
    }
  }
}

void GroupView::UpdateProperties(
    const winrt::Microsoft::ReactNative::IComponentProps &props,
    const winrt::Microsoft::ReactNative::IComponentProps &oldProps,
    bool forceUpdate,
    bool invalidate) noexcept {
  auto renderableProps = props.as<SvgGroupCommonProps>();
  auto oldRenderableProps = oldProps ? oldProps.as<SvgGroupCommonProps>() : nullptr;

  auto const &parent{Parent().try_as<RNSVG::GroupView>()};

  if (!oldRenderableProps || renderableProps->font != oldRenderableProps->font) {
    if (forceUpdate || !m_fontPropMap[RNSVG::FontProp::FontSize]) {
      if (renderableProps->font.fontSize) {
        m_fontSize =
            renderableProps->font.fontSize ? renderableProps->font.fontSize : (parent ? parent.FontSize() : 12.0f);
      }

      m_fontPropMap[RNSVG::FontProp::FontSize] = !!renderableProps->font.fontSize;
    }

    if (forceUpdate || !m_fontPropMap[RNSVG::FontProp::FontFamily]) {
      if (renderableProps->font.fontSize) {
        m_fontFamily = !renderableProps->font.fontFamily.empty() ? winrt::to_hstring(renderableProps->font.fontFamily)
                                                                 : (parent ? parent.FontFamily() : L"Segoe UI");
      }

      m_fontPropMap[RNSVG::FontProp::FontFamily] = !renderableProps->font.fontFamily.empty();
    }

    if (forceUpdate || !m_fontPropMap[RNSVG::FontProp::FontWeight]) {
      if (!renderableProps->font.fontWeight.empty()) {
        m_fontWeight = !renderableProps->font.fontWeight.empty() ? winrt::to_hstring(renderableProps->font.fontWeight)
                                                                 : (parent ? parent.FontWeight() : L"auto");
      }

      m_fontPropMap[RNSVG::FontProp::FontWeight] = !renderableProps->font.fontWeight.empty();
    }
  }

  base_type::UpdateProperties(props, oldProps, forceUpdate, false);

  for (auto const &child : Children()) {
    child.as<IRenderable>().UpdateProperties(props, oldProps, false, false);
  }

  if (invalidate && Parent()) {
    SvgRoot().Invalidate();
  }
}

void GroupView::CreateGeometry(RNSVG::D2DDeviceContext const &context) {
  std::vector<ID2D1Geometry *> geometries;
  for (auto const childComponent : Children()) {
    auto const child = childComponent.as<IRenderable>();
    if (!child.Geometry()) {
      child.CreateGeometry(context);
    }

    if (child.Geometry()) {
      com_ptr<ID2D1Geometry> geometry{get_self<D2DGeometry>(child.Geometry())->Get()};

      // This takes advantage of the fact that geometry elements are alive for
      // the duration of this method and so are their D2D resources.
      geometries.emplace_back(geometry.get());
    }
  }

  if (!geometries.empty()) {
    com_ptr<ID2D1DeviceContext> deviceContext{get_self<D2DDeviceContext>(context)->Get()};

    com_ptr<ID2D1Factory> factory;
    deviceContext->GetFactory(factory.put());

    com_ptr<ID2D1GeometryGroup> group;
    check_hresult(factory->CreateGeometryGroup(
        D2DHelpers::GetFillRule(FillRule()), &geometries[0], static_cast<uint32_t>(geometries.size()), group.put()));

    Geometry(make<RNSVG::implementation::D2DGeometry>(group.as<ID2D1Geometry>()));
  }
}

void GroupView::SaveDefinition() {
  __super::SaveDefinition();

  for (auto const &child : Children()) {
    child.as<IRenderable>().SaveDefinition();
  }
}

void GroupView::MergeProperties(RNSVG::RenderableView const &other) {
  __super::MergeProperties(other);

  for (auto const &child : Children()) {
    child.as<IRenderable>().MergeProperties(*this);
  }
}

void GroupView::Draw(RNSVG::D2DDeviceContext const &context, Size const &size) {
  com_ptr<ID2D1DeviceContext> deviceContext{get_self<D2DDeviceContext>(context)->Get()};

  D2D1_MATRIX_3X2_F transform{D2DHelpers::GetTransform(deviceContext.get())};

  if (m_propSetMap[RNSVG::BaseProp::Matrix]) {
    deviceContext->SetTransform(D2DHelpers::AsD2DTransform(SvgTransform()) * transform);
  }

  com_ptr<ID2D1Geometry> clipPathGeometry;

  if (ClipPathGeometry(context)) {
    clipPathGeometry = get_self<D2DGeometry>(ClipPathGeometry(context))->Get();
  }

  D2DHelpers::PushOpacityLayer(deviceContext.get(), clipPathGeometry.get(), m_opacity);

  if (Children().Size() == 0) {
    __super::Draw(context, size);
  } else {
    DrawGroup(context, size);
  }

  deviceContext->PopLayer();

  deviceContext->SetTransform(transform);
}

void GroupView::DrawGroup(RNSVG::D2DDeviceContext const &context, Size const &size) {
  for (auto const &child : Children()) {
    child.as<IRenderable>().Draw(context, size);
  }
}

void GroupView::CreateResources() {
  for (auto const &child : Children()) {
    child.as<IRenderable>().CreateResources();
  }
}

void GroupView::Unload() {
  for (auto const &child : Children()) {
    child.as<IRenderable>().Unload();
  }

  m_reactContext = nullptr;
  m_fontPropMap.clear();

  __super::Unload();
}

winrt::RNSVG::IRenderable GroupView::HitTest(Point const &point) {
  RNSVG::IRenderable renderable{nullptr};
  if (IsResponsible()) {
    for (auto const &child : Children()) {
      if (auto const &hit{child.as<IRenderable>().HitTest(point)}) {
        renderable = hit;
      }
    }
    if (renderable && !renderable.IsResponsible()) {
      return *this;
    } else if (!renderable) {
      if (!Geometry()) {
        assert(false); // Do we need a real context here... I think the geometry should already be created by the time
                       // we get here?
        CreateGeometry(nullptr);
      }

      if (Geometry()) {
        com_ptr<ID2D1Geometry> geometry{get_self<D2DGeometry>(Geometry())->Get()};

        D2D1_RECT_F bounds;
        check_hresult(geometry->GetBounds(nullptr, &bounds));

        if (xaml::RectHelper::Contains(D2DHelpers::FromD2DRect(bounds), point)) {
          return *this;
        }
      }
    }
  }
  return renderable;
}

void GroupView::RegisterComponent(const winrt::Microsoft::ReactNative::IReactPackageBuilderFabric &builder) noexcept {
  builder.AddViewComponent(
      L"RNSVGGroup", [](winrt::Microsoft::ReactNative::IReactViewComponentBuilder const &builder) noexcept {
        builder.SetCreateProps([](winrt::Microsoft::ReactNative::ViewProps props) noexcept {
          return winrt::make<winrt::RNSVG::implementation::SvgGroupCommonProps>(props);
        });
        builder.SetCreateComponentView([](const winrt::Microsoft::ReactNative::CreateComponentViewArgs &args) noexcept {
          return winrt::make<winrt::RNSVG::implementation::GroupView>(args);
        });
      });
}

} // namespace winrt::RNSVG::implementation

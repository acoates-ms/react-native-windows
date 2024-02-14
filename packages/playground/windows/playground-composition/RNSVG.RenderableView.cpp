#include "pch.h"
#include "RNSVG.RenderableView.h"
#if __has_include("RNSVG.RenderableView.g.cpp")
#include "RNSVG.RenderableView.g.cpp"
#endif

#include "JSValueXaml.h"
#include "RNSVG.SvgView.h"
#include "RNSVG.Utils.h"

using namespace winrt;
using namespace Microsoft::ReactNative;

namespace winrt::RNSVG::implementation {

SvgNodeCommonProps::SvgNodeCommonProps(const winrt::Microsoft::ReactNative::ViewProps &props) : m_props(props) {}

void SvgNodeCommonProps::SetProp(
    uint32_t hash,
    winrt::hstring propName,
    winrt::Microsoft::ReactNative::IJSValueReader value) noexcept {
  winrt::Microsoft::ReactNative::ReadProp(hash, propName, value, *this);
}

SvgRenderableCommonProps::SvgRenderableCommonProps(const winrt::Microsoft::ReactNative::ViewProps &props)
    : base_type(props) {}

void SvgRenderableCommonProps::SetProp(
    uint32_t hash,
    winrt::hstring propName,
    winrt::Microsoft::ReactNative::IJSValueReader value) noexcept {
  winrt::Microsoft::ReactNative::ReadProp(hash, propName, value, *this);
}

RenderableView::RenderableView(const winrt::Microsoft::ReactNative::CreateComponentViewArgs &args)
    : base_type(args), m_reactContext(args.ReactContext()) {}

void RenderableView::UpdateProps(
    const winrt::Microsoft::ReactNative::IComponentProps &props,
    const winrt::Microsoft::ReactNative::IComponentProps &oldProps) noexcept {
  UpdateProperties(props, oldProps);
}

void RenderableView::UpdateProperties(
    const winrt::Microsoft::ReactNative::IComponentProps &props,
    const winrt::Microsoft::ReactNative::IComponentProps &oldProps,
    bool forceUpdate,
    bool invalidate) noexcept {
  auto renderableProps = props.as<SvgRenderableCommonProps>();
  auto oldRenderableProps = oldProps ? oldProps.as<SvgRenderableCommonProps>() : nullptr;

  bool fillSet{
      renderableProps->propList &&
      std::find(renderableProps->propList->begin(), renderableProps->propList->end(), "fill") !=
          renderableProps->propList->end()};
  bool strokeSet{
      renderableProps->propList &&
      std::find(renderableProps->propList->begin(), renderableProps->propList->end(), "stroke") !=
          renderableProps->propList->end()};

  auto const &parent{Parent().try_as<RNSVG::RenderableView>()};

  // name is not a prop we want to propagate to child elements
  // so we only set it when forceUpdate = true

  if (forceUpdate && (!oldRenderableProps || renderableProps->name != oldRenderableProps->name)) {
    if (parent) {
      SvgRoot().Templates().Remove(m_id);
    }
    m_id = winrt::to_hstring(renderableProps->name);
    if (parent) {
      SaveDefinition();
    }
  }

  if (!oldRenderableProps || renderableProps->strokeWidth != oldRenderableProps->strokeWidth) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeWidth]) {
      m_strokeWidth = (renderableProps->strokeWidth != std::nullopt)
          ? *renderableProps->strokeWidth
          : (parent ? parent.StrokeWidth() : RNSVG::SVGLength{1.0f, RNSVG::LengthType::Pixel});
    }

    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::StrokeWidth] = renderableProps->strokeWidth != std::nullopt;
    }
  }

  if (!oldRenderableProps || renderableProps->strokeOpacity != oldRenderableProps->strokeOpacity) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeOpacity]) {
      m_strokeOpacity = (renderableProps->strokeOpacity != std::nullopt) ? *renderableProps->strokeOpacity
                                                                         : (parent ? parent.StrokeOpacity() : 1.0f);
    }
    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::StrokeOpacity] = renderableProps->strokeOpacity != std::nullopt;
    }
  }

  if (!oldRenderableProps || renderableProps->fillOpacity != oldRenderableProps->fillOpacity) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::FillOpacity]) {
      m_fillOpacity = (renderableProps->fillOpacity != std::nullopt) ? *renderableProps->fillOpacity
                                                                     : (parent ? parent.FillOpacity() : 1.0f);
    }
    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::FillOpacity] = renderableProps->fillOpacity != std::nullopt;
    }
  }

  if (!oldRenderableProps || renderableProps->stroke != oldRenderableProps->stroke) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::Stroke]) {
      if (!m_strokeBrushId.empty()) {
        m_strokeBrushId.clear();
      }

      SetColor(
          renderableProps->stroke,
          ((parent && !strokeSet) ? parent.Stroke() : winrt::Microsoft::ReactNative::Color::Transparent()),
          "stroke");
    }
    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      m_propSetMap[RNSVG::BaseProp::Stroke] = strokeSet;
    }
  }

  if (!oldRenderableProps || renderableProps->fill != oldRenderableProps->fill) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::Fill]) {
      winrt::Microsoft::ReactNative::Color fallbackColor{winrt::Microsoft::ReactNative::Color::Black()};
      if (renderableProps->fill == std::nullopt && fillSet) {
        fallbackColor = winrt::Microsoft::ReactNative::Color::Transparent();
      } else if (parent) {
        fallbackColor = parent.Fill();
      }

      if (!m_fillBrushId.empty()) {
        m_fillBrushId.clear();
      }

      SetColor(renderableProps->fill, fallbackColor, "fill");
    }
    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      m_propSetMap[RNSVG::BaseProp::Fill] = fillSet;
    }
  }

  if (!oldRenderableProps || renderableProps->strokeLinecap != oldRenderableProps->strokeLinecap) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeLineCap]) {
      m_strokeLineCap =
          renderableProps->strokeLinecap != std::nullopt ? *renderableProps->strokeLinecap : parent.StrokeLineCap();
    }
    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::StrokeLineCap] = renderableProps->strokeLinecap != std::nullopt;
    }
  }

  if (!oldRenderableProps || renderableProps->strokeLinejoin != oldRenderableProps->strokeLinejoin) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeLineJoin]) {
      m_strokeLineJoin =
          renderableProps->strokeLinejoin != std::nullopt ? *renderableProps->strokeLinejoin : parent.StrokeLineJoin();
    }
    // forceUpdate = true means the property is being set on an element
    // instead of being inherited from the parent.
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::StrokeLineJoin] = renderableProps->strokeLinejoin != std::nullopt;
    }
  }

  if (!oldRenderableProps || renderableProps->fillRule != oldRenderableProps->fillRule) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::FillRule]) {
      m_fillRule = renderableProps->fillRule != std::nullopt ? *renderableProps->fillRule : parent.FillRule();
    }
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::FillRule] = renderableProps->fillRule != std::nullopt;
    }
  }

  if (!oldRenderableProps || renderableProps->strokeDashoffset != oldRenderableProps->strokeDashoffset) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeDashOffset]) {
      m_strokeDashOffset = renderableProps->strokeDashoffset != std::nullopt
          ? *renderableProps->strokeDashoffset
          : (parent ? parent.StrokeDashOffset() : 0.0f);
    }
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::StrokeDashOffset] = renderableProps->strokeDashoffset != std::nullopt;
    }
  }
  if (!oldRenderableProps || renderableProps->strokeMiterlimit != oldRenderableProps->strokeMiterlimit) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeMiterLimit]) {
      m_strokeMiterLimit = renderableProps->strokeMiterlimit != std::nullopt
          ? *renderableProps->strokeMiterlimit
          : (parent ? parent.StrokeMiterLimit() : 0.0f);
    }
    if (forceUpdate) {
      // If the optional is null, that generally means the prop was deleted
      m_propSetMap[RNSVG::BaseProp::StrokeMiterLimit] = renderableProps->strokeMiterlimit != std::nullopt;
    }
  }
  if (!oldRenderableProps || renderableProps->strokeDasharray != oldRenderableProps->strokeDasharray) {
    if (forceUpdate || !m_propSetMap[RNSVG::BaseProp::StrokeDashArray]) {
      if (renderableProps->strokeDasharray != std::nullopt) {
        m_strokeDashArray.Clear();

        for (auto const &item : *renderableProps->strokeDasharray) {
          m_strokeDashArray.Append(item);
        }
      } else {
        m_strokeDashArray = parent.StrokeDashArray();
      }
      if (forceUpdate) {
        // If the optional is null, that generally means the prop was deleted
        m_propSetMap[RNSVG::BaseProp::StrokeDashArray] = renderableProps->strokeDasharray != std::nullopt;
      }
    }
  }

  if (!oldRenderableProps || renderableProps->matrix != oldRenderableProps->matrix) {
    if (forceUpdate) {
      m_transformMatrix = renderableProps->matrix != std::nullopt
          ? Numerics::float3x2(
                renderableProps->matrix->at(0),
                renderableProps->matrix->at(1),
                renderableProps->matrix->at(2),
                renderableProps->matrix->at(3),
                renderableProps->matrix->at(4),
                renderableProps->matrix->at(5))
          : (parent ? parent.SvgTransform() : Numerics::make_float3x2_rotation(0));
      if (forceUpdate) {
        // If the optional is null, that generally means the prop was deleted
        m_propSetMap[RNSVG::BaseProp::Matrix] = renderableProps->matrix != std::nullopt;
      }
    }
  }

  if (forceUpdate && (!oldRenderableProps || renderableProps->opacity != oldRenderableProps->opacity)) {
    m_opacity = renderableProps->opacity != std::nullopt ? *renderableProps->opacity : 1.0f;
  }

  if (!oldRenderableProps || renderableProps->clipPath != oldRenderableProps->clipPath) {
    m_clipPathId = to_hstring(renderableProps->clipPath);
  }
  if (!oldRenderableProps || renderableProps->responsible != oldRenderableProps->responsible) {
    m_isResponsible = renderableProps->responsible;
  }

  m_recreateResources = true;

  if (invalidate && Parent()) {
    SvgRoot().Invalidate();
  }
}

void RenderableView::SaveDefinition() {
  if (m_id != L"") {
    SvgRoot().Templates().Insert(m_id, *this);
  }
}

void RenderableView::Draw(RNSVG::D2DDeviceContext const &context, Size const &size) {
  if (m_recreateResources) {
    CreateGeometry();
  }

  if (!Geometry()) {
    return;
  }

  com_ptr<ID2D1Geometry> geometry{get_self<D2DGeometry>(m_geometry)->Get()};
  com_ptr<ID2D1DeviceContext> deviceContext{get_self<D2DDeviceContext>(context)->Get()};

  D2D1_MATRIX_3X2_F transform{D2DHelpers::GetTransform(deviceContext.get())};

  if (m_propSetMap[RNSVG::BaseProp::Matrix]) {
    deviceContext->SetTransform(D2DHelpers::AsD2DTransform(SvgTransform()) * transform);
  }

  com_ptr<ID2D1Factory> factory;
  deviceContext->GetFactory(factory.put());

  com_ptr<ID2D1GeometryGroup> geometryGroup;
  ID2D1Geometry *geometries[] = {geometry.get()};
  check_hresult(factory->CreateGeometryGroup(D2DHelpers::GetFillRule(FillRule()), geometries, 1, geometryGroup.put()));

  geometry = geometryGroup;

  com_ptr<ID2D1Geometry> clipPathGeometry;
  if (ClipPathGeometry()) {
    clipPathGeometry = get_self<D2DGeometry>(ClipPathGeometry())->Get();
  }

  D2DHelpers::PushOpacityLayer(deviceContext.get(), clipPathGeometry.get(), m_opacity);

  if (FillOpacity()) {
    D2DHelpers::PushOpacityLayer(deviceContext.get(), clipPathGeometry.get(), FillOpacity());

    auto fill{Utils::GetCanvasBrush(FillBrushId(), Fill(), SvgRoot(), geometry)};
    deviceContext->FillGeometry(geometry.get(), fill.get());

    deviceContext->PopLayer();
  }

  if (StrokeOpacity()) {
    D2DHelpers::PushOpacityLayer(deviceContext.get(), clipPathGeometry.get(), StrokeOpacity());

    D2D1_CAP_STYLE capStyle{D2DHelpers::GetLineCap(m_strokeLineCap)};
    D2D1_LINE_JOIN lineJoin{D2DHelpers::GetLineJoin(m_strokeLineJoin)};

    D2D1_STROKE_STYLE_PROPERTIES strokeStyleProperties;
    strokeStyleProperties.startCap = capStyle;
    strokeStyleProperties.endCap = capStyle;
    strokeStyleProperties.dashCap = capStyle;
    strokeStyleProperties.lineJoin = lineJoin;
    strokeStyleProperties.dashOffset = StrokeDashOffset();
    strokeStyleProperties.miterLimit = StrokeMiterLimit();
    strokeStyleProperties.dashStyle = D2D1_DASH_STYLE_SOLID;

    float canvasDiagonal{Utils::GetCanvasDiagonal(size)};
    float strokeWidth{Utils::GetAbsoluteLength(StrokeWidth(), canvasDiagonal)};

    float *dashArray = nullptr;
    if (StrokeDashArray().Size() > 0) {
      strokeStyleProperties.dashStyle = D2D1_DASH_STYLE_CUSTOM;
      m_adjustedStrokeDashArray = Utils::GetAdjustedStrokeArray(StrokeDashArray(), strokeWidth, canvasDiagonal);
      dashArray = &m_adjustedStrokeDashArray[0];
    }

    com_ptr<ID2D1StrokeStyle> strokeStyle;
    check_hresult(
        factory->CreateStrokeStyle(strokeStyleProperties, dashArray, m_strokeDashArray.Size(), strokeStyle.put()));

    auto const stroke{Utils::GetCanvasBrush(StrokeBrushId(), Stroke(), SvgRoot(), geometry)};
    deviceContext->DrawGeometry(geometry.get(), stroke.get(), strokeWidth, strokeStyle.get());
    deviceContext->PopLayer();
  }

  deviceContext->PopLayer();

  deviceContext->SetTransform(transform);
}

void RenderableView::MergeProperties(RNSVG::RenderableView const &other) {
  for (auto const &prop : m_propSetMap) {
    if (!prop.second) {
      switch (prop.first) {
        case RNSVG::BaseProp::Fill:
          m_fill = other.Fill();
          m_fillBrushId = other.FillBrushId();
          break;
        case RNSVG::BaseProp::FillOpacity:
          m_fillOpacity = other.FillOpacity();
          break;
        case RNSVG::BaseProp::FillRule:
          m_fillRule = other.FillRule();
          break;
        case RNSVG::BaseProp::Stroke:
          m_stroke = other.Stroke();
          m_strokeBrushId = other.StrokeBrushId();
          break;
        case RNSVG::BaseProp::StrokeOpacity:
          m_strokeOpacity = other.StrokeOpacity();
          break;
        case RNSVG::BaseProp::StrokeWidth:
          m_strokeWidth = other.StrokeWidth();
          break;
        case RNSVG::BaseProp::StrokeMiterLimit:
          m_strokeMiterLimit = other.StrokeMiterLimit();
          break;
        case RNSVG::BaseProp::StrokeDashOffset:
          m_strokeDashOffset = other.StrokeDashOffset();
          break;
        case RNSVG::BaseProp::StrokeDashArray:
          m_strokeDashArray = other.StrokeDashArray();
          break;
        case RNSVG::BaseProp::StrokeLineCap:
          m_strokeLineCap = other.StrokeLineCap();
          break;
        case RNSVG::BaseProp::StrokeLineJoin:
          m_strokeLineJoin = other.StrokeLineJoin();
          break;
        case RNSVG::BaseProp::Unknown:
        default:
          break;
      }
    }
  }
}

RNSVG::SvgView RenderableView::SvgRoot() {
  if (Parent()) {
    if (auto const &svgView{Parent().try_as<RNSVG::SvgView>()}) {
      if (svgView.Parent()) {
        if (auto const &parent{svgView.Parent().try_as<RNSVG::RenderableView>()}) {
          return parent.SvgRoot();
        }
      } else {
        return svgView;
      }
    } else if (auto const &renderable{Parent().try_as<RNSVG::RenderableView>()}) {
      return renderable.SvgRoot();
    }
  }

  return nullptr;
}

RNSVG::D2DGeometry RenderableView::ClipPathGeometry() {
  if (!m_clipPathId.empty()) {
    if (auto const &clipPath{SvgRoot().Templates().TryLookup(m_clipPathId)}) {
      if (!clipPath.Geometry()) {
        clipPath.CreateGeometry();
      }
      return clipPath.Geometry();
    }
  }
  return nullptr;
}

void RenderableView::Unload() {
  if (m_geometry) {
    m_geometry = nullptr;
  }

  m_parent = nullptr;
  m_reactContext = nullptr;
  m_propSetMap.clear();
  m_strokeDashArray.Clear();
  m_isUnloaded = true;
}

RNSVG::IRenderable RenderableView::HitTest(Point const &point) {
  if (m_geometry) {
    BOOL strokeContainsPoint = FALSE;
    D2D1_POINT_2F pointD2D{point.X, point.Y};

    com_ptr<ID2D1Geometry> geometry{get_self<D2DGeometry>(m_geometry)->Get()};

    if (auto const &svgRoot{SvgRoot()}) {
      float canvasDiagonal{Utils::GetCanvasDiagonal(svgRoot.ActualSize())};
      float strokeWidth{Utils::GetAbsoluteLength(StrokeWidth(), canvasDiagonal)};

      check_hresult(geometry->StrokeContainsPoint(pointD2D, strokeWidth, nullptr, nullptr, &strokeContainsPoint));
    }

    BOOL fillContainsPoint = FALSE;
    check_hresult(geometry->FillContainsPoint(pointD2D, nullptr, &fillContainsPoint));

    if (fillContainsPoint || strokeContainsPoint) {
      return *this;
    }
  }
  return nullptr;
}

void RenderableView::SetColor(
    std::optional<ColorStruct> &color,
    winrt::Microsoft::ReactNative::Color const &fallbackColor,
    std::string propName) {
  if (color == std::nullopt) {
    propName == "fill" ? m_fill = fallbackColor : m_stroke = fallbackColor;
    return;
  }

  switch (color->type) {
    // https://github.com/software-mansion/react-native-svg/blob/main/src/lib/extract/extractBrush.ts#L29
    case 1: {
      propName == "fill" ? m_fillBrushId = winrt::to_hstring(color->brushRef)
                         : m_strokeBrushId = winrt::to_hstring(color->brushRef);
      break;
    }
    // https://github.com/software-mansion/react-native-svg/blob/main/src/lib/extract/extractBrush.ts#L6-L8
    case 2: // currentColor
    case 3: // context-fill
    case 4: // context-stroke
      propName == "fill" ? m_fillBrushId = L"currentColor" : m_strokeBrushId = L"currentColor";
      break;
    default: {
      auto const &c = color->payload ? color->payload : fallbackColor;
      propName == "fill" ? m_fill = c : m_stroke = c;
      break;
    }
  }
}

} // namespace winrt::RNSVG::implementation

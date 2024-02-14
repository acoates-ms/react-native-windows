#include "pch.h"
#include "RNSVG.BrushView.h"
#if __has_include("RNSVG.RectView.g.cpp")
#include "RNSVG.BrushView.g.cpp"
#endif

#include "RNSVG.D2DHelpers.h"

namespace winrt::RNSVG::implementation {
BrushView::BrushView(const winrt::Microsoft::ReactNative::CreateComponentViewArgs &args) : base_type(args) {}

void BrushView::SaveDefinition() {
  if (auto const &root{SvgRoot()}) {
    CreateBrush();
    root.Brushes().Insert(Id(), *this);
  }
}

void BrushView::SetBounds(Rect const &rect) {
  m_bounds = D2DHelpers::AsD2DRect(rect);
  UpdateBounds();
}

void BrushView::Unload() {
  if (m_brush) {
    m_brush = nullptr;
  }

  __super::Unload();
}
} // namespace winrt::RNSVG::implementation

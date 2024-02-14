#include "pch.h"
#include "DefsViewManager.g.cpp"
#include "DefsViewManager.h"

namespace winrt::RNSVG::implementation {
DefsViewManager::DefsViewManager() {
  m_class = RNSVG::SVGClass::RNSVGDefs;
  m_name = L"RNSVGDefs";
}
} // namespace winrt::RNSVG::implementation

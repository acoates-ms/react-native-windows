// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#pragma warning(push)
#pragma warning(disable : 5028) // 'Node': Alignment specified in prior declaration (1) not specified in definition
#include <react/renderer/textlayoutmanager/TextLayoutManager.h>
#pragma warning(pop)

#include <react/renderer/attributedstring/AttributedStringBox.h>
#include <react/renderer/attributedstring/ParagraphAttributes.h>
#include <react/renderer/core/LayoutConstraints.h>
#include <react/renderer/textlayoutmanager/TextLayoutContext.h>
#include <react/renderer/textlayoutmanager/TextMeasureCache.h>
#include <react/utils/ContextContainer.h>
#include <memory>

namespace facebook::react {

class WindowsTextLayoutCacheKey final {
 public:
  AttributedString attributedString{};
  ParagraphAttributes paragraphAttributes{};
  Size size{};
  //LayoutConstraints layoutConstraints{};
};

using WindowsTextLayoutCache = SimpleThreadSafeCache<WindowsTextLayoutCacheKey, winrt::com_ptr<IDWriteTextLayout>, kSimpleThreadSafeCacheSizeCap>;

inline bool operator==(const WindowsTextLayoutCacheKey &lhs, const WindowsTextLayoutCacheKey &rhs)
{
  return areAttributedStringsEquivalentLayoutWise(lhs.attributedString, rhs.attributedString) &&
      lhs.paragraphAttributes == rhs.paragraphAttributes && lhs.size == rhs.size;
}

}

namespace std {

template <>
struct hash<facebook::react::WindowsTextLayoutCacheKey> {
  size_t operator()(const facebook::react::WindowsTextLayoutCacheKey &key) const
  {
    return facebook::react::hash_combine(
        attributedStringHashLayoutWise(key.attributedString), key.paragraphAttributes, key.size);
  }
};

}

namespace facebook::react {


class WindowsTextLayoutManager : public TextLayoutManager {
 public:
  WindowsTextLayoutManager(const std::shared_ptr<const ContextContainer> &contextContainer);
  virtual ~WindowsTextLayoutManager() = default;

  /*
   * Not copyable.
   */
  WindowsTextLayoutManager(const WindowsTextLayoutManager &) = delete;
  WindowsTextLayoutManager &operator=(const WindowsTextLayoutManager &) = delete;

  /*
   * Not movable.
   */
  WindowsTextLayoutManager(WindowsTextLayoutManager &&) = delete;
  WindowsTextLayoutManager &operator=(WindowsTextLayoutManager &&) = delete;

  static void GetTextLayout(
      const AttributedStringBox &attributedStringBox,
      const ParagraphAttributes &paragraphAttributes,
      Size size,
      winrt::com_ptr<IDWriteTextLayout> &spTextLayout,
      TextMeasurement::Attachments &attachments) noexcept;

  static void GetTextLayout(
      const AttributedStringBox &attributedStringBox,
      const ParagraphAttributes &paragraphAttributes,
      LayoutConstraints layoutConstraints,
      winrt::com_ptr<IDWriteTextLayout> &spTextLayout) noexcept;

  static void GetTextLayoutByAdjustingFontSizeToFit(
      AttributedStringBox attributedStringBox,
      const ParagraphAttributes &paragraphAttributes,
      LayoutConstraints layoutConstraints,
      winrt::com_ptr<IDWriteTextLayout> &spTextLayout,
      TextMeasurement::Attachments &attachments,
      float minimumFontScale) noexcept;

  static winrt::hstring GetTransformedText(const AttributedStringBox &attributedStringBox);

 private:
  static void GetTextLayout(
      const AttributedStringBox &attributedStringBox,
      const ParagraphAttributes &paragraphAttributes,
      Size size,
      winrt::com_ptr<IDWriteTextLayout> &spTextLayout) noexcept;

    static winrt::com_ptr<IDWriteTextLayout> CreateTextLayout(
      const AttributedStringBox &attributedStringBox,
      const ParagraphAttributes &paragraphAttributes,
      Size size,
    TextMeasurement::Attachments &attachments) noexcept;


    static WindowsTextLayoutCache m_textLayoutCache;
};

} // namespace facebook::react

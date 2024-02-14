// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "LayoutContext.g.h"
#include "ShadowNode.g.h"
#include "YogaLayoutableShadowNode.g.h"
#include <react/components/rnwcore/EventEmitters.h>
#include <unordered_map>
#include "AbiState.h"
#include "AbiViewProps.h"

#include <react/renderer/components/view/ConcreteViewShadowNode.h>
#include <react/renderer/core/LayoutContext.h>

namespace winrt::Microsoft::ReactNative::implementation {

struct ShadowNode : ShadowNodeT<ShadowNode> {
  ShadowNode(facebook::react::ShadowNode::Shared shadowNode) noexcept;

  void EnsureUnsealed() noexcept;
  winrt::IInspectable Tag() const noexcept;
  void Tag(winrt::IInspectable tag) noexcept;

  winrt::IInspectable StateData() const noexcept;
  void StateData(winrt::IInspectable tag) noexcept;

 protected:
  facebook::react::ShadowNode::Shared m_shadowNode;
  winrt::IInspectable m_tag;
};

} // namespace winrt::Microsoft::ReactNative::implementation

namespace Microsoft::ReactNative {

extern const char AbiComponentName[];

class AbiShadowNode final : public facebook::react::ConcreteShadowNode<
                                    AbiComponentName,
                                    facebook::react::ShadowNode,
                                    AbiViewProps, // TODO make non-view props object
                                    facebook::react::EventEmitter,
                                    Microsoft::ReactNative::AbiStateData> {
 public:
  using ConcreteShadowNode::ConcreteShadowNode;

  void OnClone(const facebook::react::ShadowNode &sourceShadowNode) noexcept;
  void Builder(winrt::Microsoft::ReactNative::IReactViewComponentBuilder builder) noexcept;
  winrt::Microsoft::ReactNative::IReactViewComponentBuilder Builder() const noexcept;
  void Proxy(winrt::Microsoft::ReactNative::ShadowNode handle) noexcept;
  winrt::Microsoft::ReactNative::ShadowNode Proxy() const noexcept;

 private:
  winrt::Microsoft::ReactNative::ShadowNode m_proxy{nullptr};
  winrt::Microsoft::ReactNative::IReactViewComponentBuilder m_builder{nullptr};
};

} // namespace Microsoft::ReactNative

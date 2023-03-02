
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/components/view/ViewProps.h>

namespace Microsoft::ReactNative {
// Inside MS.RN this is the props object used.

class AbiViewProps final : public facebook::react::ViewProps {
 public:
  AbiViewProps() = default;
  AbiViewProps(winrt::Microsoft::ReactNative::IViewProps outer, const facebook::react::PropsParserContext& context, const AbiViewProps &sourceProps, const facebook::react::RawProps &rawProps);

  void AbiViewProps::setProp(
    const facebook::react::PropsParserContext &context,
    facebook::react::RawPropsPropNameHash /*uint32_t*/ hash,
    const char *propName,
    facebook::react::RawValue const &value);
  
  winrt::Microsoft::ReactNative::IViewProps m_outer;
};



// This is the class that external ABI uses -- ABI shadow nodes and component views use this
// We can add codegen for this
class MyViewProps : winrt::Microsoft::ReactNative::IViewProps {
 public:
  MyViewProps() = default;
  MyViewProps(Handle<AbiViewProps> viewProps)
  : m_viewProps(viewProps) {
  }

  void SetProp(uint32_t hash, string propName, PropertyValue value) {
    // Could optimize to use hash instead of propName
    if (propName == "CustomProp") {
        m_customProp = value.CastString();
    }
  }

  // Getters for all the props on ViewProps --
  Float Opacity() { return m_viewProps.Opacity; } // Could be impl as return ABI_GetViewProp_Opacity(m_viewProps)
  // maybe setters if we want too....
  void Opacity(float value) { m_viewProps.Opacity = value; } // Could be impl as return ABI_SetViewProp_Opacity(m_viewProps, value)
  // .. etc

  private:
    Handle<AbiViewProps> m_viewProps;
    std::string m_customProp;
};

} // namespace Microsoft::ReactNative
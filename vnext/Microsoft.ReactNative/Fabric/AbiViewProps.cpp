
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/components/view/ViewProps.h>

namespace Microsoft::ReactNative {

RawValue : winrt::implements<winrt::Microsoft::ReactNative::IRawValue>
{
...
}

// Inside MS.RN this is the props object used.

AbiViewProps::AbiViewProps() {
}

AbiViewProps::AbiViewProps(const facebook::react::PropsParserContext& context, const AbiViewProps &sourceProps, const facebook::react::RawProps &rawProps)
    : facebook::react::ViewProps(context, sourceProps, rawProps)
    {    
    }

void AbiViewProps::setProp(
    const facebook::react::PropsParserContext &context,
    facebook::react::RawPropsPropNameHash /*uint32_t*/ hash,
    const char *propName,
    facebook::react::RawValue const &value) {
      facebook::react::ViewProps::setProp(context, hash, propName, value);

      // RawValue is a wrapped JSI Value. -- maybe enable extracting the JSI value to something that we already an ABI for
      // That could be going through our ABI jsi layer....or a new variant type
      m_outer->SetProp(hash, winrt::to_hstring(propName), winrt::make<RawValue>(ToPropertyValueOrVariant(value));
    }


// This is the class that external ABI uses -- ABI shadow nodes and component views use this
class JSI_EXPORT MyAbiViewProps : IAbiViewProps {
 public:
  AbiViewProps() = default;
  AbiViewProps(ViewPropsHandle viewProps)
  : m_viewProps(viewProps) {
  }

  void SetProp(uint32_t hash, string propName, PropertyValue value) {
    // Could optimize to use hash instead of propName
    if (propName == "CustomProp") {
        m_customProp = value.AsString();
    }
  }

  // Getters for all the props on ViewProps --
  Float Opacity() { return m_viewProps.Opacity; } // Could be impl as return ABI_GetViewProp_Opacity(m_viewProps)
  // maybe setters if we want too....
  void Opacity(float value) { m_viewProps.Opacity = value; } // Could be impl as return ABI_SetViewProp_Opacity(m_viewProps, value)
  // .. etc

  private:
    ViewPropsHandle m_viewProps;
    std::string m_customProp;
};

} // namespace Microsoft::ReactNative
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "CoreAppPage.g.h"
#include <UI.Xaml.Markup.h>

/**
  Work around bug in experimental version of WinAppSDK
  The experimental version 1.4.230518007-experimental1, of WinAppSDK generates the following in XamlTypeInfo.g.cpp
        static_assert( is_type_complete_v<::winrt::System::Delegate>, "Please #include the implementation header for '::winrt::System::Delegate' in your precompiled header 'pch.h'." );
        static_assert( is_type_complete_v<::winrt::System::MulticastDelegate>, "Please #include the implementation header for '::winrt::System::MulticastDelegate' in your precompiled header 'pch.h'." );
  These types are not used anywhere, so just redirecting the type here bypasses the error
*/
namespace winrt::System {
using Delegate = ::winrt::Microsoft::UI::Xaml::Controls::Grid;
using MulticastDelegate = ::winrt::Microsoft::UI::Xaml::Controls::Grid;
}

namespace winrt::Microsoft::ReactNative::implementation {
struct CoreAppPage : CoreAppPageT<CoreAppPage> {
  CoreAppPage();
};
} // namespace winrt::Microsoft::ReactNative::implementation

namespace winrt::Microsoft::ReactNative::factory_implementation {
struct CoreAppPage : CoreAppPageT<CoreAppPage, implementation::CoreAppPage> {};
} // namespace winrt::Microsoft::ReactNative::factory_implementation

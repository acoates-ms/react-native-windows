// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "LocalBundle.g.h"

namespace winrt::Microsoft::ReactNative::implementation {

struct LocalBundle {
  LocalBundle() = default;
  static void Load(IReactContext, winrt::hstring const & url) noexcept;
};

} // namespace winrt::Microsoft::ReactNative::implementation

namespace winrt::Microsoft::ReactNative::factory_implementation {

struct LocalBundle : LocalBundleT<LocalBundle, implementation::LocalBundle> {};

} // namespace winrt::Microsoft::ReactNative::factory_implementation

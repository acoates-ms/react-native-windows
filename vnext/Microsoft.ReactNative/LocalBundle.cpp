// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "LocalBundle.h"
#include "LocalBundle.g.cpp"
#include "IReactContext.h"
#include "unicode.h"
#include "future/futureWinRT.h"

namespace winrt::Microsoft::ReactNative::implementation {

/*static*/ void LocalBundle::Load(IReactContext context, winrt::hstring const& url) noexcept
{
  auto contextImpl = winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactContext>(context);
  contextImpl->GetInner().LoadBundle(::Microsoft::Common::Unicode::Utf16ToUtf8(url));
}

} // namespace winrt::Microsoft::ReactNative::implementation

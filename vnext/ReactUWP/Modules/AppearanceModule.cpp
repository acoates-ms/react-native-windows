// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "AppearanceModule.h"

#include <winrt/Windows.UI.ViewManagement.h>

using Application = xaml::Application;
using ApplicationTheme = xaml::ApplicationTheme;
using UISettings = winrt::Windows::UI::ViewManagement::UISettings;

using Method = facebook::xplat::module::CxxModule::Method;

namespace react::uwp {

AppearanceChangeListener::AppearanceChangeListener(
    std::weak_ptr<IReactInstance> &&reactInstance,
    Mso::DispatchQueue const &uiQueue) noexcept
    : Mso::ActiveObject<>(uiQueue), m_weakReactInstance(std::move(reactInstance)) {
  // Ensure we're constructed on the UI thread
  VerifyElseCrash(uiQueue.HasThreadAccess());

  m_currentTheme = Application::Current().RequestedTheme();

  // UISettings will notify us on a background thread regardless of where we construct it or register for events.
  // Redirect callbacks to the UI thread where we can check app theme.
  m_revoker = m_uiSettings.ColorValuesChanged(
      winrt::auto_revoke, [weakThis{Mso::WeakPtr(this)}](const auto & /*sender*/, const auto & /*args*/) noexcept {
        if (auto strongThis = weakThis.GetStrongPtr()) {
          strongThis->InvokeInQueueStrong([strongThis]() noexcept { strongThis->OnColorValuesChanged(); });
        }
      });
}

const char *AppearanceChangeListener::GetColorScheme() const noexcept {
  return ToString(m_currentTheme);
}

const char *AppearanceChangeListener::ToString(ApplicationTheme theme) noexcept {
  return theme == ApplicationTheme::Dark ? "dark" : "light";
}

void AppearanceChangeListener::OnColorValuesChanged() noexcept {
  auto newTheme = Application::Current().RequestedTheme();
  if (m_currentTheme != newTheme) {
    m_currentTheme = newTheme;

    if (auto reactInstance = m_weakReactInstance.lock()) {
      reactInstance->CallJsFunction(
          "RCTDeviceEventEmitter", "emit", folly::dynamic::array("appearanceChanged", ToString(m_currentTheme)));
    }
  }
}

AppearanceModule::AppearanceModule(Mso::CntPtr<AppearanceChangeListener> &&appearanceListener) noexcept
    : m_changeListener(std::move(appearanceListener)) {}

std::string AppearanceModule::getName() {
  return AppearanceModule::Name;
}

std::vector<Method> AppearanceModule::getMethods() {
  return {Method(
      "getColorScheme", [this](folly::dynamic /*args*/) { return m_changeListener->GetColorScheme(); }, SyncTag)};
}

} // namespace react::uwp

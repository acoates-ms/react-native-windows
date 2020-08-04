import {HealthCheckCategory} from '@react-native-community/cli-types';

import {isWinSdkInstalled, WINDOWS_SDK_DOWNLOAD_URL, WINDOWS_SDK_MISSING_ERROR} from '../runWindows/utils/checkRequirements';

export const healthChecksWindows : HealthCheckCategory[] = process.platform === 'win32' ? [
  {
    label: 'Windows',
    healthchecks: [
      {
        label: 'Windows SDK',
        getDiagnostics: async () => ({
          needsToBeFixed: !isWinSdkInstalled()
        }),
        runAutomaticFix: async ({loader, logManualInstallation}) => {
          // SDK could have been installed by installing Visual Studio so double checking
          if (isWinSdkInstalled()) {
            loader.succeed();
            return;
          }

          loader.fail();

          logManualInstallation({
            message: WINDOWS_SDK_MISSING_ERROR,
            url: WINDOWS_SDK_DOWNLOAD_URL
          });
        },
      }
    ]
  }
] : [];
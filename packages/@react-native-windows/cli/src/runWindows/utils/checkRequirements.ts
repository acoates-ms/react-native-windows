/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */

import {execSync} from 'child_process';
import * as path from 'path';
import * as shell from 'shelljs';
import Version from './version';

const REQUIRED_WINDOWS_SDK_VERSION = new Version(10, 0);

function shortenVersion(version: Version): string {
  return /^(\d+(?:\.\d+)?)/.exec(version.toString())![1];
}

function getInstalledWindowsSdks(): Version[] {
  const installedSdks: Version[] = [];

  const execString =
    'reg query "HKLM\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows" /s /v InstallationFolder /reg:32';
  let output: string;
  try {
    output = execSync(execString).toString();
  } catch (e) {
    return installedSdks;
  }

  const re = /\\Microsoft SDKs\\Windows\\v(\d+\.\d+)\s*InstallationFolder\s+REG_SZ\s+(.*)/gim;
  let match: RegExpExecArray;
  while ((match = re.exec(output)!)) {
    const sdkPath = match[2];
    if (shell.test('-e', path.join(sdkPath, 'SDKManifest.xml'))) {
      const sdkVersion = Version.tryParse(match[1]);
      if (!sdkVersion) {
        throw new Error(`Unexpected SDK version format for '${match[1]}'`);
      }
      installedSdks.push(sdkVersion);
    }
  }

  return installedSdks;
}

export function isWinSdkInstalled(): boolean {
  const installedSdks = getInstalledWindowsSdks();
  return installedSdks.some(installedSdk =>
    installedSdk.eq(REQUIRED_WINDOWS_SDK_VERSION),
  );
}

export const WINDOWS_SDK_MISSING_ERROR = `Windows SDK not found. Ensure that you have installed Windows ${shortenVersion(
  REQUIRED_WINDOWS_SDK_VERSION,
)} SDK along with Visual Studio or install Windows ${shortenVersion(
  REQUIRED_WINDOWS_SDK_VERSION,
)} SDK separately`;

export const WINDOWS_SDK_DOWNLOAD_URL =
  'https://dev.windows.com/en-us/downloads';

export function verifyWinSdkPresent() {
  if (isWinSdkInstalled()) {
    return;
  }

  throw new Error(
    `${WINDOWS_SDK_MISSING_ERROR} from ${WINDOWS_SDK_DOWNLOAD_URL}`,
  );
}

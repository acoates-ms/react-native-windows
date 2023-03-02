/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @flow strict-local
 * @format
 */

import type {ViewProps, BubblingEventHandler} from 'react-native/Libraries/Components/View/ViewPropTypes';
import type {HostComponent} from 'react-native';
import codegenNativeCommands from 'react-native/Libraries/Utilities/codegenNativeCommands';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

type LabelChangeEvent = $ReadOnly<{|
  value: string,
|}>;

type NativeProps = $ReadOnly<{|
  
  // Props
  ...ViewProps,
  label: ?string,

  // Events
  onChange?: ?BubblingEventHandler<LabelChangeEvent>,
|}>;


type ComponentType = HostComponent<NativeProps>;

interface NativeCommands {
  +CustomCommand: (viewRef: React.ElementRef<ComponentType>, value: string) => void;
}

export const Commands: NativeCommands = codegenNativeCommands<NativeCommands>({
  supportedCommands: ['CustomCommand'],
});

export default (codegenNativeComponent<NativeProps>(
   'CustomUserControlCpp',
): ComponentType);
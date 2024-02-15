/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */
import React from 'react';
import { AppRegistry, View } from 'react-native';


import {
  setRuntimeConfigProvider,
  get,
  // @ts-ignore - NativeComponentRegistry is internal and has no types.  Need it for now since we have no viewmanager for this component
} from 'react-native/Libraries/NativeComponent/NativeComponentRegistry';

// Since CustomXamlComponentWithNativeLayout does not have a ViewManager, we need to override the config provider
// to say that CustomXamlComponentWithNativeLayout should get its config from JS, not native
setRuntimeConfigProvider((name: string) => {
  console.log('setRuntimeConfigProvider: ' + name);
  return {
    native: !name.startsWith('RNSVG'),
    strict: false,
    verify: false,
  };
});

import { Svg, Rect } from 'react-native-svg';

export default class Bootstrap extends React.Component {
  render() {
    return (
      <View
        accessible={true}
        style={{ width: 300, height: 300, margin: 10, backgroundColor: 'green' }}>
        <Svg width="200" height="60">
          <Rect
            x="5%"
            y="5%"
            width="90%"
            height="90%"
            fill="rgb(0,0,255)"
            strokeWidth="3"
            stroke="rgb(0,0,0)"
            strokeDasharray="5,10"
          />
        </Svg>
      </View>
    );
  }
}

AppRegistry.registerComponent('Bootstrap', () => Bootstrap);

/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */

import {View} from 'react-native';

import React from 'react';

interface NativeProps {
  label: string;
}

// Cannot just use codegenNativeComponent, or registerNativeComponent, since we need to provide a custom config
const MyCustomComponent = get<NativeProps>('MyCustomComponent', () => {
  return {
    uiViewClassName: 'MyCustomComponent',
    bubblingEventTypes: {},
    directEventTypes: {},
    validAttributes: {
      label: true,
    },
  };
});

class CustomComponentExample extends React.Component<{}, any> {
  constructor(props: {}) {
    super(props);
  }
  public render() {
    return (
      <View
        style={{
          borderRadius: 0,
          margin: 10,
          borderWidth: 2,
          gap: 5,
        }}>
        <MyCustomComponent label="test" style={{width: 500, height: 500}} />
      </View>
    );
  }
}

export const displayName = undefined;
export const title = 'Custom Native Fabric Component';
export const category = 'Basic';
export const description = 'Displays a custom native component.';
export const examples = [
  {
    title: 'Example',
    render: function () {
      return <CustomComponentExample />;
    },
  },
];

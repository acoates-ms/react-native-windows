/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 *
 * @format
 * @flow
 */

'use strict';

const React = require('react');
const Text = require('../Text/Text');
const Pressable = require('./Pressable/Pressable').default;
const {PlatformColor} = require('../StyleSheet/PlatformColorValueTypes');

const invariant = require('invariant');

import type {PressEvent} from '../Types/CoreEventTypes';
import type {ColorValue} from '../StyleSheet/StyleSheet';

type ButtonProps = $ReadOnly<{|
  /**
   * Text to display inside the button
   */
  title: string,

  /**
   * Handler to be called when the user taps the button
   */
  onPress: (event?: PressEvent) => mixed,

  /**
   * If true, doesn't play system sound on touch (Android Only)
   **/
  touchSoundDisabled?: ?boolean,

  /**
   * Color of the text (iOS), or background color of the button (Android)
   */
  color?: ?ColorValue,

  /**
   * TV preferred focus (see documentation for the View component).
   */
  hasTVPreferredFocus?: ?boolean,

  /**
   * TV next focus down (see documentation for the View component).
   *
   * @platform android
   */
  nextFocusDown?: ?number,

  /**
   * TV next focus forward (see documentation for the View component).
   *
   * @platform android
   */
  nextFocusForward?: ?number,

  /**
   * TV next focus left (see documentation for the View component).
   *
   * @platform android
   */
  nextFocusLeft?: ?number,

  /**
   * TV next focus right (see documentation for the View component).
   *
   * @platform android
   */
  nextFocusRight?: ?number,

  /**
   * TV next focus up (see documentation for the View component).
   *
   * @platform android
   */
  nextFocusUp?: ?number,

  /**
   * Text to display for blindness accessibility features
   */
  accessibilityLabel?: ?string,

  /**
   * If true, disable all interactions for this component.
   */
  disabled?: ?boolean,

  /**
   * Used to locate this view in end-to-end tests.
   */
  testID?: ?string,
|}>;

/**
 * A basic button component that should render nicely on any platform. Supports
 * a minimal level of customization.
 *
 * <center><img src="img/buttonExample.png"></img></center>
 *
 * If this button doesn't look right for your app, you can build your own
 * button using [TouchableOpacity](docs/touchableopacity.html)
 * or [TouchableNativeFeedback](docs/touchablenativefeedback.html).
 * For inspiration, look at the [source code for this button component](https://github.com/facebook/react-native/blob/master/Libraries/Components/Button.js).
 * Or, take a look at the [wide variety of button components built by the community](https://js.coach/react-native?search=button).
 *
 * Example usage:
 *
 * ```
 * import { Button } from 'react-native';
 * ...
 *
 * <Button
 *   onPress={onPressLearnMore}
 *   title="Learn More"
 *   color="#841584"
 *   accessibilityLabel="Learn more about this purple button"
 * />
 * ```
 *
 */

class Button extends React.Component<ButtonProps> {
  render(): React.Node {
    const {
      accessibilityLabel,
      color,
      onPress,
      touchSoundDisabled,
      title,
      hasTVPreferredFocus,
      nextFocusDown,
      nextFocusForward,
      nextFocusLeft,
      nextFocusRight,
      nextFocusUp,
      disabled,
      testID,
    } = this.props;
    const accessibilityState = {};
    if (disabled) {
      accessibilityState.disabled = true;
    }
    invariant(
      typeof title === 'string',
      'The title prop of a Button must be a string',
    );
    return (
      <Pressable
        accessibilityLabel={accessibilityLabel}
        accessibilityRole="button"
        accessibilityState={accessibilityState}
        hasTVPreferredFocus={hasTVPreferredFocus}
        nextFocusDown={nextFocusDown}
        nextFocusForward={nextFocusForward}
        nextFocusLeft={nextFocusLeft}
        nextFocusRight={nextFocusRight}
        nextFocusUp={nextFocusUp}
        testID={testID}
        disabled={disabled}
        onPress={onPress}
        touchSoundDisabled={touchSoundDisabled}
        style={({pressed}) => {
          return {
            backgroundColor: disabled
              ? PlatformColor('ButtonBackgroundDisabled')
              : pressed
              ? PlatformColor('ButtonBackgroundPressed')
              : color
              ? color
              : PlatformColor('ButtonBackground'),
            borderColor: disabled
              ? PlatformColor('ButtonBorderBrushDisabled')
              : pressed
              ? PlatformColor('ButtonBorderBrushPressed')
              : PlatformColor('ButtonBorderBrush'),
            borderWidth: 2,
            borderRadius: 0,
          };
        }}>
        {({pressed}) => (
          <Text
            style={{
              textAlign: 'center',
              marginHorizontal: 10, // Calculated from {StaticResource ButtonPadding} + {ThemeResource ButtonBorderThemeThickness}
              marginTop: 6, // Calculated from {StaticResource ButtonPadding} + {ThemeResource ButtonBorderThemeThickness}
              marginBottom: 7, // Calculated from {StaticResource ButtonPadding} + {ThemeResource ButtonBorderThemeThickness}
              fontWeight: '500',
              color: disabled
                ? PlatformColor('ButtonForegroundDisabled')
                : pressed
                ? PlatformColor('ButtonForegroundPressed')
                : PlatformColor('ButtonForeground'),
            }}
            disabled={disabled}>
            {title}
          </Text>
        )}
      </Pressable>
    );
  }
}

module.exports = Button;

# react-native-opacity

Opacity Networks library for React Native.

## Install

```
yarn add @opacity-labs/react-native-opacity
```

## iOS

- You need to bump your minimum deployment target to iOS 14 (Via Xcode → [YOUR TARGET] → General → Deployment Target)
- Do a `pod install` Module is a Turbo Module, therefore you need to enable the new arch. (RCT_ENABLE_NEW_ARCH=1 pod install)

If you are pulling on the latest version cocoapods sometimes refuses to update it's main repo specs. You can force an update via:

```sh
rm -rf ~/.cocoapods/repos/trunk
pod repo update
RCT_ENABLE_NEW_ARCH=1 pod install --repo-update
```

## Android

First add the necessary repos to download the dependencies. On your root `build.gradle` add:

```
allprojects {
  repositories {
    google()
    mavenCentral()
    maven { url "https://maven.mozilla.org/maven2/" }
    maven { url 'https://jitpack.io' }
  }
}
```

On your apps `AndroidManifest.xml` add an activity:

```xml
    // Your other activities, don't nest this tag ↓
      <activity
        android:name="com.opacitylabs.opacitycore.InAppBrowserActivity"
        android:theme="@style/Theme.AppCompat.DayNight"
      />
    // Just above the closing tag of "application"
```

## JS

You need to make sure `react-native.config.js` is properly set up for code generation to work:

```js
module.exports = {
  project: {
    android: {
      packageName: 'your.package.name', // must match your android apps package name, take a look into your apps build.gradle
    },
  },
};
```

Once everything is setup you can call the init method on your JS:

```ts
import {
  init,
  getUberRiderPorfile,
  OpacityEnvironment,
} from '@opacity-labs/react-native-opacity';

init('Your API key', false, OpacityEnvironment.PRODUCTION);

// Later you can call the methods
const res = getUberRiderProfile();
```

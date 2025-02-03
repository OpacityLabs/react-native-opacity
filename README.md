# react-native-opacity

Opacity Networks library for React Native.

## Install

```
yarn add @opacity-labs/react-native-opacity
```

# Expo

If you are using an Expo app, the native configuration changes below have been abstracted into an expo-plugin, just add it to the plugins section in you `app.json`:

```json
  plugins: [
    ...,
    "@opacity-labs/react-native-opacity"
  ]
```

# Bare React Native App

## iOS

You need to bump your minimum deployment target to iOS 14. On the latest versions of RN this should be the default target, if not you can do it via:

```
Xcode → Project Explorer → [YOUR TARGET] → General → Deployment Target
```

Do a `pod install` Module is a Turbo Module, therefore you need to enable the new arch. Again, on the latest RN versions the new architecture should be turned on by default if not run:

```
cd ios && RCT_ENABLE_NEW_ARCH=1 pod install
```

Or modify the top of your podfile and do a pod install:

```ruby
env['RCT_ENABLE_NEW_ARCH'] = '1'
```

If you are pulling on the latest version the package, sometimes cocoapods refuses to update it's main repo specs. You can force an update via:

```sh
rm -rf ~/.cocoapods/repos/trunk
pod repo update
RCT_ENABLE_NEW_ARCH=1 pod install --repo-update
```

## Android

First add the necessary repos to download the dependencies. On your root `android/build.gradle` add:

```groovy
allprojects {
  repositories {
    google()
    mavenCentral()
    // If you already have allprojects->respositories just make sure you have these two added
    maven { url "https://maven.mozilla.org/maven2/" }
    maven { url 'https://jitpack.io' }
  }
}
```

On your apps `AndroidManifest.xml` add an activity:

```xml
      // Put this on the same level as other activities
      <activity
        android:name="com.opacitylabs.opacitycore.InAppBrowserActivity"
        android:theme="@style/Theme.AppCompat.DayNight"
      />
```

# Codegen config

Indenpendtly of Expo or a bare React Native app.l You need to make sure `react-native.config.js` is properly set up for code generation to work:

```js
module.exports = {
  project: {
    android: {
      packageName: 'your.package.name', // must match your android apps package name, take a look into your apps build.gradle
    },
  },
};
```

# API

Once everything is setup you can call the init method on your JS:

```ts
// Create an OpacityInstance.ts file
import {
  init,
  get,
  OpacityEnvironment,
} from '@opacity-labs/react-native-opacity';

try {
  init('Your API key', false, OpacityEnvironment.PRODUCTION);
} catch (e) {
  console.error(`Could not start opacity SDK: ${e}`);
}

export async function getUberRiderProfile() {
  try {
    let profileResponse = await get('flow:uber_rider:profile');
    console.log(profileResponse);
  } catch (e) {
    console.error(`Could not get profile ${e}`);
  }
}
```

Then somewhere on your app you can just call the functions:

```tsx
// App.tsx
import { getUberRiderProfile } from './OpacityInstance';

return <Button onPress={getUberRiderProfile} title="Get Uber Rider Profile" />;
```

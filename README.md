# react-native-opacity

Opacity Networks library for React Native. Only new architecture is supported.

## Install

```
yarn add @opacity-labs/react-native-opacity
```

# Expo

You can automatically configure your app via CNG, add to `app.json`:

```json
{
  ...
  "plugins": [
    ...,
    "@opacity-labs/react-native-opacity"
  ]
}
```

# Bare React Native App

## iOS

Do a pod install after installing the library

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

Independently of Expo or a bare React Native app. You need to make sure `react-native.config.js` is properly set up for code generation to work:

```js
module.exports = {
  project: {
    android: {
      packageName: 'your.package.name', // must match your android apps package name
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
  let { response, error } = await get('flow:uber_rider:profile');
  if (error) {
    // handle the error returned by the library
    console.log(error.code, error.message);
  } else {
    console.log(response);
  }
}
```

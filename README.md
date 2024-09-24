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

You need to modify a bunch of stuff for Android.

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
      <activity
        android:name="com.opacitylabs.opacitycore.InAppBrowserActivity"
        android:theme="@style/Theme.AppCompat.DayNight" />
    // Just above the closing tag of "application"
    </application>
```

On your main activity you need to initialize the library (kotlin snippet):

```kotlin
import android.os.Bundle // add this import
import com.facebook.react.ReactActivity
import com.facebook.react.ReactActivityDelegate
import com.facebook.react.defaults.DefaultNewArchitectureEntryPoint.fabricEnabled
import com.facebook.react.defaults.DefaultReactActivityDelegate
import com.opacitylabs.opacitycore.OpacityCore // add this import

class MainActivity : ReactActivity() {
  override fun onCreate(savedInstanceState: Bundle?) { // add this method
    super.onCreate(savedInstanceState)
    OpacityCore.initialize(this)
  }
  // ...
}
```

## JS

You need to make sure `react-native.config.js` is properly set up for code generation to work:

```js
module.exports = {
  project: {
    android: {
      packageName: 'your.package.name', // must match your android apps package name, take a look into build.gradle
    },
  },
};
```

Once everything is setup you can call the init method on your JS:

```ts
import { init, getUberRiderPorfile } from '@opacity-labs/react-native-opacity';

// ..
useEffect(() => {
  init('Your API key', false);
}, []);
```

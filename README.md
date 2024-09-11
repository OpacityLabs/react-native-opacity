# react-native-opacity

Opacity Networks library for React Native.

## Install

```
yarn add @opacity-labs/react-native-opacity
```

- You need to bump your minimum deployment target to iOS 14 (Via Xcode → [YOUR TARGET] → General → Deployment Target)
- Do a `pod install` Module is a Turbo Module, therefore you need to enable the new arch. (RCT_ENABLE_NEW_ARCH=1 pod install)

If you are pulling on the latest version cocoapods sometimes refuses to update it's main repo specs. You can force an update via:

```sh
rm -rf ~/.cocoapods/repos/trunk
pod repo update
RCT_ENABLE_NEW_ARCH=1 pod install
```

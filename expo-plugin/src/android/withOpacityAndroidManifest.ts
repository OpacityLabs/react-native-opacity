import { type ConfigPlugin, withAndroidManifest } from '@expo/config-plugins';

export const withOpacityAndroidManifest: ConfigPlugin = (config) => {
  return withAndroidManifest(config, (config) => {
    let androidManifest = config.modResults.manifest;

    if (!androidManifest) {
      throw new Error(' No AndroidManifest found, cannot apply opacity plugin');
    }

    // Add the tools to apply permission remove
    // androidManifest.$ = {
    //   ...androidManifest.$,
    //   'xmlns:tools': 'http://schemas.android.com/tools',
    // };

    let application = androidManifest.application![0];

    if (!application) {
      throw new Error('No base application found, cannot apply opacity plugin');
    }

    application.activity = application.activity!.filter(
      (act) =>
        act.$['android:name'] !=
        'com.opacitylabs.opacitycore.InAppBrowserActivity'
    );

    application.activity.push({
      $: {
        'android:name': 'com.opacitylabs.opacitycore.InAppBrowserActivity',
        'android:theme': '@style/Theme.AppCompat.DayNight',
      },
    });

    return config;
  });
};

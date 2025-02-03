const {
  withAndroidManifest,
  withProjectBuildGradle,
} = require('@expo/config-plugins');

module.exports = function withOpacity(config) {
  config = withAndroidManifest(config, (config) => {
    let androidManifest = config.modResults.manifest;

    // Add the tools to apply permission remove
    androidManifest.$ = {
      ...androidManifest.$,
      'xmlns:tools': 'http://schemas.android.com/tools',
    };

    androidManifest.application[0].activity =
      androidManifest.application[0].activity.filter(
        (act) =>
          act.$['android:name'] !=
          'com.opacitylabs.opacitycore.InAppBrowserActivity'
      );

    androidManifest.application[0].activity.push({
      $: {
        'android:name': 'com.opacitylabs.opacitycore.InAppBrowserActivity',
        'android:theme': '@style/Theme.AppCompat.DayNight',
      },
    });

    return config;
  });

  config = withProjectBuildGradle(config, (config) => {
    if (!config.modResults.contents.match('maven.mozilla.org')) {
      config.modResults.contents = config.modResults.contents.replace(
        /mavenCentral\(\)/g,
        `
        mavenCentral()
        maven { url "https://maven.mozilla.org/maven2/" }`
      );
    }
    return config;
  });

  return config;
};

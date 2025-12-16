import {
  type ConfigPlugin,
  withProjectBuildGradle,
} from '@expo/config-plugins';

export const withOpacityProjectBuildGradle: ConfigPlugin = (config) => {
  return withProjectBuildGradle(config, async (config) => {
    let { contents } = config.modResults;

    // Add Mozilla maven repository if not present
    if (!contents.match('maven.mozilla.org')) {
      contents = contents.replace(
        /mavenCentral\(\)/g,
        `mavenCentral()
        maven { url "https://maven.mozilla.org/maven2/" }`
      );
    }

    // Add jitpack repository if not present
    if (!contents.match('jitpack.io')) {
      contents = contents.replace(
        /mavenCentral\(\)/g,
        `mavenCentral()
        maven { url 'https://jitpack.io' }`
      );
    }

    config.modResults.contents = contents;
    return config;
  });
};

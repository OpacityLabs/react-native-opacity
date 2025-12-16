import {
  type ConfigPlugin,
  withProjectBuildGradle,
} from '@expo/config-plugins';

export const withOpacityProjectBuildGradle: ConfigPlugin = (config) => {
  return withProjectBuildGradle(config, async (config) => {
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
};

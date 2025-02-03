import { type ConfigPlugin, withPlugins } from '@expo/config-plugins';
import {
  withOpacityAndroidManifest,
  withOpacityProjectBuildGradle,
} from './android';

const withOpacity: ConfigPlugin = (config) => {
  return withPlugins(config, [
    withOpacityAndroidManifest,
    withOpacityProjectBuildGradle,
  ]);
};

export default withOpacity;

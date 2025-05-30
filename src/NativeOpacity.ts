import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  init(
    apiKey: string,
    dryRun: boolean,
    environment: number,
    shouldShowErrorsInWebView: boolean
  ): Promise<void>;
  getInternal(name: string, params?: Object): Promise<Object>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('OpacityModule');

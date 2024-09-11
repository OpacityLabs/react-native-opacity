import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  init(apiKey: string, dryRun: boolean): void;
  getRiderProfile(): Promise<string>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Opacity');

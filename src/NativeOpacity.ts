import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  init(apiKey: string, dryRun: boolean, environment: number): Promise<void>;
  getInternal(
    name: string,
    params: string | null
  ): Promise<{ data: string; proof?: string }>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('OpacityModule');

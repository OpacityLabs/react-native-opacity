import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  init(
    apiKey: string,
    dryRun: boolean,
    environment: number,
    shouldShowErrorsInWebView: boolean
  ): Promise<void>;
  initializeOpenTelemetry(
    openTelemetryEndpoint: string,
    grafanaInstanceId: string,
    grafanaApiToken: string
  ): Promise<void>;
  getInternal(name: string, params?: Object): Promise<Object>;
  // `traceparent` is required, so it precedes the optional args rather than
  // mirroring the public `getWithContext` argument order.
  getInternalWithContext(
    name: string,
    traceparent: string,
    params?: Object,
    tracestate?: string
  ): Promise<Object>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('OpacityModule');

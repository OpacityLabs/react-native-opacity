import Opacity from './NativeOpacity';

type WorkflowResponse = {
  data: string;
  proof?: string;
};

export enum OpacityEnvironment {
  Test = 0,
  Local = 1,
  Staging = 2,
  Production = 3,
}

export function init(
  apiKey: string,
  dryRun: boolean,
  environment: OpacityEnvironment
): Promise<void> {
  return Opacity.init(apiKey, dryRun, environment);
}

export function get(
  name: string,
  params?: Record<string, any>
): Promise<WorkflowResponse> {
  return Opacity.getInternal(name, params);
}

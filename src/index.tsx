import Opacity from './NativeOpacity';

type WorkflowResponse = {
  json: Record<string, any>;
  signature?: string;
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

export async function get(
  name: string,
  params?: Record<string, any>
): Promise<WorkflowResponse> {
  let res = await Opacity.getInternal(name, params);
  return res as WorkflowResponse;
}

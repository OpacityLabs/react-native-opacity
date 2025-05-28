import Opacity from './NativeOpacity';

type WorkflowResponse = {
  json: Record<string, any>;
  signature?: string;
  proof?: string;
};

export enum OpacityEnvironment {
  Test = 0,
  Local = 1,
  Sandbox = 2,
  Staging = 3,
  Production = 4,
}

export async function init({
  apiKey,
  dryRun = false,
  environment,
  shouldShowErrorsInWebView = true,
}: {
  apiKey: string;
  dryRun?: boolean;
  environment: OpacityEnvironment;
  shouldShowErrorsInWebView?: boolean;
}): Promise<void> {
  try {
    let res = await Opacity.init(
      apiKey,
      dryRun,
      environment,
      shouldShowErrorsInWebView
    );
    return res;
  } catch (e) {
    console.error('Could not initialize OpacitySDK', e);
  }
}

export async function get(
  name: string,
  params?: Record<string, any>
): Promise<WorkflowResponse> {
  let res = await Opacity.getInternal(name, params);
  return res as WorkflowResponse;
}

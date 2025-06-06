import Opacity from './NativeOpacity';

type WorkflowResponse = {
  json: Record<string, any>;
  signature?: string;
  hash?: string;
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

/**
 * Main function to get a flow response from Opacity.
 *
 * Each flow might define its own parameters, so check the flow documentation for available parameters.
 *
 * **WARNING** The response is dynamic and the return type is not strictly defined, it's mostly there as a convenience.
 *
 * Therefore, be sure to check for the existence of the properties you expect in the response.
 *
 * @param name name of the flow to execute e.g. "github:profile"
 * @param params optional parameters to pass to the flow, check the flow documentation for available parameters
 * @returns WorkflowResponse containing the JSON response from the flow, and optionally a signature and hash
 */
export async function get(
  name: string,
  params?: Record<string, any>
): Promise<WorkflowResponse> {
  let res = await Opacity.getInternal(name, params);
  return res as WorkflowResponse;
}

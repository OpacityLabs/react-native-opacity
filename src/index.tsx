import Opacity from './NativeOpacity';

export class OpacityError extends Error {
  code: string;

  constructor(code: string, message: string) {
    super(message);
    this.name = 'OpacityError';
    this.code = code;

    Object.setPrototypeOf(this, new.target.prototype);
  }

  toString() {
    return `${this.name}: [${this.code}] ${this.message}`;
  }

  toJSON() {
    return {
      name: this.name,
      code: this.code,
      message: this.message,
    };
  }
}

type WorkflowResponse = {
  payload: any;
  message: string;
  signature: string;
  hash: string;
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
): Promise<{ response?: WorkflowResponse; error?: OpacityError }> {
  try {
    let res = await Opacity.getInternal(name, params);
    return {
      response: res as WorkflowResponse,
    };
  } catch (e: any) {
    // this error should be a RN error that has a non-standard "code" property that we use to describe the error codes
    if (e.code) {
      return {
        error: new OpacityError(e.code, e.message),
      };
    } else {
      return {
        error: new OpacityError('UnknownError', e.message),
      };
    }
  }
}

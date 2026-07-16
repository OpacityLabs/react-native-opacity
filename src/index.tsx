import NativeOpacity from './NativeOpacity';

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
    let res = await NativeOpacity.init(
      apiKey,
      dryRun,
      environment,
      shouldShowErrorsInWebView
    );
    return res;
  } catch (e) {
    console.error('Could not initialize OpacitySDK', e);
    throw e;
  }
}

export async function initializeOpenTelemetry({
  openTelemetryEndpoint,
  grafanaInstanceId,
  grafanaApiToken,
}: {
  openTelemetryEndpoint: string;
  grafanaInstanceId: string;
  grafanaApiToken: string;
}): Promise<void> {
  try {
    let res = await NativeOpacity.initializeOpenTelemetry(
      openTelemetryEndpoint,
      grafanaInstanceId,
      grafanaApiToken
    );
    return res;
  } catch (e) {
    console.error('Could not initialize OpenTelemetry', e);
    throw e;
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
  return toResult(() => NativeOpacity.getInternal(name, params));
}

/**
 * Same as {@link get}, but joins the flow to the caller's W3C trace: the flow's
 * root span is parented to `traceparent` instead of starting a standalone trace.
 *
 * Use this when the flow is one step of a larger operation you're already
 * tracing, so the flow's spans show up under that trace rather than detached.
 *
 * @param name name of the flow to execute e.g. "github:profile"
 * @param params optional parameters to pass to the flow, check the flow documentation for available parameters
 * @param traceparent a valid W3C traceparent to parent the flow's root span to
 * @param tracestate optional W3C tracestate; W3C allows it to be absent
 * @returns WorkflowResponse containing the JSON response from the flow, and optionally a signature and hash
 */
export async function getWithContext(
  name: string,
  params: Record<string, any> | undefined,
  traceparent: string,
  tracestate?: string
): Promise<{ response?: WorkflowResponse; error?: OpacityError }> {
  return toResult(() =>
    NativeOpacity.getInternalWithContext(name, traceparent, params, tracestate)
  );
}

async function toResult(
  call: () => Promise<Object>
): Promise<{ response?: WorkflowResponse; error?: OpacityError }> {
  try {
    let res = await call();
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

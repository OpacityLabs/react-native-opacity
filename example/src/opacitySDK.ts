import {
  init,
  initializeOpenTelemetry,
  OpacityEnvironment,
} from '@opacity-labs/react-native-opacity';
import Toast from 'react-native-toast-message';

export const opacityEnvironment = OpacityEnvironment.Staging as OpacityEnvironment;
let apiKey = process.env.OPACITY_API_KEY;

if (!apiKey) {
  console.error('API key not found');
}

// react-native-dotenv inlines `process.env.X` at build time, so each var has to
// be read as a literal member access -- destructuring or dynamic keys resolve to
// undefined.
const otelEndpoint = process.env.OTEL_ENDPOINT;
const otelInstanceId = process.env.OTEL_GRAFANA_INSTANCE_ID;
const otelApiToken = process.env.OTEL_GRAFANA_API_TOKEN;

// OpenTelemetry is optional: with no OTEL_* vars in .env, spans are never
// exported and flows run exactly as before. Must run AFTER init.
const initializeOpenTelemetryIfConfigured = async () => {
  if (!otelEndpoint || !otelInstanceId || !otelApiToken) {
    console.log('[otel] OTEL_* vars not set in .env — skipping OpenTelemetry init');
    return;
  }

  try {
    await initializeOpenTelemetry({
      openTelemetryEndpoint: otelEndpoint,
      grafanaInstanceId: otelInstanceId,
      grafanaApiToken: otelApiToken,
    });
    console.log(`[otel] OpenTelemetry initialized -> ${otelEndpoint}`);
    Toast.show({
      type: 'success',
      text1: 'OpenTelemetry initialized!',
    });
  } catch (e) {
    // Non-fatal: tracing is a diagnostic, so a bad endpoint or token must not
    // stop the example app from running flows.
    console.error('[otel] Could not initialize OpenTelemetry', e);
    Toast.show({
      type: 'error',
      text1: 'Error initializing OpenTelemetry',
      text2: `${e}`,
    });
  }
};

export const initializeSDK = async () => {
  try {
    await init({
      apiKey: process.env.OPACITY_API_KEY!,
      environment: opacityEnvironment,
      shouldShowErrorsInWebView: false,
    });
    Toast.show({
      type: 'success',
      text1: 'SDK Initialized!',
    });
  } catch (e) {
    Toast.show({
      type: 'error',
      text1: 'Error initializing SDK',
      text2: `${e}`,
    });
    return;
  }

  await initializeOpenTelemetryIfConfigured();
};

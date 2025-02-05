import { init, OpacityEnvironment } from '@opacity-labs/react-native-opacity';

export const opacityEnvironment = OpacityEnvironment.Test as OpacityEnvironment;
let apiKey = process.env.OPACITY_API_KEY;

if (!apiKey) {
  console.error('API key not found');
}

init({
  apiKey: process.env.OPACITY_API_KEY!,
  environment: opacityEnvironment,
  shouldShowErrorsInWebView: false,
}).catch((error) => {
  console.error(`FAILED TO INITIALIZE SDK: ${error}`);
});

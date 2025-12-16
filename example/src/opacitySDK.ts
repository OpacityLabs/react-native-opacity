import { init, OpacityEnvironment } from '@opacity-labs/react-native-opacity';
import Toast from 'react-native-toast-message';

export const opacityEnvironment = OpacityEnvironment.Production as OpacityEnvironment;
let apiKey = process.env.OPACITY_API_KEY;

if (!apiKey) {
  console.error('API key not found');
}

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
  }
};

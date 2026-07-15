import { get } from '@opacity-labs/react-native-opacity';
import {
  Button,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  TextInput,
} from 'react-native';
import { initializeSDK, opacityEnvironment } from './opacitySDK';
import { envToString } from './utils';
import { useEffect, useState } from 'react';
import Toast from 'react-native-toast-message';

const TEST_BUTTONS: Array<[string, string]> = [
  ['uber_rider:profile', 'uber_rider:profile'],
  ['test:open_browser_must_succeed', 'test:open_browser_must_succeed'],
  ['404 flow', '404'],
];

export default function App() {
  const [inputFlow, setInputFlow] = useState('');
  const [inputParams, setInputParams] = useState('');

  useEffect(() => {
    initializeSDK();
  }, []);

  const runFlow = async (name: string, params?: Record<string, any>) => {
    const { response, error } = await get(name.toLowerCase(), params);
    if (error) {
      console.error(error.code, error.message);
      Toast.show({
        type: 'error',
        text1: `${error.code} - ${error.message}`,
      });
    } else {
      console.log('got res: ', response);
      Toast.show({
        type: 'success',
        text1: 'Success',
      });
    }
  };

  const submitCustomFlow = async () => {
    if (!inputFlow) {
      Toast.show({ type: 'error', text1: 'Please enter a flow name' });
      return;
    }

    const trimmed = inputParams.trim();
    let params: Record<string, any> | undefined;
    if (trimmed.length > 0) {
      try {
        params = JSON.parse(trimmed);
      } catch (e) {
        Toast.show({ type: 'error', text1: 'Invalid JSON' });
        return;
      }
    }

    await runFlow(inputFlow, params);
  };

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <StatusBar barStyle={"light-content"} />
      <Text style={styles.text}>
        Env: {envToString(opacityEnvironment).toUpperCase()}
      </Text>

      {TEST_BUTTONS.map(([title, flowName]) => (
        <Button key={flowName} title={title} onPress={() => runFlow(flowName)} />
      ))}
      <Button title="re-initialize SDK" onPress={initializeSDK} />

      <TextInput
        style={styles.input}
        placeholder="Enter flow name"
        placeholderTextColor="#888"
        value={inputFlow}
        autoCapitalize="none"
        autoCorrect={false}
        onChangeText={setInputFlow}
      />
      <TextInput
        style={styles.input}
        placeholder="Enter params as a json string"
        placeholderTextColor="#888"
        value={inputParams}
        autoCapitalize="none"
        autoCorrect={false}
        onChangeText={setInputParams}
      />
      <Button title="Submit" onPress={submitCustomFlow} />
      <Toast />
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flexGrow: 1,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#333',
    gap: 12,
    paddingVertical: 40,
  },
  text: {
    color: 'white',
  },
  input: {
    height: 40,
    width: 300,
    margin: 12,
    borderWidth: 1,
    padding: 10,
    color: 'white',
    backgroundColor: 'black',
    borderRadius: 10,
  },
});

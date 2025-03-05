import { get } from '@opacity-labs/react-native-opacity';
import { Button, StyleSheet, Text, TextInput, View } from 'react-native';
import { initializeSDK, opacityEnvironment } from './opacitySDK';
import { envToString } from './utils';
import { useEffect, useState } from 'react';
import Toast from 'react-native-toast-message';

export default function App() {
  const [inputFlow, setInputFlow] = useState('');

  useEffect(() => {
    initializeSDK();
  }, []);

  const runLuaFlow = async () => {
    try {
      const res = await get(inputFlow.toLowerCase());
      console.log('got res: ', res);
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.text}>
        Env: {envToString(opacityEnvironment).toUpperCase()}
      </Text>

      <TextInput
        style={styles.input}
        placeholder="Enter flow input"
        placeholderTextColor="#888"
        value={inputFlow}
        autoFocus
        autoCapitalize="none"
        onChangeText={setInputFlow}
      />
      <Button title="Run Lua flow" onPress={runLuaFlow} />
      <Button title="Re-initiate SDK" onPress={initializeSDK} />
      <Toast />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#333',
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

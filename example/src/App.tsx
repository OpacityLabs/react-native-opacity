import { get } from '@opacity-labs/react-native-opacity';
import { Button, StyleSheet, Text, View } from 'react-native';
import { opacityEnvironment } from './opacitySDK';
import { envToString } from './utils';

export default function App() {
  const getUberRiderProfileCallback = async () => {
    try {
      const riderProfile = await get('flow:uber_rider:profile');
      console.log(riderProfile);
    } catch (error) {
      console.error(error);
    }
  };

  const getGithubProfileCallback = async () => {
    try {
      const githubProfile = await get('github:read:profile');
      console.log(githubProfile);
    } catch (error) {
      console.error(error);
    }
  };

  const runFalingLua = async () => {
    try {
      await get('Quack');
    } catch (e) {
      console.error(e);
    }
  };

  const runSampleLuaFlow = async () => {
    try {
      const res = await get('sha256');
      console.warn('got res: ', res);
    } catch (e) {
      console.error(e);
    }
  };

  const runGenerateSignature = async () => {
    try {
      const res = await get('generate_proof');
      console.log('got res: ', res);
    } catch (e) {
      console.error(e);
    }
  };

  const runLuaTerminate = async () => {
    try {
      const res = await get('terminate');
      console.log('got res: ', res);
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.text}>
        Running against environment: {envToString(opacityEnvironment)}
      </Text>
      <Button
        title="Get uber rider profile"
        onPress={getUberRiderProfileCallback}
      />
      <Button title="Get github profile" onPress={getGithubProfileCallback} />
      <Button title="Failing lua" onPress={runFalingLua} />
      <Button title="Sample Lua flow" onPress={runSampleLuaFlow} />
      <Button title="Generate signature" onPress={runGenerateSignature} />
      <Button title="Terminate" onPress={runLuaTerminate} />
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
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
});

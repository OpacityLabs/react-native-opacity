import { useEffect } from 'react';
import { StyleSheet, View, Text, Button } from 'react-native';
import {
  init,
  OpacityEnvironment,
  get,
} from '@opacity-labs/react-native-opacity';

let env = OpacityEnvironment.Test as OpacityEnvironment;
let envString = 'Test';

switch (env) {
  case OpacityEnvironment.Test:
    envString = 'Test';
    break;
  case OpacityEnvironment.Local:
    envString = 'Local';
    break;
  case OpacityEnvironment.Staging:
    envString = 'Staging';
    break;
  case OpacityEnvironment.Production:
    envString = 'Production';
    break;
  default:
    envString = 'Unknown';
    break;
}

export default function App() {
  useEffect(() => {
    let apiKey = process.env.OPACITY_API_KEY;
    if (!apiKey) {
      console.error('API key not found');
      return;
    }

    init(process.env.OPACITY_API_KEY!, false, env).catch((error) => {
      console.error(`FAILED TO INITIALIZE SDK: ${error}`);
    });
  }, []);

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

  return (
    <View style={styles.container}>
      <Text>Opacity RN app</Text>
      <Text>Running against environment: {envString}</Text>
      <Button
        title="Get uber rider profile"
        onPress={getUberRiderProfileCallback}
      />
      <Button title="Get github profile" onPress={getGithubProfileCallback} />
      <Button title="Failing lua" onPress={runFalingLua} />
      <Button title="Sample Lua flow" onPress={runSampleLuaFlow} />
      <Button title="Generate signature" onPress={runGenerateSignature} />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
});

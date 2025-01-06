import { useEffect } from 'react';
import { StyleSheet, View, Text, Button } from 'react-native';
import {
  init,
  OpacityEnvironment,
  getGustoMembersTable,
  getResource,
  get,
} from '@opacity-labs/react-native-opacity';

let env = OpacityEnvironment.Production as OpacityEnvironment;
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

  const getGustoMemberTable = async () => {
    try {
      const gustoMemberTable = await getGustoMembersTable();
      console.log(gustoMemberTable);
    } catch (error) {
      console.error(error);
    }
  };

  const getGithubProfileCallback = async () => {
    try {
      const githubProfile = await getResource('github:read:profile');
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

  return (
    <View style={styles.container}>
      <Text>Opacity RN app</Text>
      <Text>Running against environment: {envString}</Text>
      <Button
        title="Get uber rider profile"
        onPress={getUberRiderProfileCallback}
      />
      <Button
        title="Get gusto member table account"
        onPress={getGustoMemberTable}
      />
      <Button title="Get github profile" onPress={getGithubProfileCallback} />
      <Button title="Failing lua" onPress={runFalingLua} />
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

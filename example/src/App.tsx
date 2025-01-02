import { useEffect } from 'react';
import { StyleSheet, View, Text, Button } from 'react-native';
import {
  getUberRiderProfile,
  init,
  OpacityEnvironment,
  getGustoMembersTable,
  getResource,
  get,
} from '@opacity-labs/react-native-opacity';

export default function App() {
  useEffect(() => {
    let apiKey = process.env.OPACITY_API_KEY;
    if (!apiKey) {
      console.error('API key not found');
      return;
    }

    init(process.env.OPACITY_API_KEY!, false, OpacityEnvironment.Staging).catch(
      (error) => {
        console.error(`FAILED TO INITIALIZE SDK: ${error}`);
      }
    );
  }, []);

  const getUberRiderProfileCallback = async () => {
    try {
      const riderProfile = await getUberRiderProfile();
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

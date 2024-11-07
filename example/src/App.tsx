import { useEffect } from 'react';
import { StyleSheet, View, Text, Button } from 'react-native';
import {
  getUberRiderProfile,
  init,
  OpacityEnvironment,
} from '@opacity-labs/react-native-opacity';

export default function App() {
  useEffect(() => {
    console.log(
      'Initializing Opacity SDK with key: ',
      process.env.OPACITY_API_KEY
    );
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

  return (
    <View style={styles.container}>
      <Text>Opacity RN app</Text>
      <Button
        title="Get uber rider profile"
        onPress={getUberRiderProfileCallback}
      />
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

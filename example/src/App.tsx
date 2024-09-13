import { useEffect } from 'react';
import { StyleSheet, View, Text, Button } from 'react-native';
import { getUberRiderPorfile, init } from '@opacity-labs/react-native-opacity';

export default function App() {
  useEffect(() => {
    init(process.env.OPACITY_API_KEY!, false).catch((error) => {
      console.error(`FAILED TO INITIALIZE SDK: ${error}`);
    });
  }, []);

  const getUberRiderProfile = async () => {
    try {
      const riderProfile = await getUberRiderPorfile();
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
        onPress={getUberRiderProfile}
      ></Button>
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

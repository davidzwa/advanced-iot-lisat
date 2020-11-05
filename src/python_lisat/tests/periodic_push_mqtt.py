import paho.mqtt.client as mqtt
import time
import atexit
from enum import IntEnum


MQTT_CLIENT_NAME = "pc-mqtt-docker"
BROKER = "192.168.1.10"
PORT = 1883


def on_connect(client, userdata, flags, rc):
    print("PC Connected and powered on.")


def on_disconnect(client, userdata, rc):
    print("Disconnect, reason: " + str(rc))
    print("Disconnect, reason: " + str(client))


def start(client: mqtt.Client):
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    print("about to connect on: " + BROKER)
    client.connect(BROKER, PORT)
    count = 0
    while True:
        time.sleep(0.1)
        client.publish("MSP", "MSP!123{}".format(count))
        count +=1
        client.loop()
    # client.loop_forever()


if __name__ == "__main__":
    print("Starting mqtt client " + MQTT_CLIENT_NAME)
    client = mqtt.Client(MQTT_CLIENT_NAME)
    client.username_pw_set("david", "Davido12")

    # Starting daemon
    start(client)

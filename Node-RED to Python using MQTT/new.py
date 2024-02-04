import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe("/aAa")
        client.subscribe("/bBb")
        client.subscribe("/cCc")
        client.subscribe("/dDd")
       
    else:
        print("Failed to connect to MQTT broker with result code "+str(rc))

def on_message(client, userdata, msg):
    command = msg.payload.decode("utf-8")
    print("Received command:", command)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    client.connect("test.mosquitto.org", 1883, 60)
    client.loop_start()

except Exception as e:
    print("Error connecting to MQTT broker:", str(e))

while True:
    pass

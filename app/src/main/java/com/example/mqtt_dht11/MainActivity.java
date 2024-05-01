package com.example.mqtt_dht11;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.UnsupportedEncodingException;

public class MainActivity extends AppCompatActivity {

    protected MqttAndroidClient client;
    protected MqttConnectOptions options;
    static String MQTTHOST="tcp://robotics.cloud.shiftr.io:1883";
    static String USERNAME="robotics";
    static String PASSWORD="Robot123";

    TextView textnd,textda;
    Switch swtled,swtfan;
    ImageView led;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //Setup Mqtt
        String clientID=MqttClient.generateClientId();
        client=new MqttAndroidClient(this.getApplicationContext(),MQTTHOST,clientID);

        options= new MqttConnectOptions();
        options.setUserName(USERNAME);
        options.setPassword(PASSWORD.toCharArray());
//        //Xu li anh xa
        swtled = findViewById(R.id.swled);
        swtfan = findViewById(R.id.swfan);
        textnd = findViewById(R.id.txtnd);
        textda = findViewById(R.id.txtda);
        led = findViewById(R.id.imgled);


        //Ket noi MQTT
        try{
            IMqttToken token=client.connect(options);
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Toast.makeText(MainActivity.this,"connected",Toast.LENGTH_LONG).show();
                    SUB(client,"esp32/LED");
                    SUB(client,"esp32/FAN");
                    SUB(client,"esp32/temperature");
                    SUB(client,"esp32/humidity");
                }


                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Toast.makeText(MainActivity.this,"connected failed",Toast.LENGTH_LONG).show();
                }
            });
        }catch (MqttException e){
            e.printStackTrace();
        }

        swtled.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(swtled.isChecked()){
                    sendDataToMqtt("esp32/LED","ON");
                }
                else{
                    sendDataToMqtt("esp32/LED","OFF");
                }
            }
        });

        swtfan.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(swtfan.isChecked()){
                    sendDataToMqtt("esp32/FAN","ON");
                }
                else{
                    sendDataToMqtt("esp32/FAN","OFF");
                }
            }
        });

        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {

            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if(topic.equals("esp32/LED")){
                    String mydata = message.toString();
                    if(mydata.equals("ON")){
                        led.setBackgroundResource(R.drawable.ledon);
                        swtled.setChecked(true);
                    }
                    else if(mydata.equals("OFF")){
                        led.setBackgroundResource(R.drawable.ledoff);
                        swtled.setChecked(false);
                    }
                }
                if(topic.equals("esp32/FAN")){
                    String mydata = message.toString();
                    if(mydata.equals("ON")){
                        swtfan.setChecked(true);

                    }
                    else if(mydata.equals("OFF")){
                        swtfan.setChecked(false);

                    }
                }
                if(topic.equals("esp32/temperature")){
                    String mydata = message.toString();
                    textnd.setText(mydata + "°C");

                }
                if(topic.equals("esp32/humidity")){
                    String mydata = message.toString();
                    textda.setText(mydata + "%");
                }

            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });
    }//End oncreate

    public void sendDataToMqtt(String topic, String payload) {
        byte[] encodedPayload = new byte[0];

        try {
            encodedPayload = payload.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            client.publish(topic, message);

        } catch (UnsupportedEncodingException | MqttException e) {
            e.printStackTrace();
        }
    }

    public void SUB(MqttAndroidClient client,String topic){
        int qos =1 ;
        try{
            IMqttToken subToken=client.subscribe(topic,qos);
            subToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {

                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {

                }
            });
        }catch (MqttException e){
            e.printStackTrace();
        }
    }

}
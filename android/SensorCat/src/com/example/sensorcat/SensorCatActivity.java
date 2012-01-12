package com.example.sensorcat;

import java.util.List;
import java.util.Iterator;

import android.app.Activity;
import android.os.Bundle;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.widget.TextView;
import android.content.Context;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;

public class SensorCatActivity extends Activity implements SensorEventListener
{
    private static final String TAG = "SensorCat";
    private SensorManager mSensorManager;
    private Sensor mLinear;
    private TextView mLinearTV;

    private String sensor_type( int type ) {
        switch (type) {
        case Sensor.TYPE_ACCELEROMETER:
            return "Accelerometer";
        case Sensor.TYPE_GRAVITY:
            return "Gravity";
        case Sensor.TYPE_GYROSCOPE:
            return "Gyroscope";
        case Sensor.TYPE_LIGHT:
            return "Light";
        case Sensor.TYPE_LINEAR_ACCELERATION:
            return "Linear Acceleration";
        case Sensor.TYPE_MAGNETIC_FIELD:
            return "Magnetic Field";
        case Sensor.TYPE_ORIENTATION:
            return "Orientation";
        case Sensor.TYPE_PRESSURE:
            return "Pressure";
        case Sensor.TYPE_PROXIMITY:
            return "Proximity";
        case Sensor.TYPE_ROTATION_VECTOR:
            return "Rotation Vector";
        case Sensor.TYPE_TEMPERATURE:
            return "Temperature";
        }
        return "Unknown";
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);

        // List out the available sensors
        List<Sensor> deviceSensors = mSensorManager.getSensorList(Sensor.TYPE_ALL);

        TextView tv = (TextView) findViewById(R.id.TextView);
        mLinearTV = (TextView) findViewById(R.id.LinearAccel);
        Log.v(TAG, "tv = " + tv );
        tv.setText("");

        // Write text on the sensors we have
        for ( Iterator<Sensor> it = deviceSensors.iterator();
              it.hasNext(); ) {
            Sensor s = it.next();
            tv.append("Found [" + sensor_type(s.getType()) + "]\n");
            tv.append("  Name  = " + s.getName() + "\n");
            tv.append("  Power = " + s.getPower() + "\n");
            tv.append("  Res   = " + s.getResolution() + "\n");
            tv.append("  Vers  = " + s.getVersion() + "\n");
        }

        // Go ahead and store the proximity sensor
        mLinear = mSensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION);
    }

    @Override
    public final void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Don't care
    }

    @Override
    public final void onSensorChanged(SensorEvent event) {
        mLinearTV.setText( String.format("%.3f %.3f %.3f",
                                         event.values[0],
                                         event.values[1],
                                         event.values[2] ) );
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.v(TAG, "Register listener for sensor");
        mSensorManager.registerListener(this, mLinear,
                                        SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.v(TAG, "Unregistered listener for sensor");
        mSensorManager.unregisterListener(this);
    }
}

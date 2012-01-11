package com.example.sensorcat;

import java.util.List;
import java.util.Iterator;

import android.app.Activity;
import android.os.Bundle;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.widget.TextView;
import android.content.Context;

public class SensorCatActivity extends Activity
{
    private SensorManager mSensorManager;

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
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);

        // List out the available sensors
        List<Sensor> deviceSensors = mSensorManager.getSensorList(Sensor.TYPE_ALL);

        TextView tv = new TextView(this);

        // Write text on the sensors we have
        for ( Iterator<Sensor> it = deviceSensors.iterator();
              it.hasNext(); ) {
            Sensor s = it.next();
            tv.append("Found [" + sensor_type(s.getType()) + "]\n");
            tv.append(" Name = " + s.getName() + "\n");
        }

        setContentView(tv);
    }
}

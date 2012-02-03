package com.example.UsbIntent;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class UsbIntent extends UsbAccessoryActivity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.d(TAG, "Created USB Intent application");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // User can call this application directly .. but also just
        // plugging in our project causes this to pop up since we've
        // registered as a USB intent.
    }
}

package nasa.demo;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class Demo001 extends UsbAccessoryActivity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.d(TAG, "Started NASA Demo001");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
    }
}

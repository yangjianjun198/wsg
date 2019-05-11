package com.yjj.wsg;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import com.yjj.wsg.R;
import com.yjj.wsg.api.WsgManager;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("bitmapreader");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        WsgManager.getInstance().init(this);
        tv.setText(WsgManager.getInstance().getValue(1));
        File file = getExternalFilesDir(null);
        Log.d("yjj",file.getAbsolutePath());
    }
}

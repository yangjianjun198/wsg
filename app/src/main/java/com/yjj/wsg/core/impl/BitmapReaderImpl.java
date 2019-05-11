package com.yjj.wsg.core.impl;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.TextUtils;
import android.util.Log;

import com.yjj.wsg.R;
import com.yjj.wsg.core.BitmapReader;

import java.util.HashMap;
import java.util.Map;

/**
 * created by yangjianjun on 2018/8/25
 */
public class BitmapReaderImpl implements BitmapReader {
    private Context context;
    private Bitmap bitmap;
    private Map<Integer, String> cacheValues = new HashMap<>();

    public BitmapReaderImpl(Context context) {
        this.context = context;
    }

    @Override
    public String getValueByIndex(int index) {
        if (bitmap == null) {

            final BitmapFactory.Options option = new BitmapFactory.Options();
            option.inJustDecodeBounds = false;
            option.inPreferredConfig = Bitmap.Config.ARGB_8888;
            option.inDensity = 1;
            option.inScreenDensity = 1;
            option.inTargetDensity = 1;
            bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.test, option);
        }
        if (cacheValues.containsKey(index)) {
            return cacheValues.get(index);
        }
        if (bitmap != null) {
            long time = System.currentTimeMillis();
            String value = getValueByIndex(bitmap, index);
            long cost = System.currentTimeMillis() - time;
            Log.d("yjj", "cost =" + cost);
            if (TextUtils.isEmpty(value)) {
                value = "";
            }
            cacheValues.put(index, value);
            return value;
        }
        return "";
    }

    public native String getValueByIndex(Bitmap bitmap, int index);
}

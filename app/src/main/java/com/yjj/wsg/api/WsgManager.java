package com.yjj.wsg.api;

import android.content.Context;

import com.yjj.wsg.core.BitmapReader;
import com.yjj.wsg.core.impl.BitmapReaderImpl;

/**
 * created by yangjianjun on 2018/8/25
 */
public class WsgManager {
    private volatile static WsgManager instance;
    private BitmapReader bitmapReader;

    private WsgManager() {

    }

    public static WsgManager getInstance() {
        if (instance == null) {
            synchronized (WsgManager.class) {
                if (instance == null) {
                    instance = new WsgManager();
                }
            }
        }
        return instance;
    }

    public void init(Context context) {
        if (bitmapReader == null) {
            bitmapReader = new BitmapReaderImpl(context);
        }
    }

    public String getValue(int index) {
        return bitmapReader.getValueByIndex(index);
    }
}

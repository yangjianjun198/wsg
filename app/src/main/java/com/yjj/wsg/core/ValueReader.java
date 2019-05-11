package com.yjj.wsg.core;

import java.util.List;

/**
 * created by yangjianjun on 2018/8/25
 * 键值读
 */
public interface ValueReader {

    String readValueByKeyIndex(int keyIndex);

    List<String> readAll();
}

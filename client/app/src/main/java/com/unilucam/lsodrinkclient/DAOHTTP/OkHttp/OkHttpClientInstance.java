package com.unilucam.lsodrinkclient.DAOHTTP.OkHttp;

import android.content.Context;

import com.unilucam.lsodrinkclient.application.LSODrinkClientApplication;
import com.unilucam.lsodrinkclient.exceptions.UninitializedOkHttpClientInstanceException;

import java.io.File;

import okhttp3.Cache;
import okhttp3.Dispatcher;
import okhttp3.OkHttpClient;

public class OkHttpClientInstance {
    private static OkHttpClient okHttpClientInstance = null;

    public static void init(Context context, int cacheSizeInMiB){
        File httpCacheDirectory = new File(context.getCacheDir(), "http_cache");
        int cacheSize = cacheSizeInMiB * 1024 * 1024; // 10 MiB
        Cache cache = new Cache(httpCacheDirectory, cacheSize);
        okHttpClientInstance = new OkHttpClient.Builder()
                .dispatcher(new Dispatcher(LSODrinkClientApplication.getExecutorService()))
                .cache(cache)
                .build();
    }

    public static OkHttpClient getOkHttpClientInstance() throws UninitializedOkHttpClientInstanceException {
        if (okHttpClientInstance == null)
            throw new UninitializedOkHttpClientInstanceException("OkHttpClient Instance uninitialized.");
        return okHttpClientInstance;
    }

}

package com.unilucam.lsodrinkclient.application;

import android.app.Application;
import android.util.Log;

import com.unilucam.lsodrinkclient.DAOHTTP.OkHttp.OkHttpClientInstance;
import com.unilucam.lsodrinkclient.DAOHTTP.picasso.PicassoInstance;
import com.unilucam.lsodrinkclient.exceptions.UninitializedOkHttpClientInstanceException;
import com.unilucam.lsodrinkclient.sharedprefs.UserSessionManager;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class LSODrinkClientApplication extends Application {
    private static ExecutorService executorService = Executors.newFixedThreadPool(4);
    public void onCreate(){
        super.onCreate();
        UserSessionManager.init(getApplicationContext());
        Log.i("LSODrinkClientApp", "Initialized UserSessionManager");
        OkHttpClientInstance.init(getApplicationContext(), 5);
        Log.i("LSODrinkClientApp", "Initialized OkHttpClientInstance");
        try{
            PicassoInstance.init(getApplicationContext());
        }
        catch (UninitializedOkHttpClientInstanceException ex){
            Log.e("LSODrinkClientApp", "OkHttpClient instance not initialized!");
        }
        Log.i("LSODrinkClientApp", "Initialized PicassoInstance");
    }

    public static ExecutorService getExecutorService() {
        return executorService;
    }
}

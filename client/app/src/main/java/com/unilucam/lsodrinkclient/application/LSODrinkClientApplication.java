package com.unilucam.lsodrinkclient.application;

import android.app.Application;
import android.util.Log;

import com.unilucam.lsodrinkclient.DAOHTTP.picasso.PicassoInstance;
import com.unilucam.lsodrinkclient.sharedprefs.UserSessionManager;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class LSODrinkClientApplication extends Application {
    private static ExecutorService executorService = Executors.newFixedThreadPool(4);
    public void onCreate(){
        super.onCreate();
        UserSessionManager.init(getApplicationContext());
        Log.i("LSODrinkClientApp", "Initialized UserSessionManager");
        PicassoInstance.init(getApplicationContext());
        Log.i("LSODrinkClientApp", "Initialized PicassoInstance");
    }

    public static ExecutorService getExecutorService() {
        return executorService;
    }
}

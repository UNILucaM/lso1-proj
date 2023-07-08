package com.unilucam.lsodrinkclient.controllers;

import android.content.Context;
import android.util.Log;
import android.widget.Toast;

public interface ControllerUtils {
    public static void showUserFriendlyErrorMessageAndLogThrowable
            (Context context, String tag, String s, Throwable throwable){
        if (throwable != null) Log.e(tag, throwable.getMessage(), throwable);
        else Log.e(tag, s);
        if (context == null) return;
        Toast.makeText(context, s, Toast.LENGTH_LONG).show();
    }
}

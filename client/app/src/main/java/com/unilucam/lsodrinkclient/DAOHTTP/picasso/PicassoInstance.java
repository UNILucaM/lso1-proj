package com.unilucam.lsodrinkclient.DAOHTTP.picasso;

import android.content.Context;

import com.squareup.picasso.OkHttp3Downloader;
import com.squareup.picasso.Picasso;
import com.unilucam.lsodrinkclient.DAOHTTP.OkHttp.OkHttpClientInstance;
import com.unilucam.lsodrinkclient.application.LSODrinkClientApplication;
import com.unilucam.lsodrinkclient.exceptions.UninitializedOkHttpClientInstanceException;
import com.unilucam.lsodrinkclient.exceptions.UninitializedPicassoInstanceException;

import okhttp3.OkHttpClient;

public class PicassoInstance {
    private static Picasso picassoInstance = null;
    //Rimuovere i commenti risulterà in un client senza cache.
    //Utile per la demo.
    public static void init(Context context) throws UninitializedOkHttpClientInstanceException {
        OkHttpClient client = OkHttpClientInstance.getOkHttpClientInstance();
        picassoInstance = new Picasso.Builder(context)
                .executor(LSODrinkClientApplication.getExecutorService())
                .downloader(new OkHttp3Downloader(client))
                .loggingEnabled(true)
                .build();
    }

    public static Picasso getPicassoInstance() throws UninitializedPicassoInstanceException {
        if (picassoInstance == null) throw new UninitializedPicassoInstanceException
                ("Picasso Instance uninitialized.");
        return picassoInstance;
    }


}

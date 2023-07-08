package com.unilucam.lsodrinkclient.controllers;

import android.content.Intent;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.unilucam.lsodrinkclient.sharedprefs.UserSessionManager;

public class ControllerStartup extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent authIntent = new Intent(ControllerStartup.this, ControllerAuth.class);
        Intent skipLoginIntent = new Intent(ControllerStartup.this, ControllerHome.class);
        UserSessionManager userSessionManager = UserSessionManager.getInstance();
        if (userSessionManager.isLoggedIn()){
                startActivity(skipLoginIntent);
        }
        else startActivity(authIntent);

    }

}

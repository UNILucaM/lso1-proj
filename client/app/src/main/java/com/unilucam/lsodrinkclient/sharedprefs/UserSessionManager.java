package com.unilucam.lsodrinkclient.sharedprefs;

import android.content.Context;
import android.content.SharedPreferences;

import com.unilucam.lsodrinkclient.exceptions.UninitializedUserSessionException;

public class UserSessionManager {

    private static UserSessionManager instance;
    private final SharedPreferences sharedPreferences;
    private static final int SHARED_PREFERENCES_MODE = Context.MODE_PRIVATE;
    private static final String SHARED_PREFERENCES_FILENAME = "LSOdrinkclientPreferences";
    private UserSessionKeys userSessionKeys;

    private static final String PREF_KEY_USER_ID = "USER_ID";
    private static final String PREF_KEY_LOGIN_STATUS = "LOGIN_STATUS";

    public class UserSessionKeys{

        private final String USER_ID;
        private boolean isLoggedIn;

        public UserSessionKeys(String userId){
            this.USER_ID = userId;
            isLoggedIn = false;
        }

        public String getUserId() {
            return USER_ID;
        }

        protected void setLoggedIn(boolean loggedIn) {
            isLoggedIn = loggedIn;
        }

        public boolean isLoggedIn() {
            return isLoggedIn;
        }
    }

    public static void init(Context context){
        instance = new UserSessionManager(context);
    }

    public static UserSessionManager getInstance() {
        if (instance == null) throw new UninitializedUserSessionException
                ("Nessuna sessione inizializzata.");
        else return instance;
    }

    private UserSessionManager(Context context){
        sharedPreferences = context.getSharedPreferences
                (SHARED_PREFERENCES_FILENAME, SHARED_PREFERENCES_MODE);
        String userId = sharedPreferences.getString(PREF_KEY_USER_ID, null);
        Boolean isLoggedIn = sharedPreferences.getBoolean(PREF_KEY_LOGIN_STATUS, false);

        if (userId == null) return;
        setKeys(userId);
        setLoggedIn(isLoggedIn);
    }

    public void setKeys(String userId){
        this.userSessionKeys = new UserSessionKeys(userId);
    }

    public void setLoggedIn(boolean isLoggedIn){
        if (userSessionKeys == null) return;
        userSessionKeys.setLoggedIn(isLoggedIn);
    }

    public boolean saveUserSessionBlocking(){
        if (userSessionKeys == null) return false;
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(PREF_KEY_USER_ID, userSessionKeys.getUserId());
        editor.putBoolean(PREF_KEY_LOGIN_STATUS, userSessionKeys.isLoggedIn());
        return editor.commit();
    }

    public String getUserId(){
        if (userSessionKeys == null) return null;
        else return userSessionKeys.getUserId();
    }

    public boolean isLoggedIn(){
        if (userSessionKeys == null) return false;
        else return userSessionKeys.isLoggedIn();
    }

    public boolean logoutBlocking(){
        if (userSessionKeys == null) return false;
        setLoggedIn(false);
        return saveUserSessionBlocking();
    }

}

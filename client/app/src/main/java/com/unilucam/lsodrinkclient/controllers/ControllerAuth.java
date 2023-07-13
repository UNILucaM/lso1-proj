package com.unilucam.lsodrinkclient.controllers;

import android.content.Intent;
import android.os.Bundle;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.constraintlayout.widget.ConstraintSet;

import com.unilucam.lsodrinkclient.DAOFactory.DAOFactory;
import com.unilucam.lsodrinkclient.DAOs.DAOUser;
import com.unilucam.lsodrinkclient.R;
import com.unilucam.lsodrinkclient.application.LSODrinkClientApplication;
import com.unilucam.lsodrinkclient.exceptions.InvalidConnectionSettingsException;
import com.unilucam.lsodrinkclient.exceptions.UninitializedOkHttpClientInstanceException;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;
import com.unilucam.lsodrinkclient.sharedprefs.UserSessionManager;

import java.util.concurrent.ExecutorService;

public class ControllerAuth extends AppCompatActivity {

    private enum AuthMode{
        LOGIN,
        REGISTER
    }

    private AuthMode currentAuthMode = AuthMode.LOGIN;

    private ConstraintLayout constraintLayout;
    private EditText editTextUsername;
    private EditText editTextPassword;
    private EditText editTextPassword2;
    private Button btnHidePassword;
    private Button btnHidePassword2;
    private Button btnAuth;
    private TextView textViewAuthSubtext;
    private TextView textViewClickToRegisterOrGoBack;
    private ExecutorService executorService;
    private UserSessionManager userSessionManager;
    private boolean shouldHidePassword = false;
    private boolean shouldHidePassword2 = false;
    private DAOUser DAOUser;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_auth);
        constraintLayout = findViewById(R.id.constraintLayoutAuth);
        editTextUsername = findViewById(R.id.editTextUsername);
        editTextPassword = findViewById(R.id.editTextPassword);
        editTextPassword2 = findViewById(R.id.editTextPassword2);
        btnHidePassword = findViewById(R.id.btnHidePassword);
        btnHidePassword2 = findViewById(R.id.btnHidePassword2);
        btnAuth = findViewById(R.id.btnAuth);
        textViewAuthSubtext = findViewById(R.id.textViewAuthSubtext);
        textViewClickToRegisterOrGoBack = findViewById(R.id.textViewClickToRegisterOrGoBack);

        try{
            DAOUser = DAOFactory.getDAOUser();
        }
        catch (InvalidConnectionSettingsException | UninitializedOkHttpClientInstanceException ex){
            ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                    (getApplicationContext(), "Home",
                            "Impossibile autenticarsi.", ex);
        }
        userSessionManager = UserSessionManager.getInstance();
        executorService = LSODrinkClientApplication.getExecutorService();

        textViewClickToRegisterOrGoBack.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (currentAuthMode.equals(AuthMode.LOGIN)) switchModeTo(AuthMode.REGISTER);
                else if (currentAuthMode.equals(AuthMode.REGISTER)) switchModeTo(AuthMode.LOGIN);
            }
        });

        btnHidePassword.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                shouldHidePassword = !shouldHidePassword;
                if (shouldHidePassword){
                    editTextPassword.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                    btnHidePassword.getBackground().setAlpha(50);
                }
                else {
                    editTextPassword.setTransformationMethod(PasswordTransformationMethod.getInstance());
                    btnHidePassword.getBackground().setAlpha(200);
                }
            }
        });

        btnHidePassword2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                shouldHidePassword2 = !shouldHidePassword2;
                if (shouldHidePassword2){
                    editTextPassword2.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                    btnHidePassword2.getBackground().setAlpha(50);
                }
                else {
                    editTextPassword2.setTransformationMethod(PasswordTransformationMethod.getInstance());
                    btnHidePassword2.getBackground().setAlpha(200);
                }
            }
        });

        btnAuth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onBtnAuthPress();
            }
        });

        String userId = userSessionManager.getUserId();
        if (userId != null && !userId.isEmpty()) editTextUsername.setText(userId);
    }

    private void switchModeTo(AuthMode mode) {
        if (mode.equals(currentAuthMode)) return;
        editTextPassword.setError(null);
        editTextUsername.setError(null);
        editTextPassword2.setError(null);
        if (mode.equals(AuthMode.REGISTER)){
            editTextPassword2.setVisibility(View.VISIBLE);
            btnHidePassword2.setVisibility(View.VISIBLE);
            ConstraintSet constraintSet = new ConstraintSet();
            constraintSet.clone(constraintLayout);
            constraintSet.connect(R.id.btnAuth,
                    ConstraintSet.TOP,
                    R.id.editTextPassword2,
                    ConstraintSet.BOTTOM,
                    0);
            constraintSet.applyTo(constraintLayout);
            textViewAuthSubtext.setText("Oppure esegui il");
            textViewClickToRegisterOrGoBack.setText("login");
            btnAuth.setText("Registrati");
        } else{
            editTextPassword2.setVisibility(View.GONE);
            btnHidePassword2.setVisibility(View.GONE);
            ConstraintSet constraintSet = new ConstraintSet();
            constraintSet.clone(constraintLayout);
            constraintSet.connect(R.id.btnAuth,
                    ConstraintSet.TOP,
                    R.id.editTextPassword,
                    ConstraintSet.BOTTOM,
                    0);
            constraintSet.applyTo(constraintLayout);
            textViewAuthSubtext.setText("Non hai un account?");
            textViewClickToRegisterOrGoBack.setText("Registrati");
            btnAuth.setText("Login");
        }
        currentAuthMode = mode;
    }

    private void onBtnAuthPress(){
        if (DAOUser == null) return;
        if (currentAuthMode.equals(AuthMode.LOGIN)) attemptLogin();
        else if (currentAuthMode.equals(AuthMode.REGISTER)) attemptRegister();
    }

    private void attemptLogin(){
        String username = editTextUsername.getText().toString().trim();
        String password = editTextPassword.getText().toString().trim();
        executorService.execute(()-> {
            try{
                int result = DAOUser.login(username, password);
                if (result == 1) {
                    UserSessionManager userSessionManager = UserSessionManager.getInstance();
                    userSessionManager.setKeys(username);
                    userSessionManager.setLoggedIn(true);
                    userSessionManager.saveUserSessionBlocking();
                    startActivity(
                            new Intent(ControllerAuth.this, ControllerHome.class));
                }
            } catch (WrappedCRUDException wrappedCRUDException) {
                Log.e("ACTIVITY-AUTH", "Login failed with error: " +
                        wrappedCRUDException.getWrappedException());
                runOnUiThread(()->{
                    ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                            (getApplicationContext(), "Auth", "Errore nel login.",
                                    wrappedCRUDException);
                });
            }
        });
    }

    private void attemptRegister(){
        String username = editTextUsername.getText().toString().trim();
        int usernameLenght = username.length();
        String password = editTextPassword.getText().toString().trim();
        String password2 = editTextPassword2.getText().toString().trim();
        editTextUsername.setError(null);
        editTextPassword.setError(null);
        editTextPassword2.setError(null);
        if (!username.matches("^(?![0-9])[a-zA-Z0-9]+$") || usernameLenght <= 4
                || usernameLenght > 16 ){
            editTextUsername.setError("Il nome utente deve essere composto di caratteri alfanumerici " +
                    "ed essere lungo tra i 5 e i 16 caratteri.");
            editTextUsername.requestFocus();
            return;
        } else if (!password.matches
                ("(?=.{9,})(?=.*?[^\\w\\s])(?=.*?[0-9])(?=.*?[A-Z]).*?[a-z].*")){
            editTextPassword.setError("La password deve essere lunga almeno 9 caratteri, e deve " +
                    "avere almeno una lettera maiuscola,\n" +
                    " almeno una lettera minuscola, almeno un numero ed almeno un simbolo.");
            editTextPassword.requestFocus();
            return;
        } else if (!password.equals(password2)){
            editTextPassword2.setError("Le password non corrispondono.");
            editTextPassword2.requestFocus();
            return;
        }
        executorService.execute(()-> {
            try{
                int result = DAOUser.register(username, password);
                if (result == 1){
                    userSessionManager.setKeys(username);
                    userSessionManager.setLoggedIn(true);
                    userSessionManager.saveUserSessionBlocking();
                    startActivity(
                            new Intent(ControllerAuth.this, ControllerHome.class));
                }
            } catch (WrappedCRUDException wrappedCRUDException){
                Log.e("ACTIVITY-AUTH", "Register failed with error: " +
                        wrappedCRUDException.getWrappedException());
                runOnUiThread(()->{
                    ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                            (getApplicationContext(), "Auth", "Errore nella registrazione.",
                                    wrappedCRUDException);
                });
            }
        });

    }

    @Override
    public void onBackPressed() {
        if (currentAuthMode.equals(AuthMode.REGISTER)){
            switchModeTo(AuthMode.LOGIN);
        }
        else finishAffinity();

    }
}

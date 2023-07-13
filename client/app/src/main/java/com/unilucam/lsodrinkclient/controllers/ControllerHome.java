package com.unilucam.lsodrinkclient.controllers;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.material.navigation.NavigationView;
import com.unilucam.lsodrinkclient.DAOFactory.DAOFactory;
import com.unilucam.lsodrinkclient.DAOs.DAOImage;
import com.unilucam.lsodrinkclient.DAOs.DAOProduct;
import com.unilucam.lsodrinkclient.DTO.DTOPid;
import com.unilucam.lsodrinkclient.DTO.DTOProductPurchase;
import com.unilucam.lsodrinkclient.DTO.DTOPurchase;
import com.unilucam.lsodrinkclient.R;
import com.unilucam.lsodrinkclient.application.LSODrinkClientApplication;
import com.unilucam.lsodrinkclient.async.GetDrinkRunner;
import com.unilucam.lsodrinkclient.async.Result;
import com.unilucam.lsodrinkclient.drinkfeeditem.FeedItem;
import com.unilucam.lsodrinkclient.drinkfeeditem.FeedItemAdapter;
import com.unilucam.lsodrinkclient.exceptions.InvalidConnectionSettingsException;
import com.unilucam.lsodrinkclient.exceptions.UninitializedOkHttpClientInstanceException;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;
import com.unilucam.lsodrinkclient.sharedprefs.UserSessionManager;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ExecutorService;

public class ControllerHome extends AppCompatActivity implements
        NavigationView.OnNavigationItemSelectedListener {

    private ArrayList<FeedItem> cartFeedItems = new ArrayList<>();

    private enum HomeState{
        HOME_COCKTAIL,
        HOME_FRULLATI,
        HOME_SUGGESTED,
        HOME_CART,
        HOME_LOGOUT;

        @Override
        public String toString(){
            if (this.equals(HOME_COCKTAIL)) return "Cocktail";
            else if (this.equals(HOME_FRULLATI)) return "Frullati";
            else if (this.equals(HOME_SUGGESTED)) return "Consigliati";
            else if (this.equals(HOME_LOGOUT)) return "Logout";
            else if (this.equals(HOME_CART)) return "Carrello";
            return "INVALID";
        }
    }
    private DrawerLayout drawerLayout;
    private ActionBarDrawerToggle actionBarDrawerToggle;
    private NavigationView navigationView;
    private HomeState currentHomeState = HomeState.HOME_COCKTAIL;
    private DAOProduct DAOProduct;
    private DAOImage DAOImage;
    private RecyclerView recyclerViewDrink;
    private FeedItemAdapter feedItemAdapter;
    private ProgressBar progressBar;
    private TextView cartBadge;
    private int numberOfItemsInCart = 0;
    private Float cartPrice = 0f;
    private final int MAX_NUMBER_OF_CART_ITEMS_DISPLAYED = 99;
    private Button btnBuy;
    private FeedItemAdapter.ProductQuantityListener
            productQuantityListener;
    private ExecutorService executorService;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_home);
        drawerLayout = findViewById(R.id.drawerLayoutHome);
        navigationView = findViewById(R.id.navViewMenu);
        recyclerViewDrink = findViewById(R.id.recyclerViewDrink);
        progressBar = findViewById(R.id.progressBarHome);
        btnBuy = findViewById(R.id.btnBuy);


        // drawer layout instance to toggle the menu icon to open 
        // drawer and back button to close drawer

        actionBarDrawerToggle = new ActionBarDrawerToggle(this, drawerLayout, R.string.nav_open, R.string.nav_close);

        drawerLayout.addDrawerListener(actionBarDrawerToggle);
        actionBarDrawerToggle.syncState();

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        navigationView.setNavigationItemSelectedListener(this);
        try{
            DAOProduct = DAOFactory.getDAOProduct();
            DAOImage = DAOFactory.getDAOImage();
        }
        catch (InvalidConnectionSettingsException | UninitializedOkHttpClientInstanceException ex){
            ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                    (getApplicationContext(), "Home",
                    "Impossibile caricare i prodotti.", ex);
        }
        executorService = LSODrinkClientApplication.getExecutorService();

        btnBuy.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String userId = UserSessionManager.getInstance().getUserId();
                List<DTOProductPurchase> DTOProductPurchaseCart = convertCartToDTO();
                if (userId == null || userId.isEmpty() || DTOProductPurchaseCart == null ||
                DTOProductPurchaseCart.isEmpty()) return;
                executorService.execute(()->{
                    try{
                        List<DTOPid> unpurchasedProducts = DAOProduct.purchaseProducts
                                (userId, DTOProductPurchaseCart);
                        if (unpurchasedProducts == null || unpurchasedProducts.isEmpty()){
                            runOnUiThread(()->{
                                FeedItem fi;
                                while (!cartFeedItems.isEmpty()){
                                    fi = cartFeedItems.remove(0);
                                    fi.setQuantity(0);
                                }
                                cartPrice = 0f;
                                numberOfItemsInCart = 0;
                                cartBadge.setText("0");
                                cartBadge.setVisibility(View.GONE);
                                btnBuy.setText("Acquista");
                                btnBuy.setVisibility(View.GONE);
                                Toast.makeText(getApplicationContext(), "Drink acquistati.",
                                        Toast.LENGTH_LONG).show();
                                switchState(HomeState.HOME_SUGGESTED, false);
                            });
                        }
                        else{
                            float finalPrice = 0;
                            int finalCartSize = 0;
                            int nFound = 0;
                            int i = 0;
                            int j;
                            boolean isFound = false;
                            FeedItem fi;
                            DTOPid DTOPid;
                            int quantity;
                            for (; i < cartFeedItems.size(); i++){
                                fi = cartFeedItems.get(i);
                                isFound = false;
                                for (j = 0; j < unpurchasedProducts.size(); j++){
                                    DTOPid = unpurchasedProducts.get(j);
                                    if (fi.getPid() == DTOPid.getPid()){
                                        nFound++;
                                        isFound = true;
                                        quantity = fi.getQuantity();
                                        finalCartSize += quantity;
                                        finalPrice += quantity * fi.getPrice();
                                        break;
                                    }
                                }
                                if (!isFound) {
                                    fi.setQuantity(0);
                                    cartFeedItems.remove(i);
                                    numberOfItemsInCart--;
                                    i--;
                                }
                                if (nFound == unpurchasedProducts.size()) break;
                            }
                            final float realFinalPrice = finalPrice;
                            final int realFinalCartSize = finalCartSize;
                            cartPrice = finalPrice;
                            runOnUiThread(()->{
                                cartBadge.setText(realFinalCartSize+"");
                                btnBuy.setText("Acquista (€" + String.format("%.2f", realFinalPrice) + ")");
                                Toast.makeText(getApplicationContext(),
                                        "Alcuni drink non sono stati acquistati.",
                                        Toast.LENGTH_LONG).show();
                            });
                        }
                    } catch (WrappedCRUDException wcrude){
                        ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                                (getApplicationContext(), "Home",
                                        "Impossibile acquistare.", wcrude);
                    }
                });
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(@NonNull Menu menu){
        getMenuInflater().inflate(R.menu.menu_entries_actionbar, menu);
        MenuItem menuItem = menu.findItem(R.id.navMenuActionCart);
        View actionView = menuItem.getActionView();
        cartBadge = actionView.findViewById(R.id.cart_badge);
        actionView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onNavigationItemSelected(menuItem);
            }
        });
        cartBadge.setVisibility(View.GONE);

        productQuantityListener =
                new FeedItemAdapter.ProductQuantityListener()
                {
                    @Override
                    public void onChange(FeedItem feedItem, int changeAmount) {
                        boolean isFound = false;
                        int feedItemPid = feedItem.getPid();
                        FeedItem fi;
                        int i = 0;
                        numberOfItemsInCart += changeAmount;
                        cartPrice += changeAmount * feedItem.getPrice();
                        cartBadge.setText((numberOfItemsInCart > 99) ? "99" :
                                numberOfItemsInCart+"");
                        for (; i < cartFeedItems.size(); i++){
                            fi = cartFeedItems.get(i);
                            if (feedItemPid == fi.getPid()){
                                isFound = true;
                                if(changeAmount < 0){
                                    if (fi.getQuantity() == 0 &&
                                            !currentHomeState.equals(HomeState.HOME_CART)){
                                        cartFeedItems.remove(i);
                                    }
                                    if (numberOfItemsInCart == 0) cartBadge.setVisibility
                                            (View.GONE);
                                }
                                break;
                            }
                        }
                        if (changeAmount > 0){
                            if (!isFound) cartFeedItems.add(feedItem);
                            cartBadge.setVisibility(View.VISIBLE);
                        }
                        if (currentHomeState.equals(HomeState.HOME_CART))
                            btnBuy.setVisibility
                                    ((numberOfItemsInCart == 0) ? View.GONE : View.VISIBLE);
                        btnBuy.setText("Acquista (€" + String.format("%.2f", cartPrice) + ")");
                    }
                };
        Bitmap defaultBitmap =  BitmapFactory.decodeResource(this.getResources(),
                R.drawable.drink_default);
        feedItemAdapter = new FeedItemAdapter(new ArrayList<FeedItem>(),
                productQuantityListener, defaultBitmap);
        recyclerViewDrink.setAdapter(feedItemAdapter);
        recyclerViewDrink.setLayoutManager(new LinearLayoutManager(this));
        switchState(currentHomeState, true);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (actionBarDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem item) {
        int id = item.getItemId();
        HomeState newState = mapIdToHomeState(id);
        if (newState == null) return false;
        if (newState.equals(HomeState.HOME_LOGOUT)){
            UserSessionManager userSessionManager = UserSessionManager.getInstance();
            executorService.execute(()->{
                userSessionManager.setLoggedIn(false);
                userSessionManager.saveUserSessionBlocking();
                startActivity(
                        new Intent(ControllerHome.this, ControllerAuth.class));
            });
        }
        else switchState(newState, false);
        return true;
    }

    private HomeState mapIdToHomeState(int id){
        if (id == R.id.navMenuCocktail) return HomeState.HOME_COCKTAIL;
        else if (id == R.id.navMenuFrullati) return HomeState.HOME_FRULLATI;
        else if (id == R.id.navMenuSuggested) return HomeState.HOME_SUGGESTED;
        else if (id == R.id.navMenuLogout) return HomeState.HOME_LOGOUT;
        else if (id == R.id.navMenuActionCart) return HomeState.HOME_CART;
        else return null;
    }

    private void switchState(HomeState state, boolean isFirstCall){
        if (state.equals(currentHomeState) && !isFirstCall) return;
        btnBuy.setVisibility(View.GONE);
        recyclerViewDrink.setVisibility(View.GONE);
        progressBar.setVisibility(View.VISIBLE);
        currentHomeState = state;
        getSupportActionBar().setTitle(state.toString());
        if (!isFirstCall) drawerLayout.close();
        if (DAOProduct == null || DAOImage == null) return;
        if (!state.equals(HomeState.HOME_CART)){
            GetDrinkRunner getDrinkRunner = new GetDrinkRunner(DAOImage, DAOProduct);
            GetDrinkRunner.GetDrinkQueryType queryType;
            if (state.equals(HomeState.HOME_COCKTAIL))
                queryType = GetDrinkRunner.GetDrinkQueryType.COCKTAIL;
            else if (state.equals(HomeState.HOME_FRULLATI))
                queryType = GetDrinkRunner.GetDrinkQueryType.FRULLATO;
            else if (state.equals(HomeState.HOME_SUGGESTED))
                queryType = GetDrinkRunner.GetDrinkQueryType.SUGGESTED;
            else {
                ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                        (getApplicationContext(), "Home",
                                "Errore imprevisto. Contattare lo sviluppatore.", null);
                return;
            }
            getDrinkRunner.getDrinkRequest(queryType,
                    UserSessionManager.getInstance().getUserId(),
                    new GetDrinkRunner.GetDrinkCallback<ArrayList<FeedItem>>() {
                        @Override
                        public void onComplete(Result<ArrayList<FeedItem>> result) {
                            if (result instanceof Result.Success){
                                ArrayList<FeedItem> data =
                                        ((Result.Success<ArrayList<FeedItem>>) result).data;
                                for (FeedItem cartItem : cartFeedItems){
                                    for (FeedItem d : data){
                                        if (cartItem.getPid() == d.getPid()){
                                            d.setQuantity(cartItem.getQuantity());
                                            break;
                                        }
                                    }
                                }
                                runOnUiThread(()-> feedItemAdapter.setFeedItems(data));
                            }
                            else if (result instanceof Result.Error){
                                Exception exception =
                                        ((Result.Error<ArrayList<FeedItem>>) result).exception;
                                runOnUiThread(()-> {
                                    ControllerUtils.showUserFriendlyErrorMessageAndLogThrowable
                                            (getApplicationContext(), "Home",
                                                    "Errore nel caricare i prodotti.", exception);
                                });
                            }
                            runOnUiThread(() -> {
                                progressBar.setVisibility(View.GONE);
                                recyclerViewDrink.setVisibility(View.VISIBLE);
                            });
                        }
                    });
        } else {
            progressBar.setVisibility(View.GONE);
            feedItemAdapter.setFeedItems(cartFeedItems);
            recyclerViewDrink.setVisibility(View.VISIBLE);
            btnBuy.setVisibility((numberOfItemsInCart == 0) ? View.GONE : View.VISIBLE);
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            View v = getCurrentFocus();
            if (v instanceof EditText) {
                Rect outRect = new Rect();
                v.getGlobalVisibleRect(outRect);
                if (!outRect.contains((int)event.getRawX(), (int)event.getRawY())) {
                    v.clearFocus();
                    InputMethodManager imm = (InputMethodManager)
                            getSystemService(Context.INPUT_METHOD_SERVICE);
                    imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                }
            }
        }
        return super.dispatchTouchEvent(event);
    }

    private List<DTOProductPurchase> convertCartToDTO(){
        LinkedList<DTOProductPurchase> DTOPurchases = new LinkedList<>();
        for (FeedItem fi : cartFeedItems){
            if (fi.getQuantity() > 0)
                DTOPurchases.add(new DTOProductPurchase(fi.getPid(), fi.getQuantity()));
        }
        return DTOPurchases;
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        finishAffinity();
    }
}
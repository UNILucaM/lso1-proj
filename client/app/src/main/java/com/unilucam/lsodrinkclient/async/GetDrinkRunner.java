package com.unilucam.lsodrinkclient.async;

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;

import com.unilucam.lsodrinkclient.DAOs.DAOImage;
import com.unilucam.lsodrinkclient.DAOs.DAOProduct;
import com.unilucam.lsodrinkclient.application.LSODrinkClientApplication;
import com.unilucam.lsodrinkclient.configs.ImageConfig;
import com.unilucam.lsodrinkclient.drinkfeeditem.FeedItem;
import com.unilucam.lsodrinkclient.entities.Product;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class GetDrinkRunner {
    private final Executor executor;
    private DAOImage DAOImage;
    private DAOProduct DAOProduct;

    public enum GetDrinkQueryType{
        FRULLATO,
        COCKTAIL,
        SUGGESTED
    }

    public interface GetDrinkCallback<T>{
        void onComplete(Result<T> result);
    }

    public GetDrinkRunner(DAOImage DAOImage, DAOProduct DAOProduct){
        this.DAOImage = DAOImage;
        this.DAOProduct = DAOProduct;
        executor = LSODrinkClientApplication.getExecutorService();
    }

    public void getDrinkRequest(GetDrinkQueryType queryType, String userId,
                                GetDrinkCallback<ArrayList<FeedItem>> callback){
        executor.execute(new Runnable() {
            @Override
            public void run() {
                Result<ArrayList<FeedItem>> result = makeDrinkRequest(queryType, userId);
                callback.onComplete(result);
            }
        });
    }

    private Result<ArrayList<FeedItem>> makeDrinkRequest(GetDrinkQueryType queryType, String userId){
        try{
            List<Product> products = null;
            ArrayList<FeedItem> newFeedItems = null;
            if (queryType.equals(GetDrinkQueryType.COCKTAIL))
                products = DAOProduct.getCocktailProducts();
            else if (queryType.equals(GetDrinkQueryType.FRULLATO))
                products = DAOProduct.getFrullatiProducts();
            else if (queryType.equals(GetDrinkQueryType.SUGGESTED))
                products = DAOProduct.getSuggestedProductsForUser(userId);
            if (products != null) {
                newFeedItems = new ArrayList<>();
                Bitmap bm;
                for (Product p : products) {
                    try{
                        bm = DAOImage.getImage(ImageConfig.generatePath(p));
                    }
                    catch(WrappedCRUDException wcrude) {
                        bm = null;
                    }
                    newFeedItems.add(new FeedItem(p, bm));
                }
            }
            return new Result.Success<ArrayList<FeedItem>>(newFeedItems);
        } catch (WrappedCRUDException wcrude){
            return new Result.Error<ArrayList<FeedItem>>(wcrude);
        }
    }
}

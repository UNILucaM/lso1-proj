package com.unilucam.lsodrinkclient.DAOHTTP;

import android.graphics.Bitmap;

import com.squareup.picasso.Picasso;
import com.unilucam.lsodrinkclient.DAOHTTP.picasso.PicassoInstance;
import com.unilucam.lsodrinkclient.DAOs.DAOImage;
import com.unilucam.lsodrinkclient.exceptions.UninitializedPicassoInstanceException;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

import java.io.IOException;

public class DAOHTTPImage implements DAOImage {

    public DAOHTTPImage(String baseUrl){
        this.baseUrl = baseUrl;
    }
    private String baseUrl;
    @Override
    public Bitmap getImage(String imageName) throws WrappedCRUDException {
        try{
            Picasso picassoInstance = PicassoInstance.getPicassoInstance();
            return picassoInstance.
                    load(baseUrl + "/images?imagename=" + imageName).get();
        }
        catch (UninitializedPicassoInstanceException | IOException ex){
            throw new WrappedCRUDException(ex);
        }
    }
}

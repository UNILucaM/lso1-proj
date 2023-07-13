package com.unilucam.lsodrinkclient.DAOHTTP;

import androidx.annotation.NonNull;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.unilucam.lsodrinkclient.DAOHTTP.OkHttp.OkHttpClientInstance;
import com.unilucam.lsodrinkclient.DAOs.DAOProduct;
import com.unilucam.lsodrinkclient.DTO.DTOPid;
import com.unilucam.lsodrinkclient.DTO.DTOProductPurchase;
import com.unilucam.lsodrinkclient.DTO.DTOPurchase;
import com.unilucam.lsodrinkclient.entities.Product;
import com.unilucam.lsodrinkclient.enums.ProductType;
import com.unilucam.lsodrinkclient.exceptions.UninitializedOkHttpClientInstanceException;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

import java.io.IOException;
import java.util.List;

import retrofit2.Call;
import retrofit2.Response;
import retrofit2.Retrofit;
import retrofit2.converter.jackson.JacksonConverterFactory;
import retrofit2.http.Body;
import retrofit2.http.GET;
import retrofit2.http.POST;
import retrofit2.http.Query;

public class DAOHTTPProduct implements DAOProduct {
    public enum RequestProductType{
        FRULLATO,
        COCKTAIL,
        SUGGESTED;
        @NonNull
        @Override
        public String toString() {
            if (this.equals(FRULLATO)) return "frullato";
            else if (this.equals(COCKTAIL)) return "cocktail";
            else if (this.equals(SUGGESTED)) return "suggested";
            else return "invalid";
        }
    }

    private interface HTTPAPIProduct {
        @GET("/products")
        public Call<List<Product>> getProductByType
                (
                        @Query("type") RequestProductType productType,
                        @Query("username") String username
                );
        @POST("/products/purchase")
        public Call<List<DTOPid>> purchase
                (
                        @Body DTOPurchase purchase
                );
    }

    private HTTPAPIProduct APIProduct;

    public DAOHTTPProduct(String baseUrl) throws UninitializedOkHttpClientInstanceException {
        ObjectMapper objectMapper = new ObjectMapper();
        Retrofit retrofit = new Retrofit.Builder()
                .baseUrl(baseUrl)
                .client(OkHttpClientInstance.getOkHttpClientInstance())
                .addConverterFactory(JacksonConverterFactory.create(objectMapper))
                .build();
        APIProduct = retrofit.create(HTTPAPIProduct.class);
    }

    @Override
    public List<Product> getFrullatiProducts() throws WrappedCRUDException{
        try{
            Response<List<Product>> response =
                    APIProduct.getProductByType(RequestProductType.FRULLATO, null)
                    .execute();
            return DAOHTTPUtils.handleResponse(response);
        } catch (IOException ioe){
            throw new WrappedCRUDException(ioe);
        }
    }

    @Override
    public List<Product> getCocktailProducts() throws WrappedCRUDException {
        try{
            Response<List<Product>> response =
                    APIProduct.getProductByType(RequestProductType.COCKTAIL, null)
                            .execute();
            return DAOHTTPUtils.handleResponse(response);
        } catch (IOException ioe){
            throw new WrappedCRUDException(ioe);
        }
    }

    @Override
    public List<Product> getSuggestedProductsForUser(String userid) throws WrappedCRUDException {
        try{
            Response<List<Product>> response =
                    APIProduct.getProductByType(RequestProductType.SUGGESTED, userid)
                            .execute();
            return DAOHTTPUtils.handleResponse(response);
        } catch (IOException ioe){
            throw new WrappedCRUDException(ioe);
        }
    }

    @Override
    public List<DTOPid> purchaseProducts(String userid, List<DTOProductPurchase> purchases) throws
            WrappedCRUDException {
        try{
            Response<List<DTOPid>> response =
                    APIProduct.purchase(new DTOPurchase(userid, purchases))
                    .execute();
            return DAOHTTPUtils.handleResponse(response);
        } catch (IOException ioe){
            throw new WrappedCRUDException(ioe);
        }
    }
}

package com.unilucam.lsodrinkclient.DAOHTTP;

import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

import java.io.IOException;

import retrofit2.Response;

public class DAOHTTPUtils {
    public static <T extends Object>T handleResponse(Response<T> response) throws WrappedCRUDException{
        try{
            if (response.isSuccessful()){
                return response.body();
            }
            else throw new WrappedCRUDException(new Exception
                    (response.errorBody().string()));
        }
        catch (IOException ioe) {
            throw new WrappedCRUDException(ioe);
        }
    }
}

package com.unilucam.lsodrinkclient.DAOHTTP;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.unilucam.lsodrinkclient.DAOs.DAOUser;
import com.unilucam.lsodrinkclient.DTO.DTOUser;
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

public class DAOHTTPUser implements DAOUser {

    //L'implementazione dell'API sul server è estremamente semplice,
    //ovvero fornisce un codice di successo se la richiesta va a buon fine,
    //altrimenti un errore, senza un vero meccanismo di session management.
    //Per questo si ignora il corpo della risposta e si usa semplicemente
    //response.isSuccessful() (vedi DAOHTTPUtils)
    //per determinare se la richiesta va a buon fine.
    private interface HTTPAPIUser{
        @POST("/register")
        public Call<Void> register
                (
                        @Body DTOUser user
                );
        @POST("/login")
        public Call<Void> login
                (
                        @Body DTOUser user
                );
    }

    private HTTPAPIUser APIUser;

    public DAOHTTPUser(String baseUrl){
        ObjectMapper objectMapper = new ObjectMapper();
        Retrofit retrofit = new Retrofit.Builder()
                .baseUrl(baseUrl)
                .addConverterFactory(JacksonConverterFactory.create(objectMapper))
                .build();
        APIUser = retrofit.create(HTTPAPIUser.class);
    }

    @Override
    public int register(String userId, String password) throws WrappedCRUDException {
        try{
            Response<Void> response =
                    APIUser.register(new DTOUser(userId, password)).execute();
            DAOHTTPUtils.handleResponse(response);
            return 1;
        } catch (IOException ioe){
            throw new WrappedCRUDException(ioe);
        }
    }

    @Override
    public int login(String userId, String password) throws WrappedCRUDException {
        try{
            Response<Void> response =
                    APIUser.login(new DTOUser(userId, password)).execute();
            DAOHTTPUtils.handleResponse(response);
            return 1;
        } catch (IOException ioe){
            throw new WrappedCRUDException(ioe);
        }
    }
}

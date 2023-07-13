package com.unilucam.lsodrinkclient.exceptions;

public class UninitializedOkHttpClientInstanceException extends Exception{
    public UninitializedOkHttpClientInstanceException(String message){
        super(message);
    }
}

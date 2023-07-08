package com.unilucam.lsodrinkclient.exceptions;

public class UninitializedUserSessionException extends RuntimeException {
    public UninitializedUserSessionException(String message){
        super(message);
    }
}


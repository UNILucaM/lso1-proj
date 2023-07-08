package com.unilucam.lsodrinkclient.exceptions;

public class WrappedCRUDException extends Exception {

    final Exception wrappedException;

    public WrappedCRUDException(Exception wrappedException)
    {
        this.wrappedException = wrappedException;
    }

    public Exception getWrappedException() {
        return wrappedException;
    }

    public String getMessage(){
        if (wrappedException != null) return wrappedException.getMessage();
        return null;
    }
}
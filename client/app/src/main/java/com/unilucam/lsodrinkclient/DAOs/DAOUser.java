package com.unilucam.lsodrinkclient.DAOs;

import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

public interface DAOUser {
    public int register(String userId, String password) throws WrappedCRUDException;
    public int login(String userId, String password) throws WrappedCRUDException;
}

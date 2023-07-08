package com.unilucam.lsodrinkclient.DAOs;

import android.graphics.Bitmap;

import com.unilucam.lsodrinkclient.enums.DPI;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

public interface DAOImage {
    public Bitmap getImage(String str) throws WrappedCRUDException;
}

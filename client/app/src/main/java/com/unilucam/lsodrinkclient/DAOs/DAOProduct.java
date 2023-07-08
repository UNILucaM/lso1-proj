package com.unilucam.lsodrinkclient.DAOs;

import com.unilucam.lsodrinkclient.DTO.DTOPid;
import com.unilucam.lsodrinkclient.DTO.DTOProductPurchase;
import com.unilucam.lsodrinkclient.entities.Product;
import com.unilucam.lsodrinkclient.exceptions.WrappedCRUDException;

import java.util.List;

public interface DAOProduct {
    public List<Product> getFrullatiProducts() throws WrappedCRUDException;
    public List<Product> getCocktailProducts() throws WrappedCRUDException;
    public List<Product> getSuggestedProductsForUser(String userid) throws WrappedCRUDException;
    public List<DTOPid> purchaseProducts(String userid, List<DTOProductPurchase> purchases)
            throws WrappedCRUDException;
}

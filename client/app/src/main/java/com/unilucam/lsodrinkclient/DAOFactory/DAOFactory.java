package com.unilucam.lsodrinkclient.DAOFactory;

import com.unilucam.lsodrinkclient.DAOHTTP.DAOHTTPImage;
import com.unilucam.lsodrinkclient.DAOHTTP.DAOHTTPProduct;
import com.unilucam.lsodrinkclient.DAOHTTP.DAOHTTPUser;
import com.unilucam.lsodrinkclient.DAOs.DAOImage;
import com.unilucam.lsodrinkclient.DAOs.DAOProduct;
import com.unilucam.lsodrinkclient.DAOs.DAOUser;
import com.unilucam.lsodrinkclient.configs.ConnectionConfig;
import com.unilucam.lsodrinkclient.enums.ConnectionType;
import com.unilucam.lsodrinkclient.exceptions.InvalidConnectionSettingsException;

import java.sql.Connection;

public class DAOFactory {

    private static String getDefaultConnectionSettingsErrorMessage(){
        return "Invalid connection settings: " + ConnectionConfig.getConfigAsString();
    }
    public static DAOProduct getDAOProduct() throws InvalidConnectionSettingsException{
        if (ConnectionConfig.getConnectionType().equals(ConnectionType.HTTP))
            return new DAOHTTPProduct(ConnectionConfig.getBaseUrl());
        else throw new InvalidConnectionSettingsException(getDefaultConnectionSettingsErrorMessage());
    }

    public static DAOImage getDAOImage() throws InvalidConnectionSettingsException{
        if (ConnectionConfig.getConnectionType().equals(ConnectionType.HTTP))
            return new DAOHTTPImage(ConnectionConfig.getBaseUrl());
        else throw new InvalidConnectionSettingsException(getDefaultConnectionSettingsErrorMessage());
    }

    public static DAOUser getDAOUser() throws InvalidConnectionSettingsException{
        if (ConnectionConfig.getConnectionType().equals(ConnectionType.HTTP))
            return new DAOHTTPUser(ConnectionConfig.getBaseUrl());
        else throw new InvalidConnectionSettingsException(getDefaultConnectionSettingsErrorMessage());
    }

}

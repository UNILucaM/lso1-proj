package com.unilucam.lsodrinkclient.configs;

import com.unilucam.lsodrinkclient.enums.ConnectionType;

public class ConnectionConfig {
    private static final String baseUrl = "http://10.0.2.2:8080";
    private static final ConnectionType connectionType = ConnectionType.HTTP;

    public static String getBaseUrl() {
        return baseUrl;
    }

    public static ConnectionType getConnectionType(){
        return connectionType;
    }

    public static String getConfigAsString(){
        return "baseUrl: " + baseUrl +
                "\nconnectionType: " + connectionType;
    }
}

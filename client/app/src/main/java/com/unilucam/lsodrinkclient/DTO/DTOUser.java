package com.unilucam.lsodrinkclient.DTO;

import com.fasterxml.jackson.annotation.JsonProperty;

public class DTOUser {
    @JsonProperty("username")
    private String userId;
    private String password;

    public DTOUser(){};

    public DTOUser(String userId, String password) {
        this.userId = userId;
        this.password = password;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }
}

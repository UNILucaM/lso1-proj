package com.unilucam.lsodrinkclient.DTO;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;
@JsonIgnoreProperties(ignoreUnknown = true)
public class DTOPurchase {
    private String username;
    @JsonProperty("products")
    private List<DTOProductPurchase> DTOProductPurchases;

    public DTOPurchase(){}

    public DTOPurchase(String username, List<DTOProductPurchase> DTOProductPurchases) {
        this.username = username;
        this.DTOProductPurchases = DTOProductPurchases;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    @JsonIgnore
    public List<DTOProductPurchase> getDTOProductPurchases() {
        return DTOProductPurchases;
    }

    public void setDTOProductPurchases(List<DTOProductPurchase> DTOProductPurchases) {
        this.DTOProductPurchases = DTOProductPurchases;
    }
}

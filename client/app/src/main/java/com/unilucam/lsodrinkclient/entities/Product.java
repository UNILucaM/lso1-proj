package com.unilucam.lsodrinkclient.entities;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.unilucam.lsodrinkclient.enums.ProductType;

public class Product {
    private int pid;
    @JsonProperty("productname")
    private String productName;
    private float price;
    @JsonProperty("producttypevar")
    private ProductType productType;
    @JsonProperty("imagepath")
    private String imagePath;
    @JsonProperty("ingredientlist")
    private String ingredientList;
    @JsonProperty("suggestionscore")
    private Integer suggestionScore;

    public Product() {
    }

    public Product(int pid, String productName, float price, ProductType productType, String imagePath, String ingredientList, Integer suggestionScore) {
        this.pid = pid;
        this.productName = productName;
        this.price = price;
        this.productType = productType;
        this.imagePath = imagePath;
        this.ingredientList = ingredientList;
        this.suggestionScore = suggestionScore;
    }

    public int getPid() {
        return pid;
    }

    public void setPid(int pid) {
        this.pid = pid;
    }

    public String getProductName() {
        return productName;
    }

    public void setProductName(String productName) {
        this.productName = productName;
    }

    public float getPrice() {
        return price;
    }

    public void setPrice(float price) {
        this.price = price;
    }

    public ProductType getProductType() {
        return productType;
    }

    public void setProductType(ProductType productType) {
        this.productType = productType;
    }

    public String getImagePath() {
        return imagePath;
    }

    public void setImagePath(String imagePath) {
        this.imagePath = imagePath;
    }

    public String getIngredientList() {
        return ingredientList;
    }

    public void setIngredientList(String ingredientList) {
        this.ingredientList = ingredientList;
    }

    public Integer getSuggestionScore() {
        return suggestionScore;
    }

    public void setSuggestionScore(Integer suggestionScore) {
        this.suggestionScore = suggestionScore;
    }
}

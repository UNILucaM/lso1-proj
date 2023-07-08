package com.unilucam.lsodrinkclient.drinkfeeditem;

import android.graphics.Bitmap;

import com.unilucam.lsodrinkclient.entities.Product;
import com.unilucam.lsodrinkclient.enums.ProductType;

public class FeedItem {
    private int pid;
    private String productName;
    private float price;
    private int quantity;
    private String ingredientList;
    private Bitmap productBitmap;

    public FeedItem(Product product, Bitmap productBitmap){
        pid = product.getPid();
        productName = product.getProductName();
        price = product.getPrice();
        quantity = 0;
        this.productBitmap = productBitmap;
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

    public int getQuantity() {
        return quantity;
    }

    public void setQuantity(int quantity) {
        this.quantity = quantity;
    }

    public String getIngredientList() {
        return ingredientList;
    }

    public void setIngredientList(String ingredientList) {
        this.ingredientList = ingredientList;
    }

    public Bitmap getProductBitmap() {
        return productBitmap;
    }

    public void setProductBitmap(Bitmap productBitmap) {
        this.productBitmap = productBitmap;
    }

    public void increaseQuantity(){ this.quantity++;}

    public void decreaseQuantity() { this.quantity--;}
}

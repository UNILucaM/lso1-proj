package com.unilucam.lsodrinkclient.configs;

import com.unilucam.lsodrinkclient.entities.Product;

public class ImageConfig {
    private static final String defaultImageType = "jpg";
    public static String generatePath(Product p){
        String imagePath = p.getImagePath();
        if (imagePath != null && !imagePath.isEmpty()) return imagePath;
        else return p.getProductName()
                .replaceAll("[^a-zA-Z0-9_-]", "_")
                .toLowerCase() + "." + defaultImageType;
    }
}

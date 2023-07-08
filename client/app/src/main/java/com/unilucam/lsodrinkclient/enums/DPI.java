package com.unilucam.lsodrinkclient.enums;

public enum DPI {
    LDPI,
    MDPI,
    TVDPI,
    HDPI,
    XHDPI,
    XXHDPI;

    @Override
    public String toString(){
        if (this.equals(LDPI)) return "ldpi";
        else if (this.equals(TVDPI)) return "tvdpi";
        else if (this.equals(MDPI)) return "mdpi";
        else if (this.equals(HDPI)) return "hdpi";
        else if (this.equals(XHDPI)) return "xhdpi";
        else if (this.equals(XXHDPI)) return "xxhdpi";
        return "INVALID";
    }
}

package com.unilucam.lsodrinkclient.DTO;

public class DTOProductPurchase {
    private int pid;
    private int quantity;

    public DTOProductPurchase(){ }

    public DTOProductPurchase(int pid, int quantity) {
        this.pid = pid;
        this.quantity = quantity;
    }

    public int getPid() {
        return pid;
    }

    public void setPid(int pid) {
        this.pid = pid;
    }

    public int getQuantity() {
        return quantity;
    }

    public void setQuantity(int quantity) {
        this.quantity = quantity;
    }
}

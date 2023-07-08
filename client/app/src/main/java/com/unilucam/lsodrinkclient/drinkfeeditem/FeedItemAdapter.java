package com.unilucam.lsodrinkclient.drinkfeeditem;

import android.graphics.Bitmap;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.RecyclerView;

import com.unilucam.lsodrinkclient.R;

import java.util.ArrayList;

public class FeedItemAdapter extends RecyclerView.Adapter<FeedItemAdapter.FeedItemViewHolder>{
    public interface ProductQuantityListener
        {public void onChange(FeedItem feedItem, int changeAmount);}
    private ArrayList<FeedItem> feedItems;
    private ProductQuantityListener productQuantityListener;
    private boolean isButtonChange = false;
    private boolean isSelfChange = false;
    private final int QUANTITY_MAX = 99;
    private final Bitmap defaultBitmap;

    public FeedItemAdapter(ArrayList<FeedItem> feedItems,
                           ProductQuantityListener productQuantityListener,
                           Bitmap defaultBitmap){
        this.feedItems = feedItems;
        this.productQuantityListener = productQuantityListener;
        this.defaultBitmap = defaultBitmap;
    }

    @NonNull
    @Override
    public FeedItemViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View view = inflater.inflate(R.layout.item_product, parent, false);
        return new FeedItemViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull FeedItemViewHolder holder, int position) {
        FeedItem feedItem = feedItems.get(position);
        holder.textViewName.setText(feedItem.getProductName());
        holder.textViewPrice.setText(String.format("€%.2f", feedItem.getPrice()));
        holder.textViewIngredientList.setText(feedItem.getIngredientList());
        holder.editTextQuantity.setText(feedItem.getQuantity()+"");
        holder.editTextQuantity.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            int prev;
            @Override
            public void onFocusChange(View view, boolean hasFocus) {
                System.out.println("HI LOL");
                if (isButtonChange){
                    isButtonChange = false;
                    return;
                }
                EditText editText = (EditText) view;
                int currentValue;
                try{
                    currentValue = Integer.parseInt(editText.getText().toString());
                } catch (NumberFormatException nfe){
                    currentValue = -1;
                }
                if (!hasFocus){
                    int change;
                    int quantity = feedItem.getQuantity();
                    if (currentValue == -1){
                        editText.setText(0+"");
                        feedItem.setQuantity(0);
                        change = -quantity;
                    } else{
                        feedItem.setQuantity(currentValue);
                        change = currentValue - quantity;
                    }
                    System.out.println("CHANGE: " + change);
                    if (change != 0)
                        productQuantityListener.onChange(feedItem, change);
                } else editText.setText("");

            }
        });
        /*holder.editTextQuantity.addTextChangedListener(new TextWatcher() {
            int prev;
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                try{
                    prev = Integer.parseInt(charSequence.toString());
                }
                catch (NumberFormatException nfe){
                    prev = -1;
                }
            }

            @Override
            public void onTextChanged(CharSequence charSequence, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable editable) {
                if (isButtonChange || isSelfChange) {
                    isSelfChange = false;
                    isButtonChange = false;
                    return;
                }
                int n;
                try {
                    Log.i("LOL1", editable.toString() + " " + prev);
                    n = Integer.parseInt(editable.toString());
                } catch (NumberFormatException nfe) {
                    n = -1;
                }
                isSelfChange = true;
                int finalValue = 0;
                if (n == -1) {
                    finalValue = (prev == -1) ? 0 : prev;
                    editable.replace(0, editable.length(),
                            finalValue + "");
                }
                else if (n != prev) {
                    finalValue = n;
                }
                Log.i("LOL3", n + " " + prev);
                feedItem.setQuantity(finalValue);
                Log.i("LOL", (n - prev) + "");
                if (!(n == 0 && prev == -1))
                    productQuantityListener.onChange(feedItem, n - prev);
            }
        });*/
        Bitmap bm = feedItem.getProductBitmap();
        holder.imageView.setImageBitmap((bm == null) ? defaultBitmap : bm);
        holder.btnIncreaseQuantity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                System.out.println(feedItem.getQuantity());
                feedItem.increaseQuantity();
                isButtonChange = true;
                holder.editTextQuantity.setText(feedItem.getQuantity()+"");
                if (productQuantityListener != null) productQuantityListener.onChange
                        (feedItem, 1);
            }
        });
        holder.btnDecreaseQuantity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int quantity = feedItem.getQuantity();
                if (quantity == 0) return;
                feedItem.decreaseQuantity();
                isButtonChange = true;
                holder.editTextQuantity.setText(feedItem.getQuantity()+"");
                if (productQuantityListener != null) productQuantityListener.onChange
                        (feedItem, -1);
            }
        });

    }

    @Override
    public int getItemCount() {
        return feedItems == null ? 0 : feedItems.size();
    }

    public void setFeedItems(ArrayList<FeedItem> feedItems) {
        this.feedItems = feedItems;
        notifyDataSetChanged();
    }

    public ArrayList<FeedItem> getFeedItems() {
        return feedItems;
    }

    public class FeedItemViewHolder extends RecyclerView.ViewHolder{
        private TextView textViewName, textViewPrice, textViewIngredientList;
        private EditText editTextQuantity;
        private CardView cardView;
        private Button btnIncreaseQuantity, btnDecreaseQuantity;
        private ImageView imageView;
        ConstraintLayout constraintLayoutProduct;

        public FeedItemViewHolder(@NonNull View itemView) {
            super(itemView);
            textViewName = itemView.findViewById(R.id.textViewProductName);
            textViewPrice = itemView.findViewById(R.id.textViewPrice);
            textViewIngredientList = itemView.findViewById(R.id.textViewIngredients);
            editTextQuantity = itemView.findViewById(R.id.editTextQuantity);
            btnIncreaseQuantity = itemView.findViewById(R.id.btnIncreaseQuantity);
            btnDecreaseQuantity = itemView.findViewById(R.id.btnDecreaseQuantity);
            cardView = itemView.findViewById(R.id.cardViewProduct);
            constraintLayoutProduct = itemView.findViewById(R.id.constraintLayoutProduct);
            imageView = itemView.findViewById(R.id.productImage);
        }
    }
}

package com.example.keypass;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

public class PasswordAdapter extends RecyclerView.Adapter<PasswordAdapter.PasswordViewHolder> {

    private List<Password> passwordList;
    private OnItemClickListener clickListener;
    private OnItemLongClickListener longClickListener;

    public interface OnItemClickListener {
        void onItemClick(Password password);
    }

    public interface OnItemLongClickListener {
        void onItemLongClick(Password password);
    }

    public PasswordAdapter(List<Password> passwordList, OnItemClickListener clickListener, OnItemLongClickListener longClickListener) {
        this.passwordList = passwordList;
        this.clickListener = clickListener;
        this.longClickListener = longClickListener;
    }

    @NonNull
    @Override
    public PasswordViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.password_item, parent, false);
        return new PasswordViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull PasswordViewHolder holder, int position) {
        Password password = passwordList.get(position);
        holder.bind(password, clickListener, longClickListener);
    }

    @Override
    public int getItemCount() {
        return passwordList.size();
    }

    static class PasswordViewHolder extends RecyclerView.ViewHolder {
        private TextView nameTextView;

        public PasswordViewHolder(@NonNull View itemView) {
            super(itemView);
            nameTextView = itemView.findViewById(R.id.passwordName);
        }

        public void bind(final Password password, final OnItemClickListener clickListener, final OnItemLongClickListener longClickListener) {
            nameTextView.setText(password.getName());
            itemView.setOnClickListener(v -> clickListener.onItemClick(password));
            itemView.setOnLongClickListener(v -> {
                longClickListener.onItemLongClick(password);
                return true;
            });
        }
    }
}
package com.oxide.cheat;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private TextView statusText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setBackgroundColor(Color.parseColor("#0a0a0a"));

        statusText = new TextView(this);
        statusText.setText("СТАТУС: ОЖИДАНИЕ...");
        statusText.setTextColor(Color.YELLOW);
        statusText.setTextSize(24);
        statusText.setGravity(Gravity.CENTER);
        statusText.setPadding(0, 0, 0, 50);
        layout.addView(statusText);

        setContentView(layout);

        try {
            System.loadLibrary("oxide_cheat");
            statusText.setText("СТАТУС: ЧИТ ЗАГРУЖЕН!");
            statusText.setTextColor(Color.parseColor("#00ff88"));
            Toast.makeText(this, "Анти-Бан активен. Запуск игры...", Toast.LENGTH_SHORT).show();

            Intent intent = getPackageManager().getLaunchIntentForPackage("com.catsbit.oxidesurvivalisland");
            if (intent != null) {
                startActivity(intent);
            } else {
                Toast.makeText(this, "Игра не найдена!", Toast.LENGTH_LONG).show();
            }
        } catch (Throwable e) {
            statusText.setText("СТАТУС: ОШИБКА!\n" + e.getMessage());
            statusText.setTextColor(Color.RED);
        }
    }
}

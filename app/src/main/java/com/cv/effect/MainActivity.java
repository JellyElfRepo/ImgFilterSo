package com.cv.effect;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ImageView;

import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        AssetManager assetManager = getAssets();

        ImageView imageView = findViewById(R.id.image1);
        Bitmap bitmap = null;

        InputStream inputStream = null;
        try {
            inputStream = assetManager.open("dog.png");
            bitmap = BitmapFactory.decodeStream(inputStream);
            inputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        NativeFilter jniUtil = new NativeFilter();
        bitmap = bitmap.copy(Bitmap.Config.ARGB_8888,false);//转为ARGB_8888，该格式和Mat格式互转相对简单
        //bitmap = jniUtil.imgGray(bitmap,45);
        bitmap = jniUtil.colorLevelsTweak(bitmap,2,120,150,120,52,65);

        imageView.setImageBitmap(bitmap);
    }


}

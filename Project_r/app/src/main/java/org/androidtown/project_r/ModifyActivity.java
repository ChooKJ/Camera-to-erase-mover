package org.androidtown.project_r;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.File;

/**
 * Created by 추교정 on 2017-04-13.
 */


public class ModifyActivity extends Activity{

    ImageView iv;
    int idx = 0;
    Button buttonleft, buttonright, buttonselect;

    public static Bitmap loadBackgroundBitmap(Context context, String imgFilePath) {

        // 폰의 화면 사이즈를 구한다.
        Display display = ((WindowManager)context.getSystemService(
                Context.WINDOW_SERVICE)).getDefaultDisplay();
        int displayWidth = display.getWidth();
        int displayHeight = display.getHeight();

        // 읽어들일 이미지의 사이즈를 구한다.
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.RGB_565;
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(imgFilePath, options);

        // 화면 사이즈에 가장 근접하는 이미지의 스케일 팩터를 구한다.
        // 스케일 팩터는 이미지 손실을 최소화하기 위해 짝수로 한다.
        float widthScale = options.outWidth / displayWidth;
        float heightScale = options.outHeight / displayHeight;
        float scale = widthScale > heightScale ? widthScale : heightScale;

        if (scale >= 8)
            options.inSampleSize = 8;
        else if (scale >= 6)
            options.inSampleSize = 6;
        else if (scale >= 4)
            options.inSampleSize = 4;
        else if (scale >= 2)
            options.inSampleSize = 2;
        else
            options.inSampleSize = 1;
        options.inJustDecodeBounds = false;

        return BitmapFactory.decodeFile(imgFilePath, options);
    }

    public void bringToFrontButton(){
        buttonleft.bringToFront();
        buttonright.bringToFront();
        buttonselect.bringToFront();
    }

    public void putImage(){
        File sdCard = Environment.getExternalStorageDirectory();
        String imgFilePath = sdCard.getAbsolutePath() + "/camtest/img" + idx + ".jpg";

        Bitmap bitmap = loadBackgroundBitmap(ModifyActivity.this, imgFilePath);

        iv = (ImageView)findViewById(R.id.imageView);
        iv.setImageBitmap(bitmap);

        bringToFrontButton();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.modify_view);



        buttonleft = (Button)findViewById(R.id.btnLeftImg);
        buttonright = (Button)findViewById(R.id.btnRightImg);
        buttonselect = (Button)findViewById(R.id.btnselectImg);

        putImage();

        buttonleft.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                Toast.makeText(getApplicationContext(), "왼쪽 누름", Toast.LENGTH_LONG).show();
                if(idx>0) --idx;
                putImage();
            }
        });

        buttonright.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v){
                Toast.makeText(getApplicationContext(), "오른쪽 누름", Toast.LENGTH_LONG).show();

                if(idx < 4) ++idx;
                putImage();
            }

        });
        buttonselect.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                Toast.makeText(getApplicationContext(), (idx+ 1) + "번 사진 선택", Toast.LENGTH_LONG).show();
                Intent intent = new Intent(ModifyActivity.this, TouchActivity.class);
                intent.putExtra("idx", idx);
                Log.d("TAG", "before");
                startActivity(intent);
                finish();
            }
        });

    }
}
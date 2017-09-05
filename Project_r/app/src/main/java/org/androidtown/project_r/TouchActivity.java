package org.androidtown.project_r;

import android.Manifest;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.text.method.Touch;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import org.opencv.android.Utils;
import org.opencv.core.Mat;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class TouchActivity extends Activity {
    private static final String TAG = "TouchActivity";
    private final int REQUEST_CODE_STORAGE_PERMS = 321;

    ImageView iv;
    int idx = 0;
    Button buttonrewind, buttonsave;

    ImageView imageViewInput;
    ImageView imageViewOutput;
    public Mat img_input;
    private Mat img_output;
    private int x;
    private int y;
    byte[] filebytearr;

    static {
        System.loadLibrary("native-lib");
    }

    public native void returnArea(String path, int idx, long inputImage);
    public native int checkSquare(int x, int y);
    public native void change(int idx, long inputImage);
    public native int rewind(long inputImage);
    public native void save(long inputImage);


    private void requestNecessaryPermissions() {
        // make array of permissions which you want to ask from user.
        String[] permissions = new String[] {Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE};

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // have arry for permissions to requestPermissions method.
            // and also send unique Request code.
            requestPermissions(permissions, REQUEST_CODE_STORAGE_PERMS);
        }
    }

    private void refreshGallery(File file) {
        Intent mediaScanIntent = new Intent( Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
        mediaScanIntent.setData(Uri.fromFile(file));
        sendBroadcast(mediaScanIntent);
    }

    private boolean hasPermissions() {
        int res = 0;
        // list all permissions which you want to check are granted or not.
        String permissions = Manifest.permission.WRITE_EXTERNAL_STORAGE;

        res = checkCallingOrSelfPermission(permissions);
        if (!(res == PackageManager.PERMISSION_GRANTED)) {
            // it return false because your app dosen't have permissions.
            return false;
        }

        // it return true, your app has permissions.
        return true;
    }

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
        buttonrewind.bringToFront();
        buttonsave.bringToFront();
    }

    public void putImage(){
        File sdCard = Environment.getExternalStorageDirectory();
        //String imgFilePath = sdCard.getAbsolutePath() + "/camtest/img" + idx + ".jpg";
        String imgFilePath = sdCard.getAbsolutePath() + "/camtest/img"+idx+".jpg";

        Log.d(TAG, imgFilePath);
        Bitmap bitmap = loadBackgroundBitmap(TouchActivity.this, imgFilePath);

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();

        //이미지를 디바이스 방향으로 회전
        //Matrix matrix = new Matrix();
        //matrix.postRotate(90);
        //bitmap =  Bitmap.createBitmap(bitmap, 0, 0, w, h, matrix, true);

        iv = (ImageView)findViewById(R.id.imageView);
        iv.setImageBitmap(bitmap);

        //bringToFrontButton();
    }

    public void showResult(){
        Log.d("TAG", "before");
        Bitmap bitmapInput = Bitmap.createBitmap(img_input.cols(), img_input.rows(), Bitmap.Config.ARGB_8888);
        Log.d("TAG", "비트맵크기 설정 후");
        Utils.matToBitmap(img_input, bitmapInput);
        Log.d("TAG", "Mat을 비트맵으로 바꾸기");
        iv.setImageBitmap(bitmapInput);
        Log.d("TAG", "이미지 뷰에 비트맵 띄우기");
        bringToFrontButton();
    }

    public boolean onTouchEvent(MotionEvent event){

        if(event.getAction() == MotionEvent.ACTION_DOWN){
            y = (int)event.getRawX();
            x = (int)event.getRawY();
            Log.d(TAG, "touch x : "+x+", y : " + y);


            int idx = checkSquare(x, y);
            Log.d(TAG, "idx : " + idx);

            if(idx != -1) {

                img_input = new Mat();

                change(idx, img_input.getNativeObjAddr());
                showResult();
                // x, y좌표가 영역에 해당되는지 판단하고, 맞으면 지우기, C++로 함수 작성

                img_input.release();
            }
        }
        return super.onTouchEvent(event);
    }

    private Handler confirmHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.d("TAG", "프로그레스 다이어로그 완료");
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.touch_view);


        Log.d("TAG", "시작");

        Log.d("TAG", "인텐트 받기 전");

        Intent intent = getIntent();
        idx = intent.getIntExtra("idx", 0);
        Log.d("TAG", "인텐트 받음");
        Log.d("TAG", "*****************************************************************idx : "+ idx);
        if (getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA)) {
            if (!hasPermissions()) {
                // your app doesn't have permissions, ask for them.
                requestNecessaryPermissions();
            }

        } else {
            Toast.makeText(TouchActivity.this, "Camera not supported", Toast.LENGTH_LONG).show();
        }



        File sdCard = Environment.getExternalStorageDirectory();
        final String imgFilePath = sdCard.getAbsolutePath() + "/camtest";

        iv = (ImageView)findViewById(R.id.imageView);

        buttonrewind = (Button)findViewById(R.id.btnrewindImg);
        buttonrewind.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                Toast.makeText(getApplicationContext(), "되돌리기 버튼 눌림", Toast.LENGTH_LONG).show();


                img_input = new Mat();

                Log.d(TAG, "before");
                int size = rewind(img_input.getNativeObjAddr());
                if(size > 0) showResult();
                img_input.release();

                // 백업시켜둔 영역을 그냥 붙혀넣기 하자. 그리고 stack에서 pop

                Log.d(TAG, "Success");

                //finish();
            }
        });

        buttonsave = (Button)findViewById(R.id.btnsaveImg);
        buttonsave.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                Toast.makeText(getApplicationContext(), "저장 버튼 눌림", Toast.LENGTH_LONG).show();

                img_input = new Mat();

                Log.d(TAG, "before");
                save(img_input.getNativeObjAddr());
                showResult();

                try{
                    Bitmap bitmapInput = Bitmap.createBitmap(img_input.cols(), img_input.rows(), Bitmap.Config.ARGB_8888);
                    Utils.matToBitmap(img_input, bitmapInput);

                    ByteArrayOutputStream stream = new ByteArrayOutputStream();
                    bitmapInput.compress(Bitmap.CompressFormat.JPEG, 100, stream);
                    filebytearr = stream.toByteArray();

                    File dir = new File(imgFilePath);

                    String fileName = String.format("%d.jpg", System.currentTimeMillis());
                    File outFile = new File(dir, fileName);

                    FileOutputStream outStream = new FileOutputStream(outFile);
                    outStream.write(filebytearr);
                    outStream.close();

                    refreshGallery(outFile);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                img_input.release();

                // 최종 사진 저장하기
                Log.d(TAG, "Save Success");

                //finish();

            }
        });


        Log.d("TAG", "처리 전");
        //putImage();


        final ProgressDialog progDialog = new ProgressDialog( this );

        progDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        progDialog.setMessage("유동 개체 판별 중...");

        progDialog.show();

        final Handler handler = new Handler();

        //Thread 사용은 선택이 아니라 필수
        new Thread(new Runnable() {
            @Override
            public void run() {
                img_input = new Mat();
                Log.d(TAG, "before");
                returnArea(imgFilePath, idx, img_input.getNativeObjAddr());
                Log.d("TAG", "returnArea 실행 완료");
                handler.post(new Runnable() {

                    public void run() {

                        showResult();
                        Log.d("TAG", "결과 출력");
                        img_input.release();

                        Log.d("TAG", "프로그레스 다이얼로그 완료");

                    }

                });

                //dismiss(다이알로그종료)는 반드시 새로운 쓰레드 안에서 실행되어야한다
                progDialog.dismiss();
            }
        }).start();

        bringToFrontButton();
    }
}
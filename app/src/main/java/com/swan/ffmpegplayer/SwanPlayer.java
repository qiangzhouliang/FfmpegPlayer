package com.swan.ffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

/**
 * @ClassName SwanPlayer
 * @Description
 * @Author swan
 * @Date 2023/5/25 15:18
 **/
public class SwanPlayer implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("ffmpegplayer");
    }

    private SurfaceHolder surfaceHolder;
    // 绘制 NDK path surfaceView
    public void setSurfaceView(SurfaceView surfaceView){
        if (null != this.surfaceHolder){
            this.surfaceHolder.removeCallback(this);
        }
        this.surfaceHolder = surfaceView.getHolder();
        this.surfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        this.surfaceHolder = holder;
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }



    /**
     * A native method that is implemented by the 'ffmpegplayer' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public void start(String path) {
        native_start(path, surfaceHolder.getSurface());
    }

    public native void native_start(String path, Surface surface);

    /**
     * 音频解码
     * @param input
     * @param output
     */
    public native void sound(String input, String output);
}

package com.swan.ffmpegplayer;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Observer;

import android.Manifest;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

import com.swan.ffmpegplayer.databinding.ActivityMainBinding;
import com.tbruyelle.rxpermissions3.Permission;
import com.tbruyelle.rxpermissions3.RxPermissions;

import java.io.File;

import io.reactivex.rxjava3.android.schedulers.AndroidSchedulers;
import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.functions.Consumer;
import io.reactivex.rxjava3.functions.Function;
import io.reactivex.rxjava3.schedulers.Schedulers;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;

    SwanPlayer swanPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        swanPlayer = new SwanPlayer();
        System.out.println("FFmpeg 的版本为："+ swanPlayer.stringFromJNI());
        swanPlayer.setSurfaceView(binding.surfaceView);

        // 视频压缩命令 ffmpeg -i test.mp4 -b:v 1024k out.mp4

    }



    public void open(View view) {
        File file = new File(getApplication().getFilesDir().getPath()+"/那些年，我们一起追的女孩.mp4");
        swanPlayer.start(file.getPath());
    }

    public void openAudio(View view) {
        File file = new File(getApplication().getFilesDir().getPath()+"/test.mp3");
        String input = file.getAbsolutePath();
        String outPut = new File(getApplication().getFilesDir().getPath()+"/output.pcm").getAbsolutePath();
        swanPlayer.sound(input, outPut);
    }

    /**
     * Author swan
     * Description 视频压缩
     * Date 21:46 2023/6/27
     **/
    public void compressVideo(View view) {

        RxPermissions rxPermissions = new RxPermissions(this);
        rxPermissions
            .request(Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE)
            .subscribe(aBoolean -> {
                if (aBoolean){
                    compressVideo();
                }
            });

    }

    private void compressVideo() {
        // 视频压缩命令 ffmpeg -i test.mp4 -b:v 1024k out.mp4
        // -b:v 码率，码率越高视频越清晰，而且视频越大
        // 1M 1024k
        // test.mp4 需要压缩的视频路径
        // out.mp4  压缩后的路径
        // 压缩式耗时的 子线程 权限处理
        File mInFile = new File(getApplication().getFilesDir().getPath()+"/那些年，我们一起追的女孩.mp4");
        File mOutFile = new File(getApplication().getFilesDir().getPath()+"/out.mp4");

        String[] compressCommand = {"ffmpeg","-i", mInFile.getAbsolutePath(),"-b:v","1024k", mOutFile.getAbsolutePath()};
        Observable.just(compressCommand).map(new Function<String[], File>() {
            @Override
            public File apply(String[] compressCommand) throws Throwable {
                VideoCompress videoCompress = new VideoCompress();

                videoCompress.compressVideo(compressCommand, (current, total) -> {
                    Log.e("TAG", "压缩进度："+current + "/" +total);
                });
                return mOutFile;
            }
        }).subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe(file -> {
                // 压缩完成
                Log.e("TAG", "压缩完成");
            });
    }
}
package com.swan.ffmpegplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

import com.swan.ffmpegplayer.databinding.ActivityMainBinding;

import java.io.File;

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

    }



    public void open(View view) {
        File file = new File(getApplication().getFilesDir().getPath()+"/file.mp4");
        swanPlayer.start(file.getAbsolutePath());
    }

    public void openAudio(View view) {
        File file = new File(getApplication().getFilesDir().getPath()+"/test.mp3");
        String input = file.getAbsolutePath();
        String outPut = new File(getApplication().getFilesDir().getPath()+"/output.pcm").getAbsolutePath();
        swanPlayer.sound(input, outPut);
    }
}
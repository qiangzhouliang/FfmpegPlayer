package com.swan.ffmpegplayer;

/**
 * @ClassName VideoCompress
 * @Description 视频压缩
 * @Author swan
 * @Date 2023/6/27 21:39
 **/
public class VideoCompress {

    public native void compressVideo(String[] compressCommand, CompressCallback callback);


    public interface CompressCallback{
        public void onCompress(int current, int total);
    }

}

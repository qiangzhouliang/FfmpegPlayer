#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <android/log.h>

#define LOGI(args...) __android_log_print(ANDROID_LOG_INFO, "swan", args);
#define LOGE(args...) __android_log_print(ANDROID_LOG_ERROR, "swan", args);

#define MAX_AUDIO_FRME_SIZE 48000 * 4

// 因为FFmpeg是用c语言编写的，我们需要用c的编译器，所有用extern "C" 方式导入
extern "C" {
//    解码
#include <libavcodec/avcodec.h>
//    封装格式
#include "libavformat/avformat.h"
//缩放
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
//重采样
#include "libswresample/swresample.h"
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_swan_ffmpegplayer_SwanPlayer_stringFromJNI(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(av_version_info()); // av_version_info 获取FFmpeg版本
}


extern "C"
JNIEXPORT void JNICALL
Java_com_swan_ffmpegplayer_SwanPlayer_native_1start(JNIEnv *env, jobject thiz, jstring path_,
                                                    jobject surface) {
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
    const char *path = env->GetStringUTFChars(path_, 0);
    // ffmpeg 视频绘制 音频绘制
    // 初始化网络模块
    avformat_network_init();
    // 总上下文
    AVFormatContext * formatContext = avformat_alloc_context();
    AVDictionary *opts = NULL;
    // 设置超时时间
    av_dict_set(&opts, "timeout", "3000000", 0);
    // 打开视频文件
    int ret = avformat_open_input(&formatContext, path, NULL, &opts);
    if (ret){
        return;
    }

    // 找视频流
    int video_stream_idx = -1;
    avformat_find_stream_info(formatContext, NULL);
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        // 获取解码器
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            break;
        }
    }

    // 视频流索引 -》 拿到视频流解码参数
    AVCodecParameters *codecpar = formatContext->streams[video_stream_idx]->codecpar;

    // 还原解码器  找到解码器 h264
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);

    // 解码器上下文
    AVCodecContext * codecContext = avcodec_alloc_context3(dec);
    // 将解码器参数 copy 到解码器上下文
    avcodec_parameters_to_context(codecContext, codecpar);
    // 打开解码器
    avcodec_open2(codecContext, dec, NULL);
    // 解码 yuv数据
    // AVPacket
    AVPacket *packet = av_packet_alloc();
    // 从视频流中读取数据包
    SwsContext * swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                   codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, 0, 0, 0);
    ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width, codecContext->height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer outBuffer;
    while (av_read_frame(formatContext, packet) >= 0){
        avcodec_send_packet(codecContext, packet);
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)){
            continue;
        } else if (ret < 0){
            break;
        }
        // 接受的容器
        uint8_t *dst_data[4];
        // 每一行的首地址
        int dst_linesize[4];
        av_image_alloc(dst_data, dst_linesize, codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);
        // 绘制视频
        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height,
                  dst_data, dst_linesize);
        ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
        // 渲染
        uint8_t *firstWindown = static_cast<uint8_t *>(outBuffer.bits);
        // 输入源 rgb 的
        uint8_t *src_data = dst_data[0];
        // 拿到一行有多少个字节 rgba
        int destStride = outBuffer.stride * 4;
        int src_linesize = dst_linesize[0];

        for (int i = 0; i < outBuffer.height; ++i) {
            // 内存拷贝 来进行渲染
            memcpy(firstWindown + i * destStride, src_data + i * src_linesize, destStride);
        }

        ANativeWindow_unlockAndPost(nativeWindow);
        usleep(1000 * 6);
        av_frame_free(&frame);
    }




    env->ReleaseStringUTFChars(path_, path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_swan_ffmpegplayer_SwanPlayer_sound(JNIEnv *env, jobject thiz, jstring input_,
                                            jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);
    // 初始化网络模块
    avformat_network_init();
    // 总上下文
    AVFormatContext * formatContext = avformat_alloc_context();
    // 打开音频文件
    if (avformat_open_input(&formatContext, input, NULL, NULL) !=  0){
        LOGI("%s", "无法打开音频文件");
    }

    // 获取输入文件信息
    if (avformat_find_stream_info(formatContext, NULL) < 0){
        LOGI("%s", "无法获取输入文件信息");
    }

    // 音频时长（单位 微秒 us，转换为秒需要除以 1000000）
    int audio_stream_idx = -1;
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        // 获取解码器
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
            break;
        }
    }
    // 视频流索引 -》 拿到视频流解码参数
    AVCodecParameters *codecpar = formatContext->streams[audio_stream_idx]->codecpar;

    // 还原解码器  找到解码器 h264
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);

    // 解码器上下文
    AVCodecContext * codecContext = avcodec_alloc_context3(dec);
    // 将解码器参数 copy 到解码器上下文
    avcodec_parameters_to_context(codecContext, codecpar);
    avcodec_open2(codecContext, dec, NULL);

    // 转换器上下文
    SwrContext *swrContext = swr_alloc();
    // 输入的这些参数
    AVSampleFormat in_sample = codecContext->sample_fmt;
    // 输入采样率
    int in_sample_rate = codecContext->sample_rate;
    // 输入声道布局
    uint64_t in_ch_layout = codecContext->channel_layout;
    // 输出的参数 固定
    // 输出采样格式
    AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;
    // 输出采样率
    int out_sample_rate = 44100;
    // 输出声道布局
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    // 设置转换器输入和输出参数
    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample, out_sample_rate,
                       in_ch_layout, in_sample, in_sample_rate, 0, NULL);
    // 初始化转换器其他默认参数
    swr_init(swrContext);
    uint8_t *out_buffer = static_cast<uint8_t *>(av_malloc(2 * 4410));
    FILE *fp_pcm = fopen(output, "wb");


    // 读取包 压缩数据
    // AVPacket
    AVPacket *packet = av_packet_alloc();
    int count = 0;

    while (av_read_frame(formatContext, packet) >= 0){
        avcodec_send_packet(codecContext, packet);
        // 解压缩数据
        AVFrame *frame = av_frame_alloc();
        int ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)){
            continue;
        } else if (ret < 0){
            LOGE("解码完成");
            break;
        }
        if (packet->stream_index != audio_stream_idx){
            continue;
        }
        LOGE("正在解码 %d", count++);
        // frame ----> 统一的格式
        swr_convert(swrContext, &out_buffer, 2 * 44100,(const uint8_t **)frame->data, frame->nb_samples);
        int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
        // out_buffer ----> file
        // 获取缓冲区 大小
        int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples, out_sample, 1);
        fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
    }
    fclose(fp_pcm);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);

    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}
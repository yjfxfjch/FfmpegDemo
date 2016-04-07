# FfmpegDemo
最新ffmpeg库的使用例子。由于ffmpeg更新，一些API被弃用，换成新的API。而许多教程还是用旧的API，所以学习起来不是很方便。

FfmpegDemo里面代码用的是最新的ffmpeg库，并且不定时更新。

代码在VS2015下编译。目前包含如下例子。

##一.video_decode_RGB

解码视频，转换成RGB格式并以PPM图像形式保存

##二.SDL_video_play

解码视频为YUV格式，并且使用SDL2.0播放

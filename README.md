# Introduction

A beginner, learning a part of the classic SRS, has developed a simple RTMP server that utilizes coroutines similar to SRS.    This project is aimed at providing a deeper understanding of the features of the RTMP protocol.    Initially, the primary objective was to implement the server's functionality as quickly as possible without considering other aspects, resulting in code that appears subpar.   I would be greatly honored if someone could review this project and offer valuable advice.

I am currently refactoring the entire project to enhance its visual appeal and stability.    However, a significant issue persists throughout the project: the pull stream client is not consistently stable during playback.    Sometimes it operates smoothly, while at other times, it discontinues after playing for a while.    I have made every effort to identify the problem, but still nothing.  I would be immensely appreciative if someone could review this project and help address this issue.

The following document is the result of my efforts to sort out the RTMP protocol during my learning process. It is a work in progress and will continue to be refined over time.

link：https://pan.baidu.com/s/1MXuoC0nY5xEATWUrlTIwoA 
code：pxrz

# Usage

```shell
1. make
2. ./server
3. ffmpeg -re -i video_name -c copy -f flv -y rtmp://server_ip:port/live
4. vlc pull stream
```

Here are the VLC versions that support the RTMP protocol:

link：https://pan.baidu.com/s/1Snumo0Zay-6Cuma1XryqDQ 
code：u5gv
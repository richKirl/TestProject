ffmpeg -video_size 1700x900 -framerate 60 -f x11grab -i :0.0+154,148 output1.mp4;
ffmpeg -ss 00:00:16 -t 00:00:45 -i output1.mp4 -c copy output2.mp4;
#from output0
#ffmpeg -ss 00:00:00 -t 00:00:45 -i output1.mp4 -c copy output2.mp4#from output1
ffmpeg -i output2.mp4 -pix_fmt yuv420p -c:v libx264 output3.mp4;
rm output1.mp4;
rm output2.mp4;

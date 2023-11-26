# smallchat-server

说明：本项目是基于 https://github.com/antirez/smallchat 中的server项目改成的 C++项目。

该server项目基于C++ 11编写，可以windows、linux跨平台编译运行。

client端需要用到 https://github.com/antirez/smallchat中的client项目，且只支持linux



该项目在linux端采用cmake进行**编译**，编译步骤：

1：进到build目录下，终端运行:**cmake ..**  

2：之后会生成一个Makefile，直接运行:**make**即可



该服务端项目**运行**：在与build同级的bin下会生成运行文件smallchat-server

客户端运行文件在client目录下，该客户端只能linux下使用，源自 https://github.com/antirez/smallchat 项目




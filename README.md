# RtspServer

一个简单RTSP服务器实现，支持基本的RTSP请求（OPTIONS、DESCRIBE、SETUP、PLAY、TEARDOWN）。

先在Windows端编写，后移植到Linux端，已在Windows端测试过，但Linux端可能仍有bug。

```
RtspServer/
├─ CMakeLists.txt
├─ CMakePresets.json     
├─ RtspServer/
│  ├─  main.cpp                     # 程序入口：解析参数、启动服务器
│  ├─  RtspServer.h/.cpp            # RTSP 服务器对象：监听套接字、select() 事件循环
│  ├─  RtspSession.h/.cpp           # RTSP 会话管理
│  ├─  RtspRequest.h/.cpp           # RTSP 请求解析类型
│  ├─  TcpStream.h/.cpp             # TCP  套接字封装
│  ├─  RtspParser.h/.cpp            # RTSP 请求解析
│  └─  net_compat.h                 # 网络兼容性封装
├─ README.md
└─ LICENSE.txt
```

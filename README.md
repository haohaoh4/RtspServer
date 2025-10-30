# RtspServer

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

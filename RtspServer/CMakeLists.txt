# 可选：只在需要时开启



add_executable (RtspServer "main.cpp" "RtspServer.cpp" "RtspParser.cpp" "RtspSession.cpp" "TcpStream.cpp" "net_compat.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RtspServer PROPERTY CXX_STANDARD 23)
endif()


#include "RtspServer.h"
#include <variant>
int main() try {	
	std::cout << "Server Starting..." << std::endl;
	auto server = RtspServer();
	server.run();

	std::cout << "Server Closing..." << std::endl;
	return 0;

} catch (const std::exception& e) {
	std::cerr << "Fatal error: " << e.what() << std::endl;
	return 1;
}

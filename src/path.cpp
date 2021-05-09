#include <string>
#include <iostream>
#include <unistd.h>
#include <limits.h>

#ifdef WINDOWS
#	include <Libloaderapi.h>
#endif

static inline std::string Resource_GetSelfPath(){
#ifdef WINDOWS
	char buf[PATH_MAX];
	ssize_t len = GetModuleFileNameA(NULL, buf, sizeof(buf) - 1);
#else
	char buf[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
#endif
    if (len != -1) {
    	buf[len] = '\0';
    	return std::string(buf);
    }
	std::cerr << "ERROR: Failed to get path to self" << std::endl;
	exit(EXIT_FAILURE);
}

std::string Resource_GetPath(const char * str){
	if(str[0] == '/' || str[0] == 'C') return str;
	std::string rsult = Resource_GetSelfPath();
#ifdef WINDOWS	
	rsult.erase(rsult.end() - 8, rsult.end());
#else
	rsult.erase(rsult.end() - 4, rsult.end());
#endif
	rsult += "../data/";
	rsult += str;
	std::cout << "Concat str: " << rsult << std::endl;
	return rsult;
}


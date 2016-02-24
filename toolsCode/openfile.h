struct WindowObj;

#if defined(_WIN32)
bool WindowsOpenFile(char* fileName, WindowObj* owner, const char* fileFilter, const char* filterTitle, const char* dirName, int dirLength);
#endif

bool OpenFile(char* fileName, WindowObj* owner, const char* fileFilter, const char* filterTitle, const char* dirName, int dirLength){

#if defined(_WIN32)
	return WindowsOpenFile(fileName, owner, fileFilter, filterTitle, dirName, dirLength);
#elif defined(__APPLE__)
	//TODO
#else
	//TODO
#endif

}

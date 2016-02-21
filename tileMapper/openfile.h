struct WindowObj;

bool OpenFile(char* fileName, WindowObj* owner, const char* fileFilter, const char* filterTitle, const char* dirName, int dirLength){

#if defined(_WIN32)
	WindowsOpenFile(fileName, owner, fileFilter, filterTitle, dirName, dirLength);
#elif defined(__APPLE__)
	//TODO
#else
	//TODO
#endif

}

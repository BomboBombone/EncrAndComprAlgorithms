#define ZeroMemory(ptr, sz) for(int i = 0; i < sz; i++) \
								((char*)ptr)[i] = 0;
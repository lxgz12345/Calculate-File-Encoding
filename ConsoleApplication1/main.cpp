// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <map>
#include <locale>
#include <codecvt>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#include <time.h>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <atlconv.h>
//#include <format>//c++20
#include <fstream>
using namespace std;

std::wstring string2wstring(std::string str) {
	std::wstring result;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	wchar_t* buffer = new wchar_t[static_cast<size_t>(len) + (size_t)1];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), buffer, len);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}

std::string wstring2string(std::wstring wstr) {
	std::string result;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[static_cast<size_t>(len) + (size_t)1];
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}

std::vector<std::string> split(std::string str, std::string pattern) {
	int pos;
	std::vector<std::string> result;
	str += pattern;
	int size = (int)str.size();
	for (int i = 0; i < size; i++) {
		pos = (int)str.find(pattern, i);
		if (pos < size) {
			std::string s = str.substr(i, static_cast<std::basic_string<char, std::char_traits<char>, std::allocator<char>>::size_type>(pos) - i);
			result.push_back(s);
			i = pos + (int)pattern.size() - 1;
		}
	}
	return result;
}

void Wchar_tToString(std::string& szDst, wchar_t* wchar) {
	wchar_t* wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	char* psText;
	psText = new char[dwNum];
	WideCharToMultiByte(CP_ACP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	szDst = psText;
	delete[]psText;
}

wchar_t* StringToWchar_t(std::string& str) {
	wchar_t* text1 = new wchar_t[str.size() + 1];
	swprintf(text1, str.size() + 1, L"%S ", str.c_str());
	return text1;
}

std::string get_last_error(DWORD errCode = GetLastError()) {
	std::string err("");
	if (errCode == 0) errCode = GetLastError();
	LPTSTR lpBuffer = NULL;
	//失败
	if (0 == FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpBuffer, 0, NULL)) {
		char tmp[100] = { 0 };
		sprintf_s(tmp, "{未定义错误描述(%d)}", errCode);
		err = tmp;
	} else {
		//成功
		USES_CONVERSION;
		if (lpBuffer != NULL) err = wstring2string(lpBuffer);
		LocalFree(lpBuffer);
	}
	return err;
}

int IsUTF8(const void* pBuffer, long size) {
	int IsUTF8 = 1;
	unsigned char* start = (unsigned char*)pBuffer;
	unsigned char* end = (unsigned char*)pBuffer + size;
	while (start < end) {
		if (*start < 0x80) {
			start++;
		} else if (*start < (0xC0)) {
			IsUTF8 = 0; break;
		} else if (*start < (0xE0)) {
			if (start >= end - 1) break;
			if ((start[1] & (0xC0)) != 0x80) { IsUTF8 = 0; break; }
			start += 2;
		} else if (*start < (0xF0)) {
			if (start >= end - 2) break;
			if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80) { IsUTF8 = 0; break; }
			start += 3;
		} else { IsUTF8 = 0; break; }
	}
	return IsUTF8;
}


int CalculateFileEncoding(LPCSTR filePath) {
	/*	返回值说明
	* 0		文件读取失败
	* 1		UTF-8
	* 2		UTF-16LE
	* 3		UTF16_BE
	* 4		UTF8_BOM
	* 5		未知
	*/
	HANDLE pFile; char* tmpBuf;
	DWORD fileSize, dwBytesRead, dwBytesToRead;
	pFile = CreateFileA(filePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pFile == INVALID_HANDLE_VALUE) { CloseHandle(pFile); return 0; }
	fileSize = GetFileSize(pFile, NULL);
	char* buffer = new char[static_cast<size_t>(fileSize) + (size_t)1];
	buffer = (char*)malloc(static_cast<size_t>(fileSize) + (size_t)1);
	if (buffer == NULL) { return 0; }
	ZeroMemory(buffer, static_cast<size_t>(fileSize) + (size_t)1);
	dwBytesToRead = fileSize;
	dwBytesRead = 0;
	tmpBuf = buffer;
	do {
		BOOL success = ReadFile(pFile, tmpBuf, dwBytesToRead, &dwBytesRead, NULL);
		if (success == NULL) printf("ReadFile failed : %s", get_last_error().c_str());
		if (dwBytesRead == 0) break;
		dwBytesToRead -= dwBytesRead;
		tmpBuf += dwBytesRead;
	} while (dwBytesToRead > 0);
	CloseHandle(pFile);
	// 处理读到的数据 buffer
	//std::cout << "buffer0:" << (int)buffer[0] << std::endl;
	//std::cout << "buffer1:" << (int)buffer[1] << std::endl;
	//std::cout << "buffer2:" << (int)buffer[2] << std::endl;
	if (buffer[0] == 0xFF && buffer[1] == 0xFE) {
		return 2;//UTF16_LE
	} else if (buffer[0] == -1 && buffer[1] == -2) {
		return 2;//UTF16_LE
	} else if (buffer[0] == 0xFE && buffer[1] == 0xFF) {
		return 3;//UTF16_BE
	} else if (buffer[0] == -2 && buffer[1] == -1) {
		return 3;//UTF16_BE
	}  else if (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
		return 4;//UTF8_BOM
	}else if (buffer[0] == -17 && buffer[1] == -69 && buffer[2] == -65) {
		return 4;//UTF8_BOM
	} else if (IsUTF8(buffer, fileSize + 1)) {
		return 1;//UTF-8
	} else {
		return 5;//以上都不是，可能是ANSI
	}
}

std::string 判断文件编码(LPCSTR filePath) {
	/*	返回值说明
	* 0		文件读取失败
	* 1		UTF-8
	* 2		UTF-16LE
	* 3		UTF16_BE
	* 4		UTF8_BOM
	* 5		未知
	*/
	switch (CalculateFileEncoding(filePath)) {
	case 0:
		return "READ_FAIL";
		break;
	case 1:
		return "UTF-8";
		break;
	case 2:
		return "UTF-16LE_BOM";
		break;
	case 3:
		return "UTF16_BE_BOM";
		break;
	case 4:
		return "UTF8_BOM";
		break;
	case 5://以上都不是，可能是ANSI
		return "ANSI";
		break;
	default:
		return "ERROR";
		break;
	}
}

int main(int argc, char** argv) {
	//std::cout << 判断文件编码("C:\\Users\\admin\\Desktop\\1.txt") << std::endl;
	//std::cout << 判断文件编码("C:\\Users\\admin\\Desktop\\8.csv") << std::endl;
	//std::cout << 判断文件编码("C:\\Users\\admin\\Desktop\\16.csv") << std::endl;
	//std::cout << 判断文件编码("C:\\Users\\admin\\Desktop\\32.txt") << std::endl;

	if (argc == 1) {
		return 1;
	}
	for (int i = 1; i < argc; i++) {
		//a(argv[i]);
		std::cout << 判断文件编码(argv[i]) << endl;
	}
	system("pause");
	return 0;
}
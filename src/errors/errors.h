#include <iostream>
using namespace std;
#include <string>

#ifndef _ERROR_CODES_
#define _ERROR_CODES_
enum ErrorCode : uint8_t{
    //Error code to signal there was no error. Desired return.
    EC_None,
    //Directory error codes
    EC_DirNotFound,
    EC_DirReadNoPerm,
    EC_DirCreateNoPerm,
    EC_DirRemoveNoPerm,
    EC_DirEmpty,
    //File error codes
    EC_FileNotFound,
    EC_FileReadNoPerm,
    EC_FileWriteNoPerm,
    EC_FileRemoveNoPerm,
    EC_FileCopy,
    EC_FileCorrupted,
    EC_FileEmpty,
    EC_WrongFile,
    //Item error codes
    EC_ItemNotFound,
    EC_ItemFound,
    //ActionCancelled
    EC_UserCancelled
};
#endif

void InvokeFatalError(ErrorCode const error_code, const string error_at);
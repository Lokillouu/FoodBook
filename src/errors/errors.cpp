#include "errors.h"

void InvokeFatalError(const ErrorCode error_code, const string error_at){
    cout << "\nError ";
    if (!error_at.empty()){
        cout << "at " + error_at + ' ';
    }
    cout << "with code ";
    switch(error_code){
        case EC_DirNotFound:
            cout << "EC_DirNotFound.\n";
            break;
        case EC_DirReadNoPerm:
            cout << "EC_DirReadNoPerm.\n";
            break;
        case EC_DirCreateNoPerm:
            cout << "EC_DirCreateNoPerm.\n";
            break;
        case EC_DirRemoveNoPerm:
            cout << "EC_DirRemoveNoPerm.\n";
            break;
        case EC_DirEmpty:
            cout << "EC_DirEmpty.\n";
            break;
        case EC_FileNotFound:
            cout << "EC_FileNotFound.\n";
            break;    
        case EC_FileReadNoPerm:
            cout << "EC_FileReadNoPerm.\n";
            break;
        case EC_FileWriteNoPerm:
            cout << "EC_FileWriteNoPerm.\n";
            break;
        case EC_FileRemoveNoPerm:
            cout << "EC_FileRemoveNoPerm.\n";
            break;
        case EC_FileCopy:
            cout << "EC_FileCopy.\n";
            break;
        case EC_FileCorrupted:
            cout << "EC_FileCorrupted.\n";
            break;
        case EC_FileEmpty:
            cout << "EC_FileEmpty.\n";
            break;
        case EC_WrongFile:
            cout << "EC_WrongFile.\n";
            break;
        case EC_ItemNotFound:
            cout << "EC_ItemNotFound.\n";
            break;
        case EC_ItemFound:
            cout << "EC_ItemNotFound.\n";
            break;
        case EC_UserCancelled:
            cout << "EC_UserCancelled.\n";
            break;
        default:
            cout << "unknown.\n";
            break;
    }
    exit(1);
}
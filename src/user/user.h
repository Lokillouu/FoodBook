#include <iostream>
using namespace std;
#include <string>
#include <vector>
#include "../errors/errors.h"
#include "../io/io_fb.h"
using namespace io_fb;
#include "../date/date.h"

//User amount fits inside uint8_t. Be mindful about it if you want to bump this number! You might need to change some (uint8_t)s scattered among the codebase.
#define MAX_USERS 255

namespace user_lib {
#pragma region User Class
class user {
    #pragma region User
    //Public
    public:
    ErrorCode LoadUser(const string& usrname);
    void LogOut();
    ErrorCode DeleteUser();
    ErrorCode RestoreData();
    ErrorCode BackupFiles();
    string GetUserName();
    //Private
    private:
    string username = "";
    ErrorCode CreateUserFiles();
    ErrorCode CreateDailyData();
    #pragma endregion
    #pragma region Macros
    //Public
    public:
    ErrorCode PrintCurrentMacros(const uint8_t timeframe, const bool console_wait = 1);
    ErrorCode BrowseHistory();
    //Private
    private:
    ErrorCode PrintDateMacros(const uint8_t timeframe, date::s_date& date);
    #pragma endregion
    #pragma region Food
    //Public
    public:
    ErrorCode FoodBook();
    ErrorCode EatFood();
    ErrorCode ModifyFood(string* food = NULL);
    ErrorCode RegisterFood();
    ErrorCode RemoveFood();
    #pragma endregion
};
#pragma endregion
#pragma region Free Functions
ErrorCode RegisterNewUser(const string& name);
ErrorCode IsUsernameTaken(const string& name);
ErrorCode PrintAllUsers(vector<string>& all_users);
#pragma endregion
}
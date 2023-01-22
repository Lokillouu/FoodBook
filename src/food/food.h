#include <iostream>
using namespace std;
#include <string>
#include <vector>
#include "../errors/errors.h"
#include "../date/date.h"

namespace food {
#pragma region Macros
ErrorCode GetDateMacros(const string& username, vector<double>& macros, date::s_date& date_data);
ErrorCode GetYearMacros(const string& username, vector<double>& macros, date::s_date& date_data);
ErrorCode GetMonthMacros(const string& username, vector<double>& macros, date::s_date& date_data);
ErrorCode GetWeekMacros(const string& username, vector<double>& macros, date::s_date& date_data);
ErrorCode GetDayMacros(const string& username, vector<double>& macros, date::s_date& date_data);
#pragma endregion
#pragma region Food
ErrorCode PrintFoodList(const string& usr, vector<string>& foods);
ErrorCode InternalEatFood(const string& usr, const string& food, unsigned long& amount, bool portions_or_grams);
ErrorCode InternalRemoveFood(const string& usr, const string& food);
ErrorCode IsFoodRegistered(const string& usr, const string& food);
ErrorCode InternalModifyFood(const string& usr, const string& food_data);
ErrorCode InternalRegisterFood(const string& usr, const string& food);
ErrorCode GetFoodData(const string& usr, const string& food, vector<double>& macros);
void PrintMacro(const uint8_t macro_index);
#pragma endregion
}
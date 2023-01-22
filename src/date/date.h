#include <iostream>
using namespace std;
#include <string>

#ifndef _DATE_

#define _DATE_
namespace date {
#pragma region Date Data
//Enum of uint8_t type, going from 1 to 7.
enum wday_name : uint8_t{
    Monday = 1,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};
//Enum of uint8_t type, going from 1 to 12.
enum month_name : uint8_t{
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};
/**
 * @brief Small date struct that contains a date from year to week day.
 * @param year (int)
 * @param month (date::month_name) Enum of uint8_t type.
 * @param month_day (uint8_t)
 * @param week_day (date::wday_name) Enum of uint8_t type.
**/
typedef struct {
    int year;
    month_name month;
    uint8_t month_day;
    wday_name week_day;
} s_date;
#pragma endregion
#pragma region Free Functions
bool IsLeapYear(const int& year);
wday_name CalcDayOfWeek(int yr, uint8_t mth, uint8_t mth_day);
uint8_t GetMonthLength(const month_name month, const int& year);
string MonthToStr(const month_name month);
string WeekDayToStr(const wday_name wday);
#pragma endregion
#pragma region Calendar Class
/**
 * @brief Calendar class containing date functionality. It gets date from std::chrono::system_clock from time_t to tm struct. All values are corrected to natural feeling numbers and readable enums.
**/
class calendar {
    //Public functions
    public:
    calendar(){
        RefreshDate();
    }
    void PassDateToStruct(s_date& date_struct);
    void SetDate(const int& yr, const month_name mth, const uint8_t mth_day);
    void operator ++(int);
    void operator --(int);
    void RefreshDate();
    int GetYear();
    month_name GetMonth();
    uint8_t GetMonthDay();
    wday_name GetWeekDay();
    void PrintDate();

    //Private vars
    private:  
    int year;
    month_name month;
    uint8_t month_day;
    wday_name week_day;
    
    //Private functions
    private:
    void RollPreviousDate();
    void RollPosteriorDate();
};
#pragma endregion
}

#endif
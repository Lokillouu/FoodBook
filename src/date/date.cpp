#include <chrono>
using namespace std::chrono;
#include "date.h"
using namespace date;

#pragma region Free Functions
/**
 * @brief Checks if a given year is a leap year.
 * @param year Year to check.
 * @returns If leap year, it returns 1, else 0.
**/
bool date::IsLeapYear(const int& year){
    //If year % 4 is 0, keep checking
    if (year % 4 == 0){
        //If year % 100 is also 0, keep checking
        if (year % 100 == 0){
            //And if year % 400 is also 0, year is leap year.
            if (year % 400 == 0){
                return 1;
            }
            //Else, year is not a leap year.
            else {
                return 0;
            }
        }
        //Else, is a leap year
        else {
            return 1;
        }
    }
    //Else, not a leap year
    else {
        return 0;
    }
}
/**
 * @brief Calculates day of week for a given gregorian date.
 * @param yr In this year.
 * @param mth In this month.
 * @param mth_day In this day of the month.
 * @returns Enum of wday_name type containing the day of the week.
**/
wday_name date::CalcDayOfWeek(int yr, uint8_t mth, uint8_t mth_day){
    static uint8_t t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    yr -= mth < 3;
    uint8_t raw_result = (yr + yr / 4 - yr / 100 + yr / 400 + t[mth - 1] + mth_day) % 7;
    raw_result = raw_result == 0 ? 7 : raw_result;
    return static_cast<wday_name>(raw_result);
}
/**
 * @brief Get month length in days, accounting for leap years.
 * @param month Target month.
 * @param year Year the month belongs to.
 * @returns Number of days the month has, from 1 up to 31.
**/
uint8_t date::GetMonthLength(const month_name month, const int& year){
    //If month is February
    if (month == 2){
        //If inside a leap year, February has 29 days. Return.
        if (IsLeapYear(year)){
            return 29;
        }
        //Else, February has 28 days. Return.
        else {
            return 28;
        }
    }
    //If not February and less than August (January to July).
    else if (month < 8){
        //Month has 31 days, return
        if (month % 2 == 1){
            return 31;
        }
        //Month has 30 days, return
        else {
            return 30;
        }
    }
    //If not February and August to December.
    else {
        //Month has 31 days, return
        if (month % 2 == 0){
            return 31;
        }
        //Month has 30 days, return
        else {
            return 30;
        }
    }
}
/**
 * @brief Transforms a wday_name enum to a string.
 * @param wday Week day enum to transform.
 * @returns String containing name of the given day of the week.
**/
string date::WeekDayToStr(const wday_name wday){
    switch(wday){
        case Monday:
            return "Monday";
        case February:
            return "Tuesday";
        case Wednesday:
            return "Wednesday";
        case Thursday:
            return "Thursday";
        case Friday:
            return "Friday";
        case Saturday:
            return "Saturday";
        case Sunday:
            return "Sunday";
    }
}
/**
 * @brief Transforms a month_name enum to a string.
 * @param month Month enum to transform.
 * @returns String containing name of the given month.
**/
string date::MonthToStr(const month_name month){
    switch(month){
        case January:
            return "January";
        case February:
            return "February";
        case March:
            return "March";
        case April:
            return "April";
        case May:
            return "May";
        case June:
            return "June";
        case July:
            return "July";
        case August:
            return "August";
        case September:
            return "September";
        case October:
            return "October";
        case November:
            return "November";
        case December:
            return "December";
    }
}
#pragma endregion
#pragma region Calendar Class
/**
 * @brief Get saved date and copy its values to a given date_struct.
 * @param date_struct Small date struct to set date to.
**/
void calendar::PassDateToStruct(s_date& date_struct){
    date_struct.year = GetYear();
    date_struct.month = GetMonth();
    date_struct.month_day = GetMonthDay();
    date_struct.week_day = GetWeekDay();
    return;
}
/**
 * @brief Set date to the specified one.
 * @param yr New year.
 * @param mth New month.
 * @param mth_day New month day (may be corrected).
 * @warning If it is February 29 and not a leap year, month day will be corrected to 28.
**/
void calendar::SetDate(const int& yr, const month_name mth, const uint8_t mth_day){
    //Set date.
    year = yr;
    month = mth;
    month_day = mth_day;
    //If it is February 29, see if we have to reverse to 28.
    if (month == February && month_day == 29){
        if (!IsLeapYear(year)){
            month_day = 28;
        }
    }
    //Calculate week day.
    week_day = static_cast<wday_name>(CalcDayOfWeek(year, month, month_day));
    return;
    }
/**
 * @brief Set saved date one day forward.
**/
void calendar::operator ++(int){
    RollPosteriorDate();
}
/**
 * @brief Set saved date one day back.
**/
void calendar::operator --(int){
    RollPreviousDate();
}
/**
 * @brief Set saved date to current real life date (refresh date).
**/
void calendar::RefreshDate(){
    //Get current date from chrono.
    time_t epoch_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm* time_local = localtime(&epoch_time);
    //Correct year difference and set it.
    year = time_local->tm_year + 1900;
    //Correct month difference and set it.
    month = static_cast<month_name>(time_local->tm_mon + 1);
    //Set month day
    month_day = time_local->tm_mday;
    //Correct week day difference and set it.
    week_day = time_local->tm_wday == 0 ? wday_name::Sunday : static_cast<wday_name>(time_local->tm_wday);
    return;
}
/**
 * @brief Get year the calendar is currently set at.
 * @returns Currently set year (int).
 * @warning Do not confuse this year for the current real year. This is the saved year, not the refreshed year (or actual year IRL).
**/
int calendar::GetYear(){
    return year;
}
/**
 * @brief Get month the calendar is currently set at.
 * @returns Currently set month (date::month_name).
 * @warning Do not confuse this month for the current real month. This is the saved month, not the refreshed month (or actual month IRL).
**/
month_name calendar::GetMonth(){
    return month;
}
/**
 * @brief Get month day the calendar is currently set at.
 * @returns Currently set month day (uint8_t).
 * @warning Do not confuse this month day for the current real month day. This is the saved month day, not the refreshed month day (or actual month day IRL).
**/
uint8_t calendar::GetMonthDay(){
    return month_day;
}
/**
 * @brief Get week day the calendar is currently set at.
 * @returns Currently set week day (wday_name).
 * @warning Do not confuse this week day for the current real week day. This is the saved week day, not the refreshed week day (or actual week day IRL).
**/
wday_name calendar::GetWeekDay(){
    return week_day;
}
/**
 * @brief Print currently set date.
 * @warning Do not confuse this date for the real date.
**/
void calendar::PrintDate(){
    cout << WeekDayToStr(week_day) << ' ' + to_string(month_day) + " of ";
    cout << MonthToStr(month) + " of " << year << '\n';
    return;
}
/**
 * @brief Advances saved date one day into the future.
**/
void calendar::RollPosteriorDate(){
    //If weekday is Sunday, set it to Monday.
    if (week_day == wday_name::Sunday){
        week_day = wday_name::Monday;
    }
    //Else, add one to the saved week day.
    else {
        week_day = static_cast<wday_name>(week_day + 1);
    }
    //If at last day of december, change year
    if (month == 12 && month_day == 31){
            month = month_name::January;
            month_day = 1;
            year++;
        }
    //In same year
    else {
        //If end of current month, set month day to 1 and add 1 to saved month.
        if (GetMonthLength(month, year) == month_day){
            month = static_cast<month_name>(month + 1);
            month_day = 1;
        }
        //Else, add one to the month day.
        else {
            month_day++;
        }
    }
    return;
}
/**
 * @brief Rolls saved date one day into the past.
**/
void calendar::RollPreviousDate(){
    //If weekday is Monday, set it to Sunday.
    if (week_day == wday_name::Monday){
        week_day = wday_name::Sunday;
    }
    //Else, subtract one to the saved week day.
    else {
        week_day = static_cast<wday_name>(week_day - 1);
    }
    //If first month day, change month.
    if (month_day == 1){
        switch(month){
            //If January, we have to change year
            case month_name::January:{
                    month = month_name::December;
                    month_day = 31;
                    year--;
                    break;
                }
            //Change month in the same year
            default:{
                month = static_cast<month_name>(month - 1);
                month_day = GetMonthLength(month, year);
                break;
            }
        }
    }
    //Else, subtract one to the saved month day.
    else {
        month_day--;
    }
    return;
}
#pragma endregion

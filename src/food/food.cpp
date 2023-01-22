#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include "food.h"
#include "../io/io_fb.h"
using namespace io_fb;
#include "../filemanager/filemanager.h"
namespace fm = filemanager;

static date::calendar c_calendar;

#pragma region Food
/**
 * @brief Prints all foods inside user_foods.dat and retrives them inside a string vector.
 * @param usr User to read foods from.
 * @param foods Vector that will contain all found food names. Mandatory.
 * @returns Possible ErrorCodes: EC_FileReadNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by filemanager::UserFoodsDatCheck();
 * @warning This function DOES validate user_foods.dat.
**/
ErrorCode food::PrintFoodList(const string& usr, vector<string>& foods){
    //Validate user_foods.dat
    fs::path foods_p = foods_dat(usr);
    ErrorCode ec = fm::UserFoodsDatCheck(foods_p);
    //If there was a problem, return error.
    if (ec != EC_None){
        return ec;
    }
    //Open foods.dat
    ifstream data_in;
    data_in.open(foods_p);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Retrieve all foods
    string food;
    while(getline(data_in, food, '|')){
        //If not at the end of file
        if (!data_in.eof()){
            //Remove brackets
            strings::RemoveBrackets(food);
            //Get food name
            food = food.substr(0,food.find_first_of('/'));
            //Save food
            foods.push_back(food);
            //Print food
            cout << '[' << to_string(foods.size()) << ']'; 
            cout << name::InFileNameToName(food,1) << '\n';
        }
    }
    data_in.close();
    return EC_None;
}
/**
 * @brief Get food macros for the given food.
 * @param usr User to target.
 * @param food Food to remove (food name).
 * @param macros Vector of doubles to store macros. It will be cleared and resized accordingly.
 * @returns Possible ErrorCodes: EC_ItemNotFound; EC_FileReadNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of this functions: fm::UserFoodsDatCheck();
 * @warning food string must be an in-file name.
**/
ErrorCode food::GetFoodData(const string& usr, const string& food, vector<double>& macros){
    //Validate user_foods.dat
    fs::path foodsdat = foods_dat(usr);
    ErrorCode ec = fm::UserFoodsDatCheck(foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Open file
    ifstream data_in;
    data_in.open(foodsdat);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Search for food
    string data;
    while (getline(data_in, data, '|')){
        //Remove brackets
        strings::RemoveBrackets(data);
        //If string starts with food name, we found it.
        if (data.starts_with(food)){
            //Prepare macros vector
            macros.clear();
            macros.resize(NUM_OF_MACROS + 1); //Get macros + portion size
            //Remove food name and name separator from data.
            data.erase(0,data.find_first_of('/') + 1);
            //Extract food data
            for (uint8_t i = 0; i < NUM_OF_MACROS + 1; i++){
                //Get macro value
                macros[i] = stod(data.substr(0, data.find_first_of('/')));
                //Remove macro from data if not at last index
                if (i + 1 < NUM_OF_MACROS + 1){
                    data.erase(0, data.find_first_of('/') + 1);
                }
            }
            //Macros have been saved, close file and return.
            data_in.close();
            return EC_None;
        }
    }
    //Food was not found, close and return
    data_in.close();
    return EC_ItemNotFound;
}
/**
 * @brief Checks if food is registered in user_food.dat. Must be a valid in-file string.
 * @param usr User to target.
 * @param food Food to remove (food name).
 * @returns Possible ErrorCodes: EC_ItemNotFound; EC_FileReadNoPerm; EC_ItemFound;
 * @warning usr_foods.dat is not validated in this function. Make sure to do it before calling it.
**/
ErrorCode food::IsFoodRegistered(const string& usr, const string& food){
    //Open food data, exit if read is forbidden.
    fs::path foodsdat = foods_dat(usr);
    ifstream data_in;
    data_in.open(foodsdat);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Check if food is registered, exit if so.
    string data;
    while (getline(data_in, data, '|')){
        //If line is not empty
        if (!data.empty()){
            //Isolate food name
            data = data.substr(1, data.find_first_of('/') - 1);
            //See if data string is equal to desired food name
            if (data == food){
                data_in.close();
                return EC_ItemFound;
            }
        }
    }
    //Food does not exist, exit with EC_ItemNotFound    
    data_in.close();
    return EC_ItemNotFound;
}
/**
 * @brief Adds macros to the current day.
 * @param usr User to target.
 * @param food Food to add macros from (food to eat).
 * @param amount Amount of food to eat, specified in portions or grams (see boolean).
 * @param portions_or_grams Choose the counting option. 0 for portions, 1 for grams. Portions will multiply the food macros by the stored portion size times amount of portions. Grams will multiply macros by amount of grams.
 * @warning This function does NOT directly check if usr_foods.dat is valid or if food is registered. This is done by calling GetFoodData(). If you wish to modify this function, keep that in mind. Food string must be an in-file name.
 * @returns Possible ErrorCodes: EC_FileWriteNoPerm; EC_FileReadNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of these functions: GetFoodData(); fm::DayDataCheck(); (fm::CreateTempFile(); fm::SafeDeleteFile();
**/
ErrorCode food::InternalEatFood(const string& usr, const string& food, unsigned long& amount, bool portions_or_grams){
    //Try to get food data
    vector<double> macros;
    ErrorCode ec = GetFoodData(usr,food,macros);
    if (ec != EC_None){
        return ec;
    }
    //If portion count is chosen, multiply every macro by the amount of portions.
    if (portions_or_grams){
        for (double &macro : macros){
           macro *= (macros[NUM_OF_MACROS] * (double)amount);
        }
    }
    //Else mutiply every macro by the amount of grams
    else {
        for (double &macro : macros){
            macro *= (double)amount;
        }
    }
    //Refresh date and get today date.
    c_calendar.RefreshDate();
    date::s_date t_date;
    c_calendar.PassDateToStruct(t_date);
    //Get data file for today
    fs::path daydat = fm::GetDateDataPath(usr, t_date);
    //If daily data does not exists, create it.
    ofstream data_out;
    if (!fs::exists(daydat)){
        //Create file.
        data_out.open(daydat);
        if (!data_out.is_open()){
            return EC_FileWriteNoPerm;
        }
        for (uint8_t m = 0; m < NUM_OF_MACROS; m++){
            data_out << "{0.0}|";
        }
        data_out.close();
    }
    //Else, validate it.
    else {
        ec = fm::DayDataCheck(daydat);
        if (ec != EC_None){
            return ec;
        }
    }
    //Open today data file
    ifstream data_in;
    data_in.open(daydat);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Get current macros and add them with the food macros. Skip portion size.
    string data;
    for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
        getline(data_in, data, '|');
        strings::RemoveBrackets(data);
        macros[i] += stod(data);
    }
    data_in.close();
    //Create temp file
    fs::path tmp_daydat;
    ec = fm::CreateTempFile(daydat, tmp_daydat);
    if (ec != EC_None){
        return ec;
    }
    //Open temp file (read)
    data_in.open(tmp_daydat);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Open original file (write)
    data_out.open(daydat);
    if (!data_out.is_open()){
        return EC_FileWriteNoPerm;
    }
    //Insert added macros, skipping portion size and _END_ termination.
    for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
        data_out << strings::DataToFile(to_string(macros[i]));
    }
    //Close files
    data_in.close();
    data_out.close();
    //Remove temp file
    ec = fm::SafeDeleteFile(tmp_daydat);
    if (ec != EC_None){
        return ec;
    }
    //Return
    return EC_None;
}
/**
 * @brief Removes food from user database.
 * @param usr User to target.
 * @param food Food to remove (food name).
 * @returns Possible ErrorCodes: EC_ItemNotFound; EC_FileReadNoPerm; EC_FileWriteNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of these functions: fm::UserFoodsDatCheck(); fm::CreateTempFile(); fm::SafeDeleteFile(); IsFoodRegistered();
 * @warning food string must be an in-file name.
**/
ErrorCode food::InternalRemoveFood(const string& usr, const string& food){
    //Validate user_foods.dat
    fs::path foodsdat = foods_dat(usr);
    ErrorCode ec = fm::UserFoodsDatCheck(foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Check if food is registered
    ec = IsFoodRegistered(usr, food);
    if (ec == EC_None){
        return EC_ItemNotFound;
    }
    else if (ec != EC_ItemFound){
        return ec;
    }
    //Create temp file
    fs::path tmp_foodsdat;
    ec = fm::CreateTempFile(foodsdat, tmp_foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Open temp file (read)
    ifstream data_in;
    data_in.open(tmp_foodsdat);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Open original file (write)
    ofstream data_out;
    data_out.open(foodsdat);
    if (!data_out.is_open()){
        data_in.close();
        return EC_FileWriteNoPerm;
    }
    //Dump data skipping selected food
    string data;
    while(getline(data_in, data, '|')){
        //If not at the _END_ line
        if (data != "_END_"){
            //Remove brackets from data
            strings::RemoveBrackets(data);
            //If not at the wanted food line, restore it. Else, just skip it.
            if (!data.starts_with(food)){
                data_out << strings::DataToFile(data);
            }
        }
    }
    //Close files
    data_in.close();
    data_out.close();
    //Remove temp file
    ec = fm::SafeDeleteFile(tmp_foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Return
    return EC_None;
}
/**
 * @brief Modify food macros in user database.
 * @param usr User to target.
 * @param food_data Food data string to insert.
 * @returns Possible ErrorCodes: EC_FileReadNoPerm; EC_FileWriteNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of these functions: fm::UserFoodsDatCheck(); fm::CreateTempFile(); fm::SafeDeleteFile();
 * @warning food_data string should come correctly formatted into a generic data string.
 * @exception Possible exceptions if data is manipulated after the file is validated.
**/
ErrorCode food::InternalModifyFood(const string& usr, const string& food_data){
    fs::path foodsdat = foods_dat(usr);
    //Validate user_foods.dat
    ErrorCode ec = fm::UserFoodsDatCheck(foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Create temp file
    fs::path tmp_foodsdat;
    ec = fm::CreateTempFile(foodsdat, tmp_foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Open original file (write)   
    ofstream data_out;
    data_out.open(foodsdat);
    if (!data_out.is_open()){
        return EC_FileWriteNoPerm;
    }
    //Open temp data (read)
    ifstream data_in;
    data_in.open(tmp_foodsdat);
    if (!data_in.is_open()){
        data_out.close();
        return EC_FileReadNoPerm;
    }
    //Copy data and replace food string when found
    string data, food_name = food_data.substr(1,food_data.find_first_of('/'));
    while (getline(data_in, data, '|')){
        //First, make sure we are not at the _END_ line of the temp file.
        if (data != "_END_"){
            //Remove brackets
            strings::RemoveBrackets(data);
            //If current line starts with the wanted food name, input the new food data.
            if (data.starts_with(food_name)){
                data_out << food_data;
            }
            //Else, input the original food data.
            else {
                data_out << strings::DataToFile(data);
            }
        }
    }
    //Close all
    data_in.close();
    data_out.close();
    //Remove temp file
    ec = fm::SafeDeleteFile(tmp_foodsdat);
    if (ec != EC_None){
        return ec;
    }
    //Return
    return EC_None;
}
/**
 * @brief Register a new food in user_foods.dat.
 * @param usr User to target.
 * @param food_data Food data string to insert.
 * @returns Possible ErrorCodes: EC_FileWriteNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of these functions: fm::UserFoodsDatCheck(); fm::CreateTempFile(); fm::SafeDeleteFile();
 * @warning food_data string should come correctly formatted into a generic data string.
 * @exception Possible exceptions if data is manipulated after the file is validated.
**/
ErrorCode food::InternalRegisterFood(const string& usr, const string& food_data){
    //Validate user_foods.dat
    bool file_empty = 0;
    fs::path foods = foods_dat(usr);
    fs::path* tmp_foods;
    ErrorCode ec = fm::UserFoodsDatCheck(foods);
    //If there was an error, return
    if (ec != EC_None && ec != EC_FileEmpty){
        return ec;
    }
    //Prepare data_out
    ofstream data_out;
    //If file is not empty, create temp file.
    if (ec != EC_FileEmpty){
        //Create temp file
        tmp_foods = new fs::path;
        ec = fm::CreateTempFile(foods, *tmp_foods);
        if (ec != EC_None){
            return ec;
        }
        //Open data in app mode
        data_out.open(foods, ios_base::app);        
    }
    //Else, skip temp file
    else {
        file_empty = 1;
        //Open data
        data_out.open(foods); 
    }
    //If data is not open, exit with error.
    if (!data_out.is_open()){
        if (!file_empty){
            delete tmp_foods;
        }
        return EC_FileWriteNoPerm;
    }
    //Input food string
    data_out << food_data;
    //Close file.
    data_out.close();
    //If temp file was created, remove it.
    if (!file_empty){
        ec = fm::SafeDeleteFile(*tmp_foods);
        delete tmp_foods;
        if (ec != EC_None){
            return ec;
        }
    }
    //Return
    return EC_None;
}
/**
 * @brief Print macro label from macro index.
 * @param macro_index Macro index to print. Consult macro indexes inside function switch.
**/
void food::PrintMacro(const uint8_t macro_index){
    switch (macro_index){
        case 0:
            cout << "Calories(Kcal): ";
            break;
        case 1:
            cout << "Carbohydrates: ";
            break;
        case 2:
            cout << "Sugars: ";
            break;
        case 3:
            cout << "Protein: ";
            break;
        case 4:
            cout << "Fats: ";
            break;
        case (NUM_OF_MACROS):
            cout << "Portion size: ";
            break;
        default:
            cout << "Macro not found.";
            break;
    }
    return;
}
#pragma endregion
#pragma region Macros
/**
 * @brief Get macros for the given date and user.
 * @param username Name of the user to search.
 * @param macros Provide a vector of doubles to store found macros.
 * @param date_data Struct of date::s_date type.
 * @returns Possible ErrorCodes: EC_FileReadNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of these functions: fm::DayDataCheck();
 * @exception Possible exception when transforming string to double with stod(). File gets validated before loop operation, but any data manipulation while the loop is running can cause stod() to fail and crash the program.
 * @warning The function does NOT check if the file exists or the path makes sense. It just builds an untested path with the given information.
**/
ErrorCode food::GetDateMacros(const string& username, vector<double>& macros, date::s_date& date_data){
    //Clear macros vector
    macros.clear();
    //Get path to data
    fs::path data_p = fm::GetDateDataPath(username, date_data);
    //Validate file
    ErrorCode ec = fm::DayDataCheck(data_p);
    //If file is not found, set all macros to 0 and return.
    if (ec == EC_FileNotFound){
        for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
            macros.push_back(0);
        }
        return EC_None;
    }
    //If there was a problem, return error.
    else if (ec != EC_None){
        return ec;
    }
    //Open data file
    ifstream data_in;
    data_in.open(data_p);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Get data
    string data;
    for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
        //Get data
        getline(data_in, data, '|');
        //Remove brackets
        strings::RemoveBrackets(data);
        //Insert data
        macros.push_back(stod(data));
    }
    //Close and return
    data_in.close();
    return EC_None;
}
/**
 * @brief Get macros for the given year. Provide desired year inside struct.
 * @param username Name of the user to search.
 * @param macros Provide a vector of doubles to store found macros.
 * @param date_data Struct of date::s_date type. Month, month day & week day will be ignored (See warning).
 * @returns Possible ErrorCodes: EC_None;
 * @returns [OR] ErrorCodes thrown by food::GetDateMacros();
 * @warning Month & month day will be calculated automatically to the last day of December.
**/
ErrorCode food::GetYearMacros(const string& username, vector<double>& macros, date::s_date& date_data){
    //Clear and initialize macros vector
    macros.clear();
    for (uint8_t t = 0; t < NUM_OF_MACROS; t++){
        macros.push_back(0);
    }
    //Save desired year
    int target_yr = date_data.year;
    //Set desired date
    date_data.month = date::month_name::December;
    date_data.month_day = 31;
    c_calendar.SetDate(date_data.year, date_data.month, date_data.month_day);
    //Year loop
    while (c_calendar.GetYear() == target_yr){
        //Declare transitory vector
        vector<double> t_macros;
        //Pass date to struct
        c_calendar.PassDateToStruct(date_data);
        //Get daily macros
        ErrorCode ec = GetDateMacros(username, t_macros, date_data);
        switch (ec){
            //No data found, skip
            case EC_ItemNotFound:
                break;
            //Sum daily macros
            case EC_None:
                for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                    macros[i] += t_macros[i];
                }
                break;
            //Error
            default:
                macros.clear();
                return ec;
        }
        //Date back one day
        c_calendar--;
        //Clear temp macros
        t_macros.clear();
    }
    //Refresh date and return
    c_calendar.RefreshDate();
    return EC_None;
}
/**
 * @brief Get macros for the given month in given year. Provide desired month & year inside struct.
 * @param username Name of the user to search.
 * @param macros Provide a vector of doubles to store found macros.
 * @param date_data Struct of date::s_date type. Month day & week day will be ignored (See warning).
 * @returns Possible ErrorCodes: EC_None;
 * @returns [OR] ErrorCodes thrown by food::GetDateMacros();
 * @warning Month day will be calculated automatically to the last day of the month.
**/
ErrorCode food::GetMonthMacros(const string& username, vector<double>& macros, date::s_date& date_data){
    //Clear and initialize macros vector
    macros.clear();
    for (uint8_t t = 0; t < NUM_OF_MACROS; t++){
        macros.push_back(0);
    }
    //Saved desired month
    date::month_name target_month = date_data.month;
    //Calculate last month day
    date_data.month_day = date::GetMonthLength(date_data.month, date_data.year);
    //Set desired date    
    c_calendar.SetDate(date_data.year, date_data.month, date_data.month_day);
    //Month day loop
    while (c_calendar.GetMonth() == target_month){
        //Declare transitory vector
        vector<double> t_macros;
        //Pass date to struct
        c_calendar.PassDateToStruct(date_data);
        //Get daily macros
        ErrorCode ec = GetDateMacros(username, t_macros, date_data);
        switch (ec){
            //No data found, skip
            case EC_ItemNotFound:
                break;
            //Sum daily macros
            case EC_None:
                for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                    macros[i] += t_macros[i];
                }
                break;
            //Error
            default:
                macros.clear();
                return ec;
        }
        //Date back one day
        c_calendar--;
        //Clear temp macros
        t_macros.clear();
    }
    //Refresh date and return.
    c_calendar.RefreshDate();
    return EC_None;
}
/**
 * @brief Get macros for the given week in given date. Provide desired day, month & year inside struct.
 * @param username Name of the user to search.
 * @param macros Provide a vector of doubles to store found macros.
 * @param date_data Struct of date::s_date type. Week day will be ignored (See warning).
 * @returns Possible ErrorCodes: EC_None;
 * @returns [OR] ErrorCodes thrown by food::GetDateMacros();
 * @warning The week will be the same as the given day's week. Month day will be corrected to point to the next Sunday (if not Sunday already).
**/
ErrorCode food::GetWeekMacros(const string& username, vector<double>& macros, date::s_date& date_data){
    //Clear and initialize macros vector
    macros.clear();
    for (uint8_t t = 0; t < NUM_OF_MACROS; t++){
        macros.push_back(0);
    }
    //Set desired date
    c_calendar.SetDate(date_data.year, date_data.month, date_data.month_day);    
    //Get week day
    date_data.week_day = c_calendar.GetWeekDay();
    //Forward date until Sunday (if needed).
    for (uint8_t t = 7 - date_data.week_day; t > 0; t--){
        c_calendar++;
    }
    //Week day loop
    for (uint8_t d = 7; d > 0; d--){
        //Declare transitory vector
        vector<double> t_macros;
        //Pass date to struct
        c_calendar.PassDateToStruct(date_data);
        //Get daily macros
        ErrorCode ec = GetDateMacros(username, t_macros, date_data);
        switch (ec){
            //No data found, skip
            case EC_ItemNotFound: {
                break;
            }
            //Sum daily macros
            case EC_None: {
                for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                    macros[i] += t_macros[i];
                }
                break;
            }
            //Error
            default: {
                macros.clear();
                return ec;
            }
        }
        //Go back one day
        c_calendar--;
        //Clear temp macros
        t_macros.clear();
    }
    //All done, refresh date.
    c_calendar.RefreshDate();
    return EC_None;
}
/**
 * @brief Get macros for the given day in given date. Provide desired day, month & year inside struct.
 * @param username Name of the user to search.
 * @param macros Provide a vector of doubles to store found macros.
 * @param date_data Struct of date::s_date type. Week day will be ignored (see warning).
 * @returns ErrorCodes thrown by food::GetDateMacros();
 * @warning Week day is calculated automatically.
**/
ErrorCode food::GetDayMacros(const string& username, vector<double>& macros, date::s_date& date_data){
    //Set desired week day
    date_data.week_day = date::CalcDayOfWeek(date_data.year, date_data.month, date_data.month_day);
    //Get daily macros and return
    return GetDateMacros(username, macros, date_data);
}
#pragma endregion
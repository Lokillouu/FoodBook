#include <filesystem>
#include <fstream>
#include <chrono>
using namespace std::chrono;
#include "user.h"
#include "../food/food.h"
#include "../filemanager/filemanager.h"

#pragma region User Class
    #pragma region User
    /**
     * @brief Asks the user for a backup directory from where to restore the current user files. If the same file is found, it will be replaced by the backed up.
     * @returns Possible ErrorCodes: EC_UserCancelled; EC_DirNotFound; EC_None;
     * @exception Possible exception error from filesystem::copy() unhandled.
     * @exception Confirmed exception when back up directory is the same as the user directory (from & to are the same).
    **/
    ErrorCode user_lib::user::RestoreData(){
        filesystem::path from, to = user_folder(username);
        //Get valid backup path
        do {
            ClearConsole;
            //Get path input
            cout << "Please input a backup directory or leave empty to cancel:\n";
            string tmp_str;
            if (!input::GetStringInput(SM_Dir,tmp_str)){
                return EC_UserCancelled;
            }
            from = tmp_str;
            from.append(username);
            //Path does not exist or is not a directory
            if (!filesystem::exists(from)){
                cout << "Directory doesn't exist or contain ";
                cout << name::InFileNameToName(username, 2) << " folder. Try again.\n";
                input::ConsoleWait();
            }
            else if (!filesystem::is_directory(from)){
                cout << "Specify a directory, not a file. Try again.\n";
                input::ConsoleWait();
            }
            else if (from == to){
                cout << "Can't restore from user folder. Specify an external backup path.\n";
                input::ConsoleWait();
            }
            else {
                break;
            }
        } while(true);
        //Restore data
        ClearConsole;
        cout << "Please wait while the restore is being done...\n";
        filesystem::copy(from, to, filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
        cout << "Data restored successfully.\n";
        input::ConsoleWait();
        return EC_None;
    }
    /**
     * @brief Asks user for an empty backup directory. Once the path is valid, it creates the backup of the current user folder.
     * @returns Possible ErrorCodes: EC_UserCancelled; EC_None;
     * @returns [OR] ErrorCodes thrown by any of this functions: filemanager::SafeDeleteFile();
     * @exception Possible exception error from filesystem::copy() unhandled.
    **/
    ErrorCode user_lib::user::BackupFiles(){
        filesystem::path to;
        //Get valid backup path
        do {
            ClearConsole;
            //Get path input
            cout << "Please input a backup directory or leave empty to cancel:\n";
            string tmp_str;
            if (!input::GetStringInput(SM_Dir,tmp_str)){
                return EC_UserCancelled;
            }
            to = tmp_str;
            //Path does not exist or is not a directory
            if (!filesystem::exists(to)){
                cout << "Directory doesn't exist. Try again.\n";
                input::ConsoleWait();
            }
            else if (!filesystem::is_directory(to)){
                cout << "Specify a directory, not a file. Try again.\n";
                input::ConsoleWait();
            }
            //Path is not empty
            else if (!filesystem::is_empty(to)){
                cout << "Directory is not empty. Try again.\n";
                input::ConsoleWait();
            }
            //Test path permission
            else {
                filesystem::path test = to;
                test.append(".test.test");
                ofstream data_out;
                data_out.open(test);
                //No permission, retry
                if (!data_out.is_open()){
                    cout << "Cannot write to directory(permission denied). Try again.\n";
                    input::ConsoleWait();
                }
                //We have permission to write, delete test file and continue
                else {
                    data_out.close();
                    ErrorCode ec = filemanager::SafeDeleteFile(test);
                    if (ec != EC_None){
                        return ec;
                    }
                    break;
                }
            }
        } while(true);
        //Do the copy
        ClearConsole;
        cout << "Please wait while the copy is being done. Do not close the program.\n";
        filesystem::copy(user_folder(username), to.append(username), filesystem::copy_options::recursive);
        cout << "Copy successfully done.\n";
        input::ConsoleWait();
        return EC_None;
    }  
    /**
     * @brief Creates all user files and folders for the current date (Daily data and user foods data).
     * @warning It DOES refresh the common date object.
     * @returns Possible ErrorCodes: EC_FileWriteNoPerm; EC_None;
     * @returns [OR] ErrorCodes thrown by any of this functions: CreateDailyData();
    **/
    ErrorCode user_lib::user::CreateUserFiles(){
        date::s_date date;
        //Get current date and destroy object.
        date::calendar* t_calendar = new date::calendar();
        t_calendar->PassDateToStruct(date);
        delete t_calendar;
        //Create daily data and daily folder struct
        if (!filesystem::exists(filemanager::GetDateDataPath(username, date))){
            ErrorCode ec = CreateDailyData();
            if (ec != EC_None){
                return ec;
            }
        }
        //Create personal food
        if (!filesystem::exists(foods_dat(username))){
            ofstream data_out;
            data_out.open(foods_dat(username));
            if (!data_out.is_open()){
                return EC_FileWriteNoPerm;
            }
            data_out.close();
        }
        return EC_None;
    }
    /**
     * @brief Creates day macro file and folder structure if missing.
     * @warning It DOES refresh the common date object.
     * @returns Possible ErrorCodes: EC_FileWriteNoPerm; EC_DirCreateNoPerm; EC_None;
    **/
    ErrorCode user_lib::user::CreateDailyData(){
        //Create date struct
        date::s_date date;
        //Get current date and destroy object.
        date::calendar* t_calendar = new date::calendar();
        t_calendar->PassDateToStruct(date);
        delete t_calendar;
        //If file (and as such, folder structure) do EXIST, exit to avoid overriding it.
        filesystem::path path = filemanager::GetDateDataPath(username, date);
        if (filesystem::exists(path)){
            return EC_None;
        }
        //If folder structure does not exist, create it.
        if (!filesystem::exists(path.parent_path())){
            if (!filesystem::create_directories(path.parent_path())){
                return EC_DirCreateNoPerm;
            }
        }
        //Create file.
        ofstream data_out;
        data_out.open(path);
        if (!data_out.is_open()){
            return EC_FileWriteNoPerm;
        }
        for (uint8_t m = 0; m < NUM_OF_MACROS; m++){
            data_out << "{0.0}|";
        }
        //Close and return.
        data_out.close();
        return EC_None;
    }
    /**
     * @brief Load user into this object. Username is transformed into an in-file name inside this function.
     * @param usrname User to log in. Will be transformed into in-file name, in case it isn't.
     * @returns ErrorCodes thrown by private function CreateUserFiles();
    **/
    ErrorCode user_lib::user::LoadUser(const string& usrname){
        username = name::NameToInFileName(usrname);
        return CreateUserFiles();
    };
    /**
     * @brief Clear user name internally.
    **/
    void user_lib::user::LogOut(){
        username.clear();
        return;
    }
    /**
     * @brief Returns user name in user friendly fashion.
     * @returns User friendly name string.
    **/
    string user_lib::user::GetUserName(){
        return name::InFileNameToName(username, 2);
    };
    /**
     * @brief Delete user process. The function will ask the user for confirmation and instructions. It can optionally backup user files. After confirmation and possible backup are done, user files are deleted and user name is removed from users.dat.
     * @returns Possible returns: EC_UserCancelled; EC_FileReadNoPerm; EC_FileWriteNoPerm; EC_None;
     * @returns [OR] ErrorCodes thrown by any of this functions: BackupFiles(); filemanager::SafeDeleteFolder(); filemanager::SafeDeleteFile(); filemanager::UsersDataCheck(); filemanager::CreateTempFile();
    **/
    ErrorCode user_lib::user::DeleteUser(){
        //Confirm deletion
        do {
            ClearConsole;
            cout << "This will remove the user from the database.\n";
            cout << "If you want to continue, write DELETE in caps.\n";
            cout << "Write DELETE or leave it empty to cancel: ";
            string input_str;
            //If user cancels, return.
            if (!input::GetStringInput(SM_Command, input_str)){
                return EC_UserCancelled;
            }
            //If user writes delete, break.
            else if (input_str == "DELETE"){
                break;
            }
        } while(true);
        //Give option to backup files
        ErrorCode ec;
        do {
            ClearConsole;
            cout << "Do you want to backup your data files?\n";
            cout << "1.Yes\n2.No\n\n";
            uint8_t num_input = 0;
            //If user cancels, return.
            if (!input::GetNumericInput(&num_input, Mode_UInt8)){
                return EC_UserCancelled;
            }
            //Backup files
            else if (num_input == 1){
                ec = BackupFiles();
                //If backup failed, return error.
                if (ec != EC_None){
                    return ec;
                }
                break;
            }
            //Delete without backup.
            else if (num_input == 2){
                break;
            }
        } while(true);
        //Delete user files
        filesystem::path mut_path = user_folder(username);
        ec = filemanager::SafeDeleteFolder(mut_path);
        if (ec != EC_None){
            return ec;
        }
        //Check if users.dat is valid
        ec = filemanager::UsersDataCheck();
        if (ec != EC_None){
            return ec;
        }
        //Create temp file
        mut_path = users_dat_p;
        filesystem::path usersdat_tmp;
        ec = filemanager::CreateTempFile(mut_path, usersdat_tmp);
        if (ec != EC_None){
            return ec;
        }
        //Open temp file to read
        ifstream data_in;
        data_in.open(usersdat_tmp);
        if (!data_in.is_open()){
            return EC_FileReadNoPerm;
        }
        //Open original file to modify
        ofstream data_out;
        data_out.open(mut_path);
        if(!data_out.is_open()){
            data_in.close();
            return EC_FileWriteNoPerm;
        }
        //Pass data skipping target name
        string data;
        while (getline(data_in, data, '|')){
            //If data is not _END_
            if (data != "_END_"){
                //Remove brackets
                strings::RemoveBrackets(data);                  
                //If data is not username, re-input data.
                if (data != username){
                    data_out << strings::DataToFile(data);
                }
            }
        }
        //Close files
        data_in.close();
        data_out.close();
        //Delete temp file
        ec = filemanager::SafeDeleteFile(usersdat_tmp);
        if (ec != EC_None){
            return ec;
        }
        //Log out and return.
        LogOut();
        return EC_None;
    }
    #pragma endregion 
    #pragma region Food
    /**
     * @brief Asks the user for a food name and size. Size can be given in portions or grams, something the user gets to choose before.
     * @returns Possible ErrorCodes: EC_UserCancelled;
     * @returns [OR] ErrorCodes thrown by any of this functions: CreateDailyData(); food::IsFoodRegistered(); food::InternalEatFood; food::PrintFoodList();
     * @warning Daily data file is not directly checked by this function. It is done inside food::InternalEatFood();
    **/
    ErrorCode user_lib::user::EatFood(){
        uint8_t num_input;
        ErrorCode ec;
        string food;
        //Select mode to enter food.
        do {
            num_input = 0;
            food.clear();
            //Print options.
            ClearConsole;
            cout << "1.Enter food name\n2.List all foods\n\n";
            //Get user input.
            if (!input::GetNumericInput(&num_input, Mode_UInt8)){
                return EC_UserCancelled;
            }
            //Enter food name.
            if (num_input == 1){
                //Ask for food name and break.
                do {
                    ClearConsole;
                    cout << "What did you eat? (leave empty to cancel): ";
                    input::GetStringInput(SM_FoodName, food);
                    break;
                } while(true);
            }
            //List all foods and select one.
            else if (num_input == 2){
                do {
                    ClearConsole;
                    //Print and retrieve all foods.
                    vector<string> foods;
                    ec = food::PrintFoodList(username, foods);
                    if (ec != EC_None){
                        return ec;
                    }
                    //Select food
                    cout << '\n';
                    unsigned long chosen_f = 0;
                    if (!input::GetNumericInput(&chosen_f, Mode_UIntLong)){
                        break;
                    }
                    //If valid index selected, save food and break loop.
                    else if (chosen_f > 0 && chosen_f <= foods.size()){
                        //Get food
                        food = foods[chosen_f - 1];
                        break;
                    }
                } while (true);
            }
        } while(food.empty());
        ClearConsole;
        //Validate user_foods.dat
        filesystem::path userfoods = foods_dat(username);
        ec = filemanager::UserFoodsDatCheck(userfoods);
        if (ec != EC_None){
            return ec;
        }
        //Transform food to in file name
        food = name::NameToInFileName(food);
        //See if food exists
        ec = food::IsFoodRegistered(username,food);
        //If item is not found, return ErrorCode.
        if (ec != EC_ItemFound){
            return ec;
        }
        //If food item is found.
        do {
            //Select a counting option to multiply macros
            ClearConsole;
            uint8_t input = 0;
            unsigned long amount = 0;
            cout << "Choose a counting option.\n1.Portion\n2.Grams\n\n";
            if (!input::GetNumericInput(&input, Mode_UInt8)){
                return EC_UserCancelled;
            }
            //Prompt selected option
            ClearConsole;
            switch(input) {
                //Ask for amount of portions.
                case 1: {
                    cout << "How many portions of " + name::InFileNameToName(food,1) + " did you eat?:\n";
                    if (!input::GetNumericInput(&amount, Mode_UIntLong) || amount == 0){
                        return EC_UserCancelled;
                    }
                    //Daily data safety check (in case data was maliciously deleted)
                    ec = CreateDailyData();
                    if (ec != EC_None){
                        return ec;
                    }
                    //Eat food
                    return food::InternalEatFood(username, food, amount, 1);
                }
                //Ask for amount of grams.
                case 2: {
                    cout << "How many grams of " + name::InFileNameToName(food,1) + " did you consume?:\n";
                    if (!input::GetNumericInput(&amount, Mode_UIntLong) || amount == 0){
                        return EC_UserCancelled;
                    }
                    //Daily data safety check (in case data was maliciously deleted)
                    ec = CreateDailyData();
                    if (ec != EC_None){
                        return ec;
                    }
                    //Eat food
                    return food::InternalEatFood(username, food, amount, 0);
                }
                //Option not valid, retry.
                default: {
                    break;
                }
            }
        } while (true);
    }
    /**
     * @brief Remove food form. Asks the user for generic string input.
     * @returns Possible ErrorCodes: EC_UserCancelled;
     * @returns [OR] ErrorCodes thrown by any of this functions: food::InternalRemoveFood();
    **/
    ErrorCode user_lib::user::RemoveFood(){
        ClearConsole;
        cout << "Enter food name (empty to cancel): ";
        //Get food name
        string food;
        if (!input::GetStringInput(SM_FoodName, food)){
            return EC_UserCancelled;
        }
        //Transform food to in file name
        food = name::NameToInFileName(food);
        //Actually remove food
        return food::InternalRemoveFood(username, food);
    }
    /**
     * @brief Modify food form. Asks the user for a food name, macros and portion size, and replaces the old food data.
     * @param food Optional food name, defaulted at NULL. If not defined, function will allocate a new string inside of it and ask the user for a food name.
     * @returns Possible ErrorCodes: EC_UserCancelled;
     * @returns [OR] ErrorCodes thrown by any of these functions: food::IsFoodRegistered(); food::InternalModifyFood();
    **/
    ErrorCode user_lib::user::ModifyFood(string* food){
        //Ask for food name or use provided
        if (food == NULL){
            //Create new string
            food = new string;
            //Ask for food name
            ClearConsole;
            cout << "Input a food name (leave empty to cancel): ";
            if (!input::GetStringInput(SM_FoodName, *food)){
                return EC_UserCancelled;
            }
        }
        //Transform food into in-file name.
        *food = name::NameToInFileName(*food);
        //Check if food exist
        ErrorCode ec = food::IsFoodRegistered(username, *food);
        if (ec != EC_ItemFound){
            return ec;
        }
        //Food has been accepted, append '/'.
        *food += '/';
        //Ask user to enter a portion size.
        ClearConsole;
        unsigned long portion;
        cout << "Enter a portion size in grams:\n";
        if (!input::GetNumericInput(&portion, Mode_UIntLong)){
            return EC_UserCancelled;
        }
        //Enter macros
        double number_input;
        for (uint8_t i = 0; i < NUM_OF_MACROS; i++) {
            food::PrintMacro(i);
            cout << '\n';
            //Get macro value
            if (!input::GetNumericInput(&number_input, Mode_Double)){
                return EC_UserCancelled;
            }
            //Divide macro value by portion to obtain value per single gram and store it.
            food->append(to_string(number_input/portion));
            //Close macro with '/'
            food->append("/");
            //If at the last macro, append the portion size at the end
            if (i + 1 == NUM_OF_MACROS){
                food->append(to_string(portion));
            }
        }
        //Prepare food data string
        *food = strings::DataToFile(*food);
        //Call internal modify
        return food::InternalModifyFood(username, *food);
    }
    /**
     * @brief Register food form. Asks the user for a food name, macros and portion size. Checks if food is already registered. If it is, asks the user if he wants to modify it.
     * @returns Possible ErrorCodes: EC_UserCancelled;
     * @returns [OR] ErrorCodes thrown by any of this functions: ModifyFood(); food::IsFoodRegistered(); food::InternalRegisterFood();
    **/
    ErrorCode user_lib::user::RegisterFood(){
        string food;
        do {
            ClearConsole;
            //Ask for a food name.
            cout << "Enter a food name (empty to cancel): ";
            if (!input::GetStringInput(SM_FoodName, food)){
                return EC_UserCancelled;
            }
            //Validate usr_foods.dat
            filesystem::path foods_p = foods_dat(username);
            ErrorCode ec = filemanager::UserFoodsDatCheck(foods_p);
            if (ec != EC_None && ec != EC_FileEmpty){
                return ec;
            }
            //If file is not empty 
            else if (ec != EC_FileEmpty){
                //Food name to in file name.
                food = name::NameToInFileName(food);
                //Check if food is registered.
                ec = food::IsFoodRegistered(username, food);
                //If it is, ask the user for choice.
                if (ec == EC_ItemFound){
                    //Ask the user to modify the food or cancel the process.
                    uint8_t number_input;                
                    do {
                        cout << "Food is already registered. Do you want to modify it?:\n";
                        cout << "1.Modify\n2.Cancel\n\n";
                        if (!input::GetNumericInput(&number_input, Mode_UInt8)){
                            return EC_UserCancelled;
                        }
                        switch(number_input){
                            //User chose to modify, so it calls modify food.
                            case 1:
                                return ModifyFood(&food);
                            //User cancelled, reset
                            case 2:
                                break;
                        }
                    } while(number_input != 2);
                }
                //If there was a problem, return error.
                else if (ec != EC_ItemNotFound){
                    return ec;
                }
                //Else, accept food name
                else {
                    break;
                }            
            }
            //Else, accept food name.
            else {
                break;
            }
        } while (true);
        //Food has been accepted, append '/'.
        food += '/';
        //Ask the user for a portion size.
        unsigned long portion;
        cout << "Enter a portion size in grams:\n";
        if (!input::GetNumericInput(&portion, Mode_UIntLong)){
            return EC_UserCancelled;
        }
        //Ask the user for every macro value related to the portion size.
        double in_macro;
        for (uint8_t i = 0; i < NUM_OF_MACROS; i++) {
            //Print macro label
            food::PrintMacro(i);
            cout << '\n';
            //Get macro
            if (!input::GetNumericInput(&in_macro, Mode_Double)){
                return EC_UserCancelled;
            }
            //Divide macro value by portion to obtain value per single gram and store it.
            food.append(to_string(in_macro/portion));
            //Close macro with '/'
            food.append("/");
            //If at the last macro, append the portion size at the end.
            if (i + 1 == NUM_OF_MACROS){
                food.append(to_string(portion));
            }
        }
        //Format food string to data string
        food = strings::DataToFile(food);
        //Pass food string into InternalRegisterFood, where actual register takes place.
        return food::InternalRegisterFood(username, food);
    }
    /**
     * @brief Allows user to navigate through the foods registry, consulting food names and macros.
     * @returns Possible ErrorCodes: EC_UserCancelled; EC_FileReadNoPerm;
     * @returns [OR] ErrorCodes thrown by any of this functions: food::PrintFoodList(); food::GetFoodData();
    **/
    ErrorCode user_lib::user::FoodBook(){
        uint8_t num_input;
        do {
            //Print options.
            ClearConsole;
            cout << "1.Enter food name\n2.List all foods\n\n";
            //Get user input.
            if (!input::GetNumericInput(&num_input, Mode_UInt8)){
                break;
            }
            //Enter food name.
            if (num_input == 1){
                do {
                    ClearConsole;
                    cout << "Enter a food name (empty to cancel): ";
                    //Get user input.
                    string input;
                    if (!input::GetStringInput(SM_FoodName, input)){
                        break;
                    }
                    vector<double> macros;
                    //Transform name to in file name
                    input = name::NameToInFileName(input);
                    //Get food data
                    ErrorCode ec = food::GetFoodData(username, input, macros);
                    //If food is not found
                    if (ec == EC_ItemNotFound){
                        ClearConsole;
                        cout << "Food doesn't exist.\n";
                        input::ConsoleWait();
                    }
                    //Macros retrieved
                    else if (ec == EC_None){
                        ClearConsole;
                        //Print food name
                        cout << name::InFileNameToName(input,1) << ":\n\n";
                        //Mutiply macros by portion size
                        for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                            macros[i] *= macros[NUM_OF_MACROS];
                        }
                        //Print all macros
                        for (uint8_t i = 0; i < NUM_OF_MACROS + 1; i++){
                            food::PrintMacro(i);
                            cout << macros[i] << '\n';
                        }
                        cout << '\n';
                        //Wait for user confirmation.
                        input::ConsoleWait();
                    }
                    //If there was an error, return it.
                    else {
                        return ec;
                    }
                } while(true);
            }
            //List all foods and select one.
            else if (num_input == 2){
                do {
                    ClearConsole;
                    vector<string> foods;
                    ErrorCode ec = food::PrintFoodList(username, foods);
                    //If there was a problem, return error.
                    if (ec != EC_None){
                        return ec;
                    }
                    //Select food
                    cout << '\n';
                    string food;
                    unsigned long chosen_f = 0;
                    if (!input::GetNumericInput(&chosen_f, Mode_UIntLong)){
                        break;
                    }
                    //If valid index selected, print food macros.
                    else if (chosen_f > 0 && chosen_f <= foods.size()){
                        //Get food
                        food = foods[chosen_f - 1];
                        ClearConsole;
                        //Get food macros
                        vector<double> macros;
                        ec = food::GetFoodData(username, food, macros);
                        if (ec != EC_None){
                            return ec;
                        }
                        //Mutiply macros by portion size
                        for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                            macros[i] *= macros[NUM_OF_MACROS];
                        }
                        //Print food name
                        cout << name::InFileNameToName(food, 1) << ":\n\n";
                        //Print all macros
                        for (uint8_t i = 0; i < NUM_OF_MACROS + 1; i++){
                            food::PrintMacro(i);
                            cout << macros[i] << '\n';
                        }
                        cout << '\n';
                        //Wait for user confirmation.
                        input::ConsoleWait();
                        break;
                    }
                    //Invalid index selected, retry.
                    else {
                        cout << "Choose a valid food.\n";
                        input::ConsoleWait();
                    }
                } while (true);
            }
        } while(true);
        return EC_UserCancelled;
    }
    #pragma endregion
    #pragma region Macros
    /**
     * @brief Print macros for the current date.
     * @param timeframe Use timeframe 0 for day, 1 for week, 2 for month, 3 for year.
     * @param console_wait If true, wait for user confirmation before exit.
     * @returns Possible ErrorCodes: EC_None;
     * @returns [OR] ErrorCodes thrown by any of this functions: GetDayMacros(); GetWeekMacros(); GetMonthMacros(); GetYearMacros();
    **/
    ErrorCode user_lib::user::PrintCurrentMacros(const uint8_t timeframe, const bool console_wait){
        //Create date struct
        date::s_date date;
        //Get current date and destroy object.
        date::calendar* t_calendar = new date::calendar();
        t_calendar->PassDateToStruct(date);
        delete t_calendar;
        //Create vector
        vector<double> macros;
        //Get macros
        ErrorCode ec;
        switch(timeframe){
            //Day
            case 0: {
                cout << "Current day macros:\n\n";
                ec = food::GetDayMacros(username, macros, date);
                break;
            }
            //Week
            case 1: {
                cout << "Current week macros:\n\n";
                ec = food::GetWeekMacros(username, macros, date);
                break;
            }
            //Month
            case 2: {
                cout << "Current month macros:\n\n";
                ec = food::GetMonthMacros(username, macros, date);
                break;
            }
            //Year
            case 3: {
                cout << "Current year macros:\n\n";
                ec = food::GetYearMacros(username, macros, date);
                break;
            }
        }
        //Print macros
        switch (ec){
            //If macros were retrieved, print them.
            case EC_None:{
                for (uint8_t i = 0; i < macros.size(); i++){
                    food::PrintMacro(i);
                    cout << macros[i] << '\n';
                }
                cout << '\n';
                break;
            }
            //If macros were not found, print "0.0" for each macro.
            case EC_ItemNotFound:{
                for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                    food::PrintMacro(i);
                    cout << 0.0 << '\n';
                }
                cout << '\n';
                break;
            }
            //If there was a problem, return it.
            default:
                return ec;
        }
        //If we need to wait for user input
        if (console_wait){
            input::ConsoleWait();
        }
        //All good, return
        return EC_None;
    }
    /**
     * @brief Print macros for the selected timeframe.
     * @param timeframe Use timeframe 1 for year, 2 for month, 3 for day.
     * @param date Date to print macros from.
     * @returns EC_None;
     * @returns [OR] ErrorCodes returned by any of this functions: GetYearMacros(); GetMonthMacros(); GetDayMacros();
    **/
    ErrorCode user_lib::user::PrintDateMacros(const uint8_t timeframe, date::s_date& date){
        //Calculate day of the week
        date.week_day = date::CalcDayOfWeek(date.year, date.month, date.month_day);
        //Create vector
        vector<double> macros;
        //Get macros
        ErrorCode ec;
        switch(timeframe){
            //Year
            case 1: {
                //Print year label
                cout << date.year;
                cout << " Macros:\n\n";
                //Get year macros
                ec = food::GetYearMacros(username, macros, date);
                break;
            }
            //Month
            case 2: {
                //Print month label
                cout << date::MonthToStr(date.month) + '/';
                cout << date.year;
                cout << " Macros:\n\n";
                //Get month macros
                ec = food::GetMonthMacros(username, macros, date);
                break;
            }
            //Day
            case 3: {
                //Print day label
                cout << to_string(date.month_day) + '/';
                cout << date::MonthToStr(date.month) + '/';
                cout << date.year;
                cout << " Macros:\n\n";
                //Get day macros
                ec = food::GetDayMacros(username, macros, date);
                break;
            }
        }
        //Print or return
        switch (ec){
            //If macros have been retrieved, print them.
            case EC_None:{
                for (uint8_t i = 0; i < macros.size(); i++){
                    food::PrintMacro(i);
                    cout << macros[i] << '\n';
                }
                cout << '\n';
                break;
            }
            //If no macros were found, print "0.0" for each macro.
            case EC_ItemNotFound:{
                for (uint8_t i = 0; i < NUM_OF_MACROS; i++){
                    food::PrintMacro(i);
                    cout << 0.0 << '\n';
                }
                cout << '\n';
                break;
            }
            //If there was an error, return it.
            default:
                return ec;
        }
        //All good, return.
        return EC_None;
    }
    /**
     * @brief Allow user to navigate through its history with menu.
     * @returns Possible ErrorCodes: EC_DirNotFound; EC_UserCancelled;
     * @returns [OR] ErrorCodes thrown by any of this functions: PrintDateMacros();
    **/
    ErrorCode user_lib::user::BrowseHistory(){
        //Load user data folder
        filesystem::path usr_p = user_folder(username);
        //If folder does not exist or is empty, we cancel
        if (!filesystem::exists(usr_p) || filesystem::is_empty(usr_p)){
            cout << "No data available. Safety exit.\n";
            nodata:
            input::ConsoleWait();
            return EC_DirNotFound;
        }
        //Get input to select an entry. Use vector size to determine end of range.
        uint8_t mode = 1; //1 == select year, 2 == select month, 3 == select day
        unsigned long num_input;
        //Prepare date
        date::s_date date;
        do {
            ClearConsole;
            //Prepare error code
            ErrorCode ec;
            //Prepare a vector of entries.
            vector<int> entries;
            //Construct current path (cp)
            filesystem::path cp = usr_p;
            //If selecting month, append year
            if (mode == 2){
                cp.append(to_string(date.year));
            }
            //If selecting day, append year and month.
            else if (mode == 3){
                cp.append(to_string(date.year));
                cp.append(to_string(date.month));
            }
            //Fill entries vector
            for (const auto& entry : filesystem::directory_iterator(cp)) {
                //If entry is a non empty folder
                if (filesystem::is_directory(entry.path()) && !filesystem::is_empty(entry.path())){
                    //Get folder name
                    string tmp_dir = entry.path().filename();
                    switch (mode){
                        //If looking for a year folder.                        
                        case 1: {
                            //If folder is numeric string (Int), save it.
                            if (strings::IsNumericStr(tmp_dir, Mode_Int)){
                                entries.push_back(stoi(tmp_dir));
                            }
                        }
                        //If looking for a month folder
                        case 2: {
                            //If folder is numeric string (Uint8).
                            if (strings::IsNumericStr(tmp_dir, Mode_UInt8)){
                                //And is a valid month number (1 to 12), save it.
                                if (stoul(tmp_dir) > 0 && stoul(tmp_dir) <= 12){
                                    entries.push_back(stoi(tmp_dir));
                                }
                            }
                        }
                        //If looking for a day folder
                        case 3: {
                            //If folder is numeric string (Uint8).
                            if (strings::IsNumericStr(tmp_dir, Mode_UInt8)){
                                //And is a valid month day number (1 to month_length).
                                if (stoul(tmp_dir) > 0 && stoul(tmp_dir) <= date::GetMonthLength(date.month, date.year)){
                                    //Get current day and week day
                                    date.month_day = stoul(tmp_dir);
                                    date.week_day = date::CalcDayOfWeek(date.year, date.month, date.month_day);
                                    //Construct file path
                                    filesystem::path day_dat = entry.path();
                                    day_dat.append(to_string(date.week_day) + "_day.dat");
                                    //If file exists and it is a file, validate it.
                                    if (filesystem::exists(day_dat) && !filesystem::is_directory(day_dat)){
                                        ec = filemanager::DayDataCheck(day_dat);
                                        //If file is not valid, return error.
                                        if (ec != EC_None){
                                            return ec;
                                        }
                                        //We passed the validation, save entry.
                                        entries.push_back(stoi(tmp_dir));
                                        break;
                                    }                                    
                                }
                            }
                        }
                    }             
                }                
            }
            //If no valid entries are found, switch on mode
            if (entries.empty()){
                switch(mode){
                    //If no years, exit
                    case 1: {
                        cout << "No data available. Safety exit.\n";
                        goto nodata;
                    }
                    //If no months, print
                    case 2: {
                        cout << "Year is empty. No data found.\n";
                        break;
                    }
                    //If no days, print
                    case 3: {
                        cout << "Month is empty. No data found.\n";
                        break;
                    }
                }
                //Wait for user input
                input::ConsoleWait();
                //Set mode back and skip
                mode--;
                goto skip;
            }
            //Else if selecting years, sort vector in descending order
            else if (mode == 1){
                sort(entries.begin(),entries.end(),greater<int>());
            }
            //If selecting months or days, sort vector in ascending order
            else {
                sort(entries.begin(),entries.end());
            }
            //Print macros
            switch(mode){
                //Print selected year macros
                case 2: {
                    ec = PrintDateMacros(1, date);
                    if (ec != EC_None){
                        return ec;
                    }
                    break;
                }
                //Print selected month macros
                case 3: {
                    ec = PrintDateMacros(2, date);
                    if (ec != EC_None){
                        return ec;
                    }
                    break;
                }
                //If selecting a year, don't print anything.
                default:
                    break;
            }
            //Print select label
            cout << "Select a ";
            switch (mode) {
                case 1:
                    cout << "year";
                    break;
                case 2:
                    cout << "month";
                    break;
                case 3:
                    cout << "day";
                    break;
            }
            cout << ".\n\n";
            //Print entries
            for (unsigned long i = 0; i < entries.size(); i++){
                cout << '[' << (i + 1) << ']';
                //If month entry, print month string
                if (mode == 2){
                    cout << date::MonthToStr(static_cast<date::month_name>(entries[i]));
                }
                //Else, print entry
                else {
                    cout << entries[i];
                }
                cout << '\n';
            }
            cout << '\n';
            //Get selection input
            do {
                //If cancelled, exit or return.
                if (!input::GetNumericInput(&num_input, Mode_UIntLong)){      
                    //If cancelled at year selection, return.
                    if (mode == 1) {
                        return EC_UserCancelled;
                    }
                    //Else, set mode back
                    else {
                        mode--;
                        break;
                    }
                }
                //If valid input is chosen, update current date and enter next mode.
                else if (!num_input == 0 && !(num_input > entries.size())){
                    switch(mode){
                        //If selecting year, set selected year and advance mode
                        case 1: {
                            date.year = entries[num_input-1];
                            mode++;
                            break;
                        }
                        //If selecting month, set selected month and advance mode
                        case 2: {
                            date.month = static_cast<date::month_name>(entries[num_input-1]);
                            mode++;
                            break;
                        }
                        //If selecting day, print selected day macros
                        case 3: {
                            ClearConsole;
                            date.month_day = entries[num_input-1];
                            ec = PrintDateMacros(3, date);
                            if (ec != EC_None){
                                return ec;
                            }
                            //Wait for user input
                            input::ConsoleWait();
                            break;
                        }
                    }
                    break;
                }
                //If an invalid input is chosen, print and retry.
                cout << "Enter a valid number.\n";
            } while (true);
            //At loop end or skip, clear all entries, clear console and re-enter loop.
            skip:
            entries.clear();
        } while(true);
    }
    #pragma endregion
#pragma endregion
#pragma region Free Functions
/**
 * @brief Checks if an username is already taken (written inside users.dat). User name will be converted to in-file name inside this function.
 * @param name Name we want to check. It will be transformed into an in-file name.
 * @return Possible ErrorCodes: EC_FileReadNoPerm; EC_ItemFound; EC_ItemNotFound;
 * @return [OR] ErrorCodes thrown by any of this functions: filemanager::UsersDataCheck();
**/
ErrorCode user_lib::IsUsernameTaken(const string& name){
    //Validate users.dat
    ErrorCode ec = filemanager::UsersDataCheck();
    if (ec == EC_FileEmpty){
        return EC_ItemNotFound;
    }
    else if (ec != EC_None){
        return ec;
    }   
    //Check if username already registered.
    ifstream data_in;
    data_in.open(users_dat_p);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Get data
    string data;
    while(getline(data_in, data, '|')){
        //Remove brackets
        strings::RemoveBrackets(data);
        //Evaluate
        if (data == name::NameToInFileName(name)){
            data_in.close();
            return EC_ItemFound;
        }
    }
    //Username not taken, close and return.
    data_in.close();
    return EC_ItemNotFound;
}
/**
 * @brief Register a new user inside users.dat. User name will be converted to in-file name inside this function.
 * @param name Name we want to register. This must be an user friendly name, that will be transformed later on into an in-file name.
 * @return Possible ErrorCodes: EC_FileNotFound; EC_FileWriteNoPerm;
 * @return [OR] ErrorCodes thrown by any of this functions: filemanager::UsersDataCheck(); filemanager::CreateTempFile(); filemanager::SafeDeleteFile();
 * @warning This function does NOT check if username is taken, this must be manually done before.
**/
ErrorCode user_lib::RegisterNewUser(const string& name){
    filesystem::path usersdat_p = users_dat_p;
    //If user data does not exist, return error
    if (!filesystem::exists(usersdat_p)){
        return EC_FileNotFound;
    }
    //Validate users.dat
    ErrorCode ec = filemanager::UsersDataCheck();
    if (ec != EC_None && ec != EC_FileEmpty){
        return ec;
    }
    //Register new user
    else {
        ofstream data_out;
        //If data is empty, open it straight.
        if (ec == EC_FileEmpty){
            data_out.open(usersdat_p);
            //If we could not open the file, return error.
            if (!data_out.is_open()){
                return EC_FileWriteNoPerm;
            }
            //Write name to file and close
            data_out << strings::DataToFile(name);
            data_out.close();
        }
        //If data is not empty, create temp file and open the original file in app mode.
        else {
            filesystem::path usersdat_tmp;
            filemanager::CreateTempFile(usersdat_p, usersdat_tmp);            
            data_out.open(usersdat_p, std::ofstream::app);
            //If we could not open the file, delete the temp file and return error.
            if (!data_out.is_open()){
                ec = filemanager::SafeDeleteFile(usersdat_tmp);
                if (ec != EC_None){
                    return ec;
                }
                return EC_FileWriteNoPerm;
            }
            //Write name to file and close
            data_out << strings::DataToFile(name);
            data_out.close();
            //Delete temp file.
            ec = filemanager::SafeDeleteFile(usersdat_tmp);
            if (ec != EC_None){
                return ec;
            }            
        }
    }
    return EC_None;
}
/**
 * @brief Prints all users and returns a vector with all found usernames. This function is meant to be a precursor to any log in selection.
 * @param all_users Vector of strings to dump valid usernames.
 * @return Possible ErrorCodes: EC_FileNotFound; EC_FileReadNoPerm; EC_None;
 * @return [OR] ErrorCodes thrown by any of this functions: filemanager::UsersDataCheck();
**/
ErrorCode user_lib::PrintAllUsers(vector<string>& all_users){
    //Clear vector
    all_users.clear();
    //If user data is not found, return error.
    if (!filesystem::exists(users_dat_p)){
        return EC_FileNotFound;
    }
    //Validate users.dat
    ErrorCode ec = filemanager::UsersDataCheck();
    if (ec != EC_None){
        return ec;
    }
    //Open user data
    ifstream data_in;
    data_in.open(users_dat_p);
    if (!data_in.is_open()){
        return EC_FileReadNoPerm;
    }
    //Retrieve and print user names
    string tmp_data;
    uint8_t i = 0;
    while(getline(data_in, tmp_data, '|')){
        //If not at the end of file
        if (!data_in.eof()){
            //Remove brackets
            tmp_data = tmp_data.substr(1, tmp_data.size() - 2);            
            //Print user number and name
            cout << '[' + to_string(i + 1) + ']' + name::InFileNameToName(tmp_data, 2) + '\n';
            //Save user name
            all_users.push_back(tmp_data);
            //Set last index
            i++;
        }
    }
    //Close file and return
    data_in.close();
    return EC_None;
}
#pragma endregion
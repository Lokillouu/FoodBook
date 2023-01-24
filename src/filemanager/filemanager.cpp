#include <iostream>
using namespace std;
#include <string>
#include <filesystem>
namespace fs = filesystem;
#include <fstream>
#include <vector>
#include <unordered_set>
#include "filemanager.h"
using namespace filemanager;
#include "../io/io_fb.h"
using namespace io_fb;
#include "../date/date.h"

#pragma region Internal Use Functions
/**
 * @brief Purge a vector of paths, removing them differently if they are files or folders.
 * @param entries Vector of entries to purge.
 * @returns Possible ErrorCodes: EC_None;
 * @returns [OR] ErrorCodes thrown by any of these functions: SafeDeleteFolder(); SafeDeleteFile();
**/
ErrorCode PurgeEntries(vector<fs::path>& entries){
    //If vector is not empty.
    if (!entries.empty()){
        ErrorCode ec;    
        //Iterate every entry.
        for (fs::path& p : entries){
            //If it is a directory, call SafeDeleteFolder.
            if (fs::is_directory(p)){
                ec = SafeDeleteFolder(p);
            }
            //Else, it is a file, call SafeDeleteFile.
            else {
                ec = SafeDeleteFile(p);
            }
            //If there was a problem, return it.
            if (ec != EC_None){
                return ec;
            }
        }
    }
    //All done, return.
    return EC_None;
}
/**
 * @brief Checks if a given string is valid for the chosen data type.
 * @param data Data string to process.
 * @param data_from File type data is coming from.
 * @returns 1(true) if string is valid, 0(false) if it isn't.
**/
bool IsValidData(const string& data, const files data_from){
    //If string is empty, invalid
    if (data.empty()){
        return 0;
    }
    //String should start with '{' and end with '}'
    if (!data.starts_with('{') || !data.ends_with('}')){
        return 0;
    }
    //Remove opening and closing brackets from data.
    string t_data = data;
    strings::RemoveBrackets(t_data); 
    //Process data for given type.   
    switch (data_from){
        case usr_foods_dat: {
            //Count number of separators. If we find invalid chars, return invalid.
            uint8_t separators = 0;
            for (char c : t_data){
                //If separator is found, count one.
                if (c == '/'){
                    separators++;
                }
                //Else, if character is not a number, a dot, a lower case or invalid.
                else if (!isCharNumber(c) && c != '.' && !isCharLowerCase(c) && c != '_'){
                    return 0;
                }
            }
            //If not enough or excesive separators, invalid
            if (separators != NUM_OF_MACROS + 1){
                return 0;
            }
            //Get data between separators
            for (uint8_t m = 0; m < NUM_OF_MACROS + 2; m++){
                //If first macro, validate food name
                if (m == 0){
                    //Get food name
                    string f_name = t_data.substr(0,t_data.find_first_of('/'));
                    //Validate food name
                    if (!name::IsValidName(f_name, 1)){
                        return 0;
                    }
                    //Remove food name from string
                    t_data = t_data.substr(t_data.find_first_of('/') + 1);
                }
                //Else if last macro, validate portion.
                else if (m + 1 == NUM_OF_MACROS + 2){
                    //If portion is not numeric unsigned integer, fail.
                    if (!strings::IsNumericStr(t_data, Mode_UIntLong)){
                        return 0;
                    }
                }
                //Else, validate double float macro.
                else {
                    //Get macro
                    string macro = t_data.substr(0, t_data.find_first_of('/'));
                    //If macro is not numeric double (or float), fail.
                    if (!strings::IsNumericStr(macro, Mode_Double)){
                        return 0;
                    }
                    //Remove macro from string (prepare next macro value)               
                    t_data = t_data.substr(t_data.find_first_of('/') + 1);
                }
            }
            //String is valid, break.
            break;
        }
        case users_dat: {
            //If name is not valid, fail.
            if (!name::IsValidName(t_data, 1)){
                return 0;
            }
            //Else is valid, break.
            break;
        }
        case x_day_dat: {
            //If line is not numeric double (or float), fail.
            if (!strings::IsNumericStr(t_data, Mode_Double)){
                return 0;
            }
            //Else valid, break.
            break;
        }
    }
    return 1;
}
/**
 * @brief Detects any user folders that do not have their users registered, and asks the user what to do with them. If restored, it simply adds the user to the user database.
 * @param orphan_p Path to orphan folder. Must be a valid folder path.
 * @returns Possible ErrorCodes: EC_DirNotFound; EC_FileWriteNoPerm; FileRemoveNoPerm; EC_DirRemoveNoPerm; EC_None;
 * @returns [OR] CreateTempFile();
**/
ErrorCode JudgeOrphanFolder(const fs::path& orphan_p){
    //If not a folder, return error.
    if (!fs::is_directory(orphan_p)){
        return EC_DirNotFound;
    }
    string fname = orphan_p.filename().string();
    //Selection loop
    uint8_t num_input;
    do {
        ClearConsole;
        //Print input
        cout << "An unregistered user folder has been found.\n";
        cout << "User: " << name::InFileNameToName(fname,2) << '\n';
        cout << "Do you want to restore the user?\n";
        cout << "1.Yes\n2.No, delete it.\n\n";
        //Get input
        if (!input::GetNumericInput(&num_input, Mode_UInt8)){
            exit(0);
        }
        //If restore
        if (num_input == 1){
            bool temp_file = 0;
            //Validate users dat
            fs::path usrdat = users_dat_p;
            fs::path* usrdat_tmp;
            ErrorCode ec = UsersDataCheck();
            //If users.dat is valid and not empty
            if (ec == EC_None){
                //Create temp file.
                usrdat_tmp = new fs::path;
                ec = CreateTempFile(usrdat, *usrdat_tmp);
                if (ec != EC_None){
                    delete usrdat_tmp;
                    return ec;
                }
                temp_file = 1;              
            }
            //Else if there was an error
            else if (ec != EC_FileEmpty){
                return ec;
            }
            //Open users.dat in append mode.
            ofstream users;
            users.open(usrdat, ios_base::app);
            //If open file failed, delete temp file and return error.
            if (!users.is_open()){
                if (temp_file){
                    ec = SafeDeleteFile(*usrdat_tmp);
                    delete usrdat_tmp;
                    if (ec != EC_None){
                        return ec;
                    }
                }
                //Return error
                return EC_FileWriteNoPerm;
            }
            //Add user name and close file.
            users << '{' + orphan_p.filename().string() + "}|";
            users.close();
            //Remove temp file
            if (temp_file){
                ec = SafeDeleteFile(*usrdat_tmp);
                delete usrdat_tmp;
                if (ec != EC_None){
                    return ec;
                }                
            }
            break;
        }
        //If remove
        else if (num_input == 2){
            ErrorCode ec = SafeDeleteFolder(orphan_p);
            if (ec != EC_None){
                return ec;
            }
            break;
        }
    } while(true);
    //Return success.
    return EC_None;
}
/**
 * @brief Validate file following file data rules. Specify a file type and a matching file path. Corrupted files will be fixed unless there is no valid data inside them OR they are temp files (.tmp_).
 * @param file_type File type to be validated. Read files enum if unsure. Make sure to use the correct type, or else expect bugs :).
 * @param filep Path to file. The path will fail if it is empty or does not exist, but does not check if file type is correct.
 * @param temp_file If validating a temp file, set this to 1, else 0. Again, make sure to use the correct one, or expect bugs ^^.
 * @return ErrorCode enum. The EC reaction is implementation defined.
**/
ErrorCode ValidateFile(const files file_type, const fs::path &filep, const bool temp_file){
    //See if file path is valid
    if (!fs::exists(filep)){
        return EC_FileNotFound;
    }
    //See if file is empty
    if (fs::is_empty(filep)){
        return EC_FileEmpty;
    }
    //Try to open file (read)
    ifstream file_in;
    file_in.open(filep);
    if (!file_in.is_open()){
        EC_FileReadNoPerm;
    }
    //Prepare variables we need
    bool fix = 0, found_tmp_end = 0;
    vector<string> valid_data;
    string data;
    //Get all valid data.
    while (getline(file_in, data, '|')){
        //If temp file and end "flag" found
        if (temp_file && data == "_END_"){
            //Force a new getline to trigger eof
            getline(file_in, data, '|');
            //If eof is set, temp file is good
            if (file_in.eof()){
                //Set end flag found
                found_tmp_end = 1;
            }
            //Else, discard file
            else {
                fix = 1;
            }
            break;
        }
        //Else, if data is valid
        else if (IsValidData(data, file_type)){
            //If file type is users_dat or usr_foods_dat
            if (file_type == users_dat || file_type == usr_foods_dat){
                //If valid data is empty                
                if (valid_data.empty()){
                    valid_data.push_back(data);
                }
                //Else
                else {
                    //Try to find if name is duplicated
                    bool save = 1;
                    for (string& name : valid_data){
                        //Process name condition
                        switch (file_type){
                            case users_dat: {
                                save = name != data;
                            }
                            case usr_foods_dat: {
                                save = name.substr(1,name.find_first_of('/')) != data.substr(1,data.find_first_of('/'));
                            }
                        }
                        //If duplicated, break
                        if (!save){
                            break;
                        }
                    }
                    //If not, save it
                    if (save){
                        valid_data.push_back(data);
                    }
                    //If discarded, set fix to true
                    else {
                        fix = 1;
                    }
                }
            }
            //Else, for x_day_dat
            else {
                //Save string
                valid_data.push_back(data);
                //If line requirement is exceeded, remove last entry and break loop.
                if (valid_data.size() > NUM_OF_MACROS) {
                    valid_data.erase(valid_data.end()--);
                    fix = 1;
                    break;
                }
            }
        }
        //Else, discard data and set fix to true.
        else {
            fix = 1;
        }
    }
    file_in.close();
    //If temp file
    if (temp_file){
        //If no end, something invalid or no valid entries were found, file is corrupted.
        if (!found_tmp_end || fix || valid_data.empty()){
            return EC_FileCorrupted;
        }
        //Else, return success.
        else {
            return EC_None;
        }
    }
    //If normal file
    else {
        //If no valid data, return corrupted
        if (valid_data.empty()){
            return EC_FileCorrupted;
        }
        //If x_day_dat and not enough macros detected, add missing ones.
        else if (file_type == x_day_dat && valid_data.size() < NUM_OF_MACROS){
            for (uint8_t i = valid_data.size(); i < NUM_OF_MACROS; i++){
                valid_data.push_back("{0.0}");
            }
            fix = 1;
        }
        //If we need to fix the file, replace data for valid data.
        if (fix) {
            ofstream file_out;
            file_out.open(filep);
            if (!file_out.is_open()){
                return EC_FileWriteNoPerm;
            }
            for (string& data : valid_data){
                file_out << data + '|';
            }
        }
        return EC_None;
    }
}
/** 
 * @brief Check user folder integrity. This will check every file and folder recursively. Any "illegal" folders and files will be removed, and invalid data will be fixed. To check if an user folder belongs to a registered user, use IsUserFolderRegistered().
 * @param pth Path to user folder. Must be a valid reference.
 * @param mode Starting at mode 0 until mode 3, we check the folder structure in layers (0 == user folder, 1 == year folder, 2 == month folder, 3 == day folder). This function was not meant to start at anything but mode 0, and then let it call itself recursively. In theory it should work no matter the mode, but it is untested. Experiment at your own risk. Calling from 0 works fine.
 * @returns Possible ErrorCodes: EC_DirNotFound; EC_DirEmpty; EC_FileRemoveNoPerm; EC_FileCopy; EC_None;
 * @returns [OR] ErrorCodes thrown by any of this functions: ValidateFile(); ValidateUserFolder() {recursive call}.
**/
ErrorCode ValidateUserFolder(const fs::path &pth, const uint8_t mode = 0){
    if (!fs::exists(pth)){
        return EC_DirNotFound;
    }
    else if (fs::is_empty(pth)){
        return EC_DirEmpty;
    }
    //Create this just in case we need it for mode 3
    uint8_t week_day = 0;
    //Deal with allowed files first
    ErrorCode ec;
    switch (mode){
        //Main user folder
        case 0: {
            //Get username and current path.
            string username = pth.filename().string();
            //Append wanted file to path
            fs::path cpth = pth;
            cpth.append(".tmp_" + username + "_foods.dat");
            //Validate tmp file
            ec = ValidateFile(files::usr_foods_dat, cpth, 1);
            switch (ec){
                //If file is invalid or empty, remove it and jump to permanent file
                case EC_FileEmpty:
                case EC_FileCorrupted: {
                    ec = SafeDeleteFile(cpth);
                    if (ec != EC_None){
                        return ec;
                    }
                    goto validate_foods_dat;
                }
                //If file is valid, make it permanent
                case EC_None: {
                    fs::path to = pth;
                    to.append(username + "_foods.dat");
                    ec = RestoreTempFile(cpth);
                    if (ec != EC_None){
                        return ec;
                    }
                    break;
                }
                //If file is not found, try to validate permanent file.
                case EC_FileNotFound: {
                    validate_foods_dat:
                    //Get file path
                    cpth = pth;
                    cpth.append(username + "_foods.dat");
                    ec = ValidateFile(usr_foods_dat, cpth, 0);
                    //If file is corrupted or empty, remove it
                    if (ec == EC_FileCorrupted || ec == EC_FileEmpty){
                        ec = SafeDeleteFile(cpth);
                        if (ec != EC_None){
                            return ec;
                        }
                    }
                    //If there was a problem, return it
                    else if (ec != EC_None && ec != EC_FileNotFound){
                        return ec;
                    }
                    break;
                }
                //Else, return error
                default: {
                    return ec;
                }
            }
            break;
        }
        //Day food data
        case 3: {
            //Get year folder
            int year = stoi(pth.parent_path().parent_path().filename());
            //Get month folder
            date::month_name month = static_cast<date::month_name>(stoul(pth.parent_path().filename()));
            //Get day folder
            uint8_t day = stoul(pth.filename());
            //Get week day
            week_day = date::CalcDayOfWeek(year, month, day);
            //Append wanted file to path
            fs::path cpth = pth;
            cpth.append(".tmp_" + to_string(week_day) + "_day.dat");
            //Validate tmp file
            ec = ValidateFile(files::x_day_dat, cpth, 1);
            switch (ec){
                //If file is invalid, remove it and jump to permanent file
                case EC_FileCorrupted: {
                    ec = SafeDeleteFile(cpth);
                    if (ec != EC_None){
                        return ec;
                    }
                    goto validate_day_dat;
                }
                //If file is valid, make it permanent
                case EC_None: {
                    ec = RestoreTempFile(cpth);
                    if (ec != EC_None){
                        return ec;
                    }
                    break;
                }
                //If file is not found, try to validate permanent file.
                case EC_FileNotFound: {
                    validate_day_dat:
                    //Get file path
                    cpth = pth;
                    cpth.append(to_string(week_day) + "_day.dat");
                    ec = ValidateFile(x_day_dat, cpth, 0);
                    //If file is corrupted or empty, remove it
                    if (ec == EC_FileCorrupted || ec == EC_FileEmpty){
                        ec = SafeDeleteFile(cpth);
                        if (ec != EC_None){
                            return ec;
                        }
                    }
                    //If there was a problem, return it
                    else if (ec != EC_None && ec != EC_FileNotFound){
                        return ec;
                    }
                    break;
                }
                //Else, return error
                default: {
                    return ec;
                }
            }
            break;
        }
        //Anything else, break;
        default:
            break;
    }
    //Purge folder
    vector<fs::path> to_purge;
    for (const auto& entry : fs::directory_iterator(pth)){
        switch (mode){
            //Main user folder
            case 0: {
                //Get username and current path
                string username = pth.filename().string();
                string c_path = entry.path().filename().string();
                //If entry is a directory
                if (fs::is_directory(entry)){
                    //If not a year directory, delete it
                    if (!strings::IsNumericStr(c_path, Mode_Int)){
                        to_purge.push_back(entry.path());
                    }
                    //Else, validate year folder
                    else {
                        ErrorCode ec = ValidateUserFolder(entry.path(), 1);
                        //If year folder is now empty, delete it
                        if (ec == EC_DirEmpty){
                            to_purge.push_back(entry.path());
                        }
                        //If an error ocurred, return
                        else if (ec != EC_None){
                            return ec;
                        }
                        //Else, this folder is completely valid
                    }
                }
                //If it is a file and not user_foods.dat, purge it
                else if (c_path != username + "_foods.dat"){
                    to_purge.push_back(entry.path());
                }
                break;
            }
            //Case 1 is same as case 2, so we let it bleed down
            case 1:
            //Year or month folder
            case 2: {
                //If entry is a folder
                if (fs::is_directory(entry)){
                    string c_path = entry.path().filename().string();
                    //If folder is numeric and not empty
                    if (strings::IsNumericStr(c_path, Mode_UInt8) && !fs::is_empty(entry)){
                        bool validate = 0;
                        //Perform last folder check
                        switch (mode){
                            //If inside year, folder must be a valid month number
                            case 1: {
                                validate = (stoi(c_path) > 0 && stoi(c_path) <= 12);
                                break;
                            }
                            //If inside month, folder must be a valid month day number
                            case 2: {
                                //Get folder month
                                date::month_name month = static_cast<date::month_name>(stoi(c_path));
                                //Get folder year
                                string year_str = entry.path().parent_path().filename().string();
                                int year = stoi(year_str);
                                validate = (stoi(c_path) <= date::GetMonthLength(month, year) && stoi(c_path) > 0);
                                break;
                            }
                        }
                        //If folder pass every check, validate it
                        if (validate){
                            ec = ValidateUserFolder(entry.path(), mode + 1);
                            //If folder is now empty, delete it
                            if (ec == EC_DirEmpty){
                                to_purge.push_back(entry.path());
                            }
                            //If an error ocurred, return
                            else if (ec != EC_None){
                                return ec;
                            }
                            //Else, this folder is completely valid
                        }
                        //Else, purge it
                        else {
                            to_purge.push_back(entry.path());
                        }
                    }
                    //If folder is not numeric or empty, purge it.                    
                    else {
                        to_purge.push_back(entry.path());
                    }
                }
                //If entry is a file, purge it
                else {
                    to_purge.push_back(entry.path());
                }
                break;
            }
            //Day folder
            case 3: {
                //If folder, delete it
                if (fs::is_directory(entry)){
                    to_purge.push_back(entry.path());
                }
                //Else, check if date file is valid
                else {
                    //If file is not day.dat, purge it
                    if (entry.path().filename() != to_string(week_day) + "_day.dat"){
                        to_purge.push_back(entry.path());
                    }
                }
                break;
            }
        }
    }
    //If something to purge, do it
    ec = PurgeEntries(to_purge);
    if (ec != EC_None){
        return ec;
    }
    //If original path is now empty, return it
    if (fs::is_empty(pth)){
        return EC_DirEmpty;
    }
    else {
        return EC_None;
    }
}
/**
 * @brief Check if usr folder contains any "illegal" or empty files/folders, and deletes them. Validates every user folder recursively with ValidateUserFolder().
 * @param orphan_folders If set pointer is valid, it detects any user folders that do not have their users registered and returns them.
 * @returns Possible ErrorCodes: EC_DirNotFound; EC_DirRemoveNoPerm; EC_None;
 * @returns [OR] ErrorCodes thrown by any of this functions: ValidateUserFolder();
**/
ErrorCode ValidateUsrFolder(vector<fs::path>* orphan_folders = NULL){
    //If folder does not exist
    if (!fs::exists(usr_f)) {
        return EC_DirNotFound;
    }
    //Else if folder is empty
    else if (fs::is_empty(usr_f)){
        return EC_None;
    }
    //Purge data folder
    vector<fs::path> to_purge;
    //Gather "illegal" entries in usr folder.
    for (const auto &p : fs::directory_iterator(usr_f)){
        string cpath_name = p.path().filename().string();
        //If a directory with a valid name and not empty, validate it
        if (fs::is_directory(p) && name::IsValidName(cpath_name,1) && !fs::is_empty(p)){
            ErrorCode ec = ValidateUserFolder(p,0);
            //If user folder is now empty, purge it
            if (ec == EC_DirEmpty){
                to_purge.push_back(p.path());
            }
            //If there was a problem, exit
            else if (ec != EC_None){
                return ec;
            }
            //If set pointer is valid, see if user folder is orphaned.
            else if (orphan_folders != NULL){
                //Open users dat
                ifstream users;
                users.open(users_dat_p);
                if (!users.is_open()){
                    return EC_FileReadNoPerm;
                }
                //Try to find user in database
                bool orphaned = 1;
                string usrname;
                while (getline(users, usrname, '|')){
                    strings::RemoveBrackets(usrname);
                    if (usrname == cpath_name){
                        orphaned = 0;
                        break;
                    }
                }
                //If user is not found, insert it
                if (orphaned){
                    orphan_folders->push_back(p.path());
                }
                users.close();
            }
        }
        //Else, purge it
        else {
            to_purge.push_back(p.path());
        }
    }
    //Remove "illegal" entries.
    ErrorCode ec = PurgeEntries(to_purge);
    if (ec != EC_None){
        return ec;
    }
    return EC_None;
}
/** 
 * @brief Checks if data folder exists and there is no external file or folder in it. It will create any missing directory and files, as well as remove "illegal" ones. If will also validate all "legal" files.
**/
ErrorCode ValidateDataFolder(){
    //If data folder does not exist, create everything and return.
    if (!fs::exists(data_f)) {
        // Create data structure
        if (!fs::create_directories(usr_f)){
            return EC_DirCreateNoPerm;
        }
        // Create users.dat
        ofstream dat;
        dat.open(users_dat_p);
        if (!dat.is_open()){
            return EC_FileWriteNoPerm;
        }
        return EC_None;
    }
    //If data/usr does not exist, create it.
    if (!fs::exists(usr_f)) {
        if (!fs::create_directories(usr_f)){
            return EC_DirCreateNoPerm;
        }
    }
    //If .tmp_users.dat exists, validate it.
    if (fs::exists(users_t_dat_p)) {
        fs::path p = users_t_dat_p;
        ErrorCode t_ec = ValidateFile(users_dat, p, 1);
        //If temp data is corrupted or empty, remove it and validate users.dat
        if (t_ec == EC_FileCorrupted || t_ec == EC_FileEmpty){
            if (!fs::remove(p)){
                return EC_FileRemoveNoPerm;
            }
            goto validate_users_dat;
        }
        //Else, replace users.dat (if it exists) and remove tmp file.
        else if (t_ec == EC_None){
            t_ec = RestoreTempFile(p);
            if (t_ec != EC_None){
                return t_ec;
            }
        }
        //Anything else, error
        else {
            return t_ec;
        }
    }
    //Else, validate users.dat
    else {
        validate_users_dat:
        //If users.dat does not exist, create it
        if (!fs::exists(users_dat_p)){
            ofstream dat;
            dat.open(users_dat_p);
            if (!dat.is_open()){
                return EC_FileWriteNoPerm;
            }
        }
        //If users.dat exists, validate content.
        else {
            fs::path p = users_dat_p;
            ErrorCode t_ec = ValidateFile(users_dat, p, 0);
            //If file is corrupted (could not be fixed), empty it.
            if (t_ec == EC_FileCorrupted){
                ofstream reset_users;
                reset_users.open(p);
                if (!reset_users.is_open()){
                    return EC_FileWriteNoPerm;
                }
                reset_users.close();
            }
            //If there was a problem, return it.
            else if (t_ec != EC_None && t_ec != EC_FileEmpty){
                return t_ec;
            }
        }
    }
    //Purge data folder
    vector<fs::path> to_purge;
    //Gather "illegal" entries in data folder.
    for (const auto &p : fs::directory_iterator(data_f)){
        if (p.path().filename() != "usr" && p.path().filename() != "users.dat"){
            to_purge.push_back(p.path().string());
        }
    }
    //Remove "illegal" entries.
    ErrorCode ec = PurgeEntries(to_purge);
    if (ec != EC_None){
        return ec;
    }
    return EC_None;
}
#pragma endregion
#pragma region Public Functions
/**
 * @brief Performs an initial check of the program data. It is meant to fix any inconsistencies, errors or alterations inside the data files and folders.
**/
ErrorCode filemanager::InitialFilesCheck(){
    ErrorCode ec = ValidateDataFolder();
    if (ec != EC_None){
        return ec;
    }
    vector<fs::path> orphan_folders;
    ec = ValidateUsrFolder(&orphan_folders);
    if (ec != EC_None){
        return ec;
    }
    if (!orphan_folders.empty()){
        for (fs::path& p : orphan_folders){
            ec = JudgeOrphanFolder(p);
            if (ec != EC_None){
                return ec;
            }
        }
    }
    return EC_None;
}
/**
 * @brief Create temp file from an original file. It copies the original file and then adds an _END_ file termination. This does NOT check if the original file is corrupted.
 * @param origin_p Path to original file. The temp file will be created at the same location. Folder paths and empty files are not allowed.
 * @param temp_p Path object to return temp file path. Whatever is inside it, will be overwritten.
 * @return Possible ErrorCodes: EC_FileNotFound; EC_FileEmpty; EC_FileCopy; EC_FileWriteNoPerm; EC_None;
**/
ErrorCode filemanager::CreateTempFile(const fs::path& origin_p, fs::path& temp_p){
    //If path is invalid or is directory, return error.
    if (!fs::exists(origin_p) || fs::is_directory(origin_p)){
        return EC_FileNotFound;
    }
    //Else if file is empty, return error.
    else if (fs::is_empty(origin_p)){
        return EC_FileEmpty;
    }
    //Construct target path (path to temp file)
    temp_p = origin_p;
    string tmp_filename = ".tmp_" + temp_p.filename().string();
    temp_p.replace_filename(tmp_filename);
    //Copy file, return error if action failed.
    if (!fs::copy_file(origin_p, temp_p, fs::copy_options::overwrite_existing)){
        return EC_FileCopy;
    }
    //Now that we copied the file, we add the _END_ termination into a new line.
    ofstream temp_file;
    temp_file.open(temp_p, ios_base::app);
    //If file could not be opened, delete temp file and return error.
    if (!temp_file.is_open()){
        ErrorCode ec = SafeDeleteFile(temp_p);
        if (ec != EC_None){
            return ec;
        }
        return EC_FileWriteNoPerm;
    }
    //Append "_END_"
    temp_file << "_END_|";
    //Close file
    temp_file.close();
    //Return with success
    return EC_None;
}
/**
 * @brief Restore original file from a temp file. It copies the temp file and then removes the _END_ file termination. This does NOT check if the temp file data is corrupted, only if the temp file is empty (excluding _END_ termination) or if _END_ termination is misplaced. The temp file will be deleted after the operation only if it is successful.
 * @param file_p Path to temp file. The original file will be created at the same location. Folder paths and empty files are not allowed.
**/
ErrorCode filemanager::RestoreTempFile(const fs::path& file_p){
    //If path is invalid or is directory, return error.
    if (!fs::exists(file_p) || fs::is_directory(file_p)){
        return EC_FileNotFound;
    }
    //Else if file is empty, return error.
    else if (fs::is_empty(file_p)){
        return EC_FileEmpty;
    }
    //Construct target path (path to temp file)
    fs::path to = file_p;
    string or_filename = to.filename().string();
    //Check if temp file starts with temp prefix. If not, return failure
    if (!or_filename.starts_with(".tmp_")){
        return EC_WrongFile;
    }
    //Remove temp prefix
    or_filename = or_filename.substr(5);
    to.replace_filename(or_filename);
    //Open temp file
    ifstream tmp_file;
    tmp_file.open(file_p);
    if (!tmp_file.is_open()){
        return EC_FileReadNoPerm;
    }
    //Create new file
    ofstream or_file;
    or_file.open(to);
    if (!or_file.is_open()){
        return EC_FileWriteNoPerm;
    }
    //Copy data until _END_ termination is found. If not found, return error.
    string data;
    bool first_line = 1, found_end = 0;
    while (getline(tmp_file, data, '|')){
        //If we are at the first line
        if (first_line){
            //If first line is equal to _END_, delete new file and return error.
            if (data == "_END_"){
                or_file.close();
                tmp_file.close();
                if(!fs::remove(file_p)){
                    return EC_FileRemoveNoPerm;
                }
                return EC_FileCorrupted;
            }
            //Else, remove first_line flag
            else {
                first_line = 0;
            }
        }
        //If not at the first line.
        else {
            //If _END_ reached, perform EOF check.
            if (data == "_END_"){
                getline(tmp_file, data, '|');
                //If there is more data after _END_, delete new file and return error.
                if (!tmp_file.eof()){
                    or_file.close();
                    tmp_file.close();
                    if(!fs::remove(file_p)){
                        return EC_FileRemoveNoPerm;
                    }
                    return EC_FileCorrupted;
                }
                //Else, end found
                found_end = 1;
                break;
            }
        }
        //Input data
        or_file << data << '|';
    }
    //Close files
    or_file.close();
    tmp_file.close();
    //If _END_ not found, delete new file and return error.
    if (!found_end){
        if(!fs::remove(file_p)){
            return EC_FileRemoveNoPerm;
        }
        return EC_FileCorrupted;
    }
    //Else, delete temp file
    else if (!fs::remove(file_p)){
        return EC_FileRemoveNoPerm;
    }
    //Return with success
    return EC_None;
}
/**
 * @brief Validate users.dat following file data rules. If file is corrupted, returns EC_FileCorrupted. If it is empty, it returns EC_FileEmpty.
 * @returns Possible returns: EC_FileNotFound; EC_FileEmpty; EC_FileReadNoPerm; EC_FileCorrupted; EC_None;
**/
ErrorCode filemanager::UsersDataCheck(){
    fs::path usersdat = users_dat_p;
    //See if file path is valid
    if (!fs::exists(usersdat)){
        return EC_FileNotFound;
    }
    //See if file is empty
    if (fs::is_empty(usersdat)){
        return EC_FileEmpty;
    }
    //Try to open file (read)
    ifstream file_in;
    file_in.open(usersdat);
    if (!file_in.is_open()){
        EC_FileReadNoPerm;
    }
    //Check data. Do not allow anything but unique name strings.
    unordered_set<string> names;
    string data;
    while (getline(file_in, data, '|')){
        //If data is valid
        if (IsValidData(data, users_dat)){
            //If names is empty, save name
            if (names.empty()){
                names.insert(data);
            }
            //If name is already saved, return error.
            else if (names.contains(data)){
                file_in.close();
                return EC_FileCorrupted;
            }
            //Else, save it.
            else {
                names.insert(data);
            }
        }
        //If data is not valid, return error.
        else {
            file_in.close();
            return EC_FileCorrupted;
        }
    }
    //File is valid, close and return.
    file_in.close();
    return EC_None;
}
/**
 * @brief Validate x_day.dat following file data rules. If file is corrupted, returns EC_FileCorrupted.
 * @param day_p Path to file. Make sure path is valid (not a folder and not a different file).
 * @returns Possible returns: EC_FileNotFound; EC_FileReadNoPerm; EC_FileCorrupted; EC_None;
**/
ErrorCode filemanager::DayDataCheck(const fs::path& day_p){
    //See if file path is valid
    if (!fs::exists(day_p)){
        return EC_FileNotFound;
    }
    //See if file is empty
    if (fs::is_empty(day_p)){
        return EC_FileCorrupted;
    }
    //Try to open file (read)
    ifstream file_in;
    file_in.open(day_p);
    if (!file_in.is_open()){
        EC_FileReadNoPerm;
    }
    //Check data. Do not allow anything but numeric double or float.
    uint8_t lines = 0;
    string data;
    while (getline(file_in, data, '|')){
        //If data is valid
        if (IsValidData(data, x_day_dat)){
            //Sum one line
            lines++;
            //If line requirement is exceeded, return error.
            if (lines > NUM_OF_MACROS){
                file_in.close();
                return EC_FileCorrupted;
            }
        }
        //Else, return error.
        else {
            file_in.close();
            return EC_FileCorrupted;
        }
    }
    //If line requirement is not met, return error.
    if (lines != NUM_OF_MACROS){
        file_in.close();
        return EC_FileCorrupted;
    }
    //Else, all good.
    file_in.close();
    return EC_None;
}
/**
 * @brief Validate user_foods.dat following file data rules. If file is corrupted, returns EC_FileCorrupted.
 * @param usrfd_p Path to file. Make sure path is valid (not a folder and not a different file).
 * @returns Possible returns: EC_FileNotFound; EC_FileReadNoPerm; EC_FileCorrupted; EC_None;
**/
ErrorCode filemanager::UserFoodsDatCheck(const fs::path& usrfd_p){
    //See if file path is valid
    if (!fs::exists(usrfd_p) || fs::is_directory(usrfd_p)){
        return EC_FileNotFound;
    }
    //See if file is empty
    if (fs::is_empty(usrfd_p)){
        return EC_FileEmpty;
    }
    //Try to open file (read)
    ifstream file_in;
    file_in.open(usrfd_p);
    if (!file_in.is_open()){
        EC_FileReadNoPerm;
    }
    //Check data. Do not allow anything but unique and correct food strings.
    unordered_set<string> names;
    string data;
    while (getline(file_in, data, '|')){
        //If data is valid
        if (IsValidData(data, usr_foods_dat)){
            //Get food name
            string food = data.substr(1,data.find_first_of('/'));
            //If names is empty, save name
            if (names.empty()){
                names.insert(food);
            }
            //If name is already saved, return error.
            else if (names.contains(food)){
                file_in.close();
                return EC_FileCorrupted;
            }
            //Else, save it.
            else {
                names.insert(food);
            }
        }
        //If data is not valid, return error.
        else {
            file_in.close();
            return EC_FileCorrupted;
        }
    }
    file_in.close();
    return EC_None;
}
/**
 * @brief Delete a file with ErrorCode integration.
 * @param file_p Path to file. It must exists and be a path to file, not a directory.
 * @returns Possible returns: EC_FileNotFound; EC_FileRemoveNoPerm; EC_None;
**/
ErrorCode filemanager::SafeDeleteFile(const fs::path& file_p){
    //If file does not exist or it is not a file, return error.
    if (!fs::exists(file_p) || fs::is_directory(file_p)){
        return EC_FileNotFound;
    }
    //Delete file
    if (!fs::remove(file_p)){
        //If we could not remove it, return error.
        return EC_FileRemoveNoPerm;
    }
    //Operation successful, return.
    return EC_None;
}
/**
 * @brief Delete a folder and all of its contents with ErrorCode integration.
 * @param folder_p Path to folder. It must exists and be a path to a directory, not a file.
 * @returns Possible returns: EC_DirNotFound; EC_DirRemoveNoPerm; EC_None;
**/
ErrorCode filemanager::SafeDeleteFolder(const fs::path& folder){
    //If folder does not exist or it is not a folder, return error.
    if (!fs::exists(folder) || !fs::is_directory(folder)){
        return EC_DirNotFound;
    }
    //Delete folder
    if (fs::remove_all(folder) <= 0){
        //If we could not remove it, return error.
        return EC_DirRemoveNoPerm;
    }
    //Operation successful, return.
    return EC_None;
}
/**
 * @brief Get data path for the given date and user. Path points to x_day.dat for the given date.
 * @param username Name of the user to search.
 * @param date_data Struct of date::s_date type.
 * @returns filesystem::path containing path from username folder to week day x_day.dat
 * @warning The function does NOT check if the file exists or the path makes sense. It just builds an untested path with the given information.
**/
filesystem::path filemanager::GetDateDataPath(const string& username, const date::s_date& date_data){
    filesystem::path path = user_folder(username);
    path.append(to_string(date_data.year));
    path.append(to_string(date_data.month));
    path.append(to_string(date_data.month_day));
    path.append(to_string(date_data.week_day) + "_day.dat");
    return path;
}   
#pragma endregion
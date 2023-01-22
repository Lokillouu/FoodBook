#include "io_fb.h"
#include <limits>

#pragma region Strings
/**
 * @brief Converts all uppercases into lowercases.
 * @param string_a String to convert.
**/
void io_fb::strings::StrToLower(string& target){
    for (char &c : target){
        if (c >= 65 && c <= 90){
            c += 32;
        }
    }
    return;
}
/**
 * @brief Compare two strings case insensitive.
 * @param string_a First string to compare.
 * @param string_b Second string to compare.
 * @returns If strings are the same this returns 1, else 0.
**/
bool io_fb::strings::iStrCmp(const string& string_a, const string& string_b){
    //If strings do not have the same length, return false.
    if (string_a.length() != string_b.length()){
        return 0;
    }
    //Check every character
    for (unsigned long i = 0; i < string_a.length(); i++){
        //Get chars to compare
        char a = string_a[i];
        char b = string_b[i];
        //If uppercase, transform chars into lowercase
        if (a >= 65 && a <= 90){
            a += 32;
        }
        if (b >= 65 && b <= 90){
            b += 32;
        }
        //Compare chars, if not equal, return false.
        if (a != b){
            return 0;
        }
    }
    //Strings are equal, return success.
    return 1;
}
/**
 * @brief Checks if target string starts with desired start ignoring case differences (case insensitive).
 * @param target String to check.
 * @param desired_start Desired start for target. Must be no longer than target.
 * @returns If target starts with desired start, returns 1. If it does not start as desired, or desired start is longer than target string, returns 0.
 * @warning Comparation is done with iStrCmp();
**/
bool io_fb::strings::iStrStartsWith(string target, string desired_start){
    //If desired_start is longer than target, return 0
    if (target.size() < desired_start.size()){
        return 0;
    }
    //If target is longer than desired start, resize it to desired_start.
    else if(target.size() > desired_start.size()){
        target.resize(desired_start.size());
    }
    //Compare strings and return
    return iStrCmp(target, desired_start);
}
/**
 * @brief Checks if every character in a string is numeric (see mode).
 * @param target String to check.
 * @param mode Numeric mode to allow. If Mode_Float or Mode_Double is specified, it allows '.' char (just once).
 * @returns If string is numeric it returns 1, else 0.
**/
bool io_fb::strings::IsNumericStr(const string &target, const NumericMode mode){
    //If target is empty, return fail
    if (target.empty()){
        return 0;
    }
    //If we find a non numeric character (except dot if float allowed), we return false.
    else {
        bool dot_found = 0; 
        for (uint8_t i = 0; i < target.length(); i++){
            //If character is not a number
            if (!isCharNumber(target[i])){
                //If float or double and dot has not been found yet.
                if ((mode == Mode_Float || mode == Mode_Double) && !dot_found){
                    //If a dot is found, set flag and continue
                    if (target[i] == '.'){
                        dot_found = 1;
                    }
                    //If not a dot, return false
                    else {
                        return 0;
                    }
                }
                //If no float/double or dot already found, return false
                else {
                    return 0;
                }
            }
        }
    }
    return 1;
}
/**
 * @brief Checks if all string chars are ascii compliant, allowing chars from 0 to 255.
 * @param target String to check.
 * @returns If string is ascii compliant it returns 1, else 0.
**/
bool io_fb::strings::IsStringValid(const string& target){
    for (char c : target){
        if (c > 255 || c < 0){
            return 0;
        }
    }
    return 1;
}
/**
 * @brief Transforms a data string into a file string. It encapsulates the whole data with brackets and ends it with '|' separator. It also lower cases eveything and replaces spaces with '_'.
 * @warning This function transforms names to in-file names, but does not check if data itself is valid or corrupted. Make sure data is validated before.
 * @returns Formatted data ready to dump into file.
**/
string io_fb::strings::DataToFile(const string& data){
    //Transform to in_file data.
    string newdata = name::NameToInFileName(data);
    //Add brackets and end with '|'
    newdata = '{' + newdata + "}|";
    //Return formated string
    return newdata;
}
/**
 * @brief Remove opening and closing brackets from a data string.
 * @param target String to remove brackets from.
 * @warning This function does not validate the data string, simply removes the first and the last character.
**/
void io_fb::strings::RemoveBrackets(string& target){
    target = target.substr(1, target.length() - 2);
    return;
}
#pragma endregion
#pragma region Names
/**
 * @brief Transform a in file name to a valid name (replace '_' with space and add upper case depending on mode rules).
 * @param ifname String in-file naming format.
 * @param mode Mode 0 will return an all lower-case string replacing '_' with space. Mode 1 will uppercase the first letter only. Mode 2 will uppercase the first letter and every letter after a space.
 * @returns String formatted to natural name.
**/
string io_fb::name::InFileNameToName(const string& ifname, const uint8_t mode){
    string name = ifname;
    bool upper_next = 0;
    if (mode != 0){
        upper_next = 1;
    }
    for (char& c : name){
        if (upper_next){
            if (c >= 97 && c <= 122){
                c -= 32;
                upper_next = 0;
            }
        }
        if (c == '_'){
            c = ' ';
            if (mode == 2){
                upper_next = 1;
            }
        }
    }
    return name;
}
/**
 * @brief Transform a valid string name into an in-file name (replace spaces with '_' and lower case everything)
 * @param name String to format to in-file name.
 * @returns String formatted to in-file name.
**/
string io_fb::name::NameToInFileName(const string& name){
    string ifname = name;
    for (char& c : ifname){
        if (c == ' '){
            c = '_';
        }
    }
    strings::StrToLower(ifname);
    return ifname;
}
/**
 * @brief Checks if a given name string is valid (does not contains numbers or forbidden characters).
 * @param name String to check.
 * @param in_file_name If true, treat name as an in-file name.
 * @warning This function is case insensitive.
 * @returns If string is a valid name it returns 1, else 0.
**/
bool io_fb::name::IsValidName(const string& name, const bool in_file_name){
    //If string is empty, fail.
    if (name.empty()){
        return 0;
    }
    //If name is longer than allowed, fail.
    if (name.length() > MAX_USR_NAME){
        return 0;
    }
    //If in file name is chosen, set space character to '_'.
    char space;
    in_file_name ? space = '_': space = ' ';
    //Check string
    for (unsigned long i = 0; i < name.length(); i++){
        //If character is not a letter
        if (!(name[i] >= 'A' && name[i] <= 'Z') && !(name[i] >= 'a' && name[i] <= 'z')){
            //If character is a space, check if previous char was a character.            
            if (name[i] == space){
                //Do not allow a space to start/end a name or to be after another space.                
                if (i == 0 || i == name.length() - 1 || name[i - 1] == space){
                    return 0;
                }
            }
            //If not a space, return failure
            else {
                return 0;
            }
        }
    }
    return 1;
}
#pragma endregion
#pragma region Input
/**
 * @brief Asks the user to enter a string input. The allowed input will depend on the string mode (see mode).
 * @param mode String mode to allow. This will make sure the user does not input any forbidden values.
 * @param input String that will contain the recieved input. Set to empty at the function start.
 * @warning If string is not ascii compliant, the input will not be accepted.
 * @returns If input is successfully taken it returns 1. If user cancels, it returns 0.
**/
bool io_fb::input::GetStringInput(const StrModes mode, string &input){
    //Get input
    string tmp_str;
    do {
        tmp_str = "";
        getline(cin, tmp_str, '\n');
        //If string is empty, cancel
        if (tmp_str.empty()){
            return 0;
        }
        //Resize string accordingly
        switch (mode){
            case SM_FoodName:
                if (tmp_str.length() > MAX_FOOD_NAME){
                    tmp_str.resize(MAX_FOOD_NAME);
                }
                break;
            case SM_UserName:
                if (tmp_str.length() > MAX_USR_NAME){
                    tmp_str.resize(MAX_USR_NAME);
                }
                break;
            case SM_Command:
                if (tmp_str.length() > MAX_COMMAND_L){
                    tmp_str.resize(MAX_COMMAND_L);
                }
                break;
            case SM_Dir:
                if (tmp_str.length() > MAX_DIR_L){
                    tmp_str.resize(MAX_DIR_L);
                }
                break;
        }
        //If string is not ascii compliant, retry
        if (!strings::IsStringValid(tmp_str)){
            cout << "Forbidden character found. Try again.\n";
        }
        //If is a name
        else if (mode == SM_UserName || mode == SM_FoodName){
            //If name is not valid, retry
            if (!name::IsValidName(tmp_str,0)){
                cout << "Forbidden character found. Try again.\n";
            }
            //Else, break
            else {
                break;
            }
        }
        //All good, break
        else {
            break;
        }
    } while(true);
    input = tmp_str;
    return 1;
}
/**
 * @brief Asks the user to enter a numeric input. The allowed input will depend on the numeric mode(see mode).
 * @param input_here A pointer to the variable that will contain the desired input. See exception.
 * @param mode Numeric mode to allow. This will make sure the user does not input any forbidden values.
 * @exception If the input variable is not correct to the specified mode, the function may crash (wrong cast or wrong string conversion).
 * @returns If input is successfully taken and converted it returns 1. If user cancels, it returns 0.
**/
bool io_fb::input::GetNumericInput(void *input_here, const NumericMode mode){
    do {
        //Get input
        string tmp_str = "";
        cout << "Enter a number (leave empty to cancel): ";
        getline(cin, tmp_str, '\n');
        //If cancel, set number to 0 (reserved for failure/invalid input) and return 0
        if (tmp_str.empty()){
            switch (mode){
                case Mode_Int:
                    *(int *)input_here = 0;
                    return 0;
                case Mode_UIntLong:
                    *(unsigned long *)input_here = 0;
                    return 0;
                case Mode_UInt8:
                    *(uint8_t *)input_here = 0;
                    return 0;
                case Mode_Float:
                    *(float *)input_here = 0;
                    return 0;
                case Mode_Double:
                    *(double *)input_here = 0;
                    return 0;
            }
        }
        //Else if string is numeric, convert to number, set and break
        else if (strings::IsNumericStr(tmp_str, mode)){
            //Cast input according to chosen mode.
            switch (mode){
                case Mode_Int:
                    *(int *)input_here = stoi(tmp_str);
                    break;
                case Mode_UIntLong:
                    *(unsigned long *)input_here = stoul(tmp_str);
                    break;
                case Mode_UInt8:
                    *(uint8_t *)input_here = stoul(tmp_str);
                    break;
                case Mode_Float:
                    *(float *)input_here = stof(tmp_str);
                    break;
                case Mode_Double:
                    *(double *)input_here = stod(tmp_str);
                    break;
            }
            break;
        }
        //Else we reenter the loop until a valid option is selected
    } while (true);
    return 1;
}
/**
 * @brief Ask the user to press enter before continuing execution. Anything the user writes is discarded from the stream (ignored).
**/
void io_fb::input::ConsoleWait(){
    cout << "Press 'Enter' to continue...\n";
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return;
}
#pragma endregion
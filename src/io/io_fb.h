#include <iostream>
using namespace std;
#include <string>

#pragma region Macros
    //Set LINUX to false (or 0) for Windows use.
    #define LINUX true
    #if LINUX
    #define ClearConsole system("clear") //Clear console for Linux.
    #else
    #define ClearConsole system("CLS") //Clear console for Windows.
    #endif

    //Length limits
    #define MAX_USR_NAME 30
    #define MAX_FOOD_NAME 50
    #define MAX_COMMAND_L 10
    #define MAX_DIR_L 4096
    #define NUM_OF_MACROS 5 //Add 1 to allocate portion where needed

    //Handy macros
    #define isCharNumber(x) (x >= '0' && x <= '9')
    #define isCharLowerCase(x) (x >= 'a' && x <= 'z')

#pragma endregion
namespace io_fb{
#pragma region Modes
enum NumericMode{Mode_Int, Mode_UIntLong, Mode_UInt8, Mode_Float, Mode_Double};
enum StrModes{SM_UserName, SM_FoodName, SM_Command, SM_Dir};   
#pragma endregion
#pragma region Strings    
namespace strings{
    void StrToLower(string& target);
    bool iStrCmp(const string& string_a, const string& string_b);
    bool iStrStartsWith(string target, string desired_start);
    bool IsStringValid(const string& target);
    bool IsNumericStr(const string &target, const NumericMode mode);
    string DataToFile(const string& data);
    void RemoveBrackets(string& target);
}
#pragma endregion
#pragma region Names
namespace name{
    string InFileNameToName(const string& name, const uint8_t mode);
    string NameToInFileName(const string& name);
    bool IsValidName(const string& name, const bool in_file_name);      
}
#pragma endregion
#pragma region Input
namespace input{
    void ConsoleWait();
    bool GetNumericInput(void *input_here, const NumericMode mode);
    bool GetStringInput(const StrModes mode, string &input);
}
#pragma endregion
}
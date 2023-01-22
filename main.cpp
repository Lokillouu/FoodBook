#include <iostream>
using namespace std;
#include <string>
#include <filesystem>
#include <fstream>
#include "src/user/user.h"
#include "src/filemanager/filemanager.h"

#pragma region Welcome Menu
/**
 * @brief Asks the user for a valid username.
 * @param name Reference to a string that will contain the name.
 * @returns Possible ErrorCodes: EC_None; EC_UserCancelled;
 * @returns [OR] ErrorCodes thrown by user_lib::IsUsernameTaken();
**/
ErrorCode RegisterForm(string &name){
    uint8_t input_num = 0;
    do {
        //Ask for username.
        ClearConsole;
        cout << "Please enter an username (leave empty to exit):\n";
        if (!input::GetStringInput(SM_UserName, name)){
            return EC_UserCancelled;
        }
        ErrorCode ec = user_lib::IsUsernameTaken(name);
        //If name is already taken.
        if (ec == EC_ItemFound){
            cout << "\nUser already registered.\n";
            input::ConsoleWait();
        }
        //If name is not taken.
        else if (ec == EC_ItemNotFound){
            //Fix name.
            strings::StrToLower(name);
            name = name::NameToInFileName(name);
            name = name::InFileNameToName(name, 2);
            //Ask for confirmation.
            do{
                ClearConsole;
                cout << "Your username is: " << name << ".\n";
                cout << "1.Confirm.\n2.Deny.\n\n";
                input::GetNumericInput(&input_num, Mode_UInt8);
                if (input_num == 1 || input_num == 2){
                    break;
                }
            } while(true);        
        }
        else {
            return ec;
        }
    } while(input_num != 1);
    return EC_None;
}
/**
 * @brief Prints a list of all usernames and lets the user select one to log in.
 * @param l_user User object to log the desired user into.
 * @returns Possible ErrorCodes: EC_None; EC_UserCancelled;
 * @returns [OR] ErrorCodes thrown by any of these functions: user_lib::user::LoadUser(); user_lib::PrintAllUsers();
**/
ErrorCode LogIn(user_lib::user& l_user){
    do {
        ClearConsole;
        cout << "Please select a user:\n";
        uint8_t t_num;
        vector<string> all_users;
        //Print all users.
        t_num = user_lib::PrintAllUsers(all_users);
        //If there was an error, return it.
        if (t_num != EC_None){
            return static_cast<ErrorCode>(t_num);
        }
        //Select user
        cout << "\n";
        t_num = 0;
        if(!input::GetNumericInput(&t_num, Mode_UInt8)){
            return EC_UserCancelled;
        }
        //If selected user number is valid
        else if(t_num > 0 && t_num <= all_users.size()) {
            //Load selected user
            t_num = l_user.LoadUser(all_users[t_num - 1]);
            //If there was a problem, return it.
            if (t_num != EC_None){
                return static_cast<ErrorCode>(t_num);
            }
            //Else, user logged in, return.
            else {
                return EC_None;
            }
        }
    } while (true);
}
/**
 * @brief Enters a menu to handle user register and log in.
 * @param l_user User object to log.
 * @warning This function handles exceptions by itself.
**/
void WelcomeMenu(user_lib::user& l_user){
    ErrorCode ec;
    uint8_t num_input;
    //If no users are registered, ask to create a new user.
    if (filesystem::is_empty(users_dat_p)){
        do {
            ClearConsole;
            cout << "Welcome to FoodBook!\n";
            cout << "No user found. Do you want to create a new user?\n";
            cout << "1.Yes\n2.No\n\n"; 
            input::GetNumericInput(&num_input, Mode_UInt8);
            //If user creation is rejected, exit program.
            if (num_input == 2){
                exit(0);
            }
            //Else, break loop and enter register form.
            else if (num_input == 1){
                break;
            }
        } while(true);
        //Register form
        string name;
        ec = RegisterForm(name);
        //If user cancels register form, exit program.
        if (ec == EC_UserCancelled){
            exit(0);
        }
        //If there was an error, return it.
        else if (ec != EC_None){
            InvokeFatalError(ec, "MainMenu->NoUserFound->RegisterForm");
        }
        //Register user
        ec = user_lib::RegisterNewUser(name);
        if (ec != EC_None){            
            InvokeFatalError(ec, "MainMenu->NoUserFound->RegisterNewUser");
        }
        //Load user
        ec = l_user.LoadUser(name);
        if (ec != EC_None){            
            InvokeFatalError(ec, "MainMenu->NoUserFound->LoadUser");
        }
    }
    //Else, give option to login, register or exit.
    else {
        do{
            ClearConsole;
            cout << "Welcome to FoodBook!\n";
            cout << "1.Login\n2.Register\n3.Exit\n\n";
            input::GetNumericInput(&num_input, Mode_UInt8);
            //Log in
            if (num_input == 1){
                ec = LogIn(l_user);
                if (ec == EC_None){
                    break;
                }
                //If there was a problem, crash.
                else if (ec != EC_UserCancelled){
                    InvokeFatalError((ErrorCode)ec, "LogIn");
                }
            }
            //Register
            else if (num_input == 2){
                //Enter register form
                string name;
                ec = RegisterForm(name);
                //If user name was successfully input, register it.
                if (ec == EC_None){
                    //Register user
                    ec = user_lib::RegisterNewUser(name);
                    //Load user and break
                    if (ec == EC_None){
                        ec = l_user.LoadUser(name);
                        //If there was a problem, crash.
                        if (ec != EC_None){
                            InvokeFatalError((ErrorCode)ec, "RegisterNewUser->LoadUser");
                        }
                        //Else, break
                        break;
                    }
                    //If there was a problem, crash.
                    else {
                        InvokeFatalError((ErrorCode)ec, "RegisterNewUser");
                    }
                }
                //If there was a problem, crash.
                else if (ec != EC_UserCancelled){
                    InvokeFatalError(ec, "RegisterForm");
                }
            }
            //Exit option selected, exit program.
            else if (num_input == 3){
                exit(0);
            }
        } while(true);
    }
    //All good, return.
    return;
}
#pragma endregion
#pragma region Menu Options
/**
 * @brief Food options menu.
 * @param l_user User object to use.
 * @warning This function handles exceptions by itself.
**/
void FoodOptions(user_lib::user& l_user){
    ErrorCode ec;
    uint8_t num_input = 0;    
    //Enter loop
    do {
        //Print options
        ClearConsole;
        num_input = 0;
        cout << "1.Food book\n";
        cout << "2.Add new food\n";
        cout << "3.Remove food\n";
        cout << "4.Modify food\n";
        //Get user input
        if (!input::GetNumericInput(&num_input, Mode_UInt8)){
            break;
        }
        //If user input was valid
        if (num_input > 0 && num_input < 5){
            //Enter selected mode
            switch (num_input){
                //Consult food macros
                case 1: {
                    ec = l_user.FoodBook();
                    //If no foods are registered
                    if (ec == EC_FileEmpty){
                        cout << "Food book is empty.\n";
                        input::ConsoleWait();
                    }
                    //If there was an error
                    else if (ec != EC_UserCancelled){
                        InvokeFatalError(ec, "MainMenu->FoodOptions->FoodBook");
                    }
                    break;
                }
                //Register food
                case 2: {
                    //Ask the user for a food name and register it.
                    ec = l_user.RegisterFood();
                    //If there was a problem
                    if (ec != EC_UserCancelled && ec != EC_None){
                        InvokeFatalError(ec, "MainMenu->FoodOptions->RegisterFood");
                    }
                    break;
                }
                //Remove food
                case 3: {
                    //Ask the user for a food name to remove and do it
                    ec = l_user.RemoveFood();
                    //If food was not found
                    if (ec == EC_ItemNotFound){
                        cout << "\nFood is not registered.\n";
                        input::ConsoleWait();
                    }
                    //If no foods are registered
                    else if (ec == EC_FileEmpty){
                        cout << "No foods registered.\n";
                        input::ConsoleWait();
                    }
                    //If there was a problem
                    else if (ec != EC_UserCancelled && ec != EC_None){
                        InvokeFatalError(ec, "MainMenu->FoodOptions->RemoveFood");
                    }
                    break;
                }
                //Modify food
                case 4: {
                    //We ask for a food name and return in case of user cancel or success
                    ec = l_user.ModifyFood();
                    //If food not found, we inform the user and break
                    if(ec == EC_ItemNotFound){
                        cout << "\nFood is not registered.\n";
                        input::ConsoleWait();
                    }
                    //If there was a problem
                    else if (ec != EC_UserCancelled && ec != EC_None){
                        InvokeFatalError(ec, "MainMenu->FoodOptions->ModifyFood");
                    }
                    break;
                }
            }
            break;
        }
    } while (true);
    return;
}
/**
 * @brief Food macros options menu.
 * @param l_user User object to use.
 * @warning This function handles exceptions by itself.
**/
void MacrosOptions(user_lib::user& l_user){
    ErrorCode ec;
    uint8_t num_input;
    do {
        num_input = 0;
        ClearConsole;
        cout << "Choose a time frame:\n\n";
        cout << "1.Today\n2.Current week\n3.Current month\n";
        cout << "4.Current year\n5.History\n\n";
        //Get input
        if (!input::GetNumericInput(&num_input, Mode_UInt8)){
            break;
        }
        ClearConsole;
        //Current time frames
        if (num_input > 0 && num_input < 5){
            //Print desired macros
            ec = l_user.PrintCurrentMacros(num_input-1);
            //If there was an error, crash.
            if (ec != EC_None){
                InvokeFatalError(ec, "MainMenu->ConsultMacros->PrintCurrentMacros(" + to_string(num_input) + ')');
            }
        }
        //History browser
        else if (num_input == 5){
            //Browse history
            ec = l_user.BrowseHistory();
            //If there was an error, crash.
            if (ec != EC_None && ec != EC_UserCancelled){
                InvokeFatalError(ec, "MainMenu->ConsultMacros->BrowseHistory");
            }
        }    
    } while (true);
    return;
}
/**
 * @brief User options menu.
 * @param l_user User object to use.
 * @returns If 1 is returned, restart the program.
 * @warning This function handles exceptions by itself.
**/
bool UserOptions(user_lib::user& l_user){
    ErrorCode ec;
    uint8_t num_input;
    do {
        num_input = 0;
        ClearConsole;
        cout << "1.Restore user data\n2.Create backup\n3.Delete user\n\n";
        //Get input
        if (!input::GetNumericInput(&num_input, Mode_UInt8)){
            break;
        }
        //Restore user data
        else if (num_input == 1){
            ec = l_user.RestoreData();
            //User data restored, return 1 to restart.
            if (ec == EC_None){
                return 1;
            }
            //If there was a problem, crash.
            else if (ec != EC_UserCancelled){
                InvokeFatalError(ec, "MainMenu->DeleteUser");
            } 
        }
        //Create backup
        else if (num_input == 2){
            ec = l_user.BackupFiles();
            //If there was a problem, crash
            if (ec != EC_None && ec != EC_UserCancelled){
                InvokeFatalError(ec, "MainMenu->UserOptions->BackupFiles");
            }
        }
        //Delete user                    
        else if (num_input == 3){
            ec = l_user.DeleteUser();
            //User was deleted, return 1 to restart program.
            if (ec == EC_None){
                return 1;
            }
            //If there was a problem, crash.
            else if (ec != EC_UserCancelled){
                InvokeFatalError(ec, "MainMenu->DeleteUser");
            }
        }
    } while (true);
    return 0;
}     
#pragma endregion

int main() {
    start:
    user_lib::user local_user;
    //Do an initial files check
    ErrorCode ec = filemanager::InitialFilesCheck();
    //If there was an error, return it.
    if (ec != EC_None){
        InvokeFatalError(ec, "InitialFilesCheck");
    }
    //Enter welcome menu.
    WelcomeMenu(local_user);
    //Main Menu
    do {
        //Print menu options
        uint8_t num_input = 0;
        ClearConsole;
        cout << "Welcome back " << local_user.GetUserName() << ".\n\n";
        cout << "1.I ate something\n";
        cout << "2.Food options\n";
        cout << "3.Consult user macros\n";
        cout << "4.User options\n";
        cout << "5.Log out\n\n";
        //Print daily data
        ec = local_user.PrintCurrentMacros(0,0);
        if (ec != EC_None){
            InvokeFatalError(ec,"MainMenu->PrintCurrentMacros(0)");
        }      
        //Input switch
        input::GetNumericInput(&num_input, Mode_UInt8);
        switch(num_input){
            //Eat
            case 1: {
                ec = local_user.EatFood();
                //If food was not found.
                if (ec == EC_ItemNotFound){
                    cout << "Food not found.\n";
                    input::ConsoleWait();
                }
                //If user_foods.dat is empty.
                else if (ec == EC_FileEmpty){
                    cout << "There are no foods registered.\n";
                    input::ConsoleWait();
                }
                //If there was an error, crash
                else if (ec != EC_None && ec != EC_UserCancelled){
                    InvokeFatalError(ec, "MainMenu->EatFood");
                }
                break;
            }
            //Food options
            case 2: {
                FoodOptions(local_user);
                break;
            }
            //Consult macros
            case 3: {
                MacrosOptions(local_user);
                break;
            }
            //User options
            case 4: {
                //If user options returns 1, restart.
                if (UserOptions(local_user)){
                    local_user.LogOut();
                    goto start;
                }
                break;
            }
            //Log out
            case 5: {
                local_user.LogOut();
                //Re enter program
                goto start;
            }           
            //Anything else invalid, retry
            default:
                break;
        }
    } while(true);
    return 0;
}
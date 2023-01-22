#include "../errors/errors.h"
#include "../date/date.h"
#include <filesystem>

#pragma region Paths
#define data_f "data"
#define users_dat_p "data/users.dat"
#define users_t_dat_p "data/.tmp_users.dat"
#define usr_f "data/usr"
#define user_folder(username) "data/usr/" + username
#define foods_dat(username) "data/usr/" + username + "/" + username + "_foods.dat"
#pragma endregion
namespace filemanager {
#pragma region Data
//File types enums, starting at 0 with users_dat.
enum files : uint8_t {users_dat, usr_foods_dat, x_day_dat};
#pragma endregion
#pragma region Public Function Headers
ErrorCode InitialFilesCheck();
ErrorCode CreateTempFile(const filesystem::path& origin_p, filesystem::path& temp_p);
ErrorCode RestoreTempFile(const filesystem::path& file_p);
ErrorCode UsersDataCheck();
ErrorCode DayDataCheck(const filesystem::path& day_p);
ErrorCode UserFoodsDatCheck(const filesystem::path& usrfd_p);
ErrorCode SafeDeleteFile(const filesystem::path& file_p);
ErrorCode SafeDeleteFolder(const filesystem::path& file_p);
filesystem::path GetDateDataPath(const string& username, const date::s_date& date_data);
#pragma endregion
}

#include "main-win/main-win-exception.h"
#include "main-win/main-win-utils.h"
#include "net/report-error.h"
#include <sstream>

/*!
 * @brief 予期しない例外を処理する
 *
 * 予期しない例外が発生した場合、確認を取り例外のエラー情報を開発チームに送信する。
 * その後、バグ報告ページを開くかどうか尋ね、開く場合はWebブラウザで開く。
 *
 * @param e 例外オブジェクト
 */
void handle_unexpected_exception(const std::exception &e)
{
    constexpr auto caption = _(L"予期しないエラー！", L"Unexpected error!");

};

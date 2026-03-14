#include "main-win/main-win-exception.h"
#include "locale/japanese.h"
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

    std::string msg = e.what();
#ifdef JP
    const auto msg_len = guess_convert_to_system_encoding(msg.data(), msg.size());
    msg.erase(msg_len);
#endif

    // エラー内容をダイアログ表示するのみに短縮
    MessageBoxW(NULL, to_wchar(msg).wc_str(), caption, MB_ICONERROR | MB_OK);
}

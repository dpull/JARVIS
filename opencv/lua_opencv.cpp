#include "lua_opencv.h"
#include "lua_object.h"
#include "opencv2/opencv.hpp"
#include <Windows.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static void debug_img(const cv::String& win, const cv::Mat& img, cv::Point& min_loc, cv::Point& max_loc)
{
    cv::Mat result_img(img);
    cv::rectangle(result_img, min_loc, max_loc, cv::Scalar(0, 0, 255), 2);

    cv::Mat resized_img;
    cv::resize(result_img, resized_img, cv::Size(result_img.cols / 2, result_img.rows / 2));

    cv::namedWindow(win, cv::WINDOW_NORMAL);
    cv::imshow(win, resized_img);
    cv::waitKey();
}

static int lua_load_image(lua_State* L)
{
    auto path = lua_tostring(L, 1);
    if (!path)
        return luaL_argerror(L, 1, "parameter path invalid");

    auto img = cv::imread(path);
    if (img.empty())
        return luaL_error(L, "load image %s failed", path);

    lua_object<cv::Mat>::alloc(L, img);
    return 1;
}

static int lua_match_template(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter path invalid");

    auto tmpl = lua_object<cv::Mat>::toobj(L, 2);
    if (!tmpl)
        return luaL_argerror(L, 1, "parameter path invalid");

    auto method = static_cast<int>(lua_tointeger(L, 3));

    auto show = lua_toboolean(L, 4);

    cv::Mat result;
    matchTemplate(*img, *tmpl, result, method);

    double min_val, max_val, ret_val;
    cv::Point min_loc, max_loc;
    cv::minMaxLoc(result, &min_val, &max_val, &min_loc, &max_loc);

    switch (method) {
    case cv::TM_SQDIFF:
    case cv::TM_SQDIFF_NORMED:
        ret_val = min_val;
        max_loc.x = min_loc.x + tmpl->cols;
        max_loc.y = min_loc.y + tmpl->rows;
        break;
    case cv::TM_CCORR:
    case cv::TM_CCORR_NORMED:
    case cv::TM_CCOEFF:
    case cv::TM_CCOEFF_NORMED:
        ret_val = max_val;
        min_loc = max_loc;
        max_loc.x = min_loc.x + tmpl->cols;
        max_loc.y = min_loc.y + tmpl->rows;
        break;
    default:
        ret_val = 0;
    }

    if (show)
        debug_img("match_template", *img, min_loc, max_loc);

    lua_pushnumber(L, ret_val);
    lua_pushinteger(L, min_loc.x);
    lua_pushinteger(L, min_loc.y);
    lua_pushinteger(L, max_loc.x);
    lua_pushinteger(L, max_loc.y);
    return 5;
}

static bool hwnd2mat(HWND hwnd, cv::Mat& image, DWORD& err_code)
{
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        err_code = GetLastError();
        return false;
    }

    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;

    auto hdc = GetDC(hwnd);
    auto mem_dc = CreateCompatibleDC(hdc);
    auto bitmap = CreateCompatibleBitmap(hdc, width, height);
    HGDIOBJ old_object = nullptr;

    bool result = false;
    err_code = 0;
    do {
        old_object = SelectObject(mem_dc, bitmap);
        if (!old_object) {
            err_code = GetLastError();
            break;
        }

        if (!BitBlt(mem_dc, 0, 0, width, height, hdc, 0, 0, SRCCOPY)) {
            err_code = GetLastError();
            break;
        }

        image.create(height, width, CV_8UC4);
        if (GetBitmapBits(bitmap, width * height * 4, image.data) == 0) {
            err_code = GetLastError();
            break;
        }

        if (image.empty())
            break;

        result = true;
    } while (false);

    if (old_object)
        SelectObject(mem_dc, old_object);

    DeleteObject(bitmap);
    DeleteDC(mem_dc);
    ReleaseDC(hwnd, hdc);

    return result;
}

static int lua_window2image(lua_State* L)
{
    auto hwnd = static_cast<HWND>(lua_touserdata(L, 1));
    auto save_file = lua_tostring(L, 2);

    cv::Mat image;
    DWORD err_code;
    if (!hwnd2mat(hwnd, image, err_code))
        return luaL_error(L, "hwnd2mat failed:%u", err_code);

    auto lua_image = lua_object<cv::Mat>::alloc(L);
    cv::cvtColor(image, *lua_image, cv::COLOR_BGRA2BGR);
    if (save_file)
        cv::imwrite(save_file, *lua_image);

    return 1;
}

int luaopen_opencv(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "load_image", lua_load_image },
        { "match_template", lua_match_template },
        { "window2image", lua_window2image },
        { nullptr, nullptr },
    };
    luaL_newlib(L, l);

    lua_object<cv::Mat>::newmetatable(L);

    return 1;
}

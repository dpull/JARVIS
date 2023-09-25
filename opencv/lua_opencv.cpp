#include "lua_opencv.h"
#include "lua_object.hpp"
#include "opencv2/opencv.hpp"
#include <Windows.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

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

static int lua_save_image(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");

    auto path = lua_tostring(L, 2);
    if (!path)
        return luaL_argerror(L, 2, "parameter path invalid");

    auto ret = cv::imwrite(path, *img);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_imencode(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");

    auto ext = lua_tostring(L, 2);
    if (!ext)
        return luaL_argerror(L, 2, "parameter ext invalid");

    std::vector<uchar> data;
    if (!cv::imencode(ext, *img, data))
        return 0;

    lua_pushlstring(L, (char*)data.data(), data.size());
    return 1;
}

static int lua_match_template(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");

    auto tmpl = lua_object<cv::Mat>::toobj(L, 2);
    if (!tmpl)
        return luaL_argerror(L, 1, "parameter tmpl invalid");

    auto method = static_cast<int>(lua_tointeger(L, 3));

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

    lua_pushnumber(L, ret_val);
    lua_pushinteger(L, min_loc.x);
    lua_pushinteger(L, min_loc.y);
    lua_pushinteger(L, max_loc.x);
    lua_pushinteger(L, max_loc.y);
    return 5;
}

static int lua_imshow(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");
    auto win_name = lua_tostring(L, 2);
    if (!win_name)
        return luaL_argerror(L, 1, "parameter win_name invalid");

    cv::String winname(win_name);

    cv::namedWindow(win_name, cv::WINDOW_NORMAL);
    cv::imshow(win_name, *img);
    cv::waitKey();
    return 0;
}

static int lua_rectangle(lua_State* L)
{
    auto top = lua_gettop(L);
    if (top < 5)
        return luaL_argerror(L, 1, "parameter invalid");

    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");

    cv::Point min_loc, max_loc;

    min_loc.x = static_cast<int>(lua_tointeger(L, 2));
    min_loc.y = static_cast<int>(lua_tointeger(L, 3));

    max_loc.x = static_cast<int>(lua_tointeger(L, 4));
    max_loc.y = static_cast<int>(lua_tointeger(L, 5));

    cv::Scalar color(0, 0, 255);
    if (top > 5) {
        auto b = lua_tonumber(L, 6);
        auto g = lua_tonumber(L, 7);
        auto r = lua_tonumber(L, 8);
        auto a = lua_tonumber(L, 9);
        color = cv::Scalar(b, g, r, a);
    }

    auto copy_image = lua_object<cv::Mat>::alloc(L, *img);
    cv::rectangle(*copy_image, min_loc, max_loc, color, 2);
    return 1;
}

static int lua_image_size(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");
    auto width = img->cols;
    auto height = img->rows;

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_sub_image(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");

    cv::Rect roi;
    roi.x = static_cast<int>(lua_tointeger(L, 2));
    roi.y = static_cast<int>(lua_tointeger(L, 3));
    roi.width = static_cast<int>(lua_tointeger(L, 4));
    roi.height = static_cast<int>(lua_tointeger(L, 5));
    lua_object<cv::Mat>::alloc(L, (*img)(roi));
    return 1;
}

static int lua_cvt_color(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");
    auto code = static_cast<int>(lua_tointeger(L, 2));
    auto cvt_image = lua_object<cv::Mat>::alloc(L);
    cv::cvtColor(*img, *cvt_image, code);
    return 1;
}

static int lua_equalizeHist(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");
    auto cvt_image = lua_object<cv::Mat>::alloc(L);
    cv::equalizeHist(*img, *cvt_image);
    return 1;
}

static int lua_resize(lua_State* L)
{
    auto img = lua_object<cv::Mat>::toobj(L, 1);
    if (!img)
        return luaL_argerror(L, 1, "parameter img invalid");
    cv::Size size(lua_tointeger(L, 2), lua_tointeger(L, 3));
    auto cvt_image = lua_object<cv::Mat>::alloc(L);
    cv::resize(*img, *cvt_image, size);
    return 1;
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

    auto lua_image = lua_object<cv::Mat>::alloc(L);
    DWORD err_code;
    if (!hwnd2mat(hwnd, *lua_image, err_code))
        return luaL_error(L, "hwnd2mat failed:%u", err_code);

    return 1;
}

int luaopen_opencv(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "load_image", lua_load_image },
        { "save_image", lua_save_image },
        { "imencode", lua_imencode },
        { "match_template", lua_match_template },
        { "imshow", lua_imshow },
        { "rectangle", lua_rectangle },
        { "image_size", lua_image_size },
        { "sub_image", lua_sub_image },
        { "cvt_color", lua_cvt_color },
        { "equalizeHist", lua_equalizeHist },
        { "resize", lua_resize },
        { "window2image", lua_window2image },
        { nullptr, nullptr },
    };

    luaL_newlib(L, l);
    lua_object<cv::Mat>::newmetatable(L);

    return 1;
}

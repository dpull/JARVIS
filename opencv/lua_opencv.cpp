#include "lua_opencv.h"
#include "opencv2/opencv.hpp"
#include <stdbool.h>
#include <stdint.h>

static int lua_match_template(lua_State* L)
{
    auto srcImg = cv::imread("srcImg.jpg");
    auto tmplImg = cv::imread("tmplImg.jpg");

    if (srcImg.empty() || tmplImg.empty()) {
        return -1;
    }

    cv::Mat resultImg;
    matchTemplate(srcImg, tmplImg, resultImg, cv::TM_CCOEFF_NORMED);

    cv::Point maxLoc;
    cv::minMaxLoc(resultImg, NULL, NULL, NULL, &maxLoc);

    cv::rectangle(srcImg, maxLoc, cv::Point(maxLoc.x + tmplImg.cols, maxLoc.y + tmplImg.rows), cv::Scalar(0, 0, 255), 2);

    return 0;
}

int luaopen_opencv(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "match_template", lua_match_template },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}

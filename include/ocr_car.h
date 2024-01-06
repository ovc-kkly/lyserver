#ifndef OCR_CAR_H
#define OCR_CAR_H

#include <string>
#include <memory>
// #include <ocr.h>
#include "json/json.h"
namespace lyserver
{
    class my_ocr
    {
    public:
        typedef std::shared_ptr<my_ocr> ocr_ptr;
        // typedef std::shared_ptr<aip::Ocr> ocr_sdk_ptr;
        my_ocr();
        ~my_ocr();
        void init();
        std::string myocr_base64_decode(std::string &data);
        void LoadImage(const char *path);
        void Recognize();
        void Recognize(std::string &img);
        std::string getresult();
        void clear();

    private:
        // 设置APPID/AK/SK
        std::string app_id = "46122113";
        std::string api_key = "W6nyXtDoQG7WSmmY7ERQ6doZ";
        std::string secret_key = "3LP1wZRIv6tYP0HSVBcFkeesYDZE65o1";

        Json::Value result;
        std::string image;
        // ocr_sdk_ptr client;
        // aip::Ocr client(app_id, api_key, secret_key);
    };
}

#endif //__OCR_CAR_H__
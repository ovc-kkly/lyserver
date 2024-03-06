#include "ocr_car.h"
namespace lyserver
{
    my_ocr::my_ocr()
    {
        // client.reset(new aip::Ocr(app_id, api_key, secret_key));
    }
    my_ocr::~my_ocr()
    {
    }
    void my_ocr::init()
    {
    }
    std::string my_ocr::myocr_base64_decode(std::string &data)
    {
        // return aip::base64_decode(data);
        return std::string();
    }
    void my_ocr::LoadImage(const char *path)
    {
        // aip::get_file_content(path, &image);
        // aip::get_file_content(path, &image);
    }
    void my_ocr::Recognize()
    {
        // result = client->license_plate(image, aip::null);
    }
    void my_ocr::Recognize(std::string &img)
    {
        // result = client->license_plate(img, aip::null);
    }
    std::string my_ocr::getresult()
    {
        std::string color = result["words_result"]["color"].asString();
        std::string number = result["words_result"]["number"].asString();
        Json::Value v;
        v["color"] = color;
        v["number"] = number;
        std::string str = v.toStyledString();
        return str;
    }
    void my_ocr::clear()
    {
        this->image.clear();
        this->result.clear();
    }
}

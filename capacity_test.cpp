#include<iostream>
#include<fstream>
#include<assert.h>
#include<random>
#include<ctime>
#include "json.hpp"
#include"util_url_opt.h"
using json=nlohmann::json;
#define CAPACITY_FORMAT_FILE "capacity_format.json"
#define PREFIX     "https://"
#define CAPACITY_URL ""
#define	DEVICE_URL	""
#define TIMEOUT     15000   //15ç§?

std::string username="admin";
std::string password="";
std::string IP="";
std::string PORT="443";
std::vector<json>test_json;


enum
{
    CAPACITY_PARSE_OK,
    CAPACITY_FORMAT_GET_ERROR,
    CAPACITY_FORMAT_GET_SUCCESS,
    CAPACITY_PARSE_ERROR,
    CAPACITY_PARSE_SUCCESS,
    CAPACITY_PARSE_INVALID,
    CAPACITY_PARSE_MAKE_STRING,
    CAPACITY_PARSE_MAKE_INT,
    CAPACITY_PARSE_OTHER
};

int CAPACITY_PARSE_VALUE(const json &element,json &normal_format,json &false_format);
int CAPACITY_FORMAT_GET(json & format)
{
    std::ifstream file_stream(CAPACITY_FORMAT_FILE);
    if(!file_stream)return CAPACITY_FORMAT_GET_ERROR;
    file_stream >> format;
    return CAPACITY_FORMAT_GET_SUCCESS;
}

std::string MAKE_STRING(int length)
{
    std::string temp;
    for(int i=0;i<length;i++)
    {
        char c=' '+rand()%95;
        temp+=c;
    }
    return temp;
}

int CAPACITY_PARSE_STRING(const json &element,json &normal_format,json &false_format)
{
    std::string str=element;
    false_format = MAKE_STRING(str.size());
    normal_format = str;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_FLOAT(const json &element,json &normal_format,json &false_format)
{
    false_format = (rand() % (10000 - 6000 + 1) + 6000) / 100.0;
    normal_format = element;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_UNSIGNED(const json &element,json &normal_format,json &false_format)
{
    false_format = rand()%2048;
    normal_format = element;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_INTEGER(const json &element,json &normal_format,json &false_format)
{
    false_format = rand()%4096-2048;
    normal_format = element;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_BOOLEAN(const json &element,json &normal_format,json &false_format)
{
    false_format = !element;
    normal_format = element;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_ARRAY(const json &element,json &normal_format,json &false_format)
{
    int random= rand() % (element.size());
    auto ele=element[random];
    return CAPACITY_PARSE_VALUE(ele,normal_format,false_format);
}

int CAPACITY_PARSE_OBJ_JUDGE(const json &element)
{
    if(element.size()!=2)return CAPACITY_PARSE_OTHER;
    for(auto it=element.begin();it!=element.end();it++)
    {
        if(it.key()=="@max" || it.key()=="@min" )return CAPACITY_PARSE_MAKE_STRING;
        else if(it.key()=="max" || it.key()=="min")return CAPACITY_PARSE_MAKE_INT;
    }
    return CAPACITY_PARSE_OTHER;
}

int PARSE_TOINTEGER(const json &element,json &normal_format,json &false_format)
{
    int max=element["max"],min=element["min"];
    std::default_random_engine e(time(0));
    std::uniform_real_distribution<double> x(min, max);
    normal_format=(int)x(e);
    false_format=(int)x(e)+max-min;
    return CAPACITY_PARSE_SUCCESS;
}

int PARSE_TOUNSIGNED(const json &element,json &normal_format,json &false_format)
{
    unsigned max=element["max"],min=element["min"];
    std::default_random_engine e(time(0));
    std::uniform_real_distribution<double> x(min, max);
    normal_format=(unsigned)x(e);
    false_format=(unsigned)x(e)+max-min;
    return CAPACITY_PARSE_SUCCESS;
}

int PARSE_TOFLOAT(const json &element,json &normal_format,json &false_format)
{
    double max=element["max"],min=element["min"];
    std::default_random_engine e(time(0));
    std::uniform_real_distribution<double> x(min, max);
    normal_format=int(x(e)*100.0)/100.0;
    false_format=int(x(e)*100.0)/100.0+max-min;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_OBJ_TONUMBER(const json &element,json &normal_format,json &false_format)
{
    switch (element["max"].type()) {
        case json::value_t::number_float:return PARSE_TOFLOAT(element,normal_format,false_format);
        case json::value_t::number_unsigned:return PARSE_TOUNSIGNED(element,normal_format,false_format);
        case json::value_t::number_integer:return PARSE_TOINTEGER(element,normal_format, false_format);
        default: return CAPACITY_PARSE_INVALID;
    }
}

int CAPACITY_PARSE_OBJ_TOSTRING(const json &element,json &normal_format,json &false_format)
{
    int max=element["@max"],min=element["@min"];
    int length=min + rand() % (max - min + 1);
    std::string temp;
    temp=MAKE_STRING(length);
    normal_format=temp;
    length+=min;
    temp=MAKE_STRING(length);
    false_format=temp;
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_OBJ(const json &element,json &normal_format,json &false_format)
{
    //å¤„ç†minï¼Œmax
    do{
        int ret=CAPACITY_PARSE_OBJ_JUDGE(element);
        if(CAPACITY_PARSE_MAKE_STRING == ret)return CAPACITY_PARSE_OBJ_TOSTRING(element,normal_format,false_format);
        else if(CAPACITY_PARSE_MAKE_INT == ret )return CAPACITY_PARSE_OBJ_TONUMBER(element,normal_format,false_format);
        else break;
    }while(0);
    //éžminï¼Œmaxå¯¹è±¡
    for(auto it=element.begin();it!=element.end();it++)
    {
        if(CAPACITY_PARSE_SUCCESS != CAPACITY_PARSE_VALUE(it.value(),normal_format[it.key()],false_format[it.key()]))return CAPACITY_PARSE_ERROR;
    }
    return CAPACITY_PARSE_SUCCESS;
}

int CAPACITY_PARSE_VALUE(const json &element,json &normal_format,json &false_format)
{
    switch(element.type())
    {
        case json::value_t::object: return CAPACITY_PARSE_OBJ(element,normal_format,false_format);
        case json::value_t::array:return CAPACITY_PARSE_ARRAY(element,normal_format,false_format);
        case json::value_t::string:return CAPACITY_PARSE_STRING(element,normal_format,false_format);
        case json::value_t::number_float:return CAPACITY_PARSE_FLOAT(element,normal_format,false_format);
        case json::value_t::number_unsigned:return CAPACITY_PARSE_UNSIGNED(element,normal_format,false_format);
        case json::value_t::number_integer:return CAPACITY_PARSE_INTEGER(element,normal_format, false_format);
        case json::value_t::boolean:return CAPACITY_PARSE_BOOLEAN(element,normal_format,false_format);
        default:return CAPACITY_PARSE_INVALID;
    }
}

void CAPACITY_FORMAT_REPLACE(json  &test_format,json &normal_format,json &false_format)
{
    normal_format.swap(false_format);
    test_json.push_back(test_format);
    normal_format.swap(false_format);
}

void CAPACITY_FORMAT_MAKE(json &test_format,json &normal_format,json &false_format)
{
    auto it_normal=normal_format.begin(),it_false=false_format.begin();
    while(it_normal!=normal_format.end()&&it_false!=false_format.end())
    {
        if(it_normal->type() == json::value_t::object && it_false->type() == json::value_t::object)
        {
            CAPACITY_FORMAT_MAKE(test_format,it_normal.value(),it_false.value());
        }
        else
        {
            CAPACITY_FORMAT_REPLACE(test_format,it_normal.value(),it_false.value());
        }
        it_normal++;
        it_false++;
    }
}

void CAPACITY_FORAMT_PARSE(const json &format)
{
    int ret;
    json normal_format,false_format;
    /*if( CAPACITY_FORMAT_GET_ERROR == (ret = CAPACITY_FORMAT_GET(format)) )
    {
        std::cout<<"capacity format file open failed!"<<std::endl;
        return ;
    }*/
    srand((int)time(0));
    if( CAPACITY_PARSE_SUCCESS != (ret = CAPACITY_PARSE_VALUE(format,normal_format,false_format)))
    {
        std::cout<<"make json format!"<<std::endl;
        return ;
    }
    //std::cout<<normal_foramt<<std::endl<<false_format<<std::endl<<std::endl;
    test_json.push_back(normal_format);//normal format
    json &test_format=normal_format;
    CAPACITY_FORMAT_MAKE(test_format,normal_format,false_format);
    //for(int i=0;i<test_json.size();i++)std::cout<<test_json[i]<<std::endl;

}

int CAPACITY_FORAMT_GET(json &format)
{
	int ret;
	std::string temp,url=PREFIX+IP+':'+PORT+CAPACITY_URL;
  std::cout<<url<<std::endl;
	if( 200 != (ret = URL_OPT_Get(url.c_str(),temp,username,password,TIMEOUT)))
	{
    std::cout<<temp<<std::endl;
		std::cout<<"url get failed!"<<std::endl;
		return 0;
	}
  std::cout<<temp<<std::endl;
  std::cout<<"url get successed!"<<std::endl;
	format=json::parse(temp);
  return 1; 
}

void CAPACITY_JSON_SEND()
{
	int ret;
	std::string url=PREFIX+IP+':'+PORT+DEVICE_URL,body;
  
	for(int i=0;i<test_json.size();i++)
	{
    std::string temp,body = test_json[i].dump();
    std::cout<<"example:"<<i<<std::endl<<test_json[i]<<std::endl;
		if(200 != (ret = URL_OPT_Post(url.c_str(),body,temp,username,password,TIMEOUT)))
		{
			//std::cout<<"url post failed!"<<std::endl<<temp<<std::endl;
		}
    //else std::cout<<"url post successed!"<<std::endl<<temp<<std::endl;
    std::cout<<temp<<std::endl;
	}
   
}

int main()
{
	json format;
	if(!CAPACITY_FORAMT_GET(format))return 0;
        CAPACITY_FORAMT_PARSE(format);
	CAPACITY_JSON_SEND();
    return 0;
}

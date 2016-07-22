#ifndef INTERPOLATE_HEADER
#define INTERPOLATE_HEADER

#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <cstring>
#include <set>
#include <sstream>
#include <tuple>

namespace cs540{
	static int count=0;
	  static std::initializer_list<std::string> types = { 
	    typeid(std::ios_base &(std::ios_base &)).name(), 
	    typeid(std::ostream&(std::ostream &)).name(),
	    typeid(const std::result_of<decltype(std::resetiosflags)&(typename std::ios_base::fmtflags)>::type).name(),
	    typeid(const std::result_of<decltype(std::setiosflags)&(typename std::ios_base::fmtflags)>::type).name(),
	    typeid(const std::result_of<decltype(std::setbase)&(int)>::type).name(),
	    typeid(const std::result_of<decltype(std::setfill<char>)&(char)>::type).name(),
	    typeid(const std::result_of<decltype(std::setprecision)&(int)>::type).name(),
	    typeid(const std::result_of<decltype(std::setw)&(int)>::type).name(),
	    typeid(const std::result_of<decltype(std::put_money<long double>)&(long double, bool)>::type).name(),
	    typeid(const std::result_of<decltype(std::put_money<std::basic_string<char>>)&(std::basic_string<char>, bool)>::type).name(),
	    typeid(const std::result_of<decltype(std::put_time<char>)&(const std::tm*, const char*)>::type).name(),
	    typeid(const std::result_of<decltype(std::showbase)&(std::ios_base&)>::type).name()

	  };

	static std::set<std::string> manips = types;
	class WrongNumberOfArgs{};

	bool ismanip(std::string arg){
		return manips.find(arg) != manips.end();
	}

	template <typename... Ts>
	class Helper{
		public:
			Helper() : format_string(NULL){}
			Helper(const char* fs) : format_string(fs){}

			template <typename... Args>
			Helper(const char* fs, Args&&... args) : format_string(fs), args_tuple(args...){}

			template <typename... U>
			friend std::ostream& operator<<(std::ostream&, const Helper<U...>&);

		private:
			std::tuple<Ts...> args_tuple;
			const char* format_string;
	};

	void print_wrap(const char* format_string, std::ostream& os){
		for(; *format_string != '\0'; format_string++){
                        if(*format_string=='%'){
                                if(count > 0 && *(format_string-1) == '\\'){
                                        long pos = os.tellp();
                                        os.seekp(pos-1);
                                        os << *(format_string);
                                        continue;
                                }
                                throw WrongNumberOfArgs{};
                        }
                        else{
                                os << *(format_string);
                                ++count;
                        }
                }		
	}

	template <typename T, typename... Ts>
	void print_wrap(const char* format_string, std::ostream& os, const T& val, const Ts&... rest){
		if(*format_string=='\0' && !ismanip(typeid(val).name())){
                        throw WrongNumberOfArgs{};
                }

                //checking for ostream manipulators
                if(strcmp(typeid(val).name(), typeid(std::ostream &(std::ostream &)).name())==0){
                        os << val;
			print_wrap(format_string, os, rest...);
			return;
                }

		for(; *format_string != '\0'; format_string++){
                        if(*format_string == '%'){
                                if(count > 0 && *(format_string-1) == '\\'){
                                        long pos = os.tellp();
                                        os.seekp(pos - 1);
                                        os << *(format_string);
                                        continue;
                                }
                                os << val;

                                if(ismanip(typeid(val).name())){
					print_wrap(format_string, os,rest...);
				}
				else print_wrap(format_string+1, os,rest...); 
				return;
			}
			else{
				if(*format_string == '\\'){
					os << (*(++format_string));
				}
				else{
					os << *(format_string);
					++count;
				}
			}
		}
	}
	
	template <typename Tp, std::size_t... Is>
	void print(const Tp& t, const char* format_string, std::ostream& os,
			const std::index_sequence<Is...>&){
		print_wrap(format_string, os, std::get<Is>(t)...);
	}

	template <typename... Args>
	std::ostream& operator<<(std::ostream& os, const Helper<Args...>& hobj){
		print(hobj.args_tuple, hobj.format_string, os, 
			std::make_index_sequence<std::tuple_size<decltype(hobj.args_tuple)>::value>());
		return os;
	}

	template <typename... Args>
	Helper<Args...>& Interpolate(const char* format_string, Args... args){
		Helper<Args...> *helper_object = new Helper<Args...>(format_string, args...);
		return *helper_object;
	}
}

#endif

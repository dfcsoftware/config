/*--------------------------------------------------
 * File: config.hpp
 *
 * Usage: Initialize class -
 *    Config config = Config( "config.json", <- config file
 *                            "config",      <- Syslog name
 *                            "config.log"); <- Log file output
 *
 * Example (see config.cpp for more):
 *
 * Output to file:
 *   config.log
 *
 * Dependencies: 
 *  * std:: See #include below
 *  * json11:: Json class library
 *      https://github.com/dropbox/json11
 *  * log::
 *      log.hpp
 *
 * Author: Don Cohoon - November 2020
 *--------------------------------------------------
 */
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include "json/json11.hpp"
#include "log.hpp"
#include "validator.hpp"

//---------------------------------------------------
class Config
{
public:
  Config( std::string configfile = "config.json",
          std::string myname     = "config",
          std::string logfile    = "config.log")
  {
    SetConfigFile( configfile );
    log.SetLogFile( logfile );
    sys.SetLogName( myname );
  }
  ~Config() { }
  //
  class KeyValue
  {
    public:
      std::map<std::string,std::string> _key_value;
  };
  class KeyArray
  {
    public:
      std::vector<std::map<std::string,std::string>> _key_array;
  };
  class JsonMap
  {
    public:
      std::map<std::string, std::vector<std::map<std::string,std::string>>> _json_map;
  };
  //
  template<typename It>
  void PrintInsertResult(It it, bool success);
  template<typename It>
  void PrintKeyValue( It it );
  int  PrintJsonArray( const std::string & s, const json11::Json & a );
  bool InsertKeyValue ( std::string key,
                        std::string value,
                        std::map<std::string,std::string> & in_m );
  bool InsertKeyValue ( std::string jsonstring,
                        std::map<std::string,std::string> & in_m );
  bool InsertJsonMap ( std::string s,
                       std::vector<std::map<std::string,std::string>> & v,
                       std::map<std::string, std::vector<std::map<std::string,std::string>>> & in_m);
  std::string MergeJson ( json11::Json & j1,
                          json11::Json & j2,
                          json11::Json & jout);
  std::string GetConfigFile () { return _configfile; };
  void SetConfigFile (std::string cfile) { _configfile = cfile; };
  void SaveJson ( std::string filename, json11::Json & j);
  void SaveJson ( std::string filename );
  std::string GetJson ( std::string filename, json11::Json & j);
  std::string GetJson ( std::string filename );
  json11::Json GetJson ( ) { return _json; };
  int  GetKeyArray ( std::string key, json11::Json & j, json11::Json & jout);
  std::string GetKey ( std::string key, json11::Json & j);
  std::string GetKey ( std::string key );
  Filer * GetLogFile () { return logPtr; };
  Syslog * GetLogSys () { return sysPtr; };
protected:
private:
  std::string _configfile;
  json11::Json _json;
  // From log.hpp
  Syslog sys = Syslog(LogLevel::TRACE);
  Syslog* sysPtr = &sys; 
  Filer log = Filer (LogLevel::TRACE);
  Filer* logPtr = &log; 
#ifdef _db_log_to_syslog
  Syslog *mylog = GetLogSys();
#else
  Filer *mylog = GetLogFile ();
#endif
  Validator valid;
}; // Config

//=====================================================================
// Print a map insertion result
// Input it as itterator of object
// Print iterator item, and succeeded or failed
template<typename It>
void Config::PrintInsertResult(It it, bool success)
{
  mylog->Writer(LogLevel::TRACE) << "CONFIG:PrintInsertResult: Insertion of "
                                 << it->first
                                 << (success ? " succeeded\n" : " failed\n");
} // PrintInsertResult

//=====================================================================
// Print a Json (key,value) pair
// Input it as itterator of object
// Return number of items in the array
template<typename It>
void Config::PrintKeyValue( It it )
{
  mylog->Writer(LogLevel::TRACE) 
	    << "CONFIG:PrintKeyValue:  Key->"  << std::get<0>(it) // string
            << ", Value->" << std::get<1>(it).dump() // Json
            << std::endl;
} // PrintKeyValue

//=====================================================================
// Print a Json array of Json objects
// Input s as key and a as Json array
// Return number of items in the array
int Config::PrintJsonArray( const std::string & s, const json11::Json & a )
{
  int j=0;
  for (auto &k : a[s].array_items()) {
    for (auto &i : k.object_items()) {
      PrintKeyValue( i );
      j++;
    }
  }
  return j;
} // PrintJsonArray

//=====================================================================
// Insert strings key and value into map in_m
// Input key and value and in_m
// Output in_m
// Return true on success, false on error
bool Config::InsertKeyValue ( std::string key,
                              std::string value,
                              std::map<std::string,std::string> & in_m )
{
  const auto [m_insert, m_success] = in_m.insert({key,value});
  PrintInsertResult(m_insert, m_success);
  return m_success;
} // InsertKeyValue

//=====================================================================
// Insert json string into map in_m
// Input json string and in_m
// Output in_m
// Return true on success, false on error
bool Config::InsertKeyValue ( std::string jsonstring,
                              std::map<std::string,std::string> & in_m )
{
  std::string err;
  std::string key;
  std::string value;
  bool success = true;
  json11::Json json;
  mylog->Writer(LogLevel::TRACE) << "CONFIG:InsertKeyValue: jsonstring - "
                                 << jsonstring << std::endl;
  json = json11::Json::parse(jsonstring, err);
  if (err.empty())
  {
    mylog->Writer(LogLevel::TRACE) << "CONFIG:InsertKeyValue: json is - "
                                   << json.dump() << std::endl;
    for (auto &i : json.object_items()) {
      key = std::get<0>(i);
      value = std::get<1>(i).dump(); 
      mylog->Writer(LogLevel::TRACE) << "CONFIG:InsertKeyValue: key is - " << key
                                     << " value: " << value
                                     << std::endl;
      bool m_success = Config::InsertKeyValue (key, value, in_m);
      if ( ! m_success )
        success = false;
    }
    return success;
  }
  else
  {
    mylog->Writer(LogLevel::WARN) << "CONFIG:InsertKeyValue: Json::parse error - "
                                  << err << std::endl;
    return false;
  }
} // InsertKeyValue

//=====================================================================
// Insert string s and vector v into map in_m
// Input s and v and in_m
// Output in_m
// Return true on success, false on error
bool Config::InsertJsonMap ( std::string s,
                             std::vector<std::map<std::string,std::string>> & v,
                             std::map<std::string, std::vector<std::map<std::string,std::string>>> & in_m)
{
  const auto [m_insert, m_success] = in_m.insert({s, v});
    PrintInsertResult(m_insert, m_success);
  return m_success;
} // InsertJsonMap

//=====================================================================
// Merge two Jsons (j1,j2) into one Json (jout)
// Input two Json objects as j1 and j2
// Output compined Json as jout
// Returns string; empty if no errors, other error message
std::string Config::MergeJson ( json11::Json & j1,
                                json11::Json & j2,
                                json11::Json & jout)
{
  std::string s1 = j1.dump();
  std::string s2 = j2.dump();
  std::string combined;
  std::string err;
  int sz1 = s1.size();
  int sz2 = s2.size();
  // 
  mylog->Writer(LogLevel::TRACE) << "CONFIG:MergeJson:  -> Remove braces in the middle {...}{...}"
                                 << std::endl;
  combined = s1.substr(0,sz1-1) + "," + s2.substr(1,sz2-1);
  // 
  mylog->Writer(LogLevel::TRACE) << "CONFIG:MergeJson: Combine: " 
                                 << combined << std::endl;
  // Combine: {"db": "twotree", "prog1": ["u", "p", "ip", "uuid"],"k1": "v1", "k2": "v2", "k3": "v3"}
  //
  mylog->Writer(LogLevel::TRACE) << "CONFIG:MergeJson:  -> Parse string into json object"
                                 << std::endl;
  jout = json11::Json::parse(combined, err);
  return err;
} // MergeJson

//=====================================================================
// Save Json into a file
// Input file name in filename, Json in j
// Output file on disk
// Probably want to run: "cat <filename> | jq ."
//  to make it pretty
void Config::SaveJson ( std::string filename, json11::Json & j)
{
  std::ofstream ofs(filename, std::ofstream::out);
  ofs << j.dump() << std::endl;
  ofs.close();
} // SaveJson
/*
void SaveJson ( std::string filename )
{
  Config::SaveJson ( filename, _json);
}
*/

//=====================================================================
// Get Json from a file
// Input file name in filename
// Return Json in j
std::string Config::GetJson ( std::string filename, json11::Json & j)
{
  std::streampos size;
  std::string s;
  std::string err;
  char * memblock;

  std::ifstream file (filename, std::ios::in|std::ios::ate);
  if (file.is_open())
  {
    size = file.tellg();
    memblock = new char [size];
    file.seekg (0, std::ios::beg);
    file.read (memblock, size);
    file.close();

    mylog->Writer(LogLevel::TRACE) << "CONFIG:GetJson: The entire " << filename 
            << " content is in memory"
            << std::endl;

    s = memblock;

    delete[] memblock;
  }
  else
    mylog->Writer(LogLevel::WARN) << "CONFIG:GetJson: Unable to open " << filename 
	   << std::endl;

  j = json11::Json::parse(s, err);

  return err;

} // GetJson
std::string Config::GetJson ( std::string filename )
{
  return Config::GetJson ( filename, _json);  
}

//=====================================================================
// Get Key from Json Array
// Input array index as key, and Json as j
// Return array in jout, and size as function return value
int Config::GetKeyArray ( std::string key, json11::Json & j, json11::Json & jout)
{
  int cnt=0;
  std::vector<std::string> v;
  for (auto &k : j[key].array_items())
  {
    for (auto &i : k.object_items())
    {
       v.push_back(std::get<1>(i).dump());	
       cnt++;
    }
  }
  jout = json11::Json(v);
  return cnt;
} // GetKeyArray

//=====================================================================
// Get Key from Json 
// Input key as Json index and j as Json object
// Return key's value in Json
std::string Config::GetKey ( std::string key, json11::Json & j)
{
  std::string s = j[key].dump();
  valid.remove_quotes(s);
  return s;
}
std::string Config::GetKey ( std::string key )
{
  return Config::GetKey ( key, _json);
}


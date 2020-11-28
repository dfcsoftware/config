/*--------------------------------------------------
 * File: config.cpp
 *
 * Usage: Initialize class -
 *    Config config = Config( "config.json", <- config file
 *                            "config",      <- Syslog name
 *                            "config.log"); <- Log file output
 *
 * Example (see main for more):
 *
 * Output to file:
 *   config.log
 *
 * Dependencies: 
 *  * std:: See #include <below>
 *  * json11:: Json class library
 *      https://github.com/dropbox/json11
 *  * log::
 *      log.hpp
 *  * config::
 *      config.hpp
 *
 * Author: Don Cohoon - November 2020
 *--------------------------------------------------
 */
#include <string>
#include "json/json11.hpp"

// Comment out next define to log to file
#define _db_log_to_syslog

#include "config.hpp"
//=====================================================================
int main(int argc, char *argv[])
{
  Config config = Config();
  //Filer *log = config.SetLogFile ("config.log");
  //Filer *log = config.GetLogFile ();
  Syslog *log = config.GetLogSys (); // Syslog
  log->SetLogName("config"); // Syslog Name
  std::string err;

  //--------------------------------------------------------------------
  // Read up existing json configuration file and log it
  json11::Json j9;
  err=config.GetJson("config.json", j9);
  if (!err.empty()) {
    log->Writer(LogLevel::WARN) << "ERROR: File import to Json failed; " 
                                << err << std::endl;
  } else {
    log->Writer(LogLevel::INFO) << "File import json ok" << std::endl;
    log->Writer(LogLevel::INFO) << "File: " << "config.json" << std::endl
                              << j9.dump() << std::endl;
  }

  //--------------------------------------------------------------------
  // Check for a known key value
  log->Writer(LogLevel::INFO) << "db value is: " 
                              << config.GetKey("db", j9)
                              << std::endl;

  //--------------------------------------------------------------------
  // Check for a known key value in array
  json11::Json arg;
  int max=config.GetKeyArray("prog3", j9, arg);
  for (int i=0;i<max;i++)
    log->Writer(LogLevel::INFO) << "prog3[" << i << "]="
	                        << arg[i].string_value()
                                << std::endl;

  //--------------------------------------------------------------------
  // Create Key Value
  Config::KeyValue args = Config::KeyValue();
  log->Writer(LogLevel::INFO) << " => Fill in key value (args) pairs" 
                              << std::endl;
  if ( ! config.InsertKeyValue ( "0", "u", args._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;
  if (  !config.InsertKeyValue ( "1", "p", args._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;
  if (  !config.InsertKeyValue ( "2", "ip", args._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;
  if (  !config.InsertKeyValue ( "3", "uuid", args._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;

  //--------------------------------------------------------------------
  // Create Key Array from Key Value (above)
  Config::KeyArray prog = Config::KeyArray();
  log->Writer(LogLevel::INFO) << " => Add key/values (args) to array (prog)"
                             << std::endl;
  prog._key_array.push_back(args._key_value);
  log->Writer(LogLevel::INFO) << "Json(prog._key_array): " 
                             << json11::Json(prog._key_array).dump() 
                             << std::endl;

  //--------------------------------------------------------------------
  // Create Json Map from Key Array (above)
  Config::JsonMap progs = Config::JsonMap();
  log->Writer(LogLevel::INFO) << " => Create map (progs) of arrays (progs)"
                               << std::endl;
  if ( !config.InsertJsonMap ( "prog2", prog._key_array, progs._json_map) )
    log->Writer(LogLevel::WARN) << "InsertJsonMap ERROR!" << std::endl;
  if ( !config.InsertJsonMap ( "prog3", prog._key_array, progs._json_map) )
    log->Writer(LogLevel::WARN) << "InsertJsonMap ERROR!" << std::endl;
  
  //--------------------------------------------------------------------
  // Create Json from Json Map (above)
  json11::Json cfg = json11::Json(progs._json_map);
    log->Writer(LogLevel::INFO) << " => Create Json (cfg) from map (progs) of arrays (prog)" 
                               << std::endl;
    log->Writer(LogLevel::INFO) << "cfg: " << cfg.dump() << "\n";
  // cfg: {"prog2": ["u", "p", "ip", "uuid"], "prog3": ["u", "p", "ip", "uuid"]}
  
  //--------------------------------------------------------------------
  // Print Json Array from Json (above)
  int j=0;
  log->Writer(LogLevel::INFO) << " -> Dump json array (prog3) to the screen from Json (cfg)" 
                             << std::endl;
  j=config.PrintJsonArray( "prog3", cfg );
  log->Writer(LogLevel::INFO) << "Json array (prog3) parameter count: " << j 
                             << std::endl;

  //--------------------------------------------------------------------
  // Create key/value keys 
  Config::KeyValue keys = Config::KeyValue();
  log->Writer(LogLevel::INFO) << " => Key/Value pairs created and inserted into (keys)" 
                             << std::endl;
  if ( ! config.InsertKeyValue ( "db", "testdb", keys._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;
  if ( ! config.InsertKeyValue ( "user", "twotree", keys._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;
  if ( ! config.InsertKeyValue ( "key", "jheqfwh3erfhSOr4", keys._key_value) )
    log->Writer(LogLevel::WARN) << "InsertKeyValue ERROR!" << std::endl;
  log->Writer(LogLevel::INFO) << "map (keys): " << json11::Json(keys._key_value).dump() 
                             << std::endl;
  // map (keys): {"db": "testdb", "key": "jheqfwh3erfhSOr4", "user": "twotree"}
  
  //--------------------------------------------------------------------
  // Create Json from keys
  json11::Json cfg2 = json11::Json(keys._key_value);
  log->Writer(LogLevel::INFO) << " -> Create Json (cfg2) from Key/Value pair map (keys)"
                             << std::endl;

  //--------------------------------------------------------------------
  // Combine two Jsons into One Json
  json11::Json json2;
  log->Writer(LogLevel::INFO) << " => Combine two json (cfg) and (cfg2)" 
                              << std::endl;
  err = config.MergeJson ( cfg, cfg2, json2);
  if (!err.empty()) {
    log->Writer(LogLevel::WARN) << "ERROR: Combine Json failed; " << err << std::endl;
  } else {
    log->Writer(LogLevel::INFO) << "Combine json Result: " << json2.dump() << std::endl;
    // Combine json Result: {"db": "twotree", "k1": "v1", "k2": "v2", "k3": "v3", "prog1": ["u", "p", "ip", "uuid"]}
  }

  //--------------------------------------------------------------------
  // Save Json to file
  int rslt;
  std::string cf = config.GetConfigFile();
  char  pretty[80];
  strcpy(pretty,"cat /tmp/config.tmp|jq . > ");
  strncat(pretty, cf.c_str(), 50);
  config.SaveJson ( "/tmp/config.tmp", json2);
  rslt = std::system(pretty);
  if ( rslt != 0 )
     log->Writer(LogLevel::WARN) << "error saving /tmp/config.tmp, exit=" << rslt << std::endl;
  else
     log->Writer(LogLevel::INFO) << "File saved as config.json" << std::endl;
  rslt = std::system("rm /tmp/config.tmp");

  //--------------------------------------------------------------------
  return 0;
}

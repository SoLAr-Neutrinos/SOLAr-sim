/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRootUtilities.cc
 * @created     : Monday Mar 30, 2026
 */

#include "SLArRootUtilities.hh"

template TH1*    get_from_rootfile<TH1 >  (const rapidjson::Value& file_and_key); 
template TH1D*   get_from_rootfile<TH1D>  (const rapidjson::Value& file_and_key); 
template TH1F*   get_from_rootfile<TH1F>  (const rapidjson::Value& file_and_key); 
template TH2D*   get_from_rootfile<TH2D>  (const rapidjson::Value& file_and_key); 
template TH2F*   get_from_rootfile<TH2F>  (const rapidjson::Value& file_and_key); 


#include "mjolnir/statistics.h"
#include "mjolnir/graphvalidator.h"
#include "mjolnir/graphtilebuilder.h"

#include <ostream>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sqlite3.h>
#include <spatialite.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <valhalla/midgard/aabb2.h>
#include <valhalla/midgard/logging.h>
#include <valhalla/baldr/tilehierarchy.h>
#include <valhalla/baldr/graphid.h>
#include <valhalla/baldr/graphconstants.h>

using namespace valhalla::midgard;
using namespace valhalla::baldr;
using namespace valhalla::mjolnir;

namespace {
  template <class T> T merge(T a, T b) {
    T tmp(a);
    tmp.insert(b.begin(), b.end());
    return tmp;
  }
}

namespace valhalla {
namespace mjolnir {

validator_stats::validator_stats()
  : tile_lengths(), country_lengths(), tile_int_edges(),country_int_edges(),
    tile_one_way(), country_one_way(), tile_speed_info(), country_speed_info(),
    tile_named(), country_named(), tile_truck_route(), country_truck_route(),
    tile_hazmat(), country_hazmat(), tile_height(), country_height(),
    tile_width(), country_width(), tile_length(), country_length(),
    tile_weight(), country_weight(), tile_axle_load(), country_axle_load(),
    fork_signs(), exit_signs(),
    tile_areas(), tile_geometries(),iso_codes(), tile_ids() { }

void validator_stats::add_tile_road(const uint32_t& tile_id, const RoadClass& rclass, float length) {
  tile_ids.insert(tile_id);
  tile_lengths[tile_id][rclass] += length;
}

void validator_stats::add_country_road(const std::string& ctry_code, const RoadClass& rclass, float length) {
  iso_codes.insert(ctry_code);
  country_lengths[ctry_code][rclass] += length;
}

void validator_stats::add_tile_int_edge(const uint32_t& tile_id, const RoadClass& rclass, const uint32_t& count) {
  tile_int_edges[tile_id][rclass] += count;
}

void validator_stats::add_country_int_edge(const std::string& ctry_code, const RoadClass& rclass, const uint32_t& count) {
  country_int_edges[ctry_code][rclass] += count;
}

void validator_stats::add_tile_one_way(const uint32_t& tile_id, const RoadClass& rclass, float length) {
  tile_one_way[tile_id][rclass] += length;
}

void validator_stats::add_country_one_way(const std::string& ctry_code, const RoadClass& rclass, float length) {
  country_one_way[ctry_code][rclass] += length;
}

void validator_stats::add_tile_speed_info(const uint32_t& tile_id, const RoadClass& rclass, float length) {
  tile_speed_info[tile_id][rclass] += length;
}

void validator_stats::add_country_speed_info(const std::string& ctry_code, const RoadClass& rclass, float length) {
  country_speed_info[ctry_code][rclass] += length;
}

void validator_stats::add_tile_named(const uint32_t& tile_id, const RoadClass& rclass, float length) {
  tile_named[tile_id][rclass] += length;
}

void validator_stats::add_country_named(const std::string& ctry_code, const RoadClass& rclass, float length) {
  country_named[ctry_code][rclass] += length;
}

void validator_stats::add_tile_hazmat(const uint32_t& tile_id, const RoadClass& rclass, float length) {
  tile_hazmat[tile_id][rclass] += length;
}

void validator_stats::add_country_hazmat(const std::string& ctry_code, const RoadClass& rclass, float length) {
  country_hazmat[ctry_code][rclass] += length;
}

void validator_stats::add_tile_truck_route(const uint32_t& tile_id, const RoadClass& rclass, float length) {
  tile_truck_route[tile_id][rclass] += length;
}

void validator_stats::add_country_truck_route(const std::string& ctry_code, const RoadClass& rclass, float length) {
  country_truck_route[ctry_code][rclass] += length;
}

void validator_stats::add_tile_height(const uint32_t& tile_id, const RoadClass& rclass, const uint32_t& count){
  tile_height[tile_id][rclass] += count;
}

void validator_stats::add_country_height(const std::string& ctry_code, const RoadClass& rclass, const uint32_t& count){
  country_height[ctry_code][rclass] += count;
}

void validator_stats::add_tile_width(const uint32_t& tile_id, const RoadClass& rclass, const uint32_t& count){
  tile_width[tile_id][rclass] += count;

}
void validator_stats::add_country_width(const std::string& ctry_code, const RoadClass& rclass, const uint32_t& count){
  country_width[ctry_code][rclass] += count;
}

void validator_stats::add_tile_length(const uint32_t& tile_id, const RoadClass& rclass, const uint32_t& count){
  tile_length[tile_id][rclass] += count;
}

void validator_stats::add_country_length(const std::string& ctry_code, const RoadClass& rclass, const uint32_t& count){
  country_length[ctry_code][rclass] += count;
}

void validator_stats::add_tile_weight(const uint32_t& tile_id, const RoadClass& rclass, const uint32_t& count){
  tile_weight[tile_id][rclass] += count;
}

void validator_stats::add_country_weight(const std::string& ctry_code, const RoadClass& rclass, const uint32_t& count){
  country_weight[ctry_code][rclass] += count;
}

void validator_stats::add_tile_axle_load(const uint32_t& tile_id, const RoadClass& rclass, const uint32_t& count){
  tile_axle_load[tile_id][rclass] += count;
}

void validator_stats::add_country_axle_load(const std::string& ctry_code, const RoadClass& rclass, const uint32_t& count){
  country_axle_load[ctry_code][rclass] += count;
}

void validator_stats::add_fork_exitinfo(const std::pair<uint64_t, bool>& forkinfo) {
  fork_signs.insert(forkinfo);
}
void validator_stats::add_exitinfo(const std::pair<uint64_t, bool>& exitinfo) {
  exit_signs.insert(exitinfo);
}

void validator_stats::add_tile_area(const uint32_t& tile_id, const float area) {
  tile_areas[tile_id] = area;
}

void validator_stats::add_tile_geom(const uint32_t& tile_id, const AABB2<PointLL> geom) {
  tile_geometries[tile_id] = geom;
}

const std::unordered_set<uint32_t>& validator_stats::get_ids() const { return tile_ids; }

const std::unordered_set<std::string>& validator_stats::get_isos() const { return iso_codes; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_tile_lengths() const { return tile_lengths; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_country_lengths() const { return country_lengths; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher> >& validator_stats::get_tile_int_edges() const { return tile_int_edges; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher> >& validator_stats::get_country_int_edges() const { return country_int_edges; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_tile_one_way() const { return tile_one_way; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_country_one_way() const { return country_one_way; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_tile_speed_info () const { return tile_speed_info; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_country_speed_info() const { return country_speed_info; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_tile_named() const { return tile_named; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, float, validator_stats::rclassHasher> >& validator_stats::get_country_named() const { return country_named; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, float, validator_stats::rclassHasher>>& validator_stats::get_tile_hazmat() const { return tile_hazmat; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, float, validator_stats::rclassHasher>>& validator_stats::get_country_hazmat() const { return country_hazmat; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, float, validator_stats::rclassHasher>>& validator_stats::get_tile_truck_route() const { return tile_truck_route; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, float, validator_stats::rclassHasher>>& validator_stats::get_country_truck_route() const { return country_truck_route; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_tile_height() const { return tile_height; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_country_height() const { return country_height; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_tile_width() const { return tile_width; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_country_width() const { return country_width; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_tile_length() const { return tile_length; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_country_length() const { return country_length; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_tile_weight() const { return tile_weight; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_country_weight() const { return country_weight; }

const std::unordered_map<uint32_t, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_tile_axle_load() const { return tile_axle_load; }
const std::unordered_map<std::string, std::unordered_map<RoadClass, uint32_t, validator_stats::rclassHasher>>& validator_stats::get_country_axle_load() const { return country_axle_load; }

const std::unordered_map<uint32_t, float>& validator_stats::get_tile_areas() const { return tile_areas; }

const std::unordered_map<uint32_t, AABB2<PointLL>>& validator_stats::get_tile_geometries() const { return tile_geometries; }

const std::unordered_map<uint64_t, bool>& validator_stats::get_fork_info() const { return fork_signs; }

const std::unordered_map<uint64_t, bool>& validator_stats::get_exit_info() const { return exit_signs; }

void validator_stats::add (const validator_stats& stats) {
  // Combine ids and isos
  tile_ids = merge(tile_ids, stats.get_ids());
  iso_codes = merge(iso_codes, stats.get_isos());

  // Combine tile statistics
  tile_areas = merge(tile_areas, stats.get_tile_areas());
  tile_geometries = merge(tile_geometries, stats.get_tile_geometries());
  tile_lengths = merge(tile_lengths, stats.get_tile_lengths());
  tile_one_way = merge(tile_one_way, stats.get_tile_one_way());
  tile_speed_info = merge(tile_speed_info, stats.get_tile_speed_info());
  tile_int_edges = merge(tile_int_edges, stats.get_tile_int_edges());
  tile_named = merge(tile_named, stats.get_tile_named());
  tile_hazmat = merge(tile_hazmat, stats.get_tile_hazmat());
  tile_truck_route = merge(tile_truck_route, stats.get_tile_truck_route());
  tile_height = merge(tile_height, stats.get_tile_height());
  tile_width = merge(tile_width, stats.get_tile_width());
  tile_length = merge(tile_length, stats.get_tile_length());
  tile_weight = merge(tile_weight, stats.get_tile_weight());
  tile_axle_load = merge(tile_axle_load, stats.get_tile_axle_load());

  // Combine country statistics
  country_lengths = merge(country_lengths, stats.get_country_lengths());
  country_one_way = merge(country_one_way, stats.get_country_one_way());
  country_speed_info = merge(country_speed_info, stats.get_country_speed_info());
  country_int_edges = merge(country_int_edges, stats.get_country_int_edges());
  country_named = merge(country_named, stats.get_country_named());
  country_hazmat = merge(country_hazmat, stats.get_country_hazmat());
  country_truck_route = merge(country_truck_route, stats.get_country_truck_route());
  country_height = merge(country_height, stats.get_country_height());
  country_width = merge(country_width, stats.get_country_width());
  country_length = merge(country_length, stats.get_country_length());
  country_weight = merge(country_weight, stats.get_country_weight());
  country_axle_load = merge(country_axle_load, stats.get_country_axle_load());

  // Combine exit statistics
  fork_signs = merge(fork_signs, stats.get_exit_info());
  exit_signs = merge(exit_signs, stats.get_fork_info());

  // Combine roulette data
  roulette_data.Add(stats.roulette_data);
}


void validator_stats::build_db(const boost::property_tree::ptree& pt) {
  // Get the location of the db file to write
  auto database = pt.get_optional<std::string>("mjolnir.statistics");
  if(!database) {
    return;
  }
  else if(boost::filesystem::exists(*database)) {
    boost::filesystem::remove(*database);
  }

  spatialite_init(0);

  sqlite3 *db_handle;
  sqlite3_stmt *stmt;
  uint32_t ret;
  char *err_msg = NULL;
  std::string sql;

  ret = sqlite3_open_v2(database->c_str(), &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("cannot open " + *database);
    sqlite3_close(db_handle);
    db_handle = NULL;
    return;
  }

  // loading SpatiaLite as an extension
  sqlite3_enable_load_extension(db_handle, 1);
#if SQLITE_VERSION_NUMBER > 3008002
    sql = "SELECT load_extension('mod_spatialite')";
#else
    sql = "SELECT load_extension('libspatialite')";
#endif
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("load_extension() error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  LOG_INFO("Writing statistics database");

  // Turn on foreign keys
  sql = "PRAGMA foreign_keys = ON";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }

  create_tile_tables(db_handle, stmt);
  create_country_tables(db_handle, stmt);
  insert_tile_data(db_handle, stmt);
  insert_country_data(db_handle, stmt);

  // Create Index on geometry column
  sql = "SELECT CreateSpatialIndex('tiledata', 'geom')";
  ret = sqlite3_exec (db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }

  sql = "VACUUM ANALYZE";
  ret = sqlite3_exec (db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }
  sqlite3_close(db_handle);
  LOG_INFO("Finished");
}

void validator_stats::create_tile_tables(sqlite3 *db_handle, sqlite3_stmt *stmt) {
  uint32_t ret;
  char *err_msg = NULL;
  std::string sql;

  // Create table for tiles
  sql = "SELECT InitSpatialMetaData(); CREATE TABLE tiledata (";
  sql += "tileid INTEGER PRIMARY KEY,";
  sql += "tilearea REAL,";
  sql += "totalroadlen REAL,";
  sql += "motorway REAL,";
  sql += "pmary REAL,";
  sql += "secondary REAL,";
  sql += "tertiary REAL,";
  sql += "trunk REAL,";
  sql += "residential REAL,";
  sql += "unclassified REAL,";
  sql += "serviceother REAL";
  sql += ")";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  // Add tile geometry column
  sql = "SELECT AddGeometryColumn('tiledata', ";
  sql += "'geom', 4326, 'POLYGON', 2)";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  // Create table for tile data of road classes
  sql = "CREATE TABLE rclasstiledata (";
  sql += "tileid INTEGER,";
  sql += "type TEXT NOT NULL,";
  sql += "oneway REAL,";
  sql += "maxspeed REAL,";
  sql += "internaledges INTEGER,";
  sql += "named REAL,";
  sql += "FOREIGN KEY (tileid) REFERENCES tiledata(tileid)";
  sql += ")";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  // Create table for truck tile data of road classes
  sql = "CREATE TABLE truckrclasstiledata (";
  sql += "tileid INTEGER,";
  sql += "type TEXT NOT NULL,";
  sql += "hazmat REAL,";
  sql += "truck_route REAL,";
  sql += "height INTEGER,";
  sql += "width INTEGER,";
  sql += "length INTEGER,";
  sql += "weight INTEGER,";
  sql += "axle_load INTEGER,";
  sql += "FOREIGN KEY (tileid) REFERENCES tiledata(tileid)";
  sql += ")";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
}

void validator_stats::create_country_tables(sqlite3 *db_handle, sqlite3_stmt *stmt) {
  uint32_t ret;
  char *err_msg = NULL;
  std::string sql;

  // Create tables for country data
  sql = "CREATE TABLE countrydata (";
  sql += "isocode TEXT PRIMARY KEY,";
  sql += "motorway REAL,";
  sql += "pmary REAL,";
  sql += "secondary REAL,";
  sql += "tertiary REAL,";
  sql += "trunk REAL,";
  sql += "residential REAL,";
  sql += "unclassified REAL,";
  sql += "serviceother REAL";
  sql += ")";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  // Create table for country data of road classes
  sql = "CREATE TABLE rclassctrydata (";
  sql += "isocode TEXT NOT NULL,";
  sql += "type TEXT NOT NULL,";
  sql += "oneway REAL,";
  sql += "maxspeed REAL,";
  sql += "internaledges INTEGER,";
  sql += "named REAL,";
  sql += "FOREIGN KEY (isocode) REFERENCES countrydata(isocode)";
  sql += ")";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  // Create table for truck country data of road classes
  sql = "CREATE TABLE truckrclassctrydata (";
  sql += "isocode TEXT NOT NULL,";
  sql += "type TEXT NOT NULL,";
  sql += "hazmat REAL,";
  sql += "truck_route REAL,";
  sql += "height INTEGER,";
  sql += "width INTEGER,";
  sql += "length INTEGER,";
  sql += "weight INTEGER,";
  sql += "axle_load INTEGER,";
  sql += "FOREIGN KEY (isocode) REFERENCES countrydata(isocode)";
  sql += ")";
  ret = sqlite3_exec(db_handle, sql.c_str(), NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
}

void validator_stats::insert_tile_data(sqlite3* db_handle, sqlite3_stmt* stmt) {

  uint32_t ret;
  char *err_msg = NULL;
  std::string sql;

  // Begin the prepared statements for tiledata
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  sql = "INSERT INTO tiledata (tileid, tilearea, totalroadlen, motorway, pmary, secondary, tertiary, trunk, residential, unclassified, serviceother, geom) ";
  sql += "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, GeomFromText(?, 4326))";
  ret = sqlite3_prepare_v2(db_handle, sql.c_str(), strlen (sql.c_str()), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("SQL error: " + sql);
    LOG_ERROR(std::string(sqlite3_errmsg(db_handle)));
    sqlite3_close (db_handle);
    return;
  }

  // Fill DB with the tile statistics
  for (auto tileid : tile_ids) {
    uint8_t index = 1;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    // Tile ID
    sqlite3_bind_int(stmt, index, tileid);
    ++index;
    // Tile Area
    sqlite3_bind_double(stmt, index, tile_areas[tileid]);
    ++index;
    // Total Road Length
    float totalLen = 0;
    for (auto rclass : rclasses) {
      totalLen += tile_lengths[tileid][rclass];
    }
    sqlite3_bind_double(stmt, index, totalLen);
    ++index;
    // Individual Road Class Lengths
    for (auto rclass : rclasses) {
      std::string roadStr = roadClassToString.at(rclass);
      sqlite3_bind_double(stmt, index, tile_lengths[tileid][rclass]);
      ++index;
    }
    // Use tile bounding box corners to make a polygon
    if (tile_geometries.find(tileid) != tile_geometries.end()) {
      auto maxx = std::to_string(tile_geometries.at(tileid).maxx());
      auto minx = std::to_string(tile_geometries.at(tileid).minx());
      auto maxy = std::to_string(tile_geometries.at(tileid).maxy());
      auto miny = std::to_string(tile_geometries.at(tileid).miny());
      std::string polyWKT = "POLYGON (("
          + minx + " " + miny + ", "
          + minx + " " + maxy + ", "
          + maxx + " " + maxy + ", "
          + maxx + " " + miny + ", "
          + minx + " " + miny + "))";
      sqlite3_bind_text (stmt, index, polyWKT.c_str(), polyWKT.length(), SQLITE_STATIC);
    } else {
      LOG_ERROR("Geometry for tile " + std::to_string(tileid) + " not found.");
    }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
      continue;
    }
    LOG_ERROR("sqlite3_step() error: " + std::string(sqlite3_errmsg(db_handle)));
  }
  sqlite3_finalize (stmt);
  ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }

  // Begin adding the statistics for each road type of tile data
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }

  sql = "INSERT INTO rclasstiledata (tileid, type, oneway, maxspeed, internaledges, named) ";
  sql += "VALUES (?, ?, ?, ?, ?, ?)";
  ret = sqlite3_prepare_v2(db_handle, sql.c_str(), sql.length(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("SQL error: " + sql);
    LOG_ERROR(std::string(sqlite3_errmsg(db_handle)));
    sqlite3_close (db_handle);
    return;
  }

  // Fill the roadclass stats for tiles
  for (auto tileid : tile_ids) {
    for (auto rclass : rclasses) {
      uint8_t index = 1;
      sqlite3_reset(stmt);
      sqlite3_clear_bindings(stmt);
      // Tile ID (parent tile)
      sqlite3_bind_int(stmt, index, tileid);
      ++index;
      // Roadway type
      auto type = roadClassToString.at(rclass);
      sqlite3_bind_text(stmt, index, type.c_str(), type.length(), SQLITE_STATIC);
      ++index;
      // One Way data
      sqlite3_bind_double(stmt, index, tile_one_way[tileid][rclass]);
      ++index;
      // Max speed info
      sqlite3_bind_double(stmt, index, tile_speed_info[tileid][rclass]);
      ++index;
      // Internal edges count
      sqlite3_bind_int(stmt, index, tile_int_edges[tileid][rclass]);
      ++index;
      // Named roads
      sqlite3_bind_double(stmt, index, tile_named[tileid][rclass]);
      ret = sqlite3_step (stmt);
      if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
        continue;
      }
      LOG_ERROR("sqlite3_step() error: " + std::string(sqlite3_errmsg(db_handle)));
    }
  }
  sqlite3_finalize (stmt);
  ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }

  // Begin adding the truck statistics for each road type of tile data
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }

  sql = "INSERT INTO truckrclasstiledata (tileid, type, hazmat, truck_route, height, width, length, weight, axle_load) ";
  sql += "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
  ret = sqlite3_prepare_v2(db_handle, sql.c_str(), sql.length(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("SQL error: " + sql);
    LOG_ERROR(std::string(sqlite3_errmsg(db_handle)));
    sqlite3_close (db_handle);
    return;
  }

  // Fill the truck roadclass stats for tiles
  for (auto tileid : tile_ids) {
    for (auto rclass : rclasses) {
      uint8_t index = 1;
      sqlite3_reset(stmt);
      sqlite3_clear_bindings(stmt);
      // Tile ID (parent tile)
      sqlite3_bind_int(stmt, index, tileid);
      ++index;
      // Roadway type
      auto type = roadClassToString.at(rclass);
      sqlite3_bind_text(stmt, index, type.c_str(), type.length(), SQLITE_STATIC);
      ++index;
      // Hazmat
      sqlite3_bind_double(stmt, index, tile_hazmat[tileid][rclass]);
      ++index;
      // Truck Route
      sqlite3_bind_double(stmt, index, tile_truck_route[tileid][rclass]);
      ++index;
      // Height
      sqlite3_bind_int(stmt, index, tile_height[tileid][rclass]);
      ++index;
      // Width
      sqlite3_bind_int(stmt, index, tile_width[tileid][rclass]);
      ++index;
      // Length
      sqlite3_bind_int(stmt, index, tile_length[tileid][rclass]);
      ++index;
      // Weight
      sqlite3_bind_int(stmt, index, tile_weight[tileid][rclass]);
      ++index;
      // Axle Load
      sqlite3_bind_int(stmt, index, tile_axle_load[tileid][rclass]);
      ret = sqlite3_step (stmt);
      if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
        continue;
      }
      LOG_ERROR("sqlite3_step() error: " + std::string(sqlite3_errmsg(db_handle)));
    }
  }
  sqlite3_finalize (stmt);
  ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }

}

void validator_stats::insert_country_data(sqlite3* db_handle, sqlite3_stmt* stmt) {
  uint32_t ret;
  char *err_msg = NULL;
  std::string sql;

  // Begin the prepared statements for country data
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }
  sql = "INSERT INTO countrydata (isocode, motorway, pmary, secondary, tertiary, trunk, residential, unclassified, serviceother)";
  sql += "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
  ret = sqlite3_prepare_v2(db_handle, sql.c_str(), sql.length(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("SQL error: " + sql);
    LOG_ERROR(std::string(sqlite3_errmsg(db_handle)));
    sqlite3_close (db_handle);
    return;
  }

  // Fill DB with the country statistics
  for (auto country : iso_codes) {
    uint8_t index = 1;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    // Country ISO
    sqlite3_bind_text(stmt, index, country.c_str(), country.length(), SQLITE_STATIC);
    ++index;
    // Individual Road Class Lengths
    for (auto rclass : rclasses) {
      std::string roadStr = roadClassToString.at(rclass);
      sqlite3_bind_double(stmt, index, country_lengths[country][rclass]);
      ++index;
    }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
      continue;
    }
    LOG_ERROR("sqlite3_step() error: " + std::string(sqlite3_errmsg(db_handle)));
  }
  sqlite3_finalize (stmt);
  ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }

  // Begin adding the statistics for each road type of country data
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }

  sql = "INSERT INTO rclassctrydata (isocode, type, oneway, maxspeed, internaledges, named) ";
  sql += "VALUES (?, ?, ?, ?, ?, ?)";
  ret = sqlite3_prepare_v2(db_handle, sql.c_str(), sql.length(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("SQL error: " + sql);
    LOG_ERROR(std::string(sqlite3_errmsg(db_handle)));
    sqlite3_close (db_handle);
    return;
  }

  // Fill the roadclass stats for countries
  for (auto country : iso_codes) {
    for (auto rclass : rclasses) {
      uint8_t index = 1;
      sqlite3_reset(stmt);
      sqlite3_clear_bindings(stmt);
      // ISO (parent ID)
      sqlite3_bind_text(stmt, index, country.c_str(), country.length(), SQLITE_STATIC);
      ++index;
      // Roadway type
      auto type = roadClassToString.at(rclass);
      sqlite3_bind_text(stmt, index, type.c_str(), type.length(), SQLITE_STATIC);
      ++index;
      // One Way data
      sqlite3_bind_double(stmt, index, country_one_way[country][rclass]);
      ++index;
      // Max speed info
      sqlite3_bind_double(stmt, index, country_speed_info[country][rclass]);
      ++index;
      // Internal edges count
      sqlite3_bind_int(stmt, index, country_int_edges[country][rclass]);
      ++index;
      // Named Roads
      sqlite3_bind_double(stmt, index, country_named[country][rclass]);
      ret = sqlite3_step (stmt);
      if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
        continue;
      }
      LOG_ERROR("sqlite3_step() error: " + std::string(sqlite3_errmsg(db_handle)));
    }
  }
  sqlite3_finalize (stmt);
  ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }

  // Begin adding the truck statistics for each road type of country data
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free(err_msg);
    sqlite3_close(db_handle);
    return;
  }

  sql = "INSERT INTO truckrclassctrydata (isocode, type, hazmat, truck_route, height, width, length, weight, axle_load) ";
  sql += "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
  ret = sqlite3_prepare_v2(db_handle, sql.c_str(), sql.length(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOG_ERROR("SQL error: " + sql);
    LOG_ERROR(std::string(sqlite3_errmsg(db_handle)));
    sqlite3_close (db_handle);
    return;
  }

  // Fill the truck roadclass stats for countries
  for (auto country : iso_codes) {
    for (auto rclass : rclasses) {
      uint8_t index = 1;
      sqlite3_reset(stmt);
      sqlite3_clear_bindings(stmt);
      // ISO (parent ID)
      sqlite3_bind_text(stmt, index, country.c_str(), country.length(), SQLITE_STATIC);
      ++index;
      // Roadway type
      auto type = roadClassToString.at(rclass);
      sqlite3_bind_text(stmt, index, type.c_str(), type.length(), SQLITE_STATIC);
      ++index;
      // Hazmat
      sqlite3_bind_double(stmt, index, country_hazmat[country][rclass]);
      ++index;
      // Truck Route
      sqlite3_bind_double(stmt, index, country_truck_route[country][rclass]);
      ++index;
      // Height
      sqlite3_bind_int(stmt, index, country_height[country][rclass]);
      ++index;
      // Width
      sqlite3_bind_int(stmt, index, country_width[country][rclass]);
      ++index;
      // Length
      sqlite3_bind_int(stmt, index, country_length[country][rclass]);
      ++index;
      // Weight
      sqlite3_bind_int(stmt, index, country_weight[country][rclass]);
      ++index;
      // Axle Load
      sqlite3_bind_int(stmt, index, country_axle_load[country][rclass]);
      ret = sqlite3_step (stmt);
      if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
        continue;
      }
      LOG_ERROR("sqlite3_step() error: " + std::string(sqlite3_errmsg(db_handle)));
    }
  }
  sqlite3_finalize (stmt);
  ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    LOG_ERROR("Error: " + std::string(err_msg));
    sqlite3_free (err_msg);
    sqlite3_close (db_handle);
    return;
  }
}

validator_stats::RouletteData::RouletteData ()
  : node_locs(), way_IDs(), way_shapes() { }

void validator_stats::RouletteData::AddTask (const PointLL& p, const uint64_t id, const std::vector<PointLL>& shape) {
  auto result = way_IDs.insert(id);
  if (result.second)
    node_locs.insert({id, p});
    way_shapes.insert({id, shape});
}

void validator_stats::RouletteData::Add (const RouletteData& rd) {
  const auto new_ids = rd.way_IDs;
  const auto new_shapes = rd.way_shapes;
  const auto new_nodes = rd.node_locs;
  for (auto& id : new_ids) {
    AddTask(new_nodes.at(id), id, new_shapes.at(id));
  }
}

void validator_stats::RouletteData::GenerateTasks () {
  bool write_comma = false;
  std::string json_str = "";
  json_str += "[";
  for (auto& id : way_IDs) {
    if (write_comma)
      json_str += ",";
    json_str += "{\"geometries\": {\"features\": [";
    json_str += "{\"geometry\": {\"coordinates\": ["
              + std::to_string(node_locs.at(id).lng()) + "," + std::to_string(node_locs.at(id).lat())
              + "],\"type\": \"Point\"},\"id\": null,\"properties\": {},\"type\": \"Feature\"},";
    json_str += "{\"geometry\": {\"coordinates\": [ ";
    bool coord_comma = false;
    for (auto& p : way_shapes.at(id)){
      if (coord_comma)
        json_str += ",";
      json_str += "[" + std::to_string(p.lng()) + "," + std::to_string(p.lat()) + "]";
      coord_comma = true;
    }
    json_str += " ],\"type\": \"Linestring\" },\"id\": null,\"properties\": {\"osmid\": " + std::to_string(id) + "},\"type\": \"Feature\"}";
    json_str += "],\"type\": \"FeatureCollection\"},";
    json_str += "\"identifier\": \"" + std::to_string(id) + "\",";
    json_str += "\"instruction\": \"Check to see if the one way road is logical\"}";
    write_comma = true;
  }
  json_str += "]";
  std::string file_str = "/data/valhalla/tasks.json";
  if (boost::filesystem::exists(file_str))
      boost::filesystem::remove(file_str);
  std::ofstream file;
  file.open(file_str);
  file << json_str << std::endl;
  file.close();
  LOG_INFO("MapRoulette tasks saved to /data/valhalla/tasks.json");
}
}
}

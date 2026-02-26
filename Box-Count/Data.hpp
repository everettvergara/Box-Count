#pragma once

#include "Box.hpp"
#include "MotionDetection.h"

#include <nlohmann/json.hpp>
#include <string>
#include <ctime>
#include <vector>
#include <filesystem>
#include <fstream>

namespace eg::bc
{
	const std::filesystem::path k_trans_config_filename = "config/trans.json";
	const std::filesystem::path k_cams_config_filename = "config/cams.json";

	const std::filesystem::path k_box_count_trans_folder = "out/trans";
	const std::filesystem::path k_box_count_trans_image_folder = "out/trans/images";

	struct Cam
	{
		std::string type;
		std::string name;

		cv::Rect bottom;
		cv::Rect middle;
		cv::Rect top;
		cv::Rect left;
		cv::Rect right;

		ComvisConfig config;

		static std::unordered_map<std::string, Cam> load_cams()
		{
			if (not std::filesystem::exists(k_cams_config_filename))
			{
				// TODO: Create a default;
				// TODO: Log ERROR
				return {};
			}
			std::ifstream config_file(k_cams_config_filename);
			nlohmann::json j = nlohmann::json::parse(config_file, nullptr, false);
			if (j.is_discarded() or not j.is_object())
			{
				// TODO: Log ERROR
				return {};
			}

			std::unordered_map<std::string, Cam> loaded_cams;
			loaded_cams.reserve(j.size());
			for (const auto& [d, o] : j.items())
			{
				if (not o.is_object())
				{
					continue;
				}

				loaded_cams.try_emplace(d,
					o.value("type", std::string()),
					o.value("name", std::string()),

					cv::Rect
					(
						o["area"]["bottom"]["x"],
						o["area"]["bottom"]["y"],
						o["area"]["bottom"]["w"],
						o["area"]["bottom"]["h"]
					),

					cv::Rect
					(
						o["area"]["middle"]["x"],
						o["area"]["middle"]["y"],
						o["area"]["middle"]["w"],
						o["area"]["middle"]["h"]
					),

					cv::Rect
					(
						o["area"]["top"]["x"],
						o["area"]["top"]["y"],
						o["area"]["top"]["w"],
						o["area"]["top"]["h"]
					),

					cv::Rect
					(
						o["area"]["left"]["x"],
						o["area"]["left"]["y"],
						o["area"]["left"]["w"],
						o["area"]["left"]["h"]
					),

					cv::Rect
					(
						o["area"]["right"]["x"],
						o["area"]["right"]["y"],
						o["area"]["right"]["w"],
						o["area"]["right"]["h"]
					),

					(
						not o.contains("comvis") ?
						ComvisConfig() :
						ComvisConfig(
							o["comvis"].value("blur_size", k_blur_size),
							o["comvis"].value("binarize_threshold", k_binarize_threshold),
							o["comvis"].value("shadow_delta", k_shadow_delta),
							o["comvis"].value("min_contour_area", k_min_contour_area),
							o["comvis"].value("bg_alpha", k_bg_alpha),
							o["comvis"].value("merge_distance", k_merge_distance),
							o["comvis"].value("dilate_iter", k_dilate_iter),
							o["comvis"].value("erode_iter", k_erode_iter),
							o["comvis"].value("debug_contour", false)
						)
						)
				);
			}

			return loaded_cams;
		}
	};

	// TODO: Make it a class and provide getters/setters
	struct BoxCountTrans
	{
		size_t id{ 0 };
		std::string doc_type;
		std::string doc_no;
		time_t doc_date;
		time_t start_time;
		time_t end_time;

		std::string cam_data;

		BoxPolicy box_policy;
		std::vector<Box> box_count;
		std::vector<Box> reject_count;
		std::vector<Box> return_count;

		bool show_debug_windows{ false };
		bool show_track_history{ false };
		bool show_roi{ false };

		static size_t load_gid()
		{
			if (not std::filesystem::exists(k_trans_config_filename))
			{
				if (k_trans_config_filename.has_parent_path())
				{
					if (const auto parent_path = k_trans_config_filename.parent_path(); not std::filesystem::exists(parent_path))
					{
						if (not std::filesystem::create_directories(parent_path))
						{
							// TODO: Log ERROR
							return 0;
						}
					}

					nlohmann::json j;
					j["gid"] = 1ull;

					std::ofstream config(k_trans_config_filename);
					config << j.dump(4);
				}
			}

			std::ifstream config_file(k_trans_config_filename);
			nlohmann::json j = nlohmann::json::parse(config_file, nullptr, false);
			if (j.is_discarded() or not j.contains("gid") or not j["gid"].is_number_unsigned())
			{
				// TODO: Log ERROR
				return 0;
			}

			return j["gid"].get<size_t>();
		}

		static bool save_gid(size_t gid)
		{
			if (k_trans_config_filename.has_parent_path())
			{
				if (const auto parent_path = k_trans_config_filename.parent_path(); not std::filesystem::exists(parent_path))
				{
					if (not std::filesystem::create_directories(parent_path))
					{
						// TODO: Log ERROR
						return false;
					}
				}
			}

			nlohmann::json j;
			j["gid"] = gid;

			std::ofstream config_file(k_trans_config_filename);
			config_file << j.dump(4);

			return true;
		}

		void clear(size_t gid)
		{
			id = gid;
			doc_type.clear();
			doc_no.clear();
			doc_date = std::time(nullptr);
			start_time = 0;
			end_time = 0;
			box_count.clear();
			reject_count.clear();
			return_count.clear();
		}

		void save()
		{
		}

		bool load(size_t id)
		{
		}
	};
}
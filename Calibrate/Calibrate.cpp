#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#if defined(_WIN32)
#define strcasecmp _strcmpi
#else
#define strcasecmp strcmpi
#endif

#include <stdexcept>
#include <atomic>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <fstream>

constexpr int k_frame_width = 640;
constexpr int k_frame_height = 400;
constexpr int k_target_width = 320;
constexpr int k_target_height = 200;
constexpr int k_frame_center_x = k_frame_width / 2;
const auto k_center_color = cv::Scalar(0, 0, 0);

struct global_vars
{
	int flip{ 0 };						// 0 = false; 1 = true
	int conveyor_center_x{ k_frame_center_x };
	int conveyor_width{ 30 };
	int conveyor_y1{ k_frame_height / 2 - 50 };
	int conveyor_y2{ k_frame_height / 2 + 50 };

	int conveyor_x1() const
	{
		return conveyor_center_x - (conveyor_width / 2);
	}

	int conveyor_x2() const
	{
		return conveyor_center_x + (conveyor_width / 2);
	}
} global;

static void on_flip(int pos, void*);
static void on_conveyor_center(int x, void*);
static void on_conveyor_width(int w, void*);
static void on_conveyor_y1(int y, void*);
static void on_conveyor_y2(int y, void*);
static const char* validate_args(int argc, char* args[]);

int main(int argc, char* args[])
{
	try
	{
		if (const auto err = validate_args(argc, args); err)
		{
			std::cerr << err << '\n';
			return EXIT_FAILURE;
		}

		auto cap = [&]
			{
				if (strcasecmp(args[4], "usb") == 0)
				{
					int ix = std::stoi(args[2]);
					return cv::VideoCapture(ix);
				}

				return cv::VideoCapture(args[2]);
			}();

		if (not cap.isOpened())
		{
			std::cerr << "Cannot open the video stream specified.\n";
			return EXIT_FAILURE;
		}

		const cv::String win_name = "Calibrate";
		cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);
		cv::resizeWindow(win_name, k_frame_width, k_frame_height);

		cv::createTrackbar("Flip", win_name, &global.flip, 1, on_flip);
		cv::createTrackbar("Center", win_name, &global.conveyor_center_x, k_frame_width - 1, on_conveyor_center);
		cv::createTrackbar("Width", win_name, &global.conveyor_width, k_frame_width - 1, on_conveyor_width);
		cv::createTrackbar("Y1", win_name, &global.conveyor_y1, k_frame_height - 1, on_conveyor_y1);
		cv::createTrackbar("Y2", win_name, &global.conveyor_y2, k_frame_height - 1, on_conveyor_y2);

		bool is_exit = false;
		do
		{
			cv::Mat frame;
			cap >> frame;

			if (frame.empty())
			{
				break;
			}

			// Resize to 800 x whatever
			const int new_height = static_cast<int>(frame.rows * (static_cast<double>(k_frame_width) / static_cast<double>(frame.cols)));
			cv::resize(frame, frame, cv::Size(k_frame_width, new_height));

			if (global.flip)
			{
				cv::flip(frame, frame, 1);
			}

			// Draw center line
			const auto x1 = global.conveyor_x1();
			const auto x2 = global.conveyor_x2();
			cv::line(frame, cv::Point(global.conveyor_center_x, 0), cv::Point(global.conveyor_center_x, frame.rows), k_center_color, 2);
			cv::line(frame, cv::Point(x1, 0), cv::Point(x1, frame.rows), k_center_color, 2);
			cv::line(frame, cv::Point(x2, 0), cv::Point(x2, frame.rows), k_center_color, 2);
			cv::line(frame, cv::Point(x1, global.conveyor_y1), cv::Point(x2, global.conveyor_y1), k_center_color, 2);
			cv::line(frame, cv::Point(x1, global.conveyor_y2), cv::Point(x2, global.conveyor_y2), k_center_color, 2);

			// Put text hello on upper left;
			cv::putText(frame, "Press 's' to save; esc to exit.", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, k_center_color, 2);

			cv::imshow(win_name, frame);

			const auto key = cv::waitKey(16);
			switch (key)
			{
			case 27: // ESC
				is_exit = true;

				break;
			case 's': [[fallthrough]];
			case 'S':

				// Save to json;
				auto jdata = [&] { std::ifstream file(args[1]); return nlohmann::json::parse(file); }();

				// Check if ix / file exists
				const auto key = args[2];
				if (jdata.contains(key))
				{
					jdata.erase(key);
				}

				jdata[key] = nlohmann::json::object();
				jdata[key]["name"] = args[3];
				jdata[key]["flip"] = global.flip;
				jdata[key]["type"] = args[4];

				auto area = nlohmann::json::object();

				// convert frame to 320x200

				// get xframe factor

				auto xframe_factor = static_cast<double>(k_target_width) / static_cast<double>(k_frame_width);
				auto yframe_factor = static_cast<double>(k_target_height) / static_cast<double>(k_frame_height);

				area["left"]["x"] = 0;
				area["left"]["y"] = 0;
				area["left"]["width"] = static_cast<int>(global.conveyor_x1() * xframe_factor);
				area["left"]["height"] = k_target_height;
				area["right"]["x"] = static_cast<int>(global.conveyor_x2() * xframe_factor);
				area["right"]["y"] = 0;
				area["right"]["width"] = k_target_width - static_cast<int>(global.conveyor_x2() * xframe_factor);
				area["right"]["height"] = k_target_height;
				area["top"]["x"] = static_cast<int>(global.conveyor_x1() * xframe_factor);
				area["top"]["y"] = 0;
				area["top"]["width"] = static_cast<int>(global.conveyor_width * xframe_factor * 2);
				area["top"]["height"] = static_cast<int>(global.conveyor_y1 * yframe_factor);
				area["bottom"]["x"] = static_cast<int>(global.conveyor_x1() * xframe_factor);
				area["bottom"]["y"] = static_cast<int>(global.conveyor_y2 * yframe_factor);
				area["bottom"]["width"] = static_cast<int>(global.conveyor_width * xframe_factor * 2);
				area["bottom"]["height"] = k_target_height - static_cast<int>(global.conveyor_y2 * yframe_factor);
				area["middle"]["x"] = static_cast<int>(global.conveyor_x1() * xframe_factor);
				area["middle"]["y"] = static_cast<int>(global.conveyor_y1 * yframe_factor);
				area["middle"]["width"] = static_cast<int>(global.conveyor_width * xframe_factor * 2);
				area["middle"]["height"] = static_cast<int>((global.conveyor_y2 - global.conveyor_y1) * yframe_factor);

				jdata[key]["area"] = area;
				std::ofstream file_out(args[1]);
				file_out << jdata.dump(4);
				is_exit = true;

				break;
			}
		} while (not is_exit);

		cv::destroyAllWindows();
		cap.release();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
}

void on_flip(int pos, void*)
{
	global.flip = pos;
}

void on_conveyor_center(int x, void*)
{
	global.conveyor_center_x = x;
}

void on_conveyor_width(int w, void*)
{
	global.conveyor_width = w;
}

void on_conveyor_y1(int y, void*)
{
	global.conveyor_y1 = y;
}

void on_conveyor_y2(int y, void*)
{
	global.conveyor_y2 = y;
}

const char* validate_args(int argc, char* args[])
{
	if (argc not_eq 5)
	{
		return "Invalid calibrate parameters. Calibrate <config_filename> <ix/file> <name> <type>;";
	}

	if (args[1] == "")
	{
		return "<config_filename> must not be blank.";
	}

	bool is_usb = strcasecmp(args[4], "usb") == 0;
	bool is_file = strcasecmp(args[4], "file") == 0;

	if (not is_usb and not is_file)
	{
		return R"(<type> must either be "usb" or "file")";
	}

	if (is_usb)
	{
		const auto begin = args[2];
		const auto end = begin + strlen(args[2]);

		if (std::find_if(begin, end, [](char ch)
			{
				return not std::isdigit(ch);
			}) not_eq end)
		{
			return "Invalid <ix> for USB type. Must be a valid integer.";
		}
	}

	if (is_file)
	{
		std::filesystem::path filename;
		if (not std::filesystem::exists(filename))
		{
			return "The <file> specified does not exist. Must be a valid .mp4 file for debug.";
		}
	}

	std::filesystem::path config = args[1];
	if (not std::filesystem::exists(config))
	{
		// Must contain valid json
		auto data = nlohmann::json::object();
		std::ofstream file(config);

		file << data.dump(4);
	}
	else
	{
		std::ifstream file(config);

		auto data = nlohmann::json::parse(file, nullptr, false);

		if (data.is_discarded())
		{
			return "The <config_filename> specified is not a valid json file.";
		}

		if (not data.is_object())
		{
			return "The <config_filename> specified must contain a json object at the root.";
		}
	}
	return nullptr;
}